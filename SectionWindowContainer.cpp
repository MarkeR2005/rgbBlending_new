//---------------------------------------------------------------------------

#pragma hdrstop
#include "SeismicWindow.h"
#include "RgbWindow.h"
#include "SectionWindowContainer.h"
#include "SeismicData.h"
#include "RgbData.h"
#include "DataCalculator.h"
#include "UnitFormSection2D.h"
#include "DialogWrapper.h"
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Dialogs.hpp>
#include <cmath>
//---------------------------------------------------------------------------
#pragma package(smart_init)





std::vector<TSectionWindowContainer*> TSectionWindowContainer::instances_;
__fastcall TSectionWindowContainer::TSectionWindowContainer(TComponent* Owner) : TPanel(Owner){
	this->OnResize = Resize;
	this->DoubleBuffered=true;

	viewPanel = new TPanel(this);
	viewPanel->Parent = this;
	viewPanel->Align = alClient;
    updateHorizonsButton = new TButton(this);
    updateHorizonsButton->Parent = this;
    updateHorizonsButton->Align = alTop;
    updateHorizonsButton->Height = 28;
    updateHorizonsButton->Caption = L"Обновить горизонты";
    updateHorizonsButton->OnClick = UpdateHorizonsClick;
    updateHorizonsButton->Visible = false;
    rgbChangeTimer = new TTimer(this);
    rgbChangeTimer->Enabled = false;
    rgbChangeTimer->Interval = 250;
    rgbChangeTimer->OnTimer = ApplyRgbChannels;
    displayTimer = new TTimer(this);
    displayTimer->Enabled = false;
    displayTimer->Interval = 200;
    displayTimer->OnTimer = ApplyDisplayCallback;

	image1 = new TImage(this);
	image1->Height = 1440;
	image1->Width = 50;
	image1 -> Parent = this;
	image1 ->Align = alLeft;
	image1 ->Visible = false;
	image1 ->Enabled = false;

    image3 = new TImage(this);
	image3->Height = 1440;
	image3->Width = 50;
	image3 -> Parent = this;
	image3 ->Align = alRight;
	image3 ->Visible = false;
	image3 ->Enabled = false;

	image2 = new TImage(this);
	image2->Height = 50;
	image2->Width = 2560;
	image2 -> Parent = this;
	image2 ->Align = alTop;
	image2 ->Visible = false;
	image2 ->Enabled = false;

    image4 = new TImage(this);
	image4->Height = 50;
	image4->Width = 2560;
	image4 -> Parent = this;
	image4 ->Align = alBottom;
	image4 ->Visible = false;
	image4 ->Enabled = false;

    ratioPanel = new TPanel(this);
    ratioPanel->Parent = this;
    ratioPanel->SetBounds(0, Height-50, 255, 50);
    ratioPanel->BevelOuter = bvNone;
    ratioPanel->Color = clInfoBk;
    ratioPanel->Visible = false;
    ratioXLabel = new TLabel(this);
    ratioXLabel->Parent = ratioPanel;
    ratioXLabel->SetBounds(3, 3, 62, 19);
    ratioXLabel->Caption = L"X 1.00";
    ratioYLabel = new TLabel(this);
    ratioYLabel->Parent = ratioPanel;
    ratioYLabel->SetBounds(3, 26, 62, 19);
    ratioYLabel->Caption = L"T 1.00";
    ratioXSlider = new TTrackBar(this);
    ratioXSlider->Parent = ratioPanel;
    ratioXSlider->SetBounds(68, 0, 185, 23);
    ratioXSlider->Min = -10; ratioXSlider->Max = 10;
    ratioXSlider->Position = 0;
    ratioXSlider->TickStyle = tsNone;
    ratioXSlider->OnChange = RatioChange;
    ratioYSlider = new TTrackBar(this);
    ratioYSlider->Parent = ratioPanel;
    ratioYSlider->SetBounds(68, 23, 185, 23);
    ratioYSlider->Min = -10; ratioYSlider->Max = 10;
    ratioYSlider->Position = 0;
    ratioYSlider->TickStyle = tsNone;
    ratioYSlider->OnChange = RatioChange;
    ratioPanel->BringToFront();

	axisY->createAxe(0, 100, viewPanel->Height, 3);
	image1->Canvas->Brush->Color = clInfoBk;
	image1->Canvas->FillRect(Rect(0, 0, viewPanel->Width, viewPanel->Height));
	image1->Canvas->Draw(0,0, axisY->getBitmap());
    axisYR->createAxe(0, 100, viewPanel->Height, 2);
	image3->Canvas->Brush->Color = clInfoBk;
	image3->Canvas->FillRect(Rect(0, 0, viewPanel->Width, viewPanel->Height));
	image3->Canvas->Draw(0,0, axisYR->getBitmap());
	axis->createAxe(0, 100, viewPanel->Width, 0);
	image2->Canvas->Brush->Color = clInfoBk;
	image2->Canvas->FillRect(Rect(0, 0, viewPanel->Width, viewPanel->Height));
	image2 ->Canvas->Draw(50,0, axis->getBitmap());
    image4->Canvas->Brush->Color = clInfoBk;
	image4->Canvas->FillRect(Rect(0, 0, viewPanel->Width, viewPanel->Height));

	FPopupMenu->Items->Clear();
	NameInfo = new TMenuItem(FPopupMenu);
	NameInfo->Caption = "";
	FPopupMenu->Items->Add(NameInfo);

	PosInfo = new TMenuItem(FPopupMenu);
	PosInfo->Caption = "";
	FPopupMenu->Items->Add(PosInfo);

	syncButton = new TMenuItem(FPopupMenu);
	syncButton->Caption = "Sync";
	FPopupMenu->Items->Add(syncButton);
    syncButton->OnClick = LockClick;
    horizonStyleMenu = new TMenuItem(FPopupMenu);
    horizonStyleMenu->Caption = L"Horizon colors";
    horizonStyleMenu->Visible = false;
    FPopupMenu->Items->Add(horizonStyleMenu);

	instances_.push_back(this);

	this->PopupMenu = FPopupMenu;
}


