#include "UnitLoading.h"

#pragma hdrstop
#include "RgbData.h"

#pragma package(smart_init)
#define Write(x,y) Write(x, (Longint)(y))
#define Read(x,y) Read(x, (Longint)(y))
//Constructors
//------------------------------------------------------------------------------
RgbData::RgbData(const std::vector<std::vector<std::vector<float>>>& data_, const float dT_,
const float freqStart, const float freqFinish, const std::string name_, const std::string procedures_):
data(data_), dT(dT_), name(name_), procedures(procedures_)
{

	filters = data.size();
	traces = data[0].size();
	samples = data[0][0].size();
	double coeff = std::exp(1.0/(filters-1)*std::log(freqFinish*1.0/freqStart));
	frequencies.reserve(filters);
	float freq = freqStart;
	for (int i = 0; i < filters; i++) {
		frequencies.push_back(freq);
		freq *= coeff;
	}
}
//------------------------------------------------------------------------------
RgbData::RgbData(SeismicData* base, int freqStart, int freqFinish, int filtersNumber, int filterWidth): filters(filtersNumber)
{
	std::vector<std::vector<float>> baseData = base->getRawData();
	traces = baseData.size();
	samples = baseData[0].size();

	name = base->getName() + "_rgb";
	procedures = base->getProcedures() + "_rgb(fs=" + std::to_string(freqStart) + ";ff=" + std::to_string(freqFinish) +
	";nf=" + std::to_string(filtersNumber) + ";fw=" + std::to_string(filterWidth) + ")";

	dT = base->getDT();
	const double dt_val = dT / 1000000.0;
	const double coeff = std::log(freqFinish* 1.0 / freqStart) / (filters-1);
	const double df = filterWidth / 100.0;


	std::vector<double> f(filters), at(filters), b(filters), ab(filters), ab1(filters);
	std::vector<float> f_factor_(filters);
	data.assign(filters, std::vector<std::vector<float>>(traces, std::vector<float>(samples, 0.0f)));
	frequencies.reserve(filters);
	for (int i = 0; i < filters; i++) {
		f[i] = freqStart * std::exp(i * coeff);
		at[i] = std::exp(-2 * f[i] * df * dt_val);
		b[i] = 2.0 * at[i] * std::cos(2.0 * M_PI * dt_val * f[i]);
		ab[i] = at[i] * at[i];
		ab1[i] = ab[i] / 2.0;
		f_factor_[i] = std::pow(f[i], 1.1f);
		frequencies.push_back(freqStart * std::exp(i*coeff));
	}


	#pragma omp parallel for
	for (std::vector<float>& trace : baseData)
	{
		for (int i = 1; i < 5; ++i)
		{
			trace[i] *= i/5.0;
			trace[samples - i - 1] *= i/5.0;
		}
	}
	TLoading* load = new TLoading(Application->MainForm);
	load->setDuration(traces);
	load->Show();
    #pragma omp parallel
	{
		// Выделение буферов для каждого потока (исключаем аллокации в цикле)
		std::vector<float> temp(samples);
		#pragma omp for schedule(guided)
		for (int tr = 0; tr < traces; ++tr)
		{
			std::vector<float>& trace_in = baseData[tr];

			for (int frNo = 0; frNo < filters; ++frNo)
			{
				std::vector<float>& out = data[frNo][tr];
				// Инициализация граничных условий
				temp[0] = temp[1] = 0.0f;

				const float b_val = b[frNo];
				const float ab_val = ab[frNo];
				const float ab1_val = ab1[frNo];
				const float f_factor = f_factor_[frNo];
				// Прямой фильтр

				for (int n = 2; n < samples; ++n)
				{
                    const float s_val = -trace_in[n] * 0.5f + ab1_val * trace_in[n-2];
					temp[n] = s_val + b_val * temp[n-1] - ab_val * temp[n-2];
                }

                // Обратный фильтр
				for (int n = samples-3; n >= 0; --n)
				{
					const float s_val = -temp[n] * 0.5f + ab1_val * temp[n+2];
                    out[n] = s_val + b_val * out[n+1] - ab_val * out[n+2];
				}
				// Частотная коррекция и квадрат
				for (int n = 0; n < samples; ++n) {
					if (std::abs(out[n]) > 1e17)
					{
						out[n] = FLT_MAX;
					} else
					{
						const double val = out[n] * f_factor;
						out[n] = static_cast<float>(val * val);
					}
				}
			}
            load->update(1);
		}

	}
}
//------------------------------------------------------------------------------
RgbData::RgbData(const RgbData& other)
{
	data = other.data;
	dT = other.dT;
	samples = other.samples;
	traces = other.traces;
	filters = other.filters;
	frequencies = other.frequencies;
	name = other.name;
	procedures = other.procedures;
}
//------------------------------------------------------------------------------
RgbData::~RgbData(){
if (mng) {
mng->removeFromSelection(weak_from_this());
}
}
//Setters
//------------------------------------------------------------------------------
void RgbData::setCM(std::shared_ptr<ColorManager> cm){
	if (auto old_mng = mng) {
	old_mng->removeFromSelection(weak_from_this());
	}
	mng = cm;
	if (cm) {
			update = [cm, share = weak_from_this()](){cm->addToSelection(share);};
			update();
	} else {
			update = [](){};
	}
}
//------------------------------------------------------------------------------
//Getters
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::vector<bitMap> RgbData::getTexture(){
	if (mng) {
		std::vector<bitMap> out;
		out.reserve(filters);
		TLoading* load = new TLoading(Application->MainForm);
		load->setDuration(filters);
		load->Show();
		for (auto& filter : data) {
			out.push_back(mng->getTexture(filter));
			load->update(1);
		}
		return out;
	}
	return std::vector<bitMap>();
}
//------------------------------------------------------------------------------
//Transformation
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void RgbData::saveFile(const std::wstring& loc) {
	std::unique_ptr<TFileStream> stream(new TFileStream (loc.c_str(), fmCreate));;

	// Записываем параметры
	stream->Write(&dT, sizeof(dT));
	stream->Write(&samples, sizeof(samples));
	stream->Write(&traces, sizeof(traces));
	stream->Write(&filters, sizeof(filters));

	// Записываем frequencies
	float freqStart = frequencies[0];
	float freqFinish = frequencies[filters-1];
	stream->Write(&freqStart, sizeof(float));
	stream->Write(&freqFinish, sizeof(float));

	// Записываем строки
	int len = name.length();
	stream->Write(&len, sizeof(len));
	stream->Write(name.c_str(), len);

	len = procedures.length();
	stream->Write(&len, sizeof(len));
	stream->Write(procedures.c_str(), len);
	TLoading* load = new TLoading(Application->MainForm);
	load->setDuration(traces*filters);
	load->Show();
	// Записываем 3D данные
	for (auto& filter : data) {
		for (auto& trace : filter) {
			stream->Write(trace.data(), trace.size() * sizeof(float));
			load->update(1);
		}
	}
}
RgbData RgbData::loadFile(const std::wstring& loc) {
	std::unique_ptr<TFileStream> stream(new TFileStream (loc.c_str(), fmOpenRead));

	float dT;
	int samples;
	int traces;
	int filters;
	// Читаем параметры
	stream->Read(&dT, sizeof(dT));
	stream->Read(&samples, sizeof(samples));
	stream->Read(&traces, sizeof(traces));
	stream->Read(&filters, sizeof(filters));

	float freqStart;
	float freqFinish;
	// Читаем frequencies
	stream->Read(&freqStart, sizeof(freqStart));
	stream->Read(&freqFinish, sizeof(freqFinish));

	std::string name;
	std::string procedures;
	// Читаем строки
	int len;
	stream->Read(&len, sizeof(len));
	name.resize(len);
	stream->Read(&name[0], len);

	stream->Read(&len, sizeof(len));
	procedures.resize(len);
	stream->Read(&procedures[0], len);

	std::vector<std::vector<std::vector<float>>> data;
	// Читаем 3D данные
	data.resize(filters);
	TLoading* load = new TLoading(Application->MainForm);
	load->setDuration(traces*filters);
	load->Show();
	for (auto& filter : data) {
		filter.resize(traces);
		for (auto& trace : filter) {
			trace.resize(samples);
			stream->Read(trace.data(), samples * sizeof(float));
			load->update(1);
		}
	}
	return RgbData(data, dT, freqStart, freqFinish, name, procedures);
}

