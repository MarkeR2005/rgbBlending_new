//---------------------------------------------------------------------------

#pragma hdrstop

#include "Export.h"
#include "Reader.h"
#include "SeismicData.h"
#include "Dialog.h"
#include "DataCalculator.h"
#include "RgbData.h"
#include "Structures.h"
#include "UnitFormUniversal.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

SeismicData* readDataIBM(std::wstring filePath){
	try {
		Traces tr = readIBM(filePath);
		SeismicData* sd = new SeismicData(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
		return sd;
	} catch (...) {
		throw "RGBBlending: FILE ERROR";
	}
}
//Reads data from internal formats
RgbData* readDataInternalFormat(std::wstring filePath, bool isRgb){
	try {
		if (isRgb) {
			RgbData tmp = RgbData::loadFile(filePath);
			RgbData* rd = &tmp;
			return rd;
		}
//		else {
//			SeismicData tmp = SeismicData::loadFile(filePath);
//			SeismicData* sd = &tmp;
//			return sd;
//        }
	} catch (...) {
		throw "RGBBlending: FILE ERROR";
	}
}
//Reads data from IBM cube
//Types 1 - inline | 2 - crossline | 3 - slice
SeismicData* readDataIBMCube(std::wstring filePath, int type, int pos){
	if (type < 1 || type > 3) {
		throw "RGBBlending: CUBE TYPE ERROR";
	}
	try {
		Traces tr;
		switch (type) {
			case 1:
				tr = readInlineRegular(filePath, pos);
				break;
			case 2:
				tr = readCrosslineRegular(filePath, pos);
				break;
			case 3:
				tr = readTimeSliceRegular(filePath, pos);
				break;
			default:
				;
		}
		SeismicData* sd = new SeismicData(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
		return sd;
	} catch (...) {
		throw "RGBBlending: CUBE ERROR";
	}
}

TFormUniversal* CreateRgbBlendingFormSD(SeismicData* data, System::UnicodeString path){
	try {

		TFormUniversal* form = new TFormUniversal(Application->MainForm, path);
        TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<float>("Начальная частота", stnFreqStart);
		dialog->AddInput<float>("Конечная частота", stnFreqStop);
		dialog->AddInput<int>("Число фильтров", stnNumFilters);
		dialog->AddInput<int>("Ширина фильтра", stnFilterWindow);
		dialog->AddInput<int>("Ширина окна сглаживания", stnTWindow);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<IBaseData> newData = std::make_shared<RgbData>(data, stnFreqStart, stnFreqStop, stnNumFilters, stnFilterWindow);
			newData = DataCalculator::smoothT(newData, stnTWindow);
			form -> initFromData(newData);
		}
		else {
			form -> initFromData(std::make_shared<SeismicData>(*data));
		}
		delete dialog;
		return form;

	} catch (...) {
		throw "RGBBlending: UNKNOWN DATA ERROR";
	}
}
TFormUniversal* CreateRgbBlendingFormRGB(RgbData* data, System::UnicodeString path){
	try {
		TFormUniversal* form = new TFormUniversal(Application->MainForm, path);
		form -> initFromData(std::make_shared<RgbData>(*data));
		return form;
	} catch (...) {
		throw "RGBBlending: UNKNOWN DATA ERROR";
	}
}
void ShowForm(TFormUniversal* form){
form -> Show();
}