void __fastcall TSectionWindowContainer::Loaded(){};
void __fastcall TSectionWindowContainer::emitToAll(){
	for (auto windowContainer : instances_) {
		windowContainer->updateAxis(true);
	}
}
void __fastcall TSectionWindowContainer::LockClick(TObject* Sender){
	syncButton->Checked = !syncButton->Checked;
	auto SyncWindow = dynamic_cast<ISyncWindow*>(window.get());
	if (SyncWindow) {
	   SyncWindow->setSync(syncButton->Checked);
	}
}
void TSectionWindowContainer::addButton(std::string caption, std::function<void()> function){
	TMenuItem *item = new TMenuItem(FPopupMenu);
	item->Caption = caption.c_str();
	FPopupMenu->Items->Add(item);
	func[item] = function;
	item->OnClick=useFunction;
}
void TSectionWindowContainer::useFunction(TObject* Sender){
if (auto sch = func.find(Sender); sch != func.end()) sch->second();
Application->ProcessMessages();
}
void TSectionWindowContainer::changePalette(){
    if (type != SectionWindowType::SEIS) {
            return;
    }
    TAbstractDialog* dialog = new TAbstractDialog(nullptr);
    int r1 = 0, g1 = 0, b1 = 0;
    int r2 = 0, g2 = 0, b2 = 0;
		dialog->AddInput<int>("R1", r1);
        dialog->AddInput<int>("G1", g1);
        dialog->AddInput<int>("B1", b1);
        dialog->AddInput<int>("R2", r2);
        dialog->AddInput<int>("G2", g2);
        dialog->AddInput<int>("B2", b2);
		if (dialog->Execute() && dialog->ContinuePressed) {
            std::array<uint8_t, 256*3> pal = genBasicPalette(r1,g1,b1,r2,g2,b2,true);
            SeismicWindow* sw = dynamic_cast<SeismicWindow*>(window.get());
            sw->initPaletteTexture(pal);
		}
		delete dialog;
}
void TSectionWindowContainer::changeShader(GLuint shaderProgram){
    IFlatWindow* flatWin = dynamic_cast<IFlatWindow*>(window.get());
    if (flatWin != nullptr) {
        flatWin->setShaderProgram(shaderProgram);
    }
}
bool TSectionWindowContainer::setHorizonsCallback(std::function<void(const std::vector<Horizon>&)> cb) {
    if (!dynamic_cast<FlatWindow*>(window.get())) return false;
    horizonsCallback = std::move(cb);
    return true;
}
bool TSectionWindowContainer::setHorizons(const std::vector<Horizon>& value) {
    FlatWindow* flat = dynamic_cast<FlatWindow*>(window.get());
    if (!flat) return false;
    flat->setHorizons(value);
    return true;
}
bool TSectionWindowContainer::setCrosses(const std::vector<Cross>& value) {
    FlatWindow* flat = dynamic_cast<FlatWindow*>(window.get());
    if (!flat) return false;
    flat->setCrosses(value);
    return true;
}
bool TSectionWindowContainer::getOverlayVisibility(bool& showH, bool& showC) const {
    auto* flat = dynamic_cast<FlatWindow*>(window.get());
    if (!flat) return false;
    showH = flat->horizonsVisible();
    showC = flat->crossesVisible();
    return true;
}
void TSectionWindowContainer::toggleHorizons() {
    auto* flat = dynamic_cast<FlatWindow*>(window.get());
    if (flat) flat->setHorizonsVisible(!flat->horizonsVisible());
}
void TSectionWindowContainer::toggleCrosses() {
    auto* flat = dynamic_cast<FlatWindow*>(window.get());
    if (flat) flat->setCrossesVisible(!flat->crossesVisible());
}
void TSectionWindowContainer::panByPixels(int dx, int dy) {
    if (auto* flat = dynamic_cast<FlatWindow*>(window.get())) flat->panByPixels(dx, dy);
}
void TSectionWindowContainer::pollOverlayEvents() {
    auto* flat = dynamic_cast<FlatWindow*>(window.get());
    if (flat && flat->getWindow()) glfwPollEvents();
}
void __fastcall TSectionWindowContainer::UpdateHorizonsClick(TObject*) {
    FlatWindow* flat = dynamic_cast<FlatWindow*>(window.get());
    if (flat && horizonsCallback) {
        const auto snapshot = flat->getHorizons();
        horizonsCallback(snapshot);
    }
}
void TSectionWindowContainer::initialize2D(const std::shared_ptr<IBaseData>& data){
    rgbChangeTimer->Enabled = false;
    displayTimer->Enabled = false;
    appliedR = appliedG = appliedB = pendingR = pendingG = pendingB = 0;
	if (!data->getCM()) {
		data->setCM(colorManager);
	}
	std::shared_ptr<SeismicData> sData = std::dynamic_pointer_cast<SeismicData>(data);
	std::shared_ptr<RgbData> rData = std::dynamic_pointer_cast<RgbData>(data);
	if (sData) {
		std::unique_ptr<SeismicWindow> window_ = std::make_unique<SeismicWindow>();
		if (sData->getType() == DataType::SWAN || sData->getType() == DataType::EN_SWAN ||
		sData->getType() == DataType::EN_TRACE || sData->getType() == DataType::TRACE) {
			window_->setThin();
		}
		window_->initWindow(viewPanel);
		window_->initPaletteTexture(PALETTE);
		window_->initIndexTexture(sData->getTexture());
        window_->setDT(sData->getDT()/1000.0f);
		window = std::move(window_);
		type = SectionWindowType::SEIS;

	}
	else if (rData) {
		std::unique_ptr<RgbWindow> window_ = std::make_unique<RgbWindow>();
		if (rData->getSize().x == 1) {
			window_->setThin();
		}
		window_->initWindow(viewPanel);
		window_->initData(rData);
        window_->setDT(rData->getDT()/1000.0f);
        type = SectionWindowType::RGB;
		window = std::move(window_);
	}
	else {
		throw;
	}
	image1 ->Visible = true;
	image1 ->Enabled = true;
	image2 ->Visible = true;
	image2 ->Enabled = true;
    image3 ->Visible = true;
    image3 ->Enabled = true;
    image4 ->Visible = true;
    image4 ->Enabled = true;
    ratioPanel->Visible = true;
    ratioPanel->BringToFront();
	dataContainer = data;
    updateHorizonsButton->Visible = true;
    setupCallbacks();
	setupButtons();
	window->resizeWindow(viewPanel->Width, viewPanel->Height);
	updateAxis(true);
	NameInfo->Caption = data->getName().c_str();
}



