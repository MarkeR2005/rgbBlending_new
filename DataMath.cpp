//---------------------------------------------------------------------------

#pragma hdrstop

#include "DataMath.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <iostream>
#include <Windows.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
std::vector<Traces> old_tracesToSwanEn(Traces input, bool timeSwan, int nf, int fn,
										int fk, int filterWidth, float* freqs)
{
    double dt_val = input.dt / 1000000.0;
    double log_coeff = std::log(static_cast<double>(fk) / fn) / (nf-1);
    double df = filterWidth / 100.0;
    int l = input.samplesNumber;
    int w = input.tracesNumber;
    float* data = input.data;

    // Создаем копию данных с применением затухания границ
    float* data_tapered = new float[l * w];
    std::memcpy(data_tapered, data, l * w * sizeof(float));

    // Применяем затухание границ
	for (int r = 0; r < w; ++r)
	{
		for (int i = 0; i < 4; ++i)
		{
            float s = i * 0.2f;
            data_tapered[i + r * l] *= s;
            data_tapered[l - i - 1 + r * l] *= s;
        }
    }

    std::vector<Traces> output(nf);

    // Основной цикл по частотам с распараллеливанием
#pragma omp parallel for
	for (int n2 = 0; n2 < nf; ++n2)
	{
        double f = fn * std::exp(n2 * log_coeff);
        double at = std::exp(-2 * f * df * dt_val);
        double b = 2.0 * at * std::cos(2.0 * M_PI * dt_val * f);
        double ab = at * at;
        double ab1 = ab / 2.0;

        // Временные массивы для текущей частоты
        std::vector<float> h(l, 0.0f);
        std::vector<float> pref(l + 1, 0.0f);
        float* out = new float[l * w];

		for (int r = 0; r < w; ++r)
		{
            // Прямой фильтр
			for (int n = 2; n < l; ++n)
			{
				float s_val = -data_tapered[n + r * l] * 0.5f + static_cast<float>(ab1)
				* data_tapered[n - 2 + r * l];
				h[n] = s_val + static_cast<float>(b) * h[n - 1]
				- static_cast<float>(ab) * h[n - 2];
			}
                                                
			for (int n = l - 3; n >= 0; --n)
			{
                float s_val = -h[n] * 0.5f + static_cast<float>(ab1) * h[n + 2];
				out[n+r*l] = s_val + static_cast<float>(b) * out[n + r * l + 1]
				- static_cast<float>(ab) * out[n + r * l + 2];
			}
            // Обратный фильтр

            // Применение частотной коррекции
            float f_factor = std::pow(f, 1.1f);
			for (int n = 0; n < l; ++n)
			{
                out[n + r * l] *= f_factor;
				out[n + r * l] *= out[n + r * l]/1e5;
            }
        }

        freqs[n2] = f;
        Traces tr;
        tr.init(w, l, input.dt, out);
        output[n2] = tr;

#pragma omp critical
        std::cout << "Processed frequency: " << n2 << " (" << f << " Hz)" << std::endl;
    }

    delete[] data_tapered;
    return output;
}
//--
Traces traceToSWAN(Traces& input, int traceno, bool timeSwan, int nf, int fn, int fk,
					 int filterWidth, float* freqs)
{
    int I = input.samplesNumber;
    double dt = input.dt/1000000.0;
	double coeff = std::exp(1.0/(nf-1)*std::log(fk*1.0/fn));
    double df = filterWidth/100.0;
    int l = I;
    double f, s, s1;
    int i, n2;
    float* data = input.data;
    Traces tr;
    float* y = new float[l*nf];
    std::memset(y, 0, sizeof (y));
    float* h = new float [l];
	for(n2=0, f=fn; n2<nf; n2++, f*=coeff)
	{//Основной цикл
        std::memset(h, 0, sizeof (h));
		double at = std::exp(-2 * f * df * dt);
		double b = 2.0 * at * std::cos(2.0 * M_PI * dt * f);
		double ab = std::pow(at, 2.0);
		double ab1 = ab / 2.0;
		for (i = 0, s = 0.0; i < 4; ++i, s += 0.2)
		{
            data[i+traceno*l] *= s;
            data[l - i - 1+traceno*l] *= s;
        }
		for (int n = 2; n < l; n++)
		{
            s = -data[n+traceno*l] / 2.0 + ab1 * data[n - 2+traceno*l];
            s1 = s + b * h[n - 1] - ab * h[n - 2];
            h[n] = s1;
        }
		for(int n=2, n1=l-3; n<l; n++, n1--)
		{
            s=-h[n1]/2.0+ab1*h[n1+2];
            s1=s+b*y[n1+1+n2*l]-ab*y[n1+2+n2*l];
            y[n1+n2*l]=s1;
        }
		for (int n=0;n<l;n++) y[n+n2*l]*= std::pow(f,1.1);
		if (freqs != nullptr)
		{
			freqs[n2]=f;
		}
	}
    tr.init(nf, l, input.dt, y);
    return tr;
}
//--
Traces getTrace(const Traces& input, int traceno){
	int s = input.samplesNumber;
	float* tr = input.data + s*traceno;
	Traces output;
	float* tr_out = new float[s*100];
	for (int i = 0; i < s; i++) {
		for (int j = 0; j < 100; j++) {
			tr_out[i+s*j] = tr[i];
		}
	}
	output.init(100,s,input.dt, tr_out);
	return output;
}
//--
Traces traceToSWAN_E(Traces& input, int traceno, bool timeSwan, int nf, int fn, int fk,
					 int filterWidth, float* freqs, int tWidth)
{
    int I = input.samplesNumber;
    double dt = input.dt/1000000.0;
	double coeff = std::exp(1.0/(nf-1)*std::log(fk*1.0/fn));
    double df = filterWidth/100.0;
    int l = I;
    double f, s, s1;
    int i, n2;
    float* data = input.data;
    Traces tr;
    float* y = new float[l*nf];
    std::memset(y, 0, sizeof (y));
    float* h = new float [l];
	for(n2=0, f=fn; n2<nf; n2++, f*=coeff)
	{//Основной цикл
        std::memset(h, 0, sizeof (h));
		double at = std::exp(-2 * f * df * dt);
		double b = 2.0 * at * std::cos(2.0 * M_PI * dt * f);
		double ab = std::pow(at, 2.0);
		double ab1 = ab / 2.0;
		for (i = 0, s = 0.0; i < 4; ++i, s += 0.2)
		{
            data[i+traceno*l] *= s;
            data[l - i - 1+traceno*l] *= s;
        }
		for (int n = 2; n < l; n++)
		{
            s = -data[n+traceno*l] / 2.0 + ab1 * data[n - 2+traceno*l];
            s1 = s + b * h[n - 1] - ab * h[n - 2];
            h[n] = s1;
        }
		for(int n=2, n1=l-3; n<l; n++, n1--)
		{
            s=-h[n1]/2.0+ab1*h[n1+2];
            s1=s+b*y[n1+1+n2*l]-ab*y[n1+2+n2*l];
			y[n1+n2*l]=s1;
        }
		for (int n=0;n<l;n++) y[n+n2*l]*= std::pow(f,1.1);
		for (int n=0;n<l;n++) y[n+n2*l]*= y[n+n2*l]/1000000000;
		double* pref_sum = new double[l+1];
		pref_sum[0]= 0;
		for (i = 0; i < l; i++) {
			pref_sum[i+1]=pref_sum[i]+y[i+n2*l];
		}
		for (int n=0;n<l;n++) {
		int left = std::max(n-tWidth/2,0);
		int right = std::min(n+tWidth/2,l-1);
		y[n+n2*l] = (pref_sum[right]-pref_sum[left])/(right-left);
		}
		if (freqs != nullptr)
		{
			freqs[n2]=f;
		}
	}
    tr.init(nf, l, input.dt, y);
    return tr;
}
//--
std::vector<Traces> smoothingT(const std::vector<Traces>& input, int tWidth)
{
	if (tWidth <= 1)
	{
		return input;
	}
	std::vector<Traces> output = input;
    const int l = output[0].samplesNumber;
    const int n = output[0].tracesNumber;
    const int half_window = tWidth / 2;
	double *pref = new double[l + 1];
	for (int k = 0; k < output.size(); ++k)
	{
		float* data = input[k].data;
		float* out = output[k].data;
		for (int j = 0; j < n; ++j)
		{
			pref[0]=0.0;
			const int offset = j*l;
			float* trace_in = data + offset;
			float* trace_out = out + offset;
			for (int i = 0; i < l; ++i)
			{
				pref[i + 1] = pref[i] + trace_in[i];
			}

			#pragma omp parallel for
			for (int i = 0; i < l; ++i)
			{
				int left = i < half_window ? 0 : i-half_window;
				int right = i + half_window > l-1 ? l-1: i+half_window;
				int count = right - left + 1;
				double val = (pref[right + 1] - pref[left])/count;
				trace_out[i] = static_cast<float>(val);
			}
		}
	}
    return output;
}
//--
std::vector<Traces> smoothingF(const std::vector<Traces>& input, int fWidth,
									int nf1, int nf2)
{
	if (fWidth <= 1)
	{
        return input;
    }
    std::vector<Traces> output = input;
    const int l = output[0].samplesNumber;
    const int n = output[0].tracesNumber;
    const int sz = n*l;
    const int nf3 = input.size();
//    std::cout << "Started frequency smoothing" << std::endl;
    const int half = fWidth/2;
	double* data = new double[n*l];
    ZeroMemory(data, sizeof(data));
    int left = 0;
    int right = -1;
	while((left+right)/2<nf1-1)
	{
		if (left == 0 && right >= half)
		{
            const int wd = right-left+1;
            const int ind = right-half;
            float* out = output[ind].data;
			for (int i = 0; i< sz; ++i)
			{
                out[i] = static_cast<float>(data[i]/wd);
            }
        }
		else if (right - left + 1 == fWidth)
		{
            const int ind = (right+left)/2;
            float* out = output[ind].data;
			for (int i = 0; i< sz; ++i)
			{
				out[i] = static_cast<float>(data[i]/fWidth);
            }
		}
		else if (right == nf1-1 && left+half < nf1)
		{
			const int wd = right-left+1;
            const int ind = left+half;
			float* out = output[ind].data;
			for (int i = 0; i < sz; ++i)
			{
                out[i] = static_cast<float>(data[i]/wd);
            }
		}

		if ((right-left+1 < fWidth) && right < nf1-1)
		{
            ++right;
			const float* inp = input[right].data;
			for (int i = 0; i< sz; ++i)
			{
                data[i] += static_cast<double>(inp[i]);
			}
		}
		else if (right-left+1 >= fWidth || right == nf1-1)
		{
            const float* inp = input[left].data;
			for (int i = 0; i< sz; ++i)
			{
				data[i] -= static_cast<double>(inp[i]);
            }
            ++left;
        }
    }
	left = nf1;
    right = nf1-1;
	ZeroMemory(data, sizeof(data));
	while((left+right)/2<nf2-1)
	{
		if (left == nf1 && right >= nf1+half)
		{
            const int wd = right-left+1;
            const int ind = right-half;
			float* out = output[ind].data;
			for (int i = 0; i< sz; ++i)
			{
                out[i] = static_cast<float>(data[i]/wd);
            }
		}
		else if (right - left + 1 == fWidth)
		{
            const int ind = (right+left)/2;
            float* out = output[ind].data;
			for (int i = 0; i< sz; ++i)
			{
                out[i] = static_cast<float>(data[i]/fWidth);
			}
		}
		else if (right == nf2-1 && left+half < nf2)
		{
			const int wd = right-left+1;
            const int ind = left+half;
            float* out = output[ind].data;
			for (int i = 0; i< sz; ++i)
			{
                out[i] = static_cast<float>(data[i]/wd);
			}
        }

		if ((right-left+1 < fWidth)&& right < nf2-1)
		{
            ++right;
			const float* inp = input[right].data;
			for (int i = 0; i< sz; ++i)
			{
                data[i] += static_cast<double>(inp[i]);
            }
        }
		else if (right-left+1 >= fWidth || right == nf2-1)
		{
			const float* inp = input[left].data;
			for (int i = 0; i< sz; ++i)
			{
				data[i] -= static_cast<double>(inp[i]);
            }
            ++left;
		}
    }
    left = nf2;
    right = nf2-1;
	ZeroMemory(data, sizeof(data));
	while((left+right)/2<nf3-1)
	{
		if (left == nf2 && right >= half + nf2)
		{
			const int wd = right-left+1;
            const int ind = right-half;
            float* out = output[ind].data;
			for (int i = 0; i< sz; ++i)
			{
                out[i] = static_cast<float>(data[i]/wd);
            }
		}
		else if (right - left + 1 == fWidth)
		{
            const int ind = (right+left)/2;
			float* out = output[ind].data;
			for (int i = 0; i< sz; ++i)
			{
				out[i] = static_cast<float>(data[i]/fWidth);
            }
        }
		else if (right == nf3-1 && left+half < nf3)
		{
            const int wd = right-left+1;
            const int ind = left+half;
            float* out = output[ind].data;
			for (int i = 0; i< sz; ++i)
			{
                out[i] = static_cast<float>(data[i]/wd);
            }
        }

		if ((right-left+1 < fWidth) && right < nf3-1)
		{
            ++right;
            const float* inp = input[right].data;
			for (int i = 0; i< sz; ++i)
			{
                data[i] += static_cast<double>(inp[i]);
            }
		}
		else if (right-left+1 >= fWidth || right == nf3-1)
		{
			const float* inp = input[left].data;
			for (int i = 0; i< sz; ++i)
			{
				data[i] -= static_cast<double>(inp[i]);
            }
            ++left;
		}
	}
	return output;
}
//--
std::vector<Traces> tracesToSwanEn(Traces input, bool timeSwan, int nf, int fn,
									int fk, int filterWidth, float* freqs)
{
    const double dt_val = input.dt / 1000000.0;
    const double log_coeff = std::log(static_cast<double>(fk) / fn) / (nf-1);
    const double df = filterWidth / 100.0;
    const int l = input.samplesNumber;
    const int w = input.tracesNumber;
	const float* data = input.data;
    // Предварительные вычисления коэффициентов
    std::vector<double> f_(nf), at_(nf), b_(nf), ab_(nf), ab1_(nf);
	std::vector<float> f_factor_(nf);

	for (int i = 0; i < nf; ++i)
	{
        f_[i] = fn * std::exp(i * log_coeff);
        at_[i] = std::exp(-2 * f_[i] * df * dt_val);
        b_[i] = 2.0 * at_[i] * std::cos(2.0 * M_PI * dt_val * f_[i]);
        ab_[i] = at_[i] * at_[i];
        ab1_[i] = ab_[i] / 2.0;
		f_factor_[i] = std::pow(f_[i], 1.1f);
		freqs[i] = static_cast<float>(f_[i]);
	}
    // Подготовка выходных данных
    std::vector<Traces> output(nf);
	for (int i = 0; i < nf; ++i)
	{
		output[i].init(w, l, input.dt, new float[l * w]);
	}

    // Выделение памяти для данных с затуханием
	std::vector<float> data_tapered(l * w);
	#pragma omp parallel for
	for (int r = 0; r < w; ++r)
	{
        const int offset = r * l;
		for (int i = 0; i < l; ++i)
		{
            float val = data[i + offset];
			if (i < 4) val *= i * 0.2f;
			else if (i >= l-4) val *= (l - i - 1) * 0.2f;
			data_tapered[i + offset] = val;
		}
	}
    // Основной вычислительный блок
#pragma omp parallel
	{
		// Выделение буферов для каждого потока (исключаем аллокации в цикле)
		std::vector<float> h_buf(l, 0.0f);
		std::vector<float> out_buf(l, 0.0f);

        // Параллелизация по каналам
#pragma omp for schedule(guided)
		for (int r = 0; r < w; ++r)
		{
			const int trace_offset = r * l;
			float* trace_in = data_tapered.data() + trace_offset;

			for (int n2 = 0; n2 < nf; ++n2)
			{
                float* h = h_buf.data();
                float* out = out_buf.data();
				float* out_trace = output[n2].data + trace_offset;

                // Инициализация граничных условий
                h[0] = h[1] = 0.0f;
                out[l-1] = out[l-2] = 0.0f;

                const float b_val = b_[n2];
                const float ab_val = ab_[n2];
                const float ab1_val = ab1_[n2];
				const float f_factor = f_factor_[n2];
				// Прямой фильтр
				for (int n = 2; n < l; ++n)
				{
                    const float s_val = -trace_in[n] * 0.5f + ab1_val * trace_in[n-2];
                    h[n] = s_val + b_val * h[n-1] - ab_val * h[n-2];
                }

                // Обратный фильтр
				for (int n = l-3; n >= 0; --n)
				{
                    const float s_val = -h[n] * 0.5f + ab1_val * h[n+2];
                    out[n] = s_val + b_val * out[n+1] - ab_val * out[n+2];
				}
				// Частотная коррекция и квадрат
				for (int n = 0; n < l; ++n) {
					if (std::abs(out[n]) > 1e17)
					{
						out_trace[n] = FLT_MAX;
					} else
					{
						const double val = out[n] * f_factor;
						out_trace[n] = static_cast<float>(val * val);
					}
				}
			}
		}
    }

    return output;
}
//--
std::vector<Traces> smoothing4D(const std::vector<std::vector<Traces>>& toSmooth, const int dt, const int nf)
{
    // Проверка входных данных
    if (toSmooth.empty() || toSmooth[0].empty()) {
        return std::vector<Traces>();
    }

    const int yCount = toSmooth.size();
    const int actualNf = std::min(nf, static_cast<int>(toSmooth[0].size()));
    const int w = toSmooth[0][0].tracesNumber;
    const int l = toSmooth[0][0].samplesNumber;
    const int sz = w * l;

    // Проверка согласованности размеров
    for (const auto& ySlice : toSmooth) {
        if (ySlice.size() < actualNf) {
            throw std::runtime_error("Inconsistent number of frequencies");
        }
        for (int i = 0; i < actualNf; ++i) {
            if (ySlice[i].tracesNumber != w || ySlice[i].samplesNumber != l) {
                throw std::runtime_error("Inconsistent dimensions in input data");
            }
        }
    }

    std::vector<Traces> output(actualNf);

    #pragma omp parallel for
    for (int freq = 0; freq < actualNf; ++freq) {
        std::vector<float> result(sz, 0.0f);

        // Суммируем по всем Y
        for (int y = 0; y < yCount; ++y) {
            const float* data = toSmooth[y][freq].data;
            for (int i = 0; i < sz; ++i) {
                result[i] += data[i];
            }
        }

        // Делим на количество Y
        const float invCount = 1.0f / yCount;
        for (int i = 0; i < sz; ++i) {
            result[i] *= invCount;
        }

        // Копируем данные в динамическую память
        float* dataCopy = new float[sz];
        std::copy(result.begin(), result.end(), dataCopy);
        output[freq].init(w, l, dt, dataCopy);
    }

    return output;
}
//--
std::vector<Traces> smoothingX(const std::vector<Traces>& input, int xWidth)
{
    if (xWidth <= 1) {
        return input;
    }

    std::vector<Traces> output = input;
    const int w = input.size();
    const int l = input[0].samplesNumber;
    const int n = input[0].tracesNumber;
    const int half_window = xWidth / 2;

    #pragma omp parallel for
    for (int k = 0; k < w; ++k) {
        const float* data = input[k].data;
        float* out = output[k].data;

        // Для каждого временного отсчета создаем префиксные суммы
        for (int i = 0; i < l; ++i) {
            std::vector<double> pref(n + 1, 0.0);
            for (int j = 0; j < n; ++j) {
                pref[j + 1] = pref[j] + data[j * l + i];
            }

            // Применяем скользящее среднее по трассам
            for (int j = 0; j < n; ++j) {
                int left = std::max(0, j - half_window);
                int right = std::min(n - 1, j + half_window);
                int count = right - left + 1;
                double sum = pref[right + 1] - pref[left];
                out[j * l + i] = static_cast<float>(sum / count);
            }
        }
    }

    return output;
}
//--
