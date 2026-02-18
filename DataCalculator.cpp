//---------------------------------------------------------------------------

#pragma hdrstop

#include "DataCalculator.h"
#include <string>
#include "UnitLoading.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

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
throw;
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