int TSectionWindowContainer::getFreqSize(){
    auto rgb = std::dynamic_pointer_cast<RgbData>(dataContainer);
    if (rgb) return rgb->getSize().f;
    return std::dynamic_pointer_cast<SeismicData>(dataContainer) ? 0 : -1;
}
float TSectionWindowContainer::getFreqByIndex(int idx){
    auto rgb = std::dynamic_pointer_cast<RgbData>(dataContainer);
    if (!rgb || idx < 0 || idx >= rgb->getSize().f) return -1.0f;
    return rgb->getFreqs()[idx];
}

__fastcall TSectionWindowContainer::~TSectionWindowContainer(){
    if (rgbChangeTimer) rgbChangeTimer->Enabled = false;
    if (displayTimer) displayTimer->Enabled = false;
    auto it = std::find(instances_.begin(), instances_.end(), this);
    if (it != instances_.end()) {
        instances_.erase(it);
    }
    delete image1;
    delete image2;
    delete axis;
    delete axisY;
    delete viewPanel;
    delete FPopupMenu;
}


void __fastcall TSectionWindowContainer::Resize(TObject* Sender){
    if (ratioPanel) ratioPanel->Top = Height-ratioPanel->Height;
	if (!window) {
		return;
    }
	window->resizeWindow(viewPanel->Width, viewPanel->Height);
	updateAxis(true);
}
void __fastcall TSectionWindowContainer::ApplyRgbChannels(TObject*) {
    rgbChangeTimer->Enabled = false;
    auto* rgb = dynamic_cast<RgbWindow*>(window.get());
    if (!rgb) return;
    rgb->setColor(pendingR, pendingG, pendingB);
    appliedR = pendingR; appliedG = pendingG; appliedB = pendingB;
    rgb->renderWindow();
}
void TSectionWindowContainer::setViewParams(viewParams params){
	IRgbWindow* wd = dynamic_cast<IRgbWindow*>(window.get());
	FlatWindow* wd1 = dynamic_cast<FlatWindow*>(window.get());
	if (wd != nullptr) {
        wd->setView(params.isR, params.isG, params.isB);
        pendingR = params.r; pendingG = params.g; pendingB = params.b;
        rgbChangeTimer->Enabled = false;
        if (pendingR != appliedR || pendingG != appliedG || pendingB != appliedB)
            rgbChangeTimer->Enabled = true; // coalesce rapid scrollbar events
    }
	if (wd1 != nullptr) {
		wd1->setContrast(params.contrast);
	}
    auto* rgb = dynamic_cast<RgbWindow*>(window.get());
    if (rgb && rgbChangeTimer->Enabled &&
        rgb->hasCachedLayers(pendingR, pendingG, pendingB)) {
        ApplyRgbChannels(nullptr);
    } else {
        window->renderWindow();
    }
}
void __fastcall TSectionWindowContainer::RatioChange(TObject*) {
    auto* flat = dynamic_cast<FlatWindow*>(window.get());
    if (!flat || !ratioXSlider || !ratioYSlider) return;
    const float rx = std::pow(10.0f, ratioXSlider->Position/10.0f);
    const float ry = std::pow(10.0f, ratioYSlider->Position/10.0f);
    ratioXLabel->Caption = (L"X " + std::to_wstring(rx).substr(0, 4)).c_str();
    ratioYLabel->Caption = (L"T " + std::to_wstring(ry).substr(0, 4)).c_str();
    flat->setRatio(rx, ry);
    flat->resizeWindow(viewPanel->Width, viewPanel->Height);
    updateAxis(true);
    flat->renderWindow();
}

