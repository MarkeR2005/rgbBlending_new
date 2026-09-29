//---------------------------------------------------------------------------

#pragma hdrstop

#include "DataCalculator.h"
#include <string>
#include <memory>
#include <vector>
#include <cmath>
#include <queue>
#include <numeric>
#include "UnitLoading.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// Вычисление доминирующей частоты с параболической интерполяцией
static float computeDominantFreq(const std::vector<float>& spectrum, const std::vector<float>& freqs) {
    int maxIdx = 0;
    float maxVal = spectrum[0];
    for (size_t i = 1; i < spectrum.size(); ++i) {
        if (spectrum[i] > maxVal) {
            maxVal = spectrum[i];
            maxIdx = (int)i;
        }
    }
    // Интерполяция, если возможно
    if (maxIdx > 0 && maxIdx < (int)spectrum.size()-1) {
        float a = spectrum[maxIdx-1];
        float b = spectrum[maxIdx];
        float c = spectrum[maxIdx+1];
        float offset = 0.5f * (a - c) / (a - 2.0f*b + c);
        if (std::isfinite(offset)) {
            float idx = maxIdx + offset;
            // Линейная интерполяция по частотам
            float f0 = freqs[maxIdx];
            float f1 = freqs[maxIdx+1];
            return f0 + (f1 - f0) * offset;
        }
    }
    return freqs[maxIdx];
}

// Поле доминирующей частоты (traces x samples)
static std::vector<std::vector<float>> computeDominantFreqField(const std::shared_ptr<RgbData>& input) {
    const auto& rawData = input->getRawData(); // [freq][trace][sample]
    size_t nFreq = rawData.size();
    size_t nTraces = rawData[0].size();
    size_t nSamples = rawData[0][0].size();
    std::vector<float> freqs = input->getFreqs();
    if (freqs.size() != nFreq) {
        // fallback
        freqs.resize(nFreq);
        for (size_t i = 0; i < nFreq; ++i) freqs[i] = (float)i / (nFreq-1);
    }

    std::vector<std::vector<float>> field(nTraces, std::vector<float>(nSamples, 0.0f));
    for (size_t tr = 0; tr < nTraces; ++tr) {
        for (size_t sm = 0; sm < nSamples; ++sm) {
            std::vector<float> spectrum(nFreq);
            for (size_t f = 0; f < nFreq; ++f)
                spectrum[f] = rawData[f][tr][sm];
            field[tr][sm] = computeDominantFreq(spectrum, freqs);
        }
    }
    return field;
}

// Градиент по времени (вдоль samples)
static std::vector<std::vector<float>> gradientT(const std::vector<std::vector<float>>& field) {
    size_t nTraces = field.size();
    if (nTraces == 0) return {};
    size_t nSamples = field[0].size();
    std::vector<std::vector<float>> grad(nTraces, std::vector<float>(nSamples, 0.0f));
    for (size_t tr = 0; tr < nTraces; ++tr) {
        for (size_t sm = 1; sm < nSamples-1; ++sm)
            grad[tr][sm] = (field[tr][sm+1] - field[tr][sm-1]) * 0.5f;
        if (nSamples > 1) {
            grad[tr][0] = field[tr][1] - field[tr][0];
            grad[tr][nSamples-1] = field[tr][nSamples-1] - field[tr][nSamples-2];
        }
    }
    return grad;
}

// Градиент по трассам (вдоль x)
static std::vector<std::vector<float>> gradientX(const std::vector<std::vector<float>>& field) {
    size_t nTraces = field.size();
    if (nTraces == 0) return {};
    size_t nSamples = field[0].size();
    std::vector<std::vector<float>> grad(nTraces, std::vector<float>(nSamples, 0.0f));
    for (size_t tr = 1; tr < nTraces-1; ++tr)
        for (size_t sm = 0; sm < nSamples; ++sm)
            grad[tr][sm] = (field[tr+1][sm] - field[tr-1][sm]) * 0.5f;
    if (nTraces > 1) {
        for (size_t sm = 0; sm < nSamples; ++sm) {
            grad[0][sm] = field[1][sm] - field[0][sm];
            grad[nTraces-1][sm] = field[nTraces-1][sm] - field[nTraces-2][sm];
        }
    }
    return grad;
}
// Точка с вектором признаков и исходным индексом
struct PointV {
    std::vector<float> coords;
    int idx;
};

// Узел KD-дерева
struct KDNode {
    PointV point;
    KDNode* left = nullptr;
    KDNode* right = nullptr;
    int axis = 0;

    ~KDNode() { delete left; delete right; }
};

// Функция сравнения для сортировки по выбранной оси
static bool compareByAxis(const PointV& a, const PointV& b, int axis) {
    return a.coords[axis] < b.coords[axis];
}

