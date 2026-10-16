//---------------------------------------------------------------------------

#include "UnitFormUniversal.h"
#include "ColorManager.h"
#include "Dialog.h"
#include "UnitStatistics.h"

#pragma hdrstop

#include "SeismicPanel.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

__fastcall TSeismicPanel::TSeismicPanel(TComponent* Owner): TWindowPanel(Owner)
{

	SmoothX = new TMenuItem(FPopupMenu);
	SmoothX->Caption = "Smooth X";
	FPopupMenu->Items->Add(SmoothX);
	SmoothX->OnClick = applySmoothingX;

	SmoothT = new TMenuItem(FPopupMenu);
	SmoothT->Caption = "Smooth T";
	FPopupMenu->Items->Add(SmoothT);
	SmoothT->OnClick = applySmoothingT;

	GetTrace = new TMenuItem(FPopupMenu);
	GetTrace->Caption = "Get Trace";
	FPopupMenu->Items->Add(GetTrace);
	GetTrace->OnClick = getTraceF;

	GetSwan = new TMenuItem(FPopupMenu);
	GetSwan->Caption = "Get SWAN";
	FPopupMenu->Items->Add(GetSwan);
	GetSwan->OnClick = getSwanF;

	GetSwanRGB = new TMenuItem(FPopupMenu);
	GetSwanRGB->Caption = "Get SWAN as RGB Trace";
	FPopupMenu->Items->Add(GetSwanRGB);
	GetSwanRGB->OnClick = getSwanRGBF;

	ToEnergy = new TMenuItem(FPopupMenu);
	ToEnergy->Caption = "To Energy";
	FPopupMenu->Items->Add(ToEnergy);
	ToEnergy->OnClick = toEnergyF;

	ToRgb = new TMenuItem(FPopupMenu);
	ToRgb->Caption = "To RGB";
	FPopupMenu->Items->Add(ToRgb);
	ToRgb->OnClick = toRgbF;

	GetStat = new TMenuItem(FPopupMenu);
	GetStat->Caption = "Get spectrum";
	FPopupMenu->Items->Add(GetStat);
	GetStat->OnClick = GetStatF;


	this->PopupMenu = FPopupMenu;
}
__fastcall TSeismicPanel::~TSeismicPanel()
{
};
void TSeismicPanel::setData(std::shared_ptr<SeismicData> data){
	type = PanelType::SEIS;
	container = data;
	NameInfo->Caption = container->getName().c_str();
	setWindow(std::make_unique<SeismicWindow>());
	getWindow()->initPaletteTexture(PALETTE);
	getWindow()->setDT(data->getDT()/1000.0f);
	bitMap bm = getContainer()->getTexture();
	getWindow()->initIndexTexture(std::move(bm));
	if (data->getSize().x == 1) {
        window->isThin = true;
	}
	if (!data->getFreq().empty()) {
		frequens = data->getFreq();
		setType(PanelData::SWAN);
        window->isThin = true;
	}
	Resize(this);
	window->renderWindow();
}

void __fastcall TSeismicPanel::applySmoothingX(TObject* Sender){

	int window = 11;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
	dialog->AddInput<int>("Окно (отсчётов)", window);
	if (dialog->Execute() && dialog->ContinuePressed) {
		std::shared_ptr<SeismicData> other = std::make_shared<SeismicData>(*getContainer(), true);
		other->smoothX(window);
		TFormUniversal* form = new TFormUniversal(Application->MainForm);
		form -> Show();
		form ->CreateForm(other);
	}
	delete dialog;
}

void __fastcall TSeismicPanel::applySmoothingT(TObject* Sender){

	int window = 11;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
	dialog->AddInput<int>("Окно (отсчётов)", window);
	if (dialog->Execute() && dialog->ContinuePressed) {
		std::shared_ptr<SeismicData> other = std::make_shared<SeismicData>(*getContainer(), true);
		other->smoothT(window);
		TFormUniversal* form = new TFormUniversal(Application->MainForm);
		form -> Show();
		form ->CreateForm(other);
	}
	delete dialog;
}

