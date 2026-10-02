//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitFormUniversal.h"
#include "Reader.h"
#include "DataMath.h"
#include "Dialog.h"
#include "Shaders.h"
#include <System.IOUtils.hpp>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormUniversal *FormUniversal;
//---------------------------------------------------------------------------
__fastcall TFormUniversal::TFormUniversal(TComponent* Owner, System::UnicodeString path_)
	: TForm(Owner), path(path_)
{
    Caption=path_;
	SaveDialog1->Title = "Save your data with internal format";
	SaveDialog1->Filter = "Seismic files (*.sd)|*.sd|RGB files (*.rgb)|*.rgb";
	SaveDialog1->DefaultExt = "sd";
	SaveDialog1->FileName = "MyData.rgb";

	SaveDialog2->Title = "Save your horizon";
	SaveDialog2->Filter = "Text files (*.txt)|*.txt|Table files (*.csv)|*.csv|Binary files (*.dat)|*.dat";
	SaveDialog2->DefaultExt = "txt";
	SaveDialog2->FileName = "MyHorizon.txt";


	windowContainer = new TWindowContainer(this);
	windowContainer->Parent = this;
	windowContainer->Align = alClient;
    ShowHorizons->Enabled = false;
    ShowCrosses->Enabled = false;
    overlayEventsTimer = new TTimer(this);
    overlayEventsTimer->Interval = 30;
    overlayEventsTimer->OnTimer = PollOverlayEvents;
    overlayEventsTimer->Enabled = true;
	OnClose=FormClose;
}
//---------------------------------------------------------------------------
void __fastcall TFormUniversal::FormClose(TObject *Sender, TCloseAction &Action)
{
if (overlayEventsTimer) overlayEventsTimer->Enabled = false;
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
        if (!DirectoryExists(path)) {
        	CreateDirectory(path.c_str(), NULL);
        }
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
	if (windowContainer==nullptr) {
		return;
	}
	if (SaveDialog2->Execute())
	{
		std::wstring fileName = SaveDialog2->FileName.c_str();
		std::wstring fileExt = ExtractFileExt(SaveDialog2->FileName).c_str();
		try {
			windowContainer->SaveHorizon(fileName, fileExt);
		} catch (...) {
			ShowMessage("Bad Horizon");
		}
	}
}
//---
void __fastcall TFormUniversal::OpenAsHorF(TObject *Sender)
{
	if (windowContainer==nullptr) {
		return;
	}
	if (OpenDialog1->Execute())
	{
		std::wstring fileName = OpenDialog1->FileName.c_str();
		std::wstring fileExt = ExtractFileExt(OpenDialog1->FileName).c_str();
		try {
			windowContainer->LoadHorizon(fileName, fileExt);
		} catch (...) {
			ShowMessage("Bad Horizon");
		}
	}
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
	LabelR->Caption = FloatToStrF(windowContainer->getFreqByIndex(ScrollBarR->Position),ffFixed, 4, 2) + L" hz";
	LabelG->Caption = FloatToStrF(windowContainer->getFreqByIndex(ScrollBarG->Position),ffFixed, 4, 2) + L" hz";
	LabelB->Caption = FloatToStrF(windowContainer->getFreqByIndex(ScrollBarB->Position),ffFixed, 4, 2) + L" hz";
}
//---------------------------------------------------------------------------

void __fastcall TFormUniversal::LeftAxeClick(TObject *Sender)
{
	if (windowContainer==nullptr) {
		return;
	}
	LeftAxe->Checked = !LeftAxe->Checked;
	windowContainer->checkAxis(LeftAxe->Checked,TopAxe->Checked, RightAxe->Checked);
}
void __fastcall TFormUniversal::TopAxeClick(TObject *Sender)
{
	if (windowContainer==nullptr) {
		return;
	}
	TopAxe->Checked = !TopAxe->Checked;
	windowContainer->checkAxis(LeftAxe->Checked,TopAxe->Checked, RightAxe->Checked);
}
//---------------------------------------------------------------------------

void __fastcall TFormUniversal::Changepalette1Click(TObject *Sender)
{
	if (windowContainer==nullptr) {
		return;
	}
    windowContainer->changePalette();
}
//---------------------------------------------------------------------------

void __fastcall TFormUniversal::Changeshader1Click(TObject *Sender)
{
	if (OpenDialog1->Execute())
	{
		std::wstring fileName = OpenDialog1->FileName.c_str();
		std::wstring fileExt = ExtractFileExt(OpenDialog1->FileName).c_str();
		//this->Caption = fileName.c_str();
		bool success = false;
		if (fileExt == L".frag") {
            GLuint sp;
			try {
				sp = createShaderProgramFromFile(fileName);
				success = true;
			} catch (...) {
			}
			if (success) {
				if (windowContainer==nullptr) {
                    return;
                }
                windowContainer->changeShader(sp);
			}
		}
    }
}
//---------------------------------------------------------------------------


void __fastcall TFormUniversal::SaveScreenShot1Click(TObject *Sender)
{
    if (windowContainer==nullptr) {
        return;
    }
    windowContainer->saveScreenshot(TPath::GetDirectoryName(TPath::GetDirectoryName(TPath::GetDirectoryName(path)))+L"\\pictures\\");
}
//---------------------------------------------------------------------------

void __fastcall TFormUniversal::RightAxeClick(TObject *Sender)
{
    if (windowContainer==nullptr) {
		return;
	}
	RightAxe->Checked = !RightAxe->Checked;
	windowContainer->checkAxis(LeftAxe->Checked,TopAxe->Checked, RightAxe->Checked);
}
//---------------------------------------------------------------------------



void __fastcall TFormUniversal::PollOverlayEvents(TObject*) {
    if (!windowContainer) return;
    windowContainer->pollOverlayEvents();
    bool horizons = false, crosses = false;
    const bool available = windowContainer->getOverlayVisibility(horizons, crosses);
    ShowHorizons->Enabled = available;
    ShowCrosses->Enabled = available;
    if (available) {
        ShowHorizons->Checked = horizons;
        ShowCrosses->Checked = crosses;
    }
}
void __fastcall TFormUniversal::ShowHorizonsClick(TObject*) {
    if (windowContainer) windowContainer->toggleHorizons();
    PollOverlayEvents(nullptr);
}
void __fastcall TFormUniversal::ShowCrossesClick(TObject*) {
    if (windowContainer) windowContainer->toggleCrosses();
    PollOverlayEvents(nullptr);
}
void __fastcall TFormUniversal::FormKeyDown(TObject*, WORD &Key, TShiftState Shift) {
    if (Shift.Contains(ssCtrl) || Shift.Contains(ssAlt)) return;
    int dx = 0, dy = 0;
    if (Key == VK_LEFT) dx = -32;
    if (Key == VK_RIGHT) dx = 32;
    if (Key == VK_UP) dy = -32;
    if (Key == VK_DOWN) dy = 32;
    if ((dx || dy) && windowContainer) {
        windowContainer->panByPixels(dx, dy);
        Key = 0;
    } else if (Key == 'H' && ShowHorizons->Enabled) {
        ShowHorizonsClick(nullptr);
        Key = 0;
    } else if (Key == 'C' && ShowCrosses->Enabled) {
        ShowCrossesClick(nullptr);
        Key = 0;
    }
}