// Построение KD-дерева из списка точек (рекурсивно)
static KDNode* buildKDTree(std::vector<PointV>& points, int depth = 0) {
    if (points.empty()) return nullptr;

    int k = points[0].coords.size(); // размерность
    int axis = depth % k;

    // Находим медиану
    size_t mid = points.size() / 2;
    std::nth_element(points.begin(), points.begin() + mid, points.end(),
        [axis](const PointV& a, const PointV& b) {
            return a.coords[axis] < b.coords[axis];
        });

    KDNode* node = new KDNode();
    node->point = points[mid];
    node->axis = axis;

    std::vector<PointV> leftPoints(points.begin(), points.begin() + mid);
    std::vector<PointV> rightPoints(points.begin() + mid + 1, points.end());

    node->left = buildKDTree(leftPoints, depth + 1);
    node->right = buildKDTree(rightPoints, depth + 1);

    return node;
}
static void preprocessForPearson(std::vector<std::vector<float>>& points) {
    for (auto& p : points) {
        // 1. Вычисляем среднее значение вектора
        float mean = 0.0f;
        for (float v : p) mean += v;
        mean /= p.size();

        // 2. Вычитаем среднее (центрирование)
        for (float& v : p) v -= mean;

        // 3. Вычисляем L2-норму
        float norm = 0.0f;
        for (float v : p) norm += v * v;
        norm = std::sqrt(norm);

        // 4. Нормализуем (если норма не нулевая)
        if (norm > 1e-9f) {
            for (float& v : p) v /= norm;
        }
        // Если норма = 0, то оставляем нулевой вектор (он будет давать корреляцию неопределённой)
    }
}
// Поиск всех соседей в радиусе eps с использованием KD-дерева
static void radiusSearch(KDNode* node, const PointV& target, float eps,
                         std::vector<int>& neighbors, float epsSq) {
    if (!node) return;

    // Вычисляем квадрат расстояния до текущей точки
    float distSq = 0.0f;
    for (size_t i = 0; i < target.coords.size(); ++i) {
        float diff = node->point.coords[i] - target.coords[i];
        distSq += diff * diff;
    }
    if (distSq <= epsSq) {
        neighbors.push_back(node->point.idx);
    }

    int axis = node->axis;
    float diff = target.coords[axis] - node->point.coords[axis];

    // Сначала исследуем сторону, которая содержит целевую точку
    KDNode* first = (diff <= 0) ? node->left : node->right;
    KDNode* second = (diff <= 0) ? node->right : node->left;

    radiusSearch(first, target, eps, neighbors, epsSq);

    // Если сфера пересекает разделяющую плоскость, исследуем вторую сторону
    if (diff * diff <= epsSq) {
        radiusSearch(second, target, eps, neighbors, epsSq);
    }
}

// Обёртка для удобства
static std::vector<int> findNeighbors(KDNode* root, const std::vector<float>& targetCoords,
                                      int targetIdx, float eps) {
    PointV target{targetCoords, targetIdx};
    std::vector<int> neighbors;
    float epsSq = eps * eps;
    radiusSearch(root, target, eps, neighbors, epsSq);
    return neighbors;
}

static float euclideanDistance(const std::vector<float>& a, const std::vector<float>& b) {
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}
static float cosineDistance(const std::vector<float>& a, const std::vector<float>& b) {
    float dot = 0.0f, normA = 0.0f, normB = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    if (normA < 1e-9f || normB < 1e-9f) return 1.0f; // нулевой вектор – максимальное расстояние
    float cosTheta = dot / (std::sqrt(normA) * std::sqrt(normB));
    // Ограничиваем из-за погрешностей
    if (cosTheta > 1.0f) cosTheta = 1.0f;
    if (cosTheta < -1.0f) cosTheta = -1.0f;
    return 1.0f - cosTheta; // расстояние в [0,2]
}

static float estimateEps(const std::vector<std::vector<float>>& points, int k = 5, double percentile = 0.9) {
    const int n = points.size();
    const int sampleSize = std::min(5000, n); // не более 5000 точек для скорости
    std::vector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::random_shuffle(indices.begin(), indices.end());

    std::vector<float> kDistances;
    kDistances.reserve(sampleSize);

    for (int i = 0; i < sampleSize; ++i) {
        int idx = indices[i];
        // Собрать расстояния до всех других точек
        std::vector<float> dists;
        dists.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (j == idx) continue;
            dists.push_back(euclideanDistance(points[idx], points[j]));
        }
        // Найти k-е наименьшее расстояние (k-й сосед)
        std::nth_element(dists.begin(), dists.begin() + k - 1, dists.end());
        kDistances.push_back(dists[k - 1]);
    }

    std::sort(kDistances.begin(), kDistances.end());
    size_t pos = static_cast<size_t>(percentile * kDistances.size());
    if (pos >= kDistances.size()) pos = kDistances.size() - 1;
    return kDistances[pos];
}

static void normalizeFeatures(std::vector<std::vector<float>>& points) {
    if (points.empty() || points[0].empty()) return;
    size_t nFeatures = points[0].size();
    size_t nPoints = points.size();
    TLoading* load = new TLoading(Application->MainForm);
	load->setDuration(nFeatures);
	load->Show();
    for (size_t f = 0; f < nFeatures; ++f) {
        double mean = 0.0;
        for (size_t i = 0; i < nPoints; ++i) mean += points[i][f];
        mean /= nPoints;

        double variance = 0.0;
        for (size_t i = 0; i < nPoints; ++i) {
            double diff = points[i][f] - mean;
            variance += diff * diff;
        }
        double stddev = std::sqrt(variance / nPoints);
        if (stddev < 1e-9) stddev = 1.0; // избегаем деления на ноль

        for (size_t i = 0; i < nPoints; ++i) {
            points[i][f] = (points[i][f] - mean) / stddev;
        }
        load->update(1);
    }
}