void __fastcall TSeismicPanel::getTraceF(TObject* Sender){
	getWindow()->highlighted_points.push_back({posx, -1});
	getWindow()->renderWindow();
	std::shared_ptr<SeismicData> other = std::make_shared<SeismicData>(getContainer()->getTrace(posx), true);
	TFormUniversal* form = new TFormUniversal(Application->MainForm);
	form -> Show();
	form ->Width = 200;
	form ->CreateForm(other);
}
void __fastcall TSeismicPanel::getSwanF(TObject* Sender){
	float startFreq = 5.0f;
	float finishFreq = 100.0f;
	int filters = 100;
	int window = 100;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
	dialog->AddInput<float>("Начальная частота", startFreq);
	dialog->AddInput<float>("Конечная частота", finishFreq);
	dialog->AddInput<int>("Количество фильтров", filters);
	dialog->AddInput<int>("Ширина фильтра", window);
	if (dialog->Execute() && dialog->ContinuePressed) {
		getWindow()->highlighted_points.push_back({posx, -1});
		getWindow()->renderWindow();
		std::shared_ptr<SeismicData> other = std::make_shared<SeismicData>(getContainer()->getSwan(posx, startFreq, finishFreq, filters, window), true);
		TFormUniversal* form = new TFormUniversal(Application->MainForm);
		form -> Show();
		form ->Width = 200;
		form ->CreateForm(other);
	}
	delete dialog;
}
void __fastcall TSeismicPanel::getSwanRGBF(TObject* Sender){
		float startFreq = 5.0f;
		float finishFreq = 100.0f;
		int filters = 100;
		int window = 100;
		int width = 11;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<float>("Начальная частота", startFreq);
		dialog->AddInput<float>("Конечная частота", finishFreq);
		dialog->AddInput<int>("Количество фильтров", filters);
		dialog->AddInput<int>("Ширина фильтра", window);
		dialog->AddInput<int>("Ширина окна", width);
	if (dialog->Execute() && dialog->ContinuePressed) {
		getWindow()->highlighted_points.push_back({posx, -1});
		getWindow()->renderWindow();
		std::shared_ptr<RgbData> other = std::make_shared<RgbData>(getContainer()->getSwanRGB(posx, startFreq, finishFreq, filters, window), true);
		TFormUniversal* form = new TFormUniversal(Application->MainForm);
		form -> Show();
		form ->Width = 200;
		form ->CreateForm(other);
	}
	delete dialog;
}
void __fastcall TSeismicPanel::toEnergyF(TObject* Sender){
	std::shared_ptr<SeismicData> other = std::make_shared<SeismicData>(*getContainer(), false);
	other->toEnergy();
	TFormUniversal* form = new TFormUniversal(Application->MainForm);
	form -> Show();
	form ->CreateForm(other);
}
void __fastcall TSeismicPanel::toRgbF(TObject* Sender){
	if (dataType == PanelData::SWAN) {
		int width = 11;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Ширина окна", width);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<SeismicData> data_ = std::make_shared<SeismicData>(*getContainer(), false);
			if (!isEnergy) {
				data_->toEnergy();
			}
			std::vector<std::vector<float>> swanData = data_->getRawData();
			std::vector<std::vector<std::vector<float>>> retData(swanData.size(), std::vector<std::vector<float>>(1, std::vector<float>(data_->getSize().t,0)));
			for (int i = 0; i < swanData.size(); i++) {
				retData[i] = {swanData[i]};
			}
			std::shared_ptr<RgbData> other = std::make_shared<RgbData>(retData, data_->getDT(), frequens[0], frequens[swanData.size()-1], data_->getName()+"_rgb", data_->getProcedures());
			other->smoothT(width);
			TFormUniversal* form = new TFormUniversal(Application->MainForm);
			form -> Show();
			form ->CreateForm(other);
		}
		delete dialog;
	}
	else {
		float startFreq = 5.0f;
		float finishFreq = 100.0f;
		int filters = 100;
		int window = 100;
		int width = 11;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<float>("Начальная частота", startFreq);
		dialog->AddInput<float>("Конечная частота", finishFreq);
		dialog->AddInput<int>("Количество фильтров", filters);
		dialog->AddInput<int>("Ширина фильтра", window);
		dialog->AddInput<int>("Ширина окна", width);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<RgbData> other = std::make_shared<RgbData>(*getContainer(), startFreq, finishFreq, filters, window);
			other->smoothT(width);
			TFormUniversal* form = new TFormUniversal(Application->MainForm);
			form -> Show();
			form ->CreateForm(other);
		}
		delete dialog;
	}
}
void __fastcall TSeismicPanel::GetStatF(TObject* Sender){
			std::vector<point2F> points;
	if (dataType == PanelData::SWAN) {
		getWindow()->highlighted_points.push_back({-1, posy});
		getWindow()->renderWindow();
		for (int i = 0; i < getContainer()->getSize().x; ++i){
			points.push_back({frequens[i], getContainer()->getRawData()[i][posy]});
		}
	}
	else {
        float startFreq = 5.0f;
		float finishFreq = 100.0f;
		int filters = 100;
		int window = 100;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<float>("Начальная частота", startFreq);
		dialog->AddInput<float>("Конечная частота", finishFreq);
		dialog->AddInput<int>("Количество фильтров", filters);
		dialog->AddInput<int>("Ширина фильтра", window);
		if (dialog->Execute() && dialog->ContinuePressed) {
			getWindow()->highlighted_points.push_back({posx, posy});
			getWindow()->renderWindow();
			std::shared_ptr<SeismicData> other = std::make_shared<SeismicData>(getContainer()->getSwan(posx, startFreq, finishFreq, filters, window), true);
			for (int i = 0; i < other->getSize().x; ++i){
				points.push_back({other->getFreq()[i], other->getRawData()[i][posy]});
			}
		}
		else{
			delete dialog;
			return;
        }
		delete dialog;
	}
	TUnitStatistic* form = new TUnitStatistic(this);
	form -> Show();
	form -> Height = 200;
    form-> Width = 300;
	form ->SetData(points);
}