void TSectionWindowContainer::rebuildHorizonStyleMenu() {
    if (!horizonStyleMenu) return;
    for (auto* action : horizonStyleActions) func.erase(action);
    horizonStyleActions.clear();
    horizonStyleMenu->Clear();
    auto* flat = dynamic_cast<FlatWindow*>(window.get());
    if (!flat) {horizonStyleMenu->Visible = false; return;}
    horizonStyleMenu->Visible = true;
    const auto& horizons = flat->getHorizons();
    horizonStyleMenu->Enabled = !horizons.empty();
    for (size_t i = 0; i < horizons.size(); ++i) {
        const auto& horizon = horizons[i];
        TMenuItem* sub = new TMenuItem(horizonStyleMenu);
        sub->Caption = horizon.name.empty() ?
            (L"Horizon " + std::to_wstring(i+1)).c_str() : horizon.name.c_str();
        horizonStyleMenu->Add(sub);
        auto addAction = [&](const wchar_t* caption, bool checked, std::function<void()> fn) {
            TMenuItem* item = new TMenuItem(sub);
            item->Caption = caption;
            item->Checked = checked;
            sub->Add(item);
            func[item] = std::move(fn);
            horizonStyleActions.push_back(item);
            item->OnClick = useFunction;
        };
        addAction(L"Choose color...", horizon.useColor, [this, i]() {
            auto* current = dynamic_cast<FlatWindow*>(window.get());
            if (!current || i >= current->getHorizons().size()) return;
            std::unique_ptr<TColorDialog> dialog(new TColorDialog(this));
            const auto& h = current->getHorizons()[i];
            dialog->Color = static_cast<TColor>(RGB(h.red, h.green, h.blue));
            if (dialog->Execute()) {
                const COLORREF c = ColorToRGB(dialog->Color);
                current->setHorizonColor(i, GetRValue(c), GetGValue(c), GetBValue(c));
            }
        });
        addAction(L"Negative (default)", !horizon.useColor, [this, i]() {
            if (auto* current = dynamic_cast<FlatWindow*>(window.get())) current->setHorizonNegative(i);
        });
        addAction(L"Contrast outline", horizon.contrast, [this, i]() {
            if (auto* current = dynamic_cast<FlatWindow*>(window.get())) {
                if (i < current->getHorizons().size())
                    current->setHorizonContrast(i, !current->getHorizons()[i].contrast);
            }
        });
    }
}

void TSectionWindowContainer::showPopupMenu(int button, int action, int mode)
{
	if (PopupMenu && PopupMenu->Items->Count > -1 && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		POINT pt;
		GetCursorPos(&pt);
        getPos();
        rebuildHorizonStyleMenu();
		PopupMenu->Popup(pt.x, pt.y);
	}
}

void TSectionWindowContainer::getPos(){
	if (type == SectionWindowType::SEIS || type == SectionWindowType::RGB) {
	IFlatWindow* flatWin = dynamic_cast<IFlatWindow*>(window.get());
		double posx, posy;
		glfwGetCursorPos(window->getWindow(), &posx, &posy);
		float pixelRatioX, pixelRatioY;
		float offsetX, offsetY;
		flatWin->getOffset(offsetX, offsetY);
		flatWin->getRatio(pixelRatioX, pixelRatioY);
		float dT = flatWin->getDT();
		float zoom = flatWin->getZoom();
		float imgX = posx*pixelRatioX/zoom + offsetX;
		float imgY = posy*pixelRatioY/zoom/dT + offsetY;
		posx_=imgX;
		posy_=imgY;
		auto sData = std::dynamic_pointer_cast<SeismicData>(dataContainer);
		if (sData) {
			if (sData->getType() == DataType::SWAN || sData->getType() == DataType::EN_SWAN) {
                const auto& frequencies = sData->getFreqRef();
                if (posx_ >= 0 && posx_ < static_cast<int>(frequencies.size())) {
                    PosInfo->Caption = (std::to_string(frequencies[posx_]) + ";" +
                                        std::to_string(posy_)).c_str();
                    return;
                }
			}
		}
		PosInfo->Caption = (std::to_string(posx_) + ";" + std::to_string(posy_)).c_str();
	}
}


