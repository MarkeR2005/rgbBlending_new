//---------------------------------------------------------------------------

#pragma hdrstop
#include "ColorManager.h"
#include "SeismicData.h"
#include "RgbData.h"
#include <algorithm>
#include "GL/glew.h"
#include "VCL.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

//Constructor
ColorManager::ColorManager(float p, bool zero):boundPercent(p), zeroMode(zero){}
ColorManager::~ColorManager(){
}
//Data addition
void ColorManager::addToSelection(std::weak_ptr<IBaseData> input){
	std::shared_ptr<SeismicData> sd = std::dynamic_pointer_cast<SeismicData>(input.lock());
	if (sd) {
		const size2 sz = sd->getSize();
		const size_t w = sz.t;
		const int n = sz.x * sz.t;
		const size_t sample_size = n > 50000 ? 50000 : n;
		const size_t step = n / sample_size;
		std::vector<std::vector<float>> data = sd->getRawDataRef();
		this->data[input] = {};
		this->data[input].reserve(sample_size);
		for (size_t i = 0; i < sample_size; ++i)
		{
			this->data[input].push_back(data[i*step/w][i*step%w]);
		}
		needsUpdate = true;
		return;
	}
	std::shared_ptr<RgbData> rd = std::dynamic_pointer_cast<RgbData>(input.lock());
	if (rd) {
		size3 sz = rd->getSize();
		const size_t w = sz.t;
		const int n = sz.x * sz.t;
		const size_t sample_size = n > 10000 ? 10000 : n;
		const size_t step = n / sample_size;

		this->data[input] = {};
		this->data[input].reserve(sample_size*sz.f);
		for (auto const& filter : rd->getRawDataRef())
		{
			for (size_t i = 0; i < sample_size; ++i)
			{
				this->data[input].push_back(filter[i*step/w][i*step%w]);
			}
		}
		needsUpdate = true;
		return;
	}
}
//Data delition
void ColorManager::clear(){
	data.clear();
	needsUpdate = true;
}
//
void ColorManager::removeFromSelection(std::weak_ptr<IBaseData> input)
{
	if (data.find(input) != data.end())
	{
		data.erase(input);
	}
	needsUpdate = true;
}
//Enumeration
void ColorManager::update(){
	std::vector<float> selection;
	selection.reserve(data.size()*50000);
	for (auto const& [key, val] : data) {
		for (auto sample : val) {
			selection.push_back(sample);
		}
	}
	const size_t sample_size = selection.size();
	if (sample_size < 1)
	{
		median = 0.0f;
		lowerBound = 0.0f;
		upperBound = 0.0f;
		return;
	}
	// 2. Вычисление статистик по выборке
	const float lower_percent = (100.0f - boundPercent) * 0.5f;
	const float upper_percent = 100.0f - lower_percent;
	const size_t k_lower = static_cast<size_t>(lower_percent * sample_size / 100.0f);
	const size_t k_upper = static_cast<size_t>(upper_percent * sample_size / 100.0f);
	const size_t k_median = sample_size / 2;
	// Ограничиваем индексы
	const size_t indices[3] =
	{
		std::min(k_lower, sample_size - 1),
		std::min(k_median, sample_size - 1),
		std::min(k_upper, sample_size - 1)
	};
	// Параллельная сортировка выборки
	std::sort(selection.begin(), selection.end());

	median = selection[indices[1]];
	lowerBound = selection[indices[0]];
	upperBound = selection[indices[2]];
	if (zeroMode && lowerBound < 0.0f) {
		median = 0.0f;
	}
//	else if (zeroMode) {
//		lowerBound = 0.0f;
//		median = (upperBound + lowerBound)/2.0f;
//	}

	range_low = median - lowerBound;
	range_high = upperBound - median;
	if (lowerBound < 0.0f) {
		inv_range_low = (range_low > 1e-9f) ? 128.0f / range_low : 0.0f;
		inv_range_high = (range_high > 1e-9f) ? 127.0f / range_high : 0.0f;
	} else {
		inv_range_low = (range_low > 1e-18f) ? 128.0f / range_low : 0.0f;
		inv_range_high = (range_high > 1e-18f) ? 127.0f / range_high : 0.0f;
    }

	needsUpdate = false;
}
//
bitMap ColorManager::getTexture(const std::vector<std::vector<float>>& dat){
	if (needsUpdate) {
		update();
	}
	int maxSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
	const int width = std::min<int>(dat.size(), maxSize);
	const int height = std::min<int>(dat[0].size(), maxSize);
	const size_t total = static_cast<size_t>(height) * width;
	std::vector<uint8_t> color(total);
	#pragma omp parallel for
	for (int r = 0; r < width; ++r)
	{
		const size_t output_offset = static_cast<size_t>(r);
		for (int s = 0; s < height; ++s)
		{
			const float value = dat[r][s];
			uint8_t color_val;
			if (!std::isfinite(value) || !std::isfinite(lowerBound) || !std::isfinite(upperBound)) {
				color_val = 0;
			}
			if (value <= median){
				const float clamped = std::max(value, lowerBound);
				color_val = 128 - static_cast<uint8_t>((median - clamped) * inv_range_low);
			} else
			{
				const float clamped = std::min(value, upperBound);
				color_val = 128 + static_cast<uint8_t>((clamped - median) * inv_range_high);
			}
			color[output_offset + static_cast<size_t>(s) * width] = color_val;
		}
	}
	return bitMap(color, height, width);
}