static void hsvToRgb(float h, float s, float v, float& r, float& g, float& b) {
    h = std::fmod(h, 360.0f);
    if (h < 0) h += 360.0f;
    int hi = static_cast<int>(h / 60.0f) % 6;
    float f = h / 60.0f - hi;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);
    switch (hi) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
}

static std::vector<int> dbscan(const std::vector<std::vector<float>>& points,
                               float eps, int minPts) {
    const int n = points.size();
    if (n == 0) return {};

    // Построение KD-дерева
    std::vector<PointV> pointList(n);
    for (int i = 0; i < n; ++i) {
        pointList[i] = {points[i], i};
    }
    KDNode* root = buildKDTree(pointList);

    std::vector<int> labels(n, -2);
    int clusterId = 0;

    for (int i = 0; i < n; ++i) {
        if (labels[i] != -2) continue;

        // Поиск соседей через KD-дерево
        std::vector<int> neighbors = findNeighbors(root, points[i], i, eps);

        if (neighbors.size() < static_cast<size_t>(minPts)) {
            labels[i] = -1;
            continue;
        }

        labels[i] = clusterId;
        std::queue<int> queue;
        for (int nb : neighbors) queue.push(nb);

        while (!queue.empty()) {
            int curr = queue.front(); queue.pop();
            if (labels[curr] == -1) labels[curr] = clusterId;
            if (labels[curr] != -2) continue;
            labels[curr] = clusterId;

            std::vector<int> currNeighbors = findNeighbors(root, points[curr], curr, eps);
            if (currNeighbors.size() >= static_cast<size_t>(minPts)) {
                for (int nb : currNeighbors) {
                    if (labels[nb] == -2 || labels[nb] == -1)
                        queue.push(nb);
                }
            }
        }
        ++clusterId;
    }

    delete root;
    return labels;
}

std::shared_ptr<IBaseData> DataCalculator::smoothT(const std::shared_ptr<IBaseData>& input, int window){
	if (window <= 1) {
		return input;
	}
	const int half_window = window / 2;
	std::shared_ptr<SeismicData> sData = std::dynamic_pointer_cast<SeismicData>(input);
	if (sData)
	{
		int samples = sData->getSize().t;
		std::vector<std::vector<float>> data = sData->getRawData();
		double *pref = new double[samples + 1];
		for (std::vector<float>& trace : data) {
			pref[0] = 0.0;
			for (int i = 0; i < samples; ++i)
			{
				pref[i + 1] = pref[i] + trace[i];
			}

			#pragma omp parallel for
			for (int i = 0; i < samples; ++i)
			{
				int left = i < half_window ? 0 : i-half_window;
				int right = i + half_window > samples-1 ? samples-1: i+half_window;
				int count = right - left + 1;
				double val = (pref[right + 1] - pref[left])/count;
				trace[i] = static_cast<float>(val);
			}
		}
		delete[] pref;
		std::string procedures = sData ->getProcedures() + "_sm(t;w=" + std::to_string(window) + ")";
		auto ret = std::make_shared<SeismicData>(data, sData->getDT(), sData->getType(), sData->getName(), procedures);
		ret ->setFreq(sData->getFreq());
		return std::move(ret);
	}
	std::shared_ptr<RgbData> rData = std::dynamic_pointer_cast<RgbData>(input);
	if (rData) {

		size3 sz = rData->getSize();
		int traces = sz.x;
		int samples = sz.t;
		int filters = sz.f;
		double *pref = new double[samples + 1];
		TLoading* load = new TLoading(Application->MainForm);
		load->setDuration(traces*filters);
		load->Show();
		std::vector<std::vector<std::vector<float>>> data = rData->getRawData();
		for (std::vector<std::vector<float>>& dat : data)
		{
			for (std::vector<float>& trace : dat) {
			pref[0] = 0.0;
				for (int i = 0; i < samples; ++i)
				{
					pref[i + 1] = pref[i] + trace[i];
				}
				#pragma omp parallel for
				for (int i = 0; i < samples; ++i)
				{
					int left = i < half_window ? 0 : i-half_window;
					int right = i + half_window > samples-1 ? samples-1: i+half_window;
					int count = right - left + 1;
					double val = (pref[right + 1] - pref[left])/count;
					trace[i] = static_cast<float>(val);
				}
				load->update(1);
			}
		}
		delete[] pref;
		std::string procedures = rData->getProcedures() + "_sm(t;w=" + std::to_string(window) + ")";
		return std::move(std::make_shared<RgbData>(data, rData->getDT(), rData->getFreqs().front(), rData->getFreqs().back(), rData->getName(), procedures));
	}
	throw;
}


