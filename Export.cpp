//---------------------------------------------------------------------------

#pragma hdrstop

#include "Export.h"
#include "LegacyDataExports.h"
#include "Reader.h"
#include "SeismicData.h"
#include "Dialog.h"
#include "DataCalculator.h"
#include "RgbData.h"
#include "Structures.h"
#include "UnitFormUniversal.h"
#include "ChooseForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
bool __cdecl setTraceCallback(TForm* f, std::function<void(int)> cb) {
    auto* form = dynamic_cast<TFormUniversal*>(f);
    if (!form || !form->windowContainer) return false;
    form->setTraceCallback(std::move(cb));
    return true;
}
bool __cdecl setDisplayCallback(TForm* f, std::function<void(int, int, std::string)> cb) {
    auto* form = dynamic_cast<TFormUniversal*>(f);
    if (!form || !form->windowContainer) return false;
    form->setDisplayCallback(std::move(cb));
    return true;
}
bool __cdecl setHorizonsCallback(TForm* f, RgbHorizonsCallback callback, void* context) {
    auto* form = dynamic_cast<TFormUniversal*>(f);
    if (!form || !form->windowContainer) return false;
    if (!callback) {
        return form->setHorizonsCallback({});
    }
    return form->setHorizonsCallback([callback, context](const std::vector<Horizon>& list) {
        RgbHorizons snapshot;
        snapshot.reserve(list.size());
        for (const auto& horizon : list) {
            snapshot.emplace_back(horizon.name, horizon.points);
        }
        callback(snapshot, context);
    });
}
bool __cdecl setHorizons(TForm* f, const RgbHorizons& input) {
    auto* form = dynamic_cast<TFormUniversal*>(f);
    if (!form || !form->windowContainer) return false;
    std::vector<Horizon> converted;
    converted.reserve(input.size());
    for (const auto& item : input) {
        Horizon horizon;
        horizon.name = item.first;
        horizon.points = item.second;
        converted.push_back(std::move(horizon));
    }
    return form->setHorizons(converted);
}
bool __cdecl setCrosses(TForm* f, const RgbCrosses& input) {
    auto* form = dynamic_cast<TFormUniversal*>(f);
    if (!form || !form->windowContainer) return false;
    std::vector<Cross> converted;
    converted.reserve(input.size());
    for (const auto& item : input) {
        Cross cross;
        cross.name = item.first;
        cross.x = item.second;
        converted.push_back(std::move(cross));
    }
    return form->setCrosses(converted);
}
void init(){
ShowMessage("Init");
return;
}
SeismicData* readDataIBM(std::wstring filePath){
	try {
		Traces tr = readIBM(filePath);
		SeismicData* sd = new SeismicData(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
		return sd;
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
int getMaxTraceS(SeismicData* data){
return data->getSize().x;
}

TForm* __cdecl CreateRgbBlendingForm(System::UnicodeString path, std::function<void(int)> callback, std::function<void(int, int, std::string)> callbackDisplay){
	try {
		TChoose* choose = new TChoose(Application->MainForm);
		choose->SetRGBPath(path);
        if (choose->Execute() && choose->execType!=0){
            TFormUniversal* form = new TFormUniversal(Application->MainForm, path);
			setTraceCallback(form, callback);
            setDisplayCallback(form, callbackDisplay);
            switch (choose->execType) {
            case 1:
            	{
                Traces tr = readIBM((path+".ibm").c_str());
				std::shared_ptr<SeismicData> sd = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
                form -> initFromData(sd);
                break;
                }
            case 2:
            	{
            	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
                dialog->AddInput<float>("Начальная частота", stnFreqStart);
                dialog->AddInput<float>("Конечная частота", stnFreqStop);
                dialog->AddInput<int>("Число фильтров", stnNumFilters);
                dialog->AddInput<int>("Ширина фильтра", stnFilterWindow);
                dialog->AddInput<int>("Ширина окна сглаживания", stnTWindow);
                if (dialog->Execute() && dialog->ContinuePressed) {
                    Traces tr = readIBM((path+".ibm").c_str());
					std::shared_ptr<SeismicData> sd = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
                    std::shared_ptr<IBaseData> newData = std::make_shared<RgbData>(sd.get(), stnFreqStart, stnFreqStop, stnNumFilters, stnFilterWindow);
                    newData = DataCalculator::smoothT(newData, stnTWindow);
                    form -> initFromData(newData);
                }
                delete dialog;
                break;
                }
            case 3:
                {
                std::shared_ptr<IBaseData> newData = std::make_shared<RgbData>(RgbData::loadFile(choose->getChosen().c_str()));
                form -> initFromData(newData);
                break;
                }
            default:
                ;
            }
            choose->Free();
            return form;
        };
        choose->Free();
        return nullptr;

	} catch (...) {
		throw "RGBBlending: UNKNOWN DATA ERROR";
	}
}

TForm* __cdecl CreateRgbBlendingFormCube(System::UnicodeString path){
	try {
		TChoose* choose = new TChoose(Application->MainForm);
		choose->SetRGBPath(path);
        if (choose->Execute() && choose->execType!=0){
            TFormUniversal* form = new TFormUniversal(Application->MainForm, path);
            switch (choose->execType) {
            case 1:
                {
                form -> initForCube(path+L".ibm");
                break;
                }
            case 2:
            	{
            	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
                dialog->AddInput<float>("Начальная частота", stnFreqStart);
                dialog->AddInput<float>("Конечная частота", stnFreqStop);
                dialog->AddInput<int>("Число фильтров", stnNumFilters);
                dialog->AddInput<int>("Ширина фильтра", stnFilterWindow);
                if (dialog->Execute() && dialog->ContinuePressed) {
                    calculateCube(path.c_str(), stnNumFilters, stnFreqStart, stnFreqStop, stnFilterWindow);
                }
                break;
                }
            case 3:
                {
                if(MessageDlg("Вы хотите конвертировать файл в slice-cube?",
                   mtConfirmation,          // Тип: вопрос, предупреждение, ошибка...
                   TMsgDlgButtons() << mbYes << mbNo, // Кнопки: Да и Нет
                   0) == mrYes) {
                        convertToSliceOrder((path+L"\\"+ExtractFileName(choose->getChosen())).c_str());
                   }
                form -> initForCube(choose->getChosen());
                break;
                }
            default:
                ;
            }
            return form;
        };
        choose->Free();

        return nullptr;

	} catch (...) {
		throw "RGBBlending: UNKNOWN DATA ERROR";
	}
}
