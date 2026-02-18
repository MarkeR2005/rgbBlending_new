#pragma hdrstop
#include "SeismicData.h"
#include "RgbData.h"
#include "UnitLoading.h"

#include <vcl.h>

#pragma package(smart_init)
#define Write(x,y) Write(x, (Longint)(y))
#define Read(x,y) Read(x, (Longint)(y))
//Constructors
//------------------------------------------------------------------------------
SeismicData::SeismicData(const std::vector<std::vector<float>> data_, const float dT_, const DataType type_,
const std::string name_, const std::string procedures_):
data(data_), dT(dT_), type(type_), name(name_), procedures(procedures_)
{
	traces = data.size();
	samples = data[0].size();
	if (traces == 1 && type == DataType::BASE) {
		type = DataType::TRACE;
	}
}
//------------------------------------------------------------------------------
SeismicData::SeismicData(const float* data_, const float dT_, const int samples_,
const int traces_, const DataType type_, const std::string name_, const std::string procedures_):
 dT(dT_), samples(samples_), traces(traces_), type(type_), name(name_), procedures(procedures_)
{
	data.reserve(traces);
	for (int i = 0; i < traces; ++i) {
		std::vector<float> vec;
		vec.assign(data_ + i * samples, data_ + (i + 1) * samples);
		data.push_back(vec);
	}
	if (traces == 1) {
		type = DataType::TRACE;
	}
}
//------------------------------------------------------------------------------
SeismicData::SeismicData(const SeismicData& other)
{
	data = other.data;
	dT = other.dT;
	samples = other.samples;
	traces = other.traces;
	name = other.name;
	procedures = other.procedures;
	freq = other.freq;
}
//------------------------------------------------------------------------------
SeismicData::~SeismicData(){
if (!mng) {
     return;
}
	mng->removeFromSelection(weak_from_this());
}
//Setters
//------------------------------------------------------------------------------
void SeismicData::setCM(std::shared_ptr<ColorManager> cm){
	if (auto old_mng = mng) {
		old_mng->removeFromSelection(weak_from_this());
	}
	mng = cm;
	if (cm) {
			update = [cm, self = weak_from_this()]() { cm->addToSelection(self); };
			update();
	} else {
			update = [](){};
	}
}
//------------------------------------------------------------------------------
//Operations
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
bitMap SeismicData::getTexture(){
	if (mng) {
		return mng->getTexture(data);
	}
	return bitMap();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SeismicData::saveFile(const std::wstring& loc) {
	std::unique_ptr<TFileStream> stream(new TFileStream (loc.c_str(), fmCreate));

	// Записываем параметры
	stream->Write(&dT, sizeof(dT));
	stream->Write(&samples, sizeof(samples));
	stream->Write(&traces, sizeof(traces));

	// Записываем строки
	int len = name.length();
	stream->Write(&len, sizeof(len));
	stream->Write(name.c_str(), len);

	len = procedures.length();
	stream->Write(&len, sizeof(len));
	stream->Write(procedures.c_str(), len);

	// Записываем данные
    TLoading* load = new TLoading(Application->MainForm);
	load->setDuration(traces);
	load->Show();
	for (auto& vec : data) {
		stream->Write(vec.data(), vec.size() * sizeof(float));
        load->update(1);
	}
}
//------------------------------------------------------------------------------
SeismicData SeismicData::loadFile(const std::wstring& loc) {
	std::unique_ptr<TFileStream> stream(new TFileStream (loc.c_str(), fmOpenRead));
	float dT;
	int samples;
	int traces;
	// Читаем параметры
	stream->Read(&dT, sizeof(dT));
	stream->Read(&samples, sizeof(samples));
	stream->Read(&traces, sizeof(traces));


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

	std::vector<std::vector<float>> data;
	// Читаем данные
	data.resize(traces);
	TLoading* load = new TLoading(Application->MainForm);
	load->setDuration(traces);
	load->Show();
	for (auto& vec : data) {
		vec.resize(samples);
		stream->Read(vec.data(), samples * sizeof(float));
		load->update(1);
	}
	return SeismicData(data, dT, DataType::BASE, name, procedures);
}
