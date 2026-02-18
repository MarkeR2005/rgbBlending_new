//---------------------------------------------------------------------------
#include "UnitFormUniversal.h"
#include "UnitStatistics.h"

#pragma hdrstop
#include "Dialog.h"

#include "RgbPanel.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
 __fastcall TRgbPanel::TRgbPanel(TComponent* Owner): TWindowPanel(Owner)
{
    isEnergy = true;
	SmoothX = new TMenuItem(FPopupMenu);
	SmoothX->Caption = "Smooth X";
	FPopupMenu->Items->Add(SmoothX);
	SmoothX->OnClick = applySmoothingX;

	SmoothT = new TMenuItem(FPopupMenu);
	SmoothT->Caption = "Smooth T";
	FPopupMenu->Items->Add(SmoothT);
	SmoothT->OnClick = applySmoothingT;

	SmoothF = new TMenuItem(FPopupMenu);
	SmoothF->Caption = "Smooth F";
	FPopupMenu->Items->Add(SmoothF);
	SmoothF->OnClick = applySmoothingF;

	GetSwan = new TMenuItem(FPopupMenu);
	GetSwan->Caption = "Get Trace as Swan";
	FPopupMenu->Items->Add(GetSwan);
	GetSwan->OnClick = getSwanF;

	GetTrace = new TMenuItem(FPopupMenu);
	GetTrace->Caption = "Get Trace";
	FPopupMenu->Items->Add(GetTrace);
	GetTrace->OnClick = getTraceF;

	GetFilter = new TMenuItem(FPopupMenu);
	GetFilter->Caption = "Get Filter";
	FPopupMenu->Items->Add(GetFilter);
	GetFilter->OnClick = getFilterF;

	GetStat = new TMenuItem(FPopupMenu);
	GetStat->Caption = "Get spectrum";
	FPopupMenu->Items->Add(GetStat);
	GetStat->OnClick = GetStatF;

	Mute = new TMenuItem(FPopupMenu);
	Mute->Caption = "Mute";
	FPopupMenu->Items->Add(Mute);
	Mute->OnClick = MuteF;

	this->PopupMenu = FPopupMenu;
}
__fastcall TRgbPanel::~TRgbPanel()
{
};
void TRgbPanel::setData(std::shared_ptr<RgbData> data){
	type = PanelType::RGB;
	container = data;
	NameInfo->Caption = container->getName().c_str();
	setWindow(std::make_unique<RgbWindow>());
	getWindow()->setDT(data->getDT()/1000.0f);
	std::vector<bitMap> bm = getContainer()->getTexture();
	getWindow()->initTexture(bm);
	if (data->getSize().x < 150) {
		window->isThin = true;
	}
	if (data->getSize().x == 1) {
		dataType = PanelData::TRACE;
	}
    frequens = data->getFreqs();
	getWindow()->setColor(0, frequens.size()/3, frequens.size()/3*2);
	getWindow()->setView(true, true, true, false);
	Resize(this);
	window->renderWindow();
}

void __fastcall TRgbPanel::CheckBoxClick(bool r, bool g, bool b){
		getWindow()->setView(r, g, b, false);
		window->renderWindow();
}

point3F __fastcall TRgbPanel::ScrollBarChange(int r, int g, int b){
		getWindow()->setColor(r, g, b);
		window->renderWindow();
        return {frequens[r], frequens[g], frequens[b]};
}

