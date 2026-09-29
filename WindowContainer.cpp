//---------------------------------------------------------------------------

#pragma hdrstop
#include "SeismicWindow.h"
#include "RgbWindow.h"
#include "ViewportWindow.h"
#include "WindowContainer.h"
#include "SeismicData.h"
#include "RgbData.h"
#include "DataCalculator.h"
#include "UnitFormUniversal.h"
#include "DialogWrapper.h"
#include "Reader.h"
#include "SeismicPlane.h"
#include "RgbPlane.h"
#include <Vcl.ComCtrls.hpp>
#include <thread>
#include <chrono>
//---------------------------------------------------------------------------
#pragma package(smart_init)





std::vector<TWindowContainer*> TWindowContainer::instances_;
__fastcall TWindowContainer::TWindowContainer(TComponent* Owner) : TPanel(Owner){
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

	instances_.push_back(this);

	this->PopupMenu = FPopupMenu;
}


void __fastcall TWindowContainer::Loaded(){};
void __fastcall TWindowContainer::emitToAll(){
	for (auto windowContainer : instances_) {
		windowContainer->updateAxis(true);
	}
}
void __fastcall TWindowContainer::LockClick(TObject* Sender){
	syncButton->Checked = !syncButton->Checked;
	auto SyncWindow = dynamic_cast<ISyncWindow*>(window.get());
	if (SyncWindow) {
	   SyncWindow->setSync(syncButton->Checked);
	}
}
void TWindowContainer::addButton(std::string caption, std::function<void()> function){
	TMenuItem *item = new TMenuItem(FPopupMenu);
	item->Caption = caption.c_str();
	FPopupMenu->Items->Add(item);
	func[item] = function;
	item->OnClick=useFunction;
}
void TWindowContainer::useFunction(TObject* Sender){
if (auto sch = func.find(Sender); sch != func.end()) sch->second();
Application->ProcessMessages();
}
void TWindowContainer::changePalette(){
    if (type != WindowType::SEIS) {
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
void TWindowContainer::changeShader(GLuint shaderProgram){
    IFlatWindow* flatWin = dynamic_cast<IFlatWindow*>(window.get());
    if (flatWin != nullptr) {
        flatWin->setShaderProgram(shaderProgram);
    }
}
bool TWindowContainer::setHorizons(const std::vector<Horizon>& value) {
    FlatWindow* flat = dynamic_cast<FlatWindow*>(window.get());
    if (!flat) return false;
    flat->setHorizons(value);
    return true;
}
bool TWindowContainer::setCrosses(const std::vector<Cross>& value) {
    FlatWindow* flat = dynamic_cast<FlatWindow*>(window.get());
    if (!flat) return false;
    flat->setCrosses(value);
    return true;
}
void __fastcall TWindowContainer::UpdateHorizonsClick(TObject*) {
    FlatWindow* flat = dynamic_cast<FlatWindow*>(window.get());
    if (flat && horizonsCallback) {
        const auto snapshot = flat->getHorizons();
        horizonsCallback(snapshot);
    }
}
void TWindowContainer::initialize(const std::shared_ptr<IBaseData>& data){
    rgbChangeTimer->Enabled = false;
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
		type = WindowType::SEIS;

	}
	else if (rData) {
		std::unique_ptr<RgbWindow> window_ = std::make_unique<RgbWindow>();
		if (rData->getSize().x == 1) {
			window_->setThin();
		}
		window_->initWindow(viewPanel);
//        std::this_thread::sleep_for(std::chrono::seconds(1));
		window_->initData(rData);
        window_->setDT(rData->getDT()/1000.0f);
        type = WindowType::RGB;
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
	dataContainer = data;
    updateHorizonsButton->Visible = true;
    setupCallbacks();
	setupButtons();
	window->resizeWindow(viewPanel->Width, viewPanel->Height);
	updateAxis(true);
	NameInfo->Caption = data->getName().c_str();
}



void TWindowContainer::initialize(){
    rgbChangeTimer->Enabled = false;
	std::unique_ptr<ViewportWindow>window_ = std::make_unique<ViewportWindow>();
	window_->initWindow(viewPanel);
//    seismicShader = createShaderProgram("volumetric", "seismic");
//	rgbShader = createShaderProgram("volumetric", "rgb");
		window_->setCamera(
		glm::vec3(1000.0f, 0.0f, 1000.0f),  // позиция камеры
		glm::vec3(0.0f, 0.0f, 0.0f),  // цель камеры
		glm::vec3(0.0f, 1.0f, 0.0f)   // вектор "вверх"
	);

	window_->setPerspective(45.0f, 0.1f, 10000.0f);
	window = std::move(window_);
	type = WindowType::VOL;
    updateHorizonsButton->Visible = false;
	syncButton->Enabled=false;
    syncButton->Visible=false;
	setupCallbacks();
    setupButtons();
	window->resizeWindow(this->Width,this->Height);
	updateAxis(true);
}
int TWindowContainer::getFreqSize(){
	if (std::dynamic_pointer_cast<SeismicData>(dataContainer)) {
		return 0;
	}
	std::shared_ptr<RgbData> rd = std::dynamic_pointer_cast<RgbData>(dataContainer);
	if (rd) {
		return rd->getSize().f;
	}
    IVolumeWindow* wnd = dynamic_cast<IVolumeWindow*>(window.get());
    if (wnd) {
        if (ExtractFileExt(fileName.c_str()) == L".ibm") {
            return 0;
        }
        else{
        	std::vector<float> frequens;
            size4 sz_ = readSizeRGB(fileName, frequens);
            return sz_.filters;
        }
    }
    if (!dataContainer) {
		return -1;
	}
	return -2;
}
float TWindowContainer::getFreqByIndex(int idx){
	if (idx < 0 || idx > getFreqSize()-1) {
		return -1;
	}
	std::shared_ptr<RgbData> rd = std::dynamic_pointer_cast<RgbData>(dataContainer);
	if (rd) {
		return rd->getFreqs()[idx];
	}
    IVolumeWindow* wnd = dynamic_cast<IVolumeWindow*>(window.get());
    if (wnd) {
        if (ExtractFileExt(fileName.c_str()) == L".ibm") {
            return -1;
        }
        else{
            std::vector<float> frequens;
            size4 sz_ = readSizeRGB(fileName, frequens);
            return frequens[idx];
        }
    }
	return -2;
}

__fastcall TWindowContainer::~TWindowContainer(){
    if (rgbChangeTimer) rgbChangeTimer->Enabled = false;
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


void __fastcall TWindowContainer::Resize(TObject* Sender){
	if (!window) {
		return;
    }
	window->resizeWindow(viewPanel->Width, viewPanel->Height);
	updateAxis(true);
}
void __fastcall TWindowContainer::ApplyRgbChannels(TObject*) {
    rgbChangeTimer->Enabled = false;
    auto* rgb = dynamic_cast<RgbWindow*>(window.get());
    if (!rgb) return;
    rgb->setColor(pendingR, pendingG, pendingB);
    appliedR = pendingR; appliedG = pendingG; appliedB = pendingB;
    rgb->renderWindow();
}
void TWindowContainer::setViewParams(viewParams params){
	IRgbWindow* wd = dynamic_cast<IRgbWindow*>(window.get());
	FlatWindow* wd1 = dynamic_cast<FlatWindow*>(window.get());
    IVolumeWindow* wd2 = dynamic_cast<IVolumeWindow*>(window.get());
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
    if (wd2 != nullptr) {
		wd2->setChannels(params.r,params.g,params.b);
		wd2->setChannelEnabled(params.isR, params.isG, params.isB);
	}
    auto* rgb = dynamic_cast<RgbWindow*>(window.get());
    if (rgb && rgbChangeTimer->Enabled &&
        rgb->hasCachedLayers(pendingR, pendingG, pendingB)) {
        ApplyRgbChannels(nullptr);
    } else {
        window->renderWindow();
    }
}
void TWindowContainer::setRatio(){
	FlatWindow* wd1 = dynamic_cast<FlatWindow*>(window.get());
	if (wd1 != nullptr) {
		float rx = 1.0f;
		float ry = 1.0f;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<float>("Сжатие по X", rx);
		dialog->AddInput<float>("Сжатие по T", ry);
		if (dialog->Execute() && dialog->ContinuePressed) {
		wd1->setRatio(rx,ry);
        updateAxis(true);
		}
	}
    window->renderWindow();
}

void TWindowContainer::showPopupMenu(int button, int action, int mode)
{
	if (PopupMenu && PopupMenu->Items->Count > -1 && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		POINT pt;
		GetCursorPos(&pt);
        getPos();
		PopupMenu->Popup(pt.x, pt.y);
	}
}

void TWindowContainer::getPos(){
	if (type == WindowType::SEIS || type == WindowType::RGB) {
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
				PosInfo->Caption = (std::to_string(sData->getFreq()[posx_]) + ";" + std::to_string(posy_)).c_str();
				return;
			}
		}
		PosInfo->Caption = (std::to_string(posx_) + ";" + std::to_string(posy_)).c_str();
	}
}


void TWindowContainer::setupCallbacks()
{
	ICallbackWindow* winCall = dynamic_cast<ICallbackWindow*>(window.get());
	if (winCall != nullptr) {
		switch (type) {
		case WindowType::SEIS:
			winCall->setMouseButtonCallback(showPopupMenu);
			winCall->setScrollCallback([this](double ox, double oy){
			if (syncButton->Checked) {
					TWindowContainer::emitToAll();
					return;
				}
				updateAxis(true);});
			winCall->setCursorPosCallback([this](double ox, double oy){
			if (syncButton->Checked) {
					TWindowContainer::emitToAll();
					return;
				}
				updateAxis(false);
                getPos();
                callback(posx_);
                IFlatWindow* flatWin = dynamic_cast<IFlatWindow*>(window.get());
                float dT = flatWin->getDT();
                callbackDisp(posx_, posy_*dT);
                });
		break;
		case WindowType::RGB:
			winCall->setMouseButtonCallback(showPopupMenu);
			winCall->setScrollCallback([this](double ox, double oy){
			if (syncButton->Checked) {
					emitToAll();
					return;
				}
				updateAxis(true);});
			winCall->setCursorPosCallback([this](double ox, double oy){
			if (syncButton->Checked) {
					emitToAll();
					return;
				}
				updateAxis(false);
                getPos();
                callback(posx_);
                IFlatWindow* flatWin = dynamic_cast<IFlatWindow*>(window.get());
                float dT = flatWin->getDT();
                callbackDisp(posx_, posy_*dT);
                });
		break;
		case WindowType::VOL:
			winCall->setMouseButtonCallback(showPopupMenu);
		break;
		default:
		throw;
		}
	}
}

void TWindowContainer::createWindow(const std::shared_ptr<IBaseData>& newData){
	TFormUniversal* form = new TFormUniversal (Application->MainForm, dynamic_cast<TFormUniversal*>(Parent)->getPath());
    form->setTraceCallback(callback);
	form -> Show();
	form -> initFromData(std::move(newData));
}
void TWindowContainer::createWindow(const std::shared_ptr<IBaseData>& newData, int pos){
	TFormUniversal* form = new TFormUniversal (Application->MainForm, dynamic_cast<TFormUniversal*>(Parent)->getPath());
    auto clb = [&](int input){
        callback(pos);
    };
    form->setTraceCallback(clb);
	form -> Show();
	form -> initFromData(std::move(newData));
}

void TWindowContainer::setupFlatButtons(){
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
        for (size_t i = 0; i < horizons.size(); ++i) {
            const std::wstring name = horizons[i].name.empty()
                ? L"Горизонт " + std::to_wstring(i+1) : horizons[i].name;
            tree->Items->AddChild(root, name.c_str());
        }
        TTreeNode* createNode = tree->Items->AddChild(root, L"+ Новый горизонт");
        root->Expand(true);
        tree->Selected = horizons.empty() ? createNode : tree->Items->Item[1];

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
createWindow(newData, posx_);
//                    ShowMessage("ok");
		}
	});
}