std::shared_ptr<IBaseData> DataCalculator::smoothX(const std::shared_ptr<IBaseData>& input, int window){
	if (window <= 1) {
		return input;
	}
	const int half_window = window / 2;
	std::shared_ptr<SeismicData> sData = std::dynamic_pointer_cast<SeismicData>(input);
	if (sData)
	{
		int samples = sData->getSize().t;
		int traces = sData->getSize().x;
		std::vector<std::vector<float>> data = sData->getRawData();
		#pragma omp parallel for
		for (int i = 0; i < samples; ++i) {
			double *pref = new double[traces + 1];
			pref[0] = 0.0;
			int j = 0;
			for (std::vector<float>& trace : data)
			{
				pref[j + 1] = pref[j] + trace[i];
				++j;
			}
			j = 0;
			for (std::vector<float>& trace : data)
			{
				int left = j < half_window ? 0 : j-half_window;
				int right = j + half_window > traces-1 ? traces-1: j+half_window;
				int count = right - left + 1;
				double val = (pref[right + 1] - pref[left])/count;
				trace[i] = static_cast<float>(val);
				++j;
			}
			delete[] pref;
		}
		std::string procedures = sData ->getProcedures() + "_sm(x;w=" + std::to_string(window) + ")";
		auto ret = std::make_shared<SeismicData>(data, sData->getDT(), sData->getType(), sData->getName(), procedures);
		ret ->setFreq(sData->getFreq());
		return std::move(ret);
	}
	std::shared_ptr<RgbData> rData = std::dynamic_pointer_cast<RgbData>(input);
	if (rData) {

		size3 sz = rData->getSize();
		int traces = sz.x;
		int samples = sz.t;
		int filters = sz.f;
		TLoading* load = new TLoading(Application->MainForm);
		load->setDuration(filters);
		load->Show();
		std::vector<std::vector<std::vector<float>>> data = rData->getRawData();
		for (std::vector<std::vector<float>>& dat : data) {
			#pragma omp parallel for
			for (int i = 0; i < samples; ++i) {
				double *pref = new double[traces + 1];
				pref[0] = 0.0;
				int j = 0;
				for (std::vector<float>& trace : dat)
				{
					pref[j + 1] = pref[j] + trace[i];
					++j;
				}
				j = 0;
				for (std::vector<float>& trace : dat)
				{
					int left = j < half_window ? 0 : j-half_window;
					int right = j + half_window > traces-1 ? traces-1: j+half_window;
					int count = right - left + 1;
					double val = (pref[right + 1] - pref[left])/count;
					trace[i] = static_cast<float>(val);
					++j;
				}
				delete[] pref;
			}
			load->update(1);
		}
		std::string procedures = rData->getProcedures() + "_sm(x;w=" + std::to_string(window) + ")";
		return std::move(std::make_shared<RgbData>(data, rData->getDT(), rData->getFreqs().front(), rData->getFreqs().back(), rData->getName(), procedures));
	}
	throw;
}


std::shared_ptr<RgbData> DataCalculator::smoothF(const std::shared_ptr<RgbData>& input, int window, int freqNo1, int freqNo2)
{
	if (window <= 1) {
		return input;
	}
	size3 sz = input->getSize();
	int traces = sz.x;
	int samples = sz.t;
	int filters = sz.f;
	const int half_window = window / 2;
	if (freqNo1 < 0) {
		freqNo1 = 0;
	}
	if (freqNo2 < 0) {
		freqNo2 = filters;
	}

	TLoading* load = new TLoading(Application->MainForm);
	load->setDuration(traces);
	load->Show();

	std::vector<std::vector<std::vector<float>>> data = input->getRawData();
	double *pref = new double[freqNo2-freqNo1+1];
	for (int tr = 0; tr < traces; ++tr) {
		for (int sp = 0; sp < samples; ++sp) {
			pref[0] = 0.0;
			for (int i = freqNo1; i < freqNo2; ++i) {
				pref[i + 1] = pref[i] + data[i][tr][sp];
			}
			#pragma omp parallel for
			for (int i = freqNo1; i < freqNo2; ++i)
			{
				int left = i < half_window + freqNo1 ? freqNo1 : i-half_window;
				int right = i + half_window > freqNo2-1 ? freqNo2-1: i+half_window;
				int count = right - left + 1;
				double val = (pref[right + 1] - pref[left])/count;
				data[i][tr][sp] = static_cast<float>(val);
			}
		}
		load->update(1);
	}
	delete[] pref;
	std::string procedures = input->getProcedures() + "_sm(f;w=" + std::to_string(window) + ")";
	return std::move(std::make_shared<RgbData>(data, input->getDT(), input->getFreqs().front(), input->getFreqs().back(), input->getName(), procedures));
}