void __fastcall TRgbPanel::applySmoothingT(TObject* Sender){
	int window = 11;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
	dialog->AddInput<int>("Окно (отсчётов)", window);
	if (dialog->Execute() && dialog->ContinuePressed) {
		std::shared_ptr<RgbData> other = std::make_shared<RgbData>(*getContainer(), true);
		other->smoothT(window);
		TFormUniversal* form = new TFormUniversal(Application->MainForm);
		form -> Show();
		form ->CreateForm(other);
	}
	delete dialog;
}
void __fastcall TRgbPanel::applySmoothingX(TObject* Sender){
	int window = 11;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
	dialog->AddInput<int>("Окно (отсчётов)", window);
	if (dialog->Execute() && dialog->ContinuePressed) {
		std::shared_ptr<RgbData> other = std::make_shared<RgbData>(*getContainer(), true);
		other->smoothX(window);
		TFormUniversal* form = new TFormUniversal(Application->MainForm);
		form -> Show();
		form ->CreateForm(other);
	}
	delete dialog;
}
void __fastcall TRgbPanel::applySmoothingF(TObject* Sender){
	int window = 11;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
	dialog->AddInput<int>("Окно (отсчётов)", window);
	if (dialog->Execute() && dialog->ContinuePressed) {
		std::shared_ptr<RgbData> other = std::make_shared<RgbData>(*getContainer(), true);
		other->smoothF(window);
		TFormUniversal* form = new TFormUniversal(Application->MainForm);
		form -> Show();
		form ->CreateForm(other);
	}
	delete dialog;
}

void __fastcall TRgbPanel::getTraceF(TObject* Sender){
	getWindow()->highlighted_points.push_back({posx, -1});
	getWindow()->renderWindow();
	std::shared_ptr<RgbData> other = std::make_shared<RgbData>(getContainer()->getTrace(posx), true);
	TFormUniversal* form = new TFormUniversal(Application->MainForm);
	form -> Show();
	form ->Width = 200;
	form -> CreateForm(other);

}
void __fastcall TRgbPanel::getSwanF(TObject* Sender){
	getWindow()->highlighted_points.push_back({posx, -1});
	getWindow()->renderWindow();
	std::shared_ptr<SeismicData> other = std::make_shared<SeismicData>(getContainer()->getTraceSW(posx), true);
	TFormUniversal* form = new TFormUniversal(Application->MainForm);
	form -> Show();
	form ->Width = 200;
	form ->CreateForm(other);
}
void __fastcall TRgbPanel::getFilterF(TObject* Sender){
	if (dataType == PanelData::TRACE) {
		std::shared_ptr<SeismicData> other = std::make_shared<SeismicData>(getContainer()->getTraceSW(0), true);
		TFormUniversal* form = new TFormUniversal(Application->MainForm);
		form -> Show();
			form ->Width = 200;
		form -> CreateForm(other);
	}
	else {
		int num = 0;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Номер фильтра", num);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<SeismicData> other = std::make_shared<SeismicData>(getContainer()->getFilter(num), true);
			TFormUniversal* form = new TFormUniversal(Application->MainForm);
			form -> Show();
			form -> CreateForm(other);
		}
		delete dialog;
	}
}
void __fastcall TRgbPanel::GetStatF(TObject* Sender){
	std::vector<point2F> points;
	getWindow()->highlighted_points.push_back({posx, posy});
	getWindow()->renderWindow();
	for (int i = 0; i < frequens.size(); ++i){
		points.push_back({frequens[i], getContainer()->getRawData()[i][posx][posy]});
	}
	TUnitStatistic* form = new TUnitStatistic(this);
	form -> Show();
	form -> Height = 200;
    form-> Width = 300;
	form ->SetData(points);
}
void __fastcall TRgbPanel::MuteF(TObject* Sender){
	int start = 0;
	int stop = -1;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
	dialog->AddInput<int>("От", start);
	dialog->AddInput<int>("До", stop);
	if (dialog->Execute() && dialog->ContinuePressed) {
    	float dT = window->getDT();
		std::shared_ptr<RgbData> other = std::make_shared<RgbData>(getContainer()->getInterval(start/dT, stop/dT), false);
		std::shared_ptr<ColorManager> cm = std::make_shared<ColorManager>(99.9, true);
        other->setCM(cm);
	TFormUniversal* form = new TFormUniversal(Application->MainForm);
		form -> Show();
		form ->Width = 200;
		form ->CreateForm(other);
	}
    delete dialog;
}