void TWindowContainer::setupSeisButtons(){
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
createWindow(newData, posx_);
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
createWindow(newData, posx_);
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
			createWindow(newData);
		}
		delete dialog;
		}
	});
 }
}
void TWindowContainer::setupRgbButtons(){
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

void TWindowContainer::setupVolumButtons(){
//GLuint SEISMIC_SHADER = createShaderProgram("volumetric", "seismic");
//GLuint RGB_SHADER = createShaderProgram("volumetric", "rgb");
	addButton("Add slice", [&](){
		int num = 0;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Номер", num);
        if (dialog->Execute() && dialog->ContinuePressed) {
            bool success = false;
            glm::vec3 norm = glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 pos;
            glm::vec2 _sz = glm::vec2(1.0f, 1.0f);
            if (ExtractFileExt(fileName.c_str()) == L".ibm") {
                size sz = readSize(fileName);
                std::shared_ptr<SeismicData> seismicData;
				bool flipV = false, flipH = false;
                flipV=true;
				Traces tr = readTimeSliceRegular(ChangeFileExt(fileName.c_str(), L".sbm").c_str(), num);
				seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);

                norm = glm::vec3(0.0f, 1.0f, 0.0f);
                pos = glm::vec3(0.0f, (float)sz.samples/2-num, 0.0f);
                _sz = glm::vec2((float)sz.lines, (float)sz.traces);
                if (!seismicData->getCM()) {
                    seismicData->setCM(colorManager);
                }
                auto seismicPlane = std::make_shared<SeismicPlane>(
					pos,  // позиция
					norm,   // нормаль (смотрит вперед)
					_sz,         // размер
					createShaderProgram("volumetric", "seismic"),
					[fName = fileName, num](){Traces tr = readTimeSliceRegular(ChangeFileExt(fName.c_str(), L".sbm").c_str(), num);return std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);},
					flipV,
					flipH
				);
				seismicPlane->updateTexture(seismicData->getTexture());
				seismicPlane->initPaletteTexture(PALETTE);
				(dynamic_cast<IVolumeWindow*>(window.get()))->addPlane(seismicPlane);
            }
            else {
            	std::vector<float> frequens;
                size4 sz = readSizeRGB(fileName, frequens);
                std::shared_ptr<RgbData> rgbData;
				bool flipV = false, flipH = false;
                flipV = true;
				rgbData = readTimeSliceRGB(ChangeFileExt(fileName.c_str(), L".srgb").c_str(), num);
                norm = glm::vec3(0.0f, 1.0f, 0.0f);
                pos = glm::vec3(0.0f, (float)sz.samples/2-num, 0.0f);
                _sz = glm::vec2((float)sz.lines, (float)sz.traces);
                if (!rgbData->getCM()) {
                    rgbData->setCM(colorManager);
                }
                auto rgbPlane = std::make_shared<RgbPlane>(
					pos,  // позиция
					norm,   // нормаль (смотрит вперед)
					_sz,         // размер
					createShaderProgram("volumetric", "rgb"),
					[fName = fileName, num](){return readTimeSliceRGB(ChangeFileExt(fName.c_str(), L".srgb").c_str(), num);},
					flipV,
                    flipH
				);
				rgbPlane->initTextureArray(rgbData->getTexture());
				(dynamic_cast<IVolumeWindow*>(window.get()))->addPlane(rgbPlane);
            }
        }
	});
    addButton("Add inline", [&](){
		int num = 0;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Номер", num);
        if (dialog->Execute() && dialog->ContinuePressed) {
            bool success = false;
            glm::vec3 norm = glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 pos;
            glm::vec2 _sz = glm::vec2(1.0f, 1.0f);
            if (ExtractFileExt(fileName.c_str()) == L".ibm") {
                size sz = readSize(fileName);
                std::shared_ptr<SeismicData> seismicData;
				bool flipV = false, flipH = false;
				Traces tr = readInlineRegular(fileName, num);
				seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);

					pos = glm::vec3(0.0f, 0.0f, (float)sz.lines/2-num);
					_sz = glm::vec2((float)sz.traces, (float)sz.samples);
					flipV=true;
                if (!seismicData->getCM()) {
                    seismicData->setCM(colorManager);
                }
                auto seismicPlane = std::make_shared<SeismicPlane>(
					pos,  // позиция
					norm,   // нормаль (смотрит вперед)
					_sz,         // размер
					createShaderProgram("volumetric", "seismic"),
					[fName = fileName, num](){Traces tr = readInlineRegular(fName, num);return std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);},
					flipV,
                    flipH
				);
				seismicPlane->updateTexture(seismicData->getTexture());
				seismicPlane->initPaletteTexture(PALETTE);
				(dynamic_cast<IVolumeWindow*>(window.get()))->addPlane(seismicPlane);
            }
            else {
            	std::vector<float> frequens;
                size4 sz = readSizeRGB(fileName, frequens);
                std::shared_ptr<RgbData> rgbData;
				bool flipV = false, flipH = false;
				rgbData = readInlineRGB(fileName, num);
                pos = glm::vec3(0.0f, 0.0f, (float)sz.lines/2-num);
                _sz = glm::vec2((float)sz.traces, (float)sz.samples);
                flipV=true;
                if (!rgbData->getCM()) {
                    rgbData->setCM(colorManager);
                }
                auto rgbPlane = std::make_shared<RgbPlane>(
					pos,  // позиция
					norm,   // нормаль (смотрит вперед)
					_sz,         // размер
					createShaderProgram("volumetric", "rgb"),
					[fName = fileName, num](){return readInlineRGB(fName, num);},
					flipV,
                    flipH
				);
				rgbPlane->initTextureArray(rgbData->getTexture());
				(dynamic_cast<IVolumeWindow*>(window.get()))->addPlane(rgbPlane);
            }
        }
	});
    addButton("Add crossline", [&](){
		int num = 0;
		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
		dialog->AddInput<int>("Номер", num);
        if (dialog->Execute() && dialog->ContinuePressed) {
            bool success = false;
            glm::vec3 norm = glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 pos;
            glm::vec2 _sz = glm::vec2(1.0f, 1.0f);
            if (ExtractFileExt(fileName.c_str()) == L".ibm") {
                size sz = readSize(fileName);
                std::shared_ptr<SeismicData> seismicData;
				bool flipV = false, flipH = false;
				Traces tr = readXlineRegular(fileName, num);
                //Traces tr = readInlineRegular(ChangeFileExt(fileName.c_str(), L".xbm").c_str(), num);
				seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);

					norm = glm::vec3(1.0f, 0.0f, 0.0f);
					pos = glm::vec3((float)sz.traces/2-num, 0.0f, 0.0f);
					_sz = glm::vec2((float)sz.lines, (float)sz.samples);
					flipV=true;
                    flipH=true;
                if (!seismicData->getCM()) {
                    seismicData->setCM(colorManager);
                }
				auto seismicPlane = std::make_shared<SeismicPlane>(
					pos,  // позиция
					norm,   // нормаль (смотрит вперед)
					_sz,         // размер
					createShaderProgram("volumetric", "seismic"),
					[fName = fileName, num](){Traces tr = readXlineRegular(fName, num);return std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);},
					flipV,
					flipH
				);
				seismicPlane->updateTexture(seismicData->getTexture());
				seismicPlane->initPaletteTexture(PALETTE);
				(dynamic_cast<IVolumeWindow*>(window.get()))->addPlane(seismicPlane);
            }
            else {
            	std::vector<float> frequens;
                size4 sz = readSizeRGB(fileName, frequens);
                std::shared_ptr<RgbData> rgbData;
				bool flipV = false, flipH = false;
				rgbData = readCrosslineRGB(fileName, num);
                norm = glm::vec3(1.0f, 0.0f, 0.0f);
                pos = glm::vec3((float)sz.traces/2-num, 0.0f, 0.0f);
				_sz = glm::vec2((float)sz.lines, (float)sz.samples);
				flipV=true;
				flipH=true;
                if (!rgbData->getCM()) {
                    rgbData->setCM(colorManager);
                }
                auto rgbPlane = std::make_shared<RgbPlane>(
					pos,  // позиция
					norm,   // нормаль (смотрит вперед)
					_sz,         // размер
					createShaderProgram("volumetric", "rgb"),
					[fName = fileName, num](){return readCrosslineRGB(fName, num);},
					flipV,
                    flipH
				);
				rgbPlane->initTextureArray(rgbData->getTexture());
				(dynamic_cast<IVolumeWindow*>(window.get()))->addPlane(rgbPlane);
            }
        }
	});
    addButton("Delete panel", [&](){
        IVolumeWindow* viewport = dynamic_cast<IVolumeWindow*>(window.get());
    	viewport->removePlane(viewport->getSelectedPlane());
		window->renderWindow();
	});
	addButton("Show panel", [&](){
		IVolumeWindow* viewport = dynamic_cast<IVolumeWindow*>(window.get());
		auto sel = viewport->getSelectedPlane();
		std::shared_ptr<IBaseData> newData = sel->getData();
		createWindow(newData);
	});
    addButton("Set Transparency", [&](){
        float num = 0;
        TAbstractDialog* dialog = new TAbstractDialog(nullptr);
        dialog->AddInput<float>("Прозрачность", num);
        if (dialog->Execute() && dialog->ContinuePressed) {
        	IVolumeWindow* viewport = dynamic_cast<IVolumeWindow*>(window.get());
            viewport->getSelectedPlane()->setTransparency(num/100.0f);
            window->renderWindow();
        }
        delete dialog;
    });
}

