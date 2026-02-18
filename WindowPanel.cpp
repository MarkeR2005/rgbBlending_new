#include <vcl.h>
#include <algorithm>
#pragma hdrstop

#include "Dialog.h"
#include "WindowPanel.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
std::vector<TWindowPanel*> TWindowPanel::instances_;
__fastcall TWindowPanel::TWindowPanel(TComponent* Owner) : TPanel(Owner)
{
instances_.push_back(this);

	this->OnResize=&Resize;
	this->DoubleBuffered=true;

	viewPanel = new TPanel(this);
	viewPanel->Parent = this;
	viewPanel->Align = alClient;

	Width = 200;
	Height = 100;

	image1 = new TImage(this);
	image1->Height = 1440;
	image1->Width = 50;
	image1 -> Parent = this;
	image1 ->Align = alLeft;
	image2 = new TImage(this);
	image2->Height = 50;
	image2->Width = 2560;
	image2 -> Parent = this;
	image2 ->Align = alTop;

	axis->createAxe(0, 100, viewPanel->Height, 3);
	image1->Canvas->Brush->Color = clWhite;
	image1->Canvas->FillRect(Rect(0, 0, viewPanel->Width, viewPanel->Height));
	image1->Canvas->Draw(0,0, axis->getBitmap());
	axis->createAxe(0, 100, viewPanel->Width, 0);
	image2->Canvas->Brush->Color = clWhite;
	image2->Canvas->FillRect(Rect(0, 0, viewPanel->Width, viewPanel->Height));
	image2 ->Canvas->Draw(50,0, axis->getBitmap());

	FPopupMenu->Items->Clear();
	NameInfo = new TMenuItem(FPopupMenu);
	NameInfo->Caption = "";
	FPopupMenu->Items->Add(NameInfo);
	NameInfo->OnClick = onNameClick;

	PosInfo = new TMenuItem(FPopupMenu);
	PosInfo->Caption = "";
	FPopupMenu->Items->Add(PosInfo);

	Draw = new TMenuItem(FPopupMenu);
	Draw->Caption = "Start/Stop Drawing";
	FPopupMenu->Items->Add(Draw);
	Draw->OnClick = DrawClick;

	Lock = new TMenuItem(FPopupMenu);
	Lock->Caption = "Lock position";
	FPopupMenu->Items->Add(Lock);
    Lock->OnClick = LockClick;

	Update = new TMenuItem(FPopupMenu);
	Update->Caption = "Update";
	FPopupMenu->Items->Add(Update);
	Update->OnClick = UpdateClick;

	Clear = new TMenuItem(FPopupMenu);
	Clear->Caption = "Clear Highlights";
	FPopupMenu->Items->Add(Clear);
	Clear->OnClick = ClearClick;

	this->PopupMenu = FPopupMenu;
}
__fastcall TWindowPanel::~TWindowPanel()
{
    auto it = std::find(instances_.begin(), instances_.end(), this);
    if (it != instances_.end()) {
        instances_.erase(it);
	}
};
void __fastcall TWindowPanel::UpdateClick(TObject* Sender){
	window->emitToAll();
}
void __fastcall TWindowPanel::LockClick(TObject* Sender){
	Lock->Checked = !Lock->Checked;
	window->isSync = Lock->Checked;
}
void __fastcall TWindowPanel::emitToAll(){
	for (auto pan : instances_) {
		if (pan->Lock->Checked) {
			pan->updateAxis(true);
		}
	}
}
void __fastcall TWindowPanel::onNameClick(TObject* Sender)
{
	TForm* f = new TForm(this);
	TMemo* m = new TMemo(f);
	m->Parent = f;
	m->Align = alClient;
	m->Text = container->getProcedures().c_str();
	f->Show();
}
void __fastcall TWindowPanel::DrawClick(TObject* Sender){
	int num = 0;
	TAbstractDialog* dialog = new TAbstractDialog(nullptr);
	dialog->AddInput<int>("Номер горизонта", num);
	if (!Draw->Checked) {
		if (dialog->Execute() && dialog->ContinuePressed) {
			Draw->Checked = !Draw->Checked;
			int width, height;
			std::vector<float> h = window->getHorizon();
			window->getSize(width, height);
			if (h.size() < width * (num+1)) {
				std::vector<float> hor(width*(num+1)-h.size(), -1);
				h.insert(h.end(), hor.begin(), hor.end());
				window->setHorizon(h);
			}
			window->horNum = num;
			window->renderWindow();
		}
		delete dialog;
	}
	else {
        Draw->Checked = !Draw->Checked;
	}
    isDrawing = Draw->Checked;
}
//
void __fastcall TWindowPanel::ClearClick(TObject* Sender){
	window->highlighted_points = {};
	window->renderWindow();
}
//
void __fastcall TWindowPanel::Loaded()
{
	TPanel::Loaded();
}
//
void TWindowPanel::setWindow(std::unique_ptr<BaseWindow> newWindow)
{
	window = std::move(newWindow);

	if (window) {
		window->initWindow(viewPanel);
		GLFWwindow* w = window->getWindow();
		glfwSetWindowUserPointer(w, this);

		// ????????????? callback ??? ??????? ????? ? GLFW
		glfwSetMouseButtonCallback(w, [](GLFWwindow* win, int button, int action, int mods)
		{
			TWindowPanel* panel = static_cast<TWindowPanel*>(glfwGetWindowUserPointer(win));
			if (button == GLFW_MOUSE_BUTTON_LEFT)
			{
				panel->window->handleMouseButtonCallback(button, action, mods);
			}
			else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
			{
				// ???????? ??????? ???????
				POINT pt;
				GetCursorPos(&pt);

				double posx, posy;
				glfwGetCursorPos(win, &posx, &posy);
				float pixelRatioX, pixelRatioY;
				float offsetX, offsetY;
				panel->window->getOffset(offsetX, offsetY);
				panel->window->getRatio(pixelRatioX, pixelRatioY);
				float dT = panel->window->getDT();
				float zoom = panel->window->getZoom();
				float imgX = posx*pixelRatioX/zoom + offsetX;
				float imgY = posy*pixelRatioY/zoom/dT + offsetY;
				panel->setPos(imgX, imgY);

                panel->showPopupMenu(pt.x, pt.y);
			}
		});
		//
		glfwSetCursorPosCallback(w, [](GLFWwindow* win, double xpos, double ypos)
		{
			TWindowPanel* panel = static_cast<TWindowPanel*>(glfwGetWindowUserPointer(win));
			if (panel->isDrawing) {
				panel->window->handleCursorPosDrawCallback(xpos, ypos);
			}
			else {
				panel->window->handleCursorPosMoveCallback(xpos, ypos);
				if (panel->Lock->Checked) {
					emitToAll();
					return;
				}
				panel->updateAxis(false);
			}
		});
		//
		glfwSetScrollCallback(w, [](GLFWwindow* win, double offsetx, double offsety)
		{
			TWindowPanel* panel = static_cast<TWindowPanel*>(glfwGetWindowUserPointer(win));
			panel->window->handleScrollCallback(offsetx, offsety);
			if (panel->Lock->Checked) {
					emitToAll();
					return;
			}
			panel->updateAxis(true);
		});
		 glfwSetCursorEnterCallback(w, [](GLFWwindow* win, int entered){
			TWindowPanel* panel = static_cast<TWindowPanel*>(glfwGetWindowUserPointer(win));
				panel->window->setDragging(false);
		 });
	}
	//
	window->renderWindow();
}
//
void TWindowPanel::showPopupMenu(int screenX, int screenY)
{
	if (PopupMenu && PopupMenu->Items->Count > 0) {
		PopupMenu->Popup(screenX, screenY);
	}
}
//--