void TSectionWindowContainer::scheduleDisplay(double x, double y) {
    if (!callbackDisp) return;
    pendingMouseX = x;
    pendingMouseY = y;
    displayTimer->Enabled = false;
    displayTimer->Enabled = true;
}
void __fastcall TSectionWindowContainer::ApplyDisplayCallback(TObject*) {
    displayTimer->Enabled = false;
    if (!callbackDisp) return;
    auto* flat = dynamic_cast<FlatWindow*>(window.get());
    if (!flat || !flat->getWindow()) return;
    double mouseX = 0, mouseY = 0;
    glfwGetCursorPos(flat->getWindow(), &mouseX, &mouseY);
    if (std::abs(mouseX-pendingMouseX) > 0.5 || std::abs(mouseY-pendingMouseY) > 0.5) {
        scheduleDisplay(mouseX, mouseY);
        return;
    }
    int winWidth = 0, winHeight = 0, fbWidth = 0, fbHeight = 0;
    glfwGetWindowSize(flat->getWindow(), &winWidth, &winHeight);
    glfwGetFramebufferSize(flat->getWindow(), &fbWidth, &fbHeight);
    if (winWidth <= 0 || winHeight <= 0 || mouseX < 0 || mouseY < 0 ||
        mouseX >= winWidth || mouseY >= winHeight) return;
    const double screenX = mouseX*fbWidth/winWidth;
    const double screenY = mouseY*fbHeight/winHeight;
    float ratioX, ratioY, offsetX, offsetY;
    flat->getRatio(ratioX, ratioY);
    flat->getOffset(offsetX, offsetY);
    const float zoom = flat->getZoom(), dT = flat->getDT();
    if (zoom <= 0 || dT <= 0 || ratioX <= 0 || ratioY <= 0) return;
    const int x = static_cast<int>(std::floor(screenX*ratioX/zoom+offsetX));
    const int y = static_cast<int>(std::floor(screenY*ratioY/zoom/dT+offsetY));
    int width, height;
    flat->getSize(width, height);
    if (x < 0 || y < 0 || x >= width || y >= height) return;
    std::string info = flat->hoveredOverlay(screenX, screenY);
    if (info.empty()) {
        if (auto* seismic = dynamic_cast<SeismicWindow*>(window.get())) {
            int index = 0;
            if (!seismic->getIndex(x, y, index)) return;
            info = "Ampl: " + std::to_string(index-127);
        } else if (auto* rgb = dynamic_cast<RgbWindow*>(window.get())) {
            int r = 0, g = 0, b = 0;
            if (!rgb->getPixelComponents(x, y, r, g, b)) return;
            info = "R:" + std::to_string(r) + ", G:" + std::to_string(g) +
                   ", B:" + std::to_string(b);
        } else return;
    }
    callbackDisp(x, static_cast<int>(y*dT), info);
}

void TSectionWindowContainer::setupCallbacks()
{
    auto* callbacks = dynamic_cast<ICallbackWindow*>(window.get());
    if (!callbacks) return;
    callbacks->setMouseButtonCallback([this](int button, int action, int mode) {
        showPopupMenu(button, action, mode);
    });
    callbacks->setScrollCallback([this](double, double) {
        if (syncButton->Checked) {
            emitToAll();
            return;
        }
        updateAxis(true);
    });
    callbacks->setCursorPosCallback([this](double x, double y) {
        scheduleDisplay(x, y);
        if (syncButton->Checked) {
            emitToAll();
            return;
        }
        updateAxis(false);
        getPos();
        callback(posx_);
    });
    callbacks->setCursorEnterCallback([this](int entered) {
        if (!entered) displayTimer->Enabled = false;
    });
}

void TSectionWindowContainer::createWindow(const std::shared_ptr<IBaseData>& newData,
    OverlayMapping mapping, int sourceTrace)
{
    if (!newData) return;
    const auto source = dynamic_cast<FlatWindow*>(window.get());
    auto sourceSeis = std::dynamic_pointer_cast<SeismicData>(dataContainer);
    auto sourceRgb = std::dynamic_pointer_cast<RgbData>(dataContainer);
    auto targetSeis = std::dynamic_pointer_cast<SeismicData>(newData);
    auto targetRgb = std::dynamic_pointer_cast<RgbData>(newData);
    const int sourceWidth = sourceSeis ? sourceSeis->getSize().x
        : sourceRgb ? sourceRgb->getSize().x : 0;
    const int targetWidth = targetSeis ? targetSeis->getSize().x
        : targetRgb ? targetRgb->getSize().x : 0;

    TransferredOverlays overlays;
    if (source && dataContainer && sourceWidth && targetWidth &&
        std::abs(dataContainer->getDT() - newData->getDT()) < 0.0001f)
        overlays = transferOverlays(source->getHorizons(), source->getCrosses(),
            sourceWidth, targetWidth, mapping, sourceTrace);

    std::unique_ptr<TFormSection2D> form(new TFormSection2D(Application->MainForm, basePath));
    if (mapping == OverlayMapping::SameGrid)
        form->setTraceCallback(callback);
    else
        form->setTraceCallback([sourceCallback = callback, sourceTrace](int) {
            sourceCallback(sourceTrace);
        });
    if (mapping == OverlayMapping::SameGrid)
        form->setDisplayCallback(callbackDisp);
    else
        form->setDisplayCallback([sourceDisplay = callbackDisp, sourceTrace](
            int, int sample, std::string info) {
            if (sourceDisplay) sourceDisplay(sourceTrace, sample, std::move(info));
        });
    form->initFromData(newData);
    form->setHorizons(overlays.horizons);
    form->setCrosses(overlays.crosses);
    form->setHorizonsCallback(horizonsCallback);
    form->Show();
    form.release();
}