std::shared_ptr<IBaseData> DataCalculator::getTrace(const std::shared_ptr<IBaseData>& input, int pos){
	std::shared_ptr<SeismicData> sData = std::dynamic_pointer_cast<SeismicData>(input);
	if (sData){
		DataType typeIn = sData->getType();
		DataType typeO = DataType::TRACE;
		switch (typeIn) {
		case DataType::TRACE:
			return input;
		case DataType::BASE:
			break;
		case DataType::SWAN:
			break;
		default:
			typeO = DataType::EN_TRACE;
			break;
		}

		auto ret = std::make_shared<SeismicData>(sData->getRawDataRef()[pos].data(), sData->getDT(), sData->getSize().t, 1, typeO, sData->getName(), sData->getProcedures() + "_tr" + std::to_string(pos));
		ret -> setCM(input->getCM());
		ret ->setFreq(sData->getFreq());
		return ret;
	}
	std::shared_ptr<RgbData> rData = std::dynamic_pointer_cast<RgbData>(input);
	if (rData) {
		int traces = rData->getSize().x;
		if (traces == 1) {
            return input;
		}
		int filters = rData->getSize().f;
		std::vector<std::vector<std::vector<float>>> data = rData->getRawData();
		std::vector<std::vector<std::vector<float>>> temp;
		temp.reserve(filters);
		for (auto filter : data) {
			temp.push_back({filter[pos]});
		}
		return std::move(std::make_shared<RgbData>(temp, rData->getDT(), rData->getFreqs().front(), rData->getFreqs().back(), rData->getName(), rData->getProcedures() + "_tr" + std::to_string(pos)));
	}
	throw;
}


std::shared_ptr<SeismicData> DataCalculator::toEnergy(const std::shared_ptr<SeismicData>& input){
	DataType typeIn = input->getType();
	DataType typeO;
	switch (typeIn) {
	case DataType::TRACE:
		typeO = DataType::EN_TRACE;
		break;
	case DataType::BASE:
		typeO = DataType::EN_BASE;
		break;
	case DataType::SWAN:
		typeO = DataType::EN_SWAN;
		break;
	default:
		return input;
	}
	std::vector<std::vector<float>> data = input->getRawDataRef();
	for (std::vector<float>& trace : data) {
			for (float& sample : trace) {
				sample = ((double)sample * (double)sample / 1000000.0);
			}
	}
	std::string procedures = input->getProcedures() + "_en";
	auto ret = std::make_shared<SeismicData>(data, input->getDT(), typeO, input->getName(), procedures);
	ret -> setFreq(input->getFreq());
	return std::move(ret);
}

std::shared_ptr<SeismicData> DataCalculator::getFilter(const std::shared_ptr<RgbData>& input, int freqNo)
{
	return std::make_shared<SeismicData>(input->getRawData()[freqNo], input->getDT(), DataType::EN_BASE, input->getName() + std::to_string(input->getFreqs()[freqNo]), input ->getProcedures());
}

std::shared_ptr<SeismicData> DataCalculator::getSwan(const std::shared_ptr<SeismicData>& input, int pos, const float freqStart, const float freqFinish, const int filtersNumber, const int filterWidth){
	std::vector<float> traceData = input->getRawData()[pos];
	int samples = input->getSize().t;
	std::string curProc = input->getProcedures() + "_tr" + std::to_string(pos) + "_swan(fs=" + std::to_string(freqStart) +
	";ff=" + std::to_string(freqFinish) + ";nf=" + std::to_string(filtersNumber) + ";fw=" + std::to_string(filterWidth) + ")";

	for (int i = 1; i < 5; ++i)
	{
		traceData[i] *= i/5.0;
		traceData[samples - i - 1] *= i/5.0;
	}

	double coeff = std::exp(1.0/(filtersNumber-1)*std::log(freqFinish*1.0/freqStart));
	double dt = input->getDT()/1000000.0;
	std::vector<std::vector<float>> outData(filtersNumber, std::vector<float>(samples));
	std::vector<float> temp(samples, 0.0f);

	std::vector<float> frequens;
	frequens.reserve(filtersNumber);
	float freq = freqStart;
	for (auto& filter : outData) {
		double at = std::exp(-2 * freq * filterWidth/100.0 * dt);
		double b = 2.0 * at * std::cos(2.0 * M_PI * dt * freq);
		double ab = std::pow(at, 2.0);
		double ab1 = ab / 2.0;

		for (int i = 2; i < samples; ++i)
		{
			double s = -traceData[i] / 2.0 + ab1 * traceData[i - 2];
			double s1 = s + b * temp[i - 1] - ab * temp[i - 2];
			temp[i] = s1;
		}

		for (int i = samples - 3; i > -1; --i)
		{
			double s = -temp[i] / 2.0 + ab1 * temp[i + 2];
			double s1 = s + b * filter[i + 1] - ab * filter[i+2];
			filter[i] = s1;
		}

		for (int i = 0; i < samples; ++i) filter[i] *= std::pow(freq,1.1);
		frequens.push_back(freq);
		freq *= coeff;
		memset(temp.data(), 0.0f, samples * sizeof(float));
	}
	std::shared_ptr<SeismicData> ret = std::make_shared<SeismicData>(outData, input->getDT(), DataType::SWAN, input->getName(), curProc);
	ret->setFreq(frequens);
	return std::move(ret);
}