void TWindowPanel::setPos(int _posx, int _posy){
int w, h;
window->getSize(w, h);
posx = std::min(_posx, w);
posy = std::min(_posy, h);
std::string fr = "";
if (_posx<frequens.size() && dataType == PanelData::SWAN) {
	fr = "; fr" + std::to_string(frequens[_posx]);
}
this->PosInfo->Caption = IntToStr(posx) + ";" + IntToStr(static_cast<int>(posy*window->getDT())) + fr.c_str();
}
void __fastcall TWindowPanel::updateAxis(bool reeval){
	if (window) {
	float zoom = window->getZoom();
	int sx, sy;
	window->getSize(sx, sy);
	if (reeval) {
		axisY->createAxe(0, sy*window->getDT(), sy*zoom*window->getDT(), 3);
		if (dataType == PanelData::SWAN) {
			float rx, ry;
			window->getRatio(rx, ry);
			DynamicArray < TAxeInterval > intervals;
			intervals.Length=2;
			intervals[0].startValue = frequens.front();
			intervals[0].endValue = frequens.back();
			intervals[0].sizeInPixels = viewPanel->Width;
			intervals[1].startValue = frequens.size();
			intervals[1].sizeInPixels = 0;
			axis->createAxe(intervals, 4);
		}
		else {
		axis->createAxe(0, sx, sx*zoom, 0);
		}
	}
	float ox, oy;
	window->getOffset(ox, oy);


	image1->Canvas->Brush->Color = clWhite;
	image1->Canvas->FillRect(Rect(0, 0, 50, viewPanel->Height));
	image1->Canvas->Draw(0, (-oy)*zoom*window->getDT(), axisY->getBitmap());
	image2->Canvas->Brush->Color = clWhite;
	image2->Canvas->FillRect(Rect(0, 0, viewPanel->Width+50, 50));
	image2 ->Canvas->Draw((image1->Visible ? 50 : 0)-ox*zoom,0, axis->getBitmap());
	}
}
void __fastcall TWindowPanel::Resize(TObject* Sender){
window->resizeWindow(viewPanel->Width,viewPanel->Height);
if (window) {
	window->renderWindow();
}
if (Lock->Checked) {
	emitToAll();
	return;
}
updateAxis(true);
}
void TWindowPanel::changeContrast(float c)
{
	window->setContrast(c);
    window->renderWindow();
}
void TWindowPanel::LoadHorizon(const std::wstring& filename, const std::wstring& fileext) {
	std::vector<float> horizon;

    try {
        if (fileext == L".csv" || fileext == L".txt") {
            TStringList* list = new TStringList();
            list->LoadFromFile(filename.c_str());

            for (int i = 0; i < list->Count; i++) {
                horizon.push_back(list->Strings[i].ToDouble());
            }

            delete list;
        }
        else if (fileext == L".dat") {
			std::unique_ptr<TFileStream> stream(new TFileStream(filename.c_str(), fmOpenRead));
			int size = stream->Size / sizeof(float);
            horizon.resize(size);
			stream->Read(horizon.data(), size * sizeof(float));
        }
    }
    catch (...) {
		// В случае ошибки возвращаем пустой вектор
        ShowMessage("Wrong Format");
    }
	window->setHorizon(horizon);
	window->renderWindow();
}
void TWindowPanel::SaveHorizon(const std::wstring& filename, const std::wstring& fileext) {
   std::vector<float> horizon = window->getHorizon();
   if (horizon.empty()) return;

    try {
		if (fileext == L".csv" || fileext == L".txt") {
            TStringList* list = new TStringList();

            for (float value : horizon) {
                list->Add(FloatToStr(value));
            }

			list->SaveToFile(filename.c_str());
            delete list;
        }
		else if (fileext == L".dat") {
			std::unique_ptr<TFileStream> stream(new TFileStream(filename.c_str(), fmCreate));
			stream->Write(horizon.data(), horizon.size() * sizeof(float));
        }
    }
    catch (...) {
        // Ошибка при сохранении
	}
}