void TSectionWindowContainer::setupFlatButtons(){
    addButton("Edit horizon (draw with mouse)", [this]() {
        auto* flat = dynamic_cast<FlatWindow*>(window.get());
        if (!flat) return;
        if (flat->horizonEditing()) {flat->setHorizonEditing(false); return;}

        std::unique_ptr<TForm> picker(new TForm(this));
        picker->Caption = L"Выбор горизонта для редактирования";
        picker->BorderStyle = bsDialog;
        picker->Position = poScreenCenter;
        picker->ClientWidth = 370;
        picker->ClientHeight = 380;
        TTreeView* tree = new TTreeView(picker.get());
        tree->Parent = picker.get();
        tree->SetBounds(12, 12, 346, 270);
        TTreeNode* root = tree->Items->Add(nullptr, L"Горизонты");
        const auto& horizons = flat->getHorizons();
        TTreeNode* firstNode = nullptr;
        for (size_t i = 0; i < horizons.size(); ++i) {
            const std::wstring name = horizons[i].name.empty()
                ? L"Горизонт " + std::to_wstring(i+1) : horizons[i].name;
            TTreeNode* node = tree->Items->AddChild(root, name.c_str());
            if (!firstNode) firstNode = node;
        }
        TTreeNode* createNode = tree->Items->AddChild(root, L"+ Новый горизонт");
        root->Expand(true);
        tree->Selected = firstNode ? firstNode : createNode;

        TLabel* nameLabel = new TLabel(picker.get());
        nameLabel->Parent = picker.get();
        nameLabel->SetBounds(12, 292, 340, 20);
        nameLabel->Caption = L"Имя нового горизонта:";
        TEdit* nameEdit = new TEdit(picker.get());
        nameEdit->Parent = picker.get();
        nameEdit->SetBounds(12, 312, 346, 24);
        nameEdit->Text = (L"Горизонт " + std::to_wstring(horizons.size()+1)).c_str();

        TButton* accept = new TButton(picker.get());
        accept->Parent = picker.get();
        accept->SetBounds(182, 345, 84, 25);
        accept->Caption = L"Выбрать";
        accept->ModalResult = mrOk;
        accept->Default = true;
        TButton* cancel = new TButton(picker.get());
        cancel->Parent = picker.get();
        cancel->SetBounds(274, 345, 84, 25);
        cancel->Caption = L"Отмена";
        cancel->ModalResult = mrCancel;
        cancel->Cancel = true;

        if (picker->ShowModal() != mrOk || !tree->Selected || tree->Selected->Parent != root)
            return;
        const size_t index = static_cast<size_t>(tree->Selected->Index);
        if (tree->Selected == createNode) {
            const std::wstring name = nameEdit->Text.c_str();
            flat->selectHorizon(index, name);
        } else {
            flat->selectHorizon(index);
        }
    });
	addButton("Smooth T", [&](){
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Окно (отсчётов)", stnTWindow);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<IBaseData> newData = DataCalculator::smoothT(dataContainer, stnTWindow);
            createWindow(newData);
		}
		delete dialog;
	});
	addButton("Smooth X", [&](){
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Окно (отсчётов)", stnXWindow);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<IBaseData> newData = DataCalculator::smoothX(dataContainer, stnXWindow);
createWindow(newData);
		}
		delete dialog;
	});
	addButton("Mute", [&](){
		int start = 0;
		int stop = 1000;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Сохранить данные с", start);
		dialog->AddInput<int>("Сохранить данные по", stop);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<IBaseData> newData = DataCalculator::mute(dataContainer, start, stop);
createWindow(newData);
		}
		delete dialog;
	});
	addButton("Get Trace", [&](){
		{
			getPos();
			std::shared_ptr<IBaseData> newData = DataCalculator::getTrace(dataContainer, posx_);
createWindow(newData, OverlayMapping::Trace, posx_);
//                    ShowMessage("ok");
		}
	});
}