std::shared_ptr<RgbData> DataCalculator::getSwanRGB(const std::shared_ptr<SeismicData>& input, int pos, const float freqStart, const float freqFinish, const int filtersNumber, const int filterWidth){
	std::shared_ptr<SeismicData> swan = getSwan(input, pos,freqStart,freqFinish,filtersNumber,filterWidth);
	std::shared_ptr<SeismicData> enSwan = toEnergy(swan);
	std::vector<std::vector<float>> swanData = enSwan->getRawData();
	std::vector<std::vector<std::vector<float>>> retData(filtersNumber, std::vector<std::vector<float>>(1, std::vector<float>(swan->getSize().t,0)));
	for (int i = 0; i < filtersNumber; i++) {
		retData[i] = {swanData[i]};
	}
	return std::move(std::make_shared<RgbData>(retData, input->getDT(), freqStart, freqFinish, enSwan->getName()+"_rgb", enSwan->getProcedures()));
}


std::shared_ptr<SeismicData> DataCalculator::getSwan(const std::shared_ptr<RgbData>& input, int pos){
	int filters = input->getSize().f;
	std::vector<std::vector<std::vector<float>>> data = input->getRawData();
	std::vector<std::vector<float>> temp(filters, std::vector<float>(input->getSize().t));
	for (int i = 0; i < filters; ++i) {
		temp[i] = data[i][pos];
	}
	std::shared_ptr<SeismicData> ret = std::make_shared<SeismicData>(temp, input->getDT(), DataType::EN_SWAN, input->getName(), input->getProcedures() + "_tr" + std::to_string(pos));
	ret->setFreq(input->getFreqs());
	return std::move(ret);
}

std::shared_ptr<IBaseData> DataCalculator::mute(const std::shared_ptr<IBaseData>& input, int start, int stop){
	std::shared_ptr<SeismicData> sData = std::dynamic_pointer_cast<SeismicData>(input);
	if (sData){
		int samples = sData->getSize().t;
		int traces = sData->getSize().x;
		std::vector<std::vector<float>> data = sData->getRawData();
		std::vector<std::vector<float>> data_;
		data_.assign(traces, std::vector<float>(samples, 0));
		for (int j = 0; j < traces; ++j) {
			#pragma omp parallel for
			for (int t = start; t < stop; ++t) {
				data_[j][t] = data[j][t];
			}
		}
		auto ret = std::make_shared<SeismicData>(data_, sData->getDT(), sData->getType(), sData->getName(), sData->getProcedures() + "_mute");
		ret->setCM(input->getCM());
		ret ->setFreq(sData->getFreq());
		return std::move(ret);
	}
	std::shared_ptr<RgbData> rData = std::dynamic_pointer_cast<RgbData>(input);
	if (rData){
			int samples = rData->getSize().t;
		int traces = rData->getSize().x;
		int filters = rData->getSize().f;
		std::vector<std::vector<std::vector<float>>> data = rData -> getRawData();
		std::vector<std::vector<std::vector<float>>> data_;
		data_.assign(filters, std::vector<std::vector<float>>(traces, std::vector<float>(samples, 0)));
		TLoading* load = new TLoading(Application->MainForm);
		load->setDuration(traces*filters);
		load->Show();
		for (int i = 0; i < filters; ++i) {

			for (int j = 0; j < traces; ++j) {
				#pragma omp parallel for
				for (int t = start; t < stop; ++t) {
					data_[i][j][t] = data[i][j][t];
				}
				load->update(1);
			}

		}
		auto ret = std::make_shared<RgbData>(data_, rData->getDT(), rData->getFreqs().front(), rData->getFreqs().back(), rData->getName(), rData->getProcedures() + "_mute");
		ret->setCM(input->getCM());
		return std::move(ret);
		}
	throw;
}

std::shared_ptr<RgbData> DataCalculator::SwanToRGB(const std::shared_ptr<SeismicData>& input, int window){
        if (input->getType() != DataType::SWAN && input->getType() != DataType::EN_SWAN) {
            return std::make_shared<RgbData>();
        }
        std::shared_ptr<SeismicData> inp = input;
        if (input->getType() == DataType::SWAN) {
            inp = toEnergy(input);
        }
        std::shared_ptr<SeismicData> smoothInput = std::dynamic_pointer_cast<SeismicData>(smoothT(inp, window));
    	int samples = smoothInput->getSize().t;
		std::vector<float> freqs = smoothInput->getFreq();
        auto data = smoothInput->getRawDataRef();
        std::vector<std::vector<std::vector<float>>> output(freqs.size());
        for (int i = 0; i < freqs.size(); ++i) {
            output[i] = {data[i]};
        }
        auto ret = std::make_shared<RgbData>(output, input->getDT(), freqs[0], freqs.back(), smoothInput->getName(), smoothInput->getProcedures()+"to_rgb");
        return std::move(ret);
}


