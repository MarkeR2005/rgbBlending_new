//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitFormUniversal.h"
#include "Reader.h"
#include "DataMath.h"
#include "Dialog.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormUniversal *FormUniversal;
//---------------------------------------------------------------------------
__fastcall TFormUniversal::TFormUniversal(TComponent* Owner, System::UnicodeString path_)
	: TForm(Owner), path(path_)
{
	SaveDialog1->Title = "Save your data with internal format";
	SaveDialog1->Filter = "Seismic files (*.sd)|*.sd|RGB files (*.rgb)|*.rgb";
	SaveDialog1->DefaultExt = "sd";
	SaveDialog1->FileName = "MyData.sd";

	SaveDialog2->Title = "Save your horizon";
	SaveDialog2->Filter = "Text files (*.txt)|*.txt|Table files (*.csv)|*.csv|Binary files (*.dat)|*.dat";
	SaveDialog2->DefaultExt = "txt";
	SaveDialog2->FileName = "MyHorizon.txt";


	windowContainer = new TWindowContainer(this);
	windowContainer->Parent = this;
	windowContainer->Align = alClient;
	OnClose=FormClose;
}
//---------------------------------------------------------------------------
void __fastcall TFormUniversal::FormClose(TObject *Sender, TCloseAction &Action)
{
Free();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TFormUniversal::OpenAsClick(TObject *Sender)
{
//	if (OpenDialog1->Execute())
//	{
//		std::wstring fileName = OpenDialog1->FileName.c_str();
//		std::wstring fileExt = ExtractFileExt(OpenDialog1->FileName).c_str();
//		//this->Caption = fileName.c_str();
//		bool success = false;
//		if (fileExt == L".sd") {
//			std::shared_ptr<SeismicData> seismicData;
//			try {
//				seismicData = std::make_shared<SeismicData>(SeismicData::loadFile(fileName));
//				success = true;
//			} catch (...) {
//			}
//			if (success) {
//				initFromData(seismicData);
//			}
//		}
//		else if  (fileExt == L".sgy") {
//			std::shared_ptr<SeismicData> seismicData;
//			try {
//				Traces tr = readSEGY(fileName);
//				success = true;
//				seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
//			} catch (...) {
//			}
//			if (success) {
//				initFromData(seismicData);
//			}
//		}
//		else if  (fileExt == L".rgb") {
//			std::shared_ptr<RgbData> rgbData;
//			try {
//				rgbData = std::make_shared<RgbData>(RgbData::loadFile(fileName));
//				success = true;
//			} catch (...) {
//			}
//			if (success) {
//				initFromData(rgbData);
//			}
//		}
//		else if  (fileExt == L".ibm") {
//		std::shared_ptr<SeismicData> seismicData;
//			try {
//				Traces tr = readIBM(fileName);
//				success = true;
//				seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
//			} catch (...) {
//			}
//			if (success) {
//				initFromData(seismicData);
//			}
//		}
//		else if  (fileExt == L".irgb") {
//        windowContainer->initialize();
//		windowContainer->setFile(fileName);
//		}
//		else ShowMessage("Wrong extension");
//	}
}
//---------------------------------------------------------------------------
void __fastcall TFormUniversal::SaveAsF(TObject *Sender)
{
	std::string name;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
    dialog->AddInput<std::string>("Название", name);
    if (dialog->Execute() && dialog->ContinuePressed) {
    	windowContainer->saveFile((path+("\\"+name).c_str()).c_str());
    }
    delete dialog;

//	if (panel1==nullptr) {
//		return;
//	}
//	if (SaveDialog1->Execute())
//	{
//		std::wstring fileName = SaveDialog1->FileName.c_str();
//		std::wstring fileExt = ExtractFileExt(SaveDialog1->FileName).c_str();
//		if (fileExt == L".sd") {
//			SeismicData* seismicData;
//			bool success = false;
//			try {
//				seismicData = static_cast<TSeismicPanel*>(panel1)->getContainer();
//				success = true;
//			} catch (...) {
//				ShowMessage("Wrong extension/data");
//			}
//			if (success) {
//				seismicData->saveFile(SaveDialog1->FileName.c_str());
//			}
//		}
//		else if (fileExt == L".rgb") {
//			RgbData* rgbData;
//			bool success = false;
//			try {
//				rgbData = static_cast<TRgbPanel*>(panel1)->getContainer();
//				success = true;
//			} catch (...) {
//				ShowMessage("Wrong extension/data");
//			}
//			if (success) {
//				rgbData->saveFile(SaveDialog1->FileName.c_str());
//			}
//		}
//		else ShowMessage("Wrong extension");
//	}
}
//---------------------------------------------------------------------------
void __fastcall TFormUniversal::SaveAsHorF(TObject *Sender)
{
//	if (panel1==nullptr) {
//		return;
//	}
//	if (SaveDialog2->Execute())
//	{
//		std::wstring fileName = SaveDialog2->FileName.c_str();
//		std::wstring fileExt = ExtractFileExt(SaveDialog2->FileName).c_str();
//		try {
//			panel1->SaveHorizon(fileName, fileExt);
//		} catch (...) {
//			ShowMessage("Bad Horizon");
//		}
//	}
}
//---
void __fastcall TFormUniversal::OpenAsHorF(TObject *Sender)
{
//	if (panel1==nullptr) {
//		return;
//	}
//	if (OpenDialog1->Execute())
//	{
//		std::wstring fileName = OpenDialog1->FileName.c_str();
//		std::wstring fileExt = ExtractFileExt(OpenDialog1->FileName).c_str();
//		try {
//			panel1->LoadHorizon(fileName, fileExt);
//		} catch (...) {
//			ShowMessage("Bad Horizon");
//		}
//	}
}


void __fastcall TFormUniversal::ScrollBar1Change(TObject *Sender)
{
	if (windowContainer==nullptr) {
		return;
	}
	viewParams par = {
		CheckBoxR->Checked,CheckBoxG->Checked,CheckBoxB->Checked,
		ScrollBarR->Position,ScrollBarG->Position,ScrollBarB->Position,
		10.0f/(ScrollBar1->Position+1)
	};
	windowContainer->setViewParams(par);
}
//---------------------------------------------------------------------------

void __fastcall TFormUniversal::ContrastBarClick(TObject *Sender)
{
	ContrastBar->Checked = !ContrastBar->Checked;
	Panel1->Enabled = ContrastBar->Checked;
	 Panel1->Visible = ContrastBar->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TFormUniversal::ControlPanelClick(TObject *Sender)
{
	ControlPanel->Checked = !ControlPanel->Checked;
	Panel2->Enabled = ControlPanel->Checked;
	Panel2->Visible = ControlPanel->Checked;
}
//---------------------------------------------------------------------------



void __fastcall TFormUniversal::CheckBoxBClick(TObject *Sender)
{
	if (windowContainer==nullptr) {
		return;
	}
	viewParams par = {
		CheckBoxR->Checked,CheckBoxG->Checked,CheckBoxB->Checked,
		ScrollBarR->Position,ScrollBarG->Position,ScrollBarB->Position,
		10.0f/(ScrollBar1->Position+1)
	};
	windowContainer->setViewParams(par);
}
//---------------------------------------------------------------------------

void __fastcall TFormUniversal::ScrollBarRChange(TObject *Sender)
{
	if (windowContainer==nullptr) {
		return;
	}
	viewParams par = {
		CheckBoxR->Checked,CheckBoxG->Checked,CheckBoxB->Checked,
		ScrollBarR->Position,ScrollBarG->Position,ScrollBarB->Position,
		10.0f/(ScrollBar1->Position+1)
	};
	windowContainer->setViewParams(par);
//	if (panel1==nullptr || panel1->getType()!=PanelType::RGB) {
//		return;
//	}
//	point3F res = static_cast<TRgbPanel*>(panel1)->ScrollBarChange(ScrollBarR->Position,ScrollBarG->Position,ScrollBarB->Position);
	LabelR->Caption = (std::to_string(windowContainer->getFreqByIndex(ScrollBarR->Position)) + " hz").c_str();
	LabelG->Caption = (std::to_string(windowContainer->getFreqByIndex(ScrollBarG->Position)) + " hz").c_str();
	LabelB->Caption = (std::to_string(windowContainer->getFreqByIndex(ScrollBarB->Position)) + " hz").c_str();
}
//---------------------------------------------------------------------------

void __fastcall TFormUniversal::LeftAxeClick(TObject *Sender)
{
//	if (panel1 == nullptr) {
//		return;
//	}
//	LeftAxe->Checked = !LeftAxe->Checked;
//	panel1->image1->Visible = LeftAxe->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TFormUniversal::TopAxeClick(TObject *Sender)
{
//	if (panel1 == nullptr) {
//		return;
//	}
//	TopAxe->Checked = !TopAxe->Checked;
//	panel1->image2->Visible = TopAxe->Checked;
}
//---------------------------------------------------------------------------

