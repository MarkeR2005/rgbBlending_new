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
#include "UnitFormVolume.h"
#include <memory>
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
bool __cdecl setHorizons(TForm* f, const RgbHorizons& input, const RgbHorizonColors& colors) {
    auto* form = dynamic_cast<TFormUniversal*>(f);
    if (!form || !form->windowContainer) return false;
    std::vector<Horizon> converted;
    converted.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        const auto& item = input[i];
        Horizon horizon;
        horizon.name = item.first;
        horizon.points = item.second;
        if (i < colors.size()) {
            const std::uint32_t color = colors[i];
            horizon.useColor = true;
            horizon.red = static_cast<unsigned char>((color >> 16) & 0xFF);
            horizon.green = static_cast<unsigned char>((color >> 8) & 0xFF);
            horizon.blue = static_cast<unsigned char>(color & 0xFF);
        }
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
		std::unique_ptr<TChoose> choose(new TChoose(Application->MainForm));
		choose->SetRGBPath(path);
        if (choose->Execute() && choose->execType!=0){
            std::unique_ptr<TFormUniversal> form(new TFormUniversal(Application->MainForm, path));
			setTraceCallback(form.get(), callback);
            setDisplayCallback(form.get(), callbackDisplay);
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
                } else {
                    delete dialog;
                    return nullptr;
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
                return nullptr;
            }
            return form.release();
        };
        return nullptr;

	} catch (...) {
		throw "RGBBlending: UNKNOWN DATA ERROR";
	}
}

TForm* __cdecl CreateRgbBlendingFormCube(System::UnicodeString path){
    try {
        std::unique_ptr<TChoose> choose(new TChoose(Application->MainForm));
        choose->SetRGBPath(path);
        if (!choose->Execute() || choose->execType == 0) return nullptr;

        System::UnicodeString cubeFile;
        switch (choose->execType) {
        case 1:
            cubeFile = path + L".ibm";
            break;
        case 2: {
            std::unique_ptr<TAbstractDialog> dialog(new TAbstractDialog(nullptr));
            dialog->AddInput<float>("Start frequency", stnFreqStart);
            dialog->AddInput<float>("Stop frequency", stnFreqStop);
            dialog->AddInput<int>("Number of filters", stnNumFilters);
            dialog->AddInput<int>("Filter window", stnFilterWindow);
            if (!dialog->Execute() || !dialog->ContinuePressed) return nullptr;
            calculateCube(path.c_str(), stnNumFilters, stnFreqStart,
                stnFreqStop, stnFilterWindow);
            cubeFile = path + L"\\" + IntToStr(stnNumFilters) + L"_" +
                IntToStr(static_cast<int>(stnFreqStart)) + L"_" +
                IntToStr(static_cast<int>(stnFreqStop)) + L".irgb";
            break;
        }
        case 3:
            cubeFile = choose->getChosen();
            if (MessageDlg("Convert this cube to slice order?", mtConfirmation,
                TMsgDlgButtons() << mbYes << mbNo, 0) == mrYes)
                convertToSliceOrder((path + L"\\" + ExtractFileName(cubeFile)).c_str());
            break;
        default:
            return nullptr;
        }
        if (!FileExists(cubeFile)) return nullptr;
        std::unique_ptr<TFormVolume> form(new TFormVolume(Application->MainForm, path));
        form->initForCube(cubeFile);
        return form.release();
    } catch (...) {
        throw "RGBBlending: UNKNOWN DATA ERROR";
    }
}