std::shared_ptr<SeismicData> DataCalculator::filter(const std::shared_ptr<SeismicData>& input, float freq, int window){
		auto dt = input->getDT()/1000000.0;
		size2 sz = input->getSize();
		auto data = input->getRawDataRef();
		std::vector<std::vector<float>> output(sz.x, std::vector<float> (sz.t, 0));
		double at = std::exp(-2 * freq * window/100.0 * dt);
		double b = 2.0 * at * std::cos(2.0 * M_PI * dt * freq);
		double ab = std::pow(at, 2.0);
		double ab1 = ab / 2.0;
		std::vector<float> temp(sz.t, 0.0f);
		int tr = 0;
		for (auto &traceData : data) {
			for (int i = 2; i < sz.t; ++i)
			{
				double s = -traceData[i] / 2.0 + ab1 * traceData[i - 2];
				double s1 = s + b * temp[i - 1] - ab * temp[i - 2];
				temp[i] = s1;
			}

			for (int i = sz.t - 3; i > -1; --i)
			{
				double s = -temp[i] / 2.0 + ab1 * temp[i + 2];
				double s1 = s + b * output[tr][i + 1] - ab * output[tr][i+2];
				output[tr][i] = s1;
			}

			for (int i = 0; i < sz.t; ++i) output[tr][i] *= std::pow(freq,1.1);
			memset(temp.data(), 0.0f, sz.t * sizeof(float));
			++tr;
		}
		auto ret = std::make_shared<SeismicData>(output, input->getDT(), input->getType(), input->getName(), input->getProcedures()+"_fil(" +std::to_string(freq) + ")");
		ret -> setFreq(input->getFreq());
		return std::move(ret);
}