void TSectionWindowContainer::setupSeisButtons(){
auto sData = std::dynamic_pointer_cast<SeismicData>(dataContainer);
auto type = sData ->getType();
if (!(type == DataType::EN_BASE || type == DataType::EN_BASE
 || type == DataType::EN_BASE)) {
	addButton("To Energy", [&](){
	  {
			std::shared_ptr<IBaseData> newData = DataCalculator::toEnergy(std::dynamic_pointer_cast<SeismicData>(dataContainer));
createWindow(newData);
	  }
	});
 }
 addButton("Get Triag Filter", [&](){
		{
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<float>("Частота", stnFilterFreq);
		dialog->AddInput<int>("Ширина фильтра", stnFilterWindow);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<IBaseData> newData = DataCalculator::filter(std::dynamic_pointer_cast<SeismicData>(dataContainer), stnFilterFreq, stnFilterWindow);
		createWindow(newData);
		}
		delete dialog;
		}
	});
 if (!(type == DataType::SWAN || type == DataType::EN_SWAN)) {
	addButton("Get SWAN", [&](){
		{
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<float>("Начальная частота", stnFreqStart);
		dialog->AddInput<float>("Конечная частота", stnFreqStop);
		dialog->AddInput<int>("Число фильтров", stnNumFilters);
		dialog->AddInput<int>("Ширина фильтра", stnFilterWindow);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<IBaseData> newData = DataCalculator::getSwan(std::dynamic_pointer_cast<SeismicData>(dataContainer), posx_, stnFreqStart, stnFreqStop, stnNumFilters, stnFilterWindow);
createWindow(newData, OverlayMapping::Spectrum, posx_);
		}
		delete dialog;
		}
	});
	addButton("Get SWAN as RGB", [&](){
		{
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<float>("Начальная частота", stnFreqStart);
		dialog->AddInput<float>("Конечная частота", stnFreqStop);
		dialog->AddInput<int>("Число фильтров", stnNumFilters);
		dialog->AddInput<int>("Ширина фильтра", stnFilterWindow);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<IBaseData> newData = DataCalculator::getSwanRGB(std::dynamic_pointer_cast<SeismicData>(dataContainer), posx_, stnFreqStart, stnFreqStop, stnNumFilters, stnFilterWindow);
createWindow(newData, OverlayMapping::Spectrum, posx_);
		}
		delete dialog;
		}
	});
	addButton("Make RGB", [&](){
		{
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<float>("Начальная частота", stnFreqStart);
		dialog->AddInput<float>("Конечная частота", stnFreqStop);
		dialog->AddInput<int>("Число фильтров", stnNumFilters);
		dialog->AddInput<int>("Ширина фильтра", stnFilterWindow);
		dialog->AddInput<int>("Ширина окна сглаживания", stnTWindow);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<IBaseData> newData = std::make_shared<RgbData>(std::dynamic_pointer_cast<SeismicData>(dataContainer).get(), stnFreqStart, stnFreqStop, stnNumFilters, stnFilterWindow);
			newData = DataCalculator::smoothT(newData, stnTWindow);
createWindow(newData);
		}
		delete dialog;
		}
	});
 }
 else{
	addButton("RGB from SWAN", [&](){
		{
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Ширина окна сглаживания", stnTWindow);
		if (dialog->Execute() && dialog->ContinuePressed) {
			std::shared_ptr<IBaseData> newData = DataCalculator::SwanToRGB(std::dynamic_pointer_cast<SeismicData>(dataContainer), stnTWindow);
			createWindow(newData, OverlayMapping::Trace, posx_);
		}
		delete dialog;
		}
	});
 }
}
void TSectionWindowContainer::setupRgbButtons(){
addButton("Cluster", [&](){
		{
        TAbstractDialog* dialog = new TAbstractDialog(nullptr);
        int minPoints = 1;
		dialog->AddInput<int>("Минимальный размер кластера", minPoints);
		if (dialog->Execute() && dialog->ContinuePressed) {
            std::shared_ptr<IBaseData> newData = DataCalculator::Cluster(std::dynamic_pointer_cast<RgbData>(dataContainer), minPoints);
            createWindow(newData);
        }
        delete dialog;
		}
	});
    addButton("FreqField", [&](){
		{
			std::shared_ptr<IBaseData> newData = DataCalculator::DominantFrequencyDirection(std::dynamic_pointer_cast<RgbData>(dataContainer));
			createWindow(newData);
		}
	});
    addButton("ClusterDir", [&](){
		{
        TAbstractDialog* dialog = new TAbstractDialog(nullptr);
        int minPoints = 1;
		dialog->AddInput<int>("Минимальный размер кластера", minPoints);
		if (dialog->Execute() && dialog->ContinuePressed) {
            std::shared_ptr<IBaseData> newData = DataCalculator::ClusterDirectionField(std::dynamic_pointer_cast<RgbData>(dataContainer), minPoints);
            createWindow(newData);
        }
        delete dialog;
		}
	});
}

void TSectionWindowContainer::setupButtons()
{
	ICallbackWindow* winCall = dynamic_cast<ICallbackWindow*>(window.get());
	if (winCall != nullptr) {
		switch (type) {
		case SectionWindowType::SEIS:
		setupFlatButtons();
        setupSeisButtons();
		break;
		case SectionWindowType::RGB:
		setupFlatButtons();
        setupRgbButtons();
		break;
		default:
		throw;
		}
		this->PopupMenu = FPopupMenu;
	};
}


void TSectionWindowContainer::updateAxis(bool reEval){
	IFlatWindow* winFlat = dynamic_cast<IFlatWindow*>(window.get());
	if (winFlat) {
		float zoom = winFlat->getZoom();
		int sx, sy;
		float rx,ry;
		winFlat->getSize(sx, sy);
		winFlat->getRatio(rx, ry);
		if (reEval) {
			axisY->createAxe(0, sy*winFlat->getDT(), sy/ry*zoom*winFlat->getDT(), 3);
            axisYR->createAxe(0, sy*winFlat->getDT(), sy/ry*zoom*winFlat->getDT(), 3);
			auto seisCont = std::dynamic_pointer_cast<SeismicData>(dataContainer);
			if (!seisCont) {
			axis->createAxe(0, sx, sx/rx*zoom, 0);
			}
			else if (seisCont->getType() == DataType::SWAN || seisCont->getType() == DataType::EN_SWAN) {
				auto frequens = seisCont->getFreq();
				DynamicArray < TAxeInterval > intervals;
				intervals.Length=2;
				intervals[0].startValue = frequens.front();
				intervals[0].endValue = frequens.back();
				intervals[0].sizeInPixels = viewPanel->Width;
				intervals[1].startValue = frequens.size();
				intervals[1].sizeInPixels = 0;
				axis->createAxe(intervals, 4);
			}
			else{
				axis->createAxe(0, sx, sx/rx*zoom, 0);
			}
		}
		float ox, oy;
		winFlat->getOffset(ox, oy);

		image1->Canvas->Brush->Color = clInfoBk;
		image1->Canvas->FillRect(Rect(0, 0, 50, viewPanel->Height));
		image1->Canvas->Draw(0, (-oy)*zoom*winFlat->getDT()/ry, axisY->getBitmap());
        image3->Canvas->Brush->Color = clInfoBk;
		image3->Canvas->FillRect(Rect(0, 0, 50, viewPanel->Height));
		image3->Canvas->Draw(0, (-oy)*zoom*winFlat->getDT()/ry, axisYR->getBitmap());
		image2->Canvas->Brush->Color = clInfoBk;
		image2->Canvas->FillRect(Rect(0, 0, viewPanel->Width+100, 50));
		image2 ->Canvas->Draw((image1->Visible ? 50 : 0)-ox*zoom/rx,0, axis->getBitmap());
        image4->Canvas->Brush->Color = clInfoBk;
		image4->Canvas->FillRect(Rect(0, 0, viewPanel->Width+100, 50));
	}
}