void TWindowContainer::setupButtons()
{
	ICallbackWindow* winCall = dynamic_cast<ICallbackWindow*>(window.get());
	if (winCall != nullptr) {
		switch (type) {
		case WindowType::SEIS:
		setupFlatButtons();
        setupSeisButtons();
		break;
		case WindowType::RGB:
		setupFlatButtons();
        setupRgbButtons();
		break;
		case WindowType::VOL:
        setupVolumButtons();
		break;
		default:
		throw;
		}
		this->PopupMenu = FPopupMenu;
	};
}


void TWindowContainer::updateAxis(bool reEval){
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

void TWindowContainer::LoadHorizon(const std::wstring& filename, const std::wstring& fileext){
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

void TWindowContainer::SaveHorizon(const std::wstring& filename, const std::wstring& fileext){
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

void TWindowContainer::saveFile(std::wstring path){
auto sData = std::dynamic_pointer_cast<SeismicData>(dataContainer);
auto rData = std::dynamic_pointer_cast<RgbData>(dataContainer);
if (sData) {
    sData->saveFile(path+L".sd");
}
if (rData) {
    rData->saveFile(path+L".rgb");
}
}

void TWindowContainer::checkAxis(bool left, bool top, bool right){
	image1->Visible = left;
    image2->Visible = top;
    image3->Visible = right;
    Resize(this);
}

void TWindowContainer::saveVolScreenshot(System::UnicodeString path){
	IVolumeWindow* winFlat = dynamic_cast<IVolumeWindow*>(window.get());
	if (!winFlat) {
		return;
	}

}

void TWindowContainer::saveScreenshot(System::UnicodeString path){
    if (rgbChangeTimer && rgbChangeTimer->Enabled) ApplyRgbChannels(nullptr);
	IFlatWindow* winFlat = dynamic_cast<IFlatWindow*>(window.get());
	if (!winFlat) {
		return saveVolScreenshot(path);
	}

    Graphics::TBitmap* result = winFlat->getScreenshotAsBitmap();
    Graphics::TBitmap* fin = new TBitmap();
    fin->Width = result->Width+100;
    fin->Height = result->Height+100;

    int sx, sy;
    float rx,ry;
    winFlat->getSize(sx, sy);
    winFlat->getRatio(rx, ry);
    axisY->createAxe(0, sy*winFlat->getDT(), sy/ry*winFlat->getDT(), 3);
    axisYR->createAxe(0, sy*winFlat->getDT(), sy/ry*winFlat->getDT(), 3);
    auto seisCont = std::dynamic_pointer_cast<SeismicData>(dataContainer);
    if (!seisCont) {
    axis->createAxe(0, sx, sx/rx, 0);
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
        axis->createAxe(0, sx, sx/rx, 0);
    }


    fin->Canvas->Brush->Color = clInfoBk;
    fin->Canvas->FillRect(Rect(0, 0, 50, result->Height));
    fin->Canvas->Draw(0, (image1->Visible ? 50 : 0) , axisY->getBitmap());
    // Рисуем верхнюю рамку (поверх левой в углу)

    fin->Canvas->FillRect(Rect(0, 0, result->Width, 50));
    fin->Canvas->Draw((image1->Visible ? 50 : 0) ,0, axis->getBitmap());
    fin->Canvas->Draw(50,50, result);
    fin->Canvas->Draw(result->Width+50, (image1->Visible ? 50 : 0), axisYR->getBitmap());
    updateAxis(true);
//    delete mainBitmap;
System::UnicodeString timestamp = FormatDateTime(L"yyyymmdd_hhnnss", TDateTime::CurrentDateTime());
    fin->SaveToFile(path + timestamp + L".bmp");
}