std::shared_ptr<RgbData> DataCalculator::Cluster(const std::shared_ptr<RgbData>& input, int minPts) {
    if (!input) return nullptr;

    // 1. Извлечение размеров и данных
    const auto& rawData = input->getRawData();
    size_t filters = rawData.size();
    if (filters == 0) return nullptr;
    size_t traces = rawData[0].size();
    if (traces == 0) return nullptr;
    size_t samples = rawData[0][0].size();
    if (samples == 0) return nullptr;

    const size_t totalPoints = traces * samples;

    // 2. Преобразование в список точек: каждая точка – вектор признаков длины filters
    //    Порядок обхода: сначала все отсчёты для первой трассы, затем для второй и т.д.
    std::vector<std::vector<float>> points(totalPoints, std::vector<float>(filters));
    for (size_t trace = 0; trace < traces; ++trace) {
        for (size_t sample = 0; sample < samples; ++sample) {
            size_t idx = trace * samples + sample;
            for (size_t f = 0; f < filters; ++f) {
                points[idx][f] = rawData[f][trace][sample];
            }
        }
    }

    // 3. Нормализация признаков (Z-score)
//    normalizeFeatures(points);
    preprocessForPearson(points);
    // 4. Параметры DBSCAN – требуют настройки под ваши данные
//    int minPts = 20;        // минимальное число точек в кластере

    // Диагностика (временно)
    {
        float avgNorm = 0.0f;
        int zeroNormCount = 0;
        for (auto& p : points) {
            float norm = std::sqrt(std::inner_product(p.begin(), p.end(), p.begin(), 0.0f));
            avgNorm += norm;
            if (norm < 1e-6f) zeroNormCount++;
        }
        avgNorm /= points.size();
        ShowMessage("Средняя норма: " + String(avgNorm) + ", нулевых: " + String(zeroNormCount));

        int sampleN = std::min(100, (int)points.size());
        float avgDist = 0.0f;
        int pc = 0;
        for (int i = 0; i < sampleN; ++i)
            for (int j = i+1; j < sampleN; ++j) {
                avgDist += euclideanDistance(points[i], points[j]);
                pc++;
            }
        avgDist /= pc;
        ShowMessage("Среднее расстояние (выборка): " + String(avgDist));
    }

    float eps = estimateEps(points, minPts, 0.9);      // радиус окрестности (после нормализации)
    ShowMessage(eps);


    // 5. Запуск DBSCAN
    std::vector<int> labels = dbscan(points, eps, minPts);

    // 6. Определение количества кластеров (исключая шум)
    int maxCluster = -1;
    for (int lbl : labels) {
        if (lbl > maxCluster) maxCluster = lbl;
    }
    int numClusters = maxCluster + 1; // если нет кластеров, то 0
    ShowMessage(numClusters);
    // 7. Построение палитры цветов (HSV -> RGB) для каждого кластера
    std::vector<std::tuple<float, float, float>> clusterColors(numClusters);
    for (int c = 0; c < numClusters; ++c) {
        float hue = (static_cast<float>(c) / numClusters) * 360.0f;
        float r, g, b;
        hsvToRgb(hue, 1.0f, 1.0f, r, g, b);
        clusterColors[c] = {r, g, b};
    }
    // Цвет шума (метка -1) – чёрный
    std::tuple<float, float, float> noiseColor = {0.0f, 0.0f, 0.0f};

    // 8. Формирование выходного RGB-куба (traces x samples x 3)
    std::vector<std::vector<std::vector<float>>> rgbData(
        3, std::vector<std::vector<float>>(traces, std::vector<float>(samples, 0.0f)));

    for (size_t trace = 0; trace < traces; ++trace) {
        for (size_t sample = 0; sample < samples; ++sample) {
            size_t idx = trace * samples + sample;
            int label = labels[idx];
            float r, g, b;
            if (label == -1) {
                std::tie(r, g, b) = noiseColor;
            } else {
                std::tie(r, g, b) = clusterColors[label];
            }
            rgbData[0][trace][sample] = r;  // R-слой
            rgbData[1][trace][sample] = g;  // G-слой
            rgbData[2][trace][sample] = b;  // B-слой
        }
    }

    // 9. Создание и возврат нового RgbData
    std::shared_ptr<RgbData> result = std::make_shared<RgbData>(
        rgbData,                     // RGB-данные
        input->getDT(),              // тот же шаг дискретизации
        1.0f, 100.0f,                 // границы частот (не используются для RGB)
        "Clusters",                 // имя
        "DBSCAN clustering (eps=" + std::to_string(eps) + ", minPts=" + std::to_string(minPts) + ")"
    );
    return result;
}
std::shared_ptr<RgbData> DataCalculator::DominantFrequencyDirection(const std::shared_ptr<RgbData>& input) {
    if (!input) return nullptr;

    auto freqField = computeDominantFreqField(input);
    auto gradT = gradientT(freqField);
    auto gradX = gradientX(freqField);

    size_t nTraces = freqField.size();
    size_t nSamples = freqField[0].size();

    std::vector<std::vector<std::vector<float>>> rgbData(
        3, std::vector<std::vector<float>>(nTraces, std::vector<float>(nSamples, 0.0f)));

    for (size_t tr = 0; tr < nTraces; ++tr) {
        for (size_t sm = 0; sm < nSamples; ++sm) {
            float gx = gradX[tr][sm];
            float gt = gradT[tr][sm];
            float angle = std::atan2(gt, gx); // [-π, π]
            // Преобразуем в hue [0,360)
            float hue = (angle + M_PI) * 180.0f / M_PI;
            float r, g, b;
            hsvToRgb(hue, 1.0f, 1.0f, r, g, b);
            rgbData[0][tr][sm] = r;
            rgbData[1][tr][sm] = g;
            rgbData[2][tr][sm] = b;
        }
    }

    return std::make_shared<RgbData>(rgbData, input->getDT(), 0.0f, 1.0f,
                                     input->getName() + "_dir",
                                     input->getProcedures() + "_gradDir");
}
std::shared_ptr<RgbData> DataCalculator::ClusterDirectionField(const std::shared_ptr<RgbData>& input,
                                                               int minPts, float epsDir) {
    if (!input) return nullptr;

    auto freqField = computeDominantFreqField(input);
    auto gradT = gradientT(freqField);
    auto gradX = gradientX(freqField);

    size_t nTraces = freqField.size();
    size_t nSamples = freqField[0].size();
    size_t nPoints = nTraces * nSamples;

    // Собираем точки: нормализованные векторы направления
    std::vector<std::vector<float>> points(nPoints, std::vector<float>(2, 0.0f));
    for (size_t tr = 0; tr < nTraces; ++tr) {
        for (size_t sm = 0; sm < nSamples; ++sm) {
            size_t idx = tr * nSamples + sm;
            float gx = gradX[tr][sm];
            float gt = gradT[tr][sm];
            float len = std::sqrt(gx*gx + gt*gt);
            if (len > 1e-6f) {
                points[idx][0] = gx / len;
                points[idx][1] = gt / len;
            } else {
                points[idx][0] = points[idx][1] = 0.0f;
            }
        }
    }

    // Вызываем DBSCAN (евклидово расстояние на плоскости)
    std::vector<int> labels = dbscan(points, epsDir, minPts);

    // Подсчёт кластеров
    int maxCluster = -1;
    for (int lbl : labels) if (lbl > maxCluster) maxCluster = lbl;
    int numClusters = maxCluster + 1;

    // Палитра
    std::vector<std::tuple<float,float,float>> clusterColors(numClusters);
    for (int c = 0; c < numClusters; ++c) {
        float hue = (c * 360.0f) / numClusters;
        float r,g,b;
        hsvToRgb(hue, 1.0f, 1.0f, r, g, b);
        clusterColors[c] = {r,g,b};
    }
    auto noiseColor = std::make_tuple(0.0f, 0.0f, 0.0f);

    // Формирование RGB вывода
    std::vector<std::vector<std::vector<float>>> rgbData(
        3, std::vector<std::vector<float>>(nTraces, std::vector<float>(nSamples, 0.0f)));

    for (size_t tr = 0; tr < nTraces; ++tr) {
        for (size_t sm = 0; sm < nSamples; ++sm) {
            size_t idx = tr * nSamples + sm;
            int label = labels[idx];
            float r,g,b;
            if (label == -1) {
                std::tie(r,g,b) = noiseColor;
            } else {
                std::tie(r,g,b) = clusterColors[label];
            }
            rgbData[0][tr][sm] = r;
            rgbData[1][tr][sm] = g;
            rgbData[2][tr][sm] = b;
        }
    }

    std::string proc = input->getProcedures() + "_clusterDir(eps=" + std::to_string(epsDir) + ",minPts=" + std::to_string(minPts) + ")";
    return std::make_shared<RgbData>(rgbData, input->getDT(), 0.0f, 1.0f, input->getName() + "_clusters", proc);
}