void TSectionWindowContainer::LoadHorizon(const std::wstring& filename, const std::wstring& fileext){
	IFlatWindow* winFlat = dynamic_cast<IFlatWindow*>(window.get());

	if (!winFlat) {
		return;
	}
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
			stream->Read(horizon.data(), (Longint)(size * sizeof(float)));
        }
    }
    catch (...) {
		// В случае ошибки возвращаем пустой вектор
        ShowMessage("Wrong Format");
    }
	winFlat->setHorizon(horizon);
	window->renderWindow();
}

void TSectionWindowContainer::SaveHorizon(const std::wstring& filename, const std::wstring& fileext){
	IFlatWindow* winFlat = dynamic_cast<IFlatWindow*>(window.get());

	if (!winFlat) {
		return;
	}
	std::vector<float> horizon = winFlat->getHorizon();
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
			stream->Write(horizon.data(), (Longint)(horizon.size() * sizeof(float)));
        }
    }
    catch (...) {
        // Ошибка при сохранении
	}
}

void TSectionWindowContainer::saveFile(std::wstring path){
auto sData = std::dynamic_pointer_cast<SeismicData>(dataContainer);
auto rData = std::dynamic_pointer_cast<RgbData>(dataContainer);
if (sData) {
    sData->saveFile(path+L".sd");
}
if (rData) {
    rData->saveFile(path+L".rgb");
}
}

void TSectionWindowContainer::checkAxis(bool left, bool top, bool right){
	image1->Visible = left;
    image2->Visible = top;
    image3->Visible = right;
    Resize(this);
}

void TSectionWindowContainer::saveScreenshot(System::UnicodeString path){
    if (rgbChangeTimer && rgbChangeTimer->Enabled) ApplyRgbChannels(nullptr);
	IFlatWindow* winFlat = dynamic_cast<IFlatWindow*>(window.get());
	if (!winFlat) return;

    std::unique_ptr<Graphics::TBitmap> result(winFlat->getScreenshotAsBitmap());
    if (!result) return;
    std::unique_ptr<Graphics::TBitmap> fin(new Graphics::TBitmap);
    fin->PixelFormat = pf24bit;
    fin->SetSize(result->Width+100, result->Height+100);

    int sx, sy;
    float rx,ry;
    winFlat->getSize(sx, sy);
    winFlat->getRatio(rx, ry);
    axisY->createAxe(0, sy*winFlat->getDT(), result->Height, 3);
    axisYR->createAxe(0, sy*winFlat->getDT(), result->Height, 3);
    auto seisCont = std::dynamic_pointer_cast<SeismicData>(dataContainer);
    if (!seisCont) {
    axis->createAxe(0, sx, result->Width, 0);
    }
    else if (seisCont->getType() == DataType::SWAN || seisCont->getType() == DataType::EN_SWAN) {
        auto frequens = seisCont->getFreq();
        DynamicArray < TAxeInterval > intervals;
        intervals.Length=2;
        intervals[0].startValue = frequens.front();
        intervals[0].endValue = frequens.back();
        intervals[0].sizeInPixels = result->Width;
        intervals[1].startValue = frequens.size();
        intervals[1].sizeInPixels = 0;
        axis->createAxe(intervals, 4);
    }
    else{
        axis->createAxe(0, sx, result->Width, 0);
    }


    fin->Canvas->Brush->Color = clInfoBk;
    fin->Canvas->FillRect(Rect(0, 0, fin->Width, fin->Height));
    fin->Canvas->FillRect(Rect(0, 0, 50, result->Height));
    fin->Canvas->Draw(0, (image1->Visible ? 50 : 0) , axisY->getBitmap());
    // Рисуем верхнюю рамку (поверх левой в углу)

    fin->Canvas->FillRect(Rect(0, 0, result->Width, 50));
    fin->Canvas->Draw((image1->Visible ? 50 : 0) ,0, axis->getBitmap());
    fin->Canvas->Draw(50,50, result.get());
    fin->Canvas->Draw(result->Width+50, (image1->Visible ? 50 : 0), axisYR->getBitmap());
    updateAxis(true);
System::UnicodeString timestamp = FormatDateTime(L"yyyymmdd_hhnnss", TDateTime::CurrentDateTime());
    fin->SaveToFile(path + timestamp + L".bmp");
}
