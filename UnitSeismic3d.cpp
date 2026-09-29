//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitSeismic3d.h"
#include "Shaders.h"
#include "SeismicPlane.h"
#include "RgbPlane.h"
#include "Reader.h"
#include "ColorManager.h"
#include "SeismicData.h"
#include <sstream>
#include "Dialog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSeismic3d *Seismic3d;
//---------------------------------------------------------------------------
__fastcall TSeismic3d::TSeismic3d(TComponent* Owner)
	: TForm(Owner)
{
	viewport.initWindow(Panel1);

	seismicShader = createShaderProgram("volumetric", "seismic");
	rgbShader = createShaderProgram("volumetric", "rgb");
    	viewport.setCamera(
		glm::vec3(1000.0f, 0.0f, 1000.0f),  // позици€ камеры
		glm::vec3(0.0f, 0.0f, 0.0f),  // цель камеры
		glm::vec3(0.0f, 1.0f, 0.0f)   // вектор "вверх"
	);

	viewport.setPerspective(45.0f, 0.1f, 10000.0f);
	viewport.setMouseButtonCallback([this](int button, int action, int mods){
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			POINT pt;
			GetCursorPos(&pt);
			this->PopupMenu1->Popup(pt.x, pt.y);
		}
	});
}
//
void TSeismic3d::FormCreate(){
}
void __fastcall TSeismic3d::Panel1Resize(TObject *Sender)
{
viewport.resizeWindow( Panel1->Width, Panel1->Height);
viewport.renderWindow();
}
//---------------------------------------------------------------------------



void __fastcall TSeismic3d::File2Click(TObject *Sender)
{
	if(OpenDialog1->Execute()){
		fileName = OpenDialog1->FileName.c_str();
		fileExt = ExtractFileExt(OpenDialog1->FileName).c_str();
		bool success = false;
		if (fileExt == L".ibm") {
				try {
					sz = readSize(fileName);
					success = true;
				} catch (...) {
				}
				if (success) {
				}
		}
		else if (fileExt == L".irgb") {
				try {
					size4 sz_ = readSizeRGB(fileName, frequens);
					sz = {sz_.samples, sz_.traces, sz_.lines};
					nf = sz_.filters;
					ScrollBarR->Max = nf-1;
					ScrollBarG->Max = nf-1;
					ScrollBarB->Max = nf-1;
					success = true;
				} catch (...) {
				}
				if (success) {
				}
		}
		else {
            ShowMessage("WrongFile");
        }
	}
}
//---------------------------------------------------------------------------

void __fastcall TSeismic3d::AddClick(TObject *Sender)
{
	if (fileName == L"") {
		File2Click(Sender);
	}
		int num = 0;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Ќомер", num);
	if (dialog->Execute() && dialog->ContinuePressed) {
		bool success = false;
		glm::vec3 norm = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 pos;
		glm::vec2 _sz = glm::vec2(1.0f, 1.0f);
		if (fileExt == L".ibm") {
			std::shared_ptr<SeismicData> seismicData;
			bool flipV = false, flipH = false;
			try {
				if (Sender == AddSlice || Sender == AddSlice1) {
					Traces tr = readTimeSliceRegular(fileName+L"s", num);
					seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
					norm = glm::vec3(0.0f, 1.0f, 0.0f);
					pos = glm::vec3(0.0f, (float)sz.samples/2-num, 0.0f);
					_sz = glm::vec2((float)sz.lines, (float)sz.traces);
				}
				else if (Sender == AddCrossline || Sender == AddCrossline1) {
					Traces tr = readCrosslineRegular(fileName, num);
					seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
					norm = glm::vec3(1.0f, 0.0f, 0.0f);
					pos = glm::vec3((float)sz.traces/2-num, 0.0f, 0.0f);
					_sz = glm::vec2((float)sz.lines, (float)sz.samples);
					flipV=true;
				}
				else {
					Traces tr = readInlineRegular(fileName, num);
					seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
					pos = glm::vec3(0.0f, 0.0f, (float)sz.lines/2-num);
					_sz = glm::vec2((float)sz.traces, (float)sz.samples);
					flipV=true;
				}
				success = true;
				seismicData->setCM(cm);
			} catch (...) {
				throw;
			}
			if (success) {
				auto seismicPlane = std::make_shared<SeismicPlane>(
					pos,  // позици€
					norm,   // нормаль (смотрит вперед)
					_sz,         // размер
					seismicShader,
                    [](){return std::make_shared<SeismicData>();},
					flipV,
                    flipH
				);
				seismicPlane->updateTexture(seismicData->getTexture());
				seismicPlane->initPaletteTexture(PALETTE);
				viewport.addPlane(seismicPlane);
			}
		}
		else {
			std::shared_ptr<RgbData> rgbData;
			bool flipV = false, flipH = false;
			try {
				if (Sender == AddSlice || Sender == AddSlice1) {
					rgbData = readTimeSliceRGB(fileName+L"s", num);
					norm = glm::vec3(0.0f, 1.0f, 0.0f);
					pos = glm::vec3(0.0f, (float)sz.samples/2-num, 0.0f);
					_sz = glm::vec2((float)sz.lines, (float)sz.traces);
				}
				else if (Sender == AddCrossline || Sender == AddCrossline1) {
					rgbData = readCrosslineRGB(fileName, num);
					norm = glm::vec3(1.0f, 0.0f, 0.0f);
					pos = glm::vec3((float)sz.traces/2-num, 0.0f, 0.0f);
					_sz = glm::vec2((float)sz.lines, (float)sz.samples);
					flipV=true;
				}
				else {
					rgbData = readInlineRGB(fileName, num);
					pos = glm::vec3(0.0f, 0.0f, (float)sz.lines/2-num);
					_sz = glm::vec2((float)sz.traces, (float)sz.samples);
					flipV=true;
				}
				success = true;
				rgbData->setCM(cm_rgb);
			} catch (...) {
				throw;
			}
			if (success) {
				auto rgbPlane = std::make_shared<RgbPlane>(
					pos,  // позици€
					norm,   // нормаль (смотрит вперед)
					_sz,         // размер
					rgbShader,
					[](){return std::make_shared<RgbData>();},
					flipV,
                    flipH
				);
				rgbPlane->initTextureArray(rgbData->getTexture());
				viewport.addPlane(rgbPlane);
			}
		}
	}
	delete dialog;
}
//---------------------------------------------------------------------------

void __fastcall TSeismic3d::DeletePlane1Click(TObject *Sender)
{
	viewport.removePlane(viewport.getSelectedPlane());
	viewport.renderWindow();
}
//---------------------------------------------------------------------------

void __fastcall TSeismic3d::View2Click(TObject *Sender)
{
	View2->Checked = !View2->Checked;
	Panel2->Enabled = View2->Checked;
	Panel2->Visible = View2->Checked;
}
//--------------------------------------------------------------------------
void __fastcall TSeismic3d::ScrollBar1Change(TObject *Sender)
{
	viewport.setChannels(ScrollBarR->Position, ScrollBarG->Position, ScrollBarB->Position);
	LabelR->Caption = (std::to_string(frequens[ScrollBarR->Position]) + " hz").c_str();
	LabelG->Caption = (std::to_string(frequens[ScrollBarG->Position]) + " hz").c_str();
	LabelB->Caption = (std::to_string(frequens[ScrollBarB->Position]) + " hz").c_str();
	viewport.renderWindow();
}
void __fastcall TSeismic3d::CheckBoxRClick(TObject *Sender)
{
	viewport.setChannelEnabled(CheckBoxR->Checked, CheckBoxG->Checked, CheckBoxB->Checked);
	viewport.renderWindow();
}
//---------------------------------------------------------------------------

void __fastcall TSeismic3d::SetTransparency1Click(TObject *Sender)
{
	float num = 0;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
	dialog->AddInput<float>("ѕрозрачность", num);
	if (dialog->Execute() && dialog->ContinuePressed) {
		viewport.getSelectedPlane()->setTransparency(num/100.0f);
        viewport.renderWindow();
	}
    delete dialog;
}
//---------------------------------------------------------------------------

