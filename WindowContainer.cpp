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

	image1 = new TImage(this);
	image1->Height = 1440;
	image1->Width = 50;
	image1 -> Parent = this;
	image1 ->Align = alLeft;
	image1 ->Visible = false;
	image1 ->Enabled = false;

	image2 = new TImage(this);
	image2->Height = 50;
	image2->Width = 2560;
	image2 -> Parent = this;
	image2 ->Align = alTop;
	image2 ->Visible = false;
	image2 ->Enabled = false;

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

void TWindowContainer::initialize(const std::shared_ptr<IBaseData>& data){
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
		window = std::move(window_);
		type = WindowType::SEIS;

	}
	else if (rData) {

		std::unique_ptr<RgbWindow> window_ = std::make_unique<RgbWindow>();
		if (rData->getSize().x == 1) {
			window_->setThin();
		}
		window_->initWindow(viewPanel);
        std::vector<bitMap> tex = rData->getTexture();
//        std::this_thread::sleep_for(std::chrono::seconds(1));
		window_->initTexture(tex);
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
	dataContainer = data;
	setupCallbacks();
	setupButtons();
	window->resizeWindow(this->Width,this->Height);
	updateAxis(true);
	NameInfo->Caption = data->getName().c_str();
}



void TWindowContainer::initialize(){
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
	syncButton->Enabled=false;
    syncButton->Visible=false;
	setupCallbacks();
    setupButtons();
	window->resizeWindow(this->Width,this->Height);
	updateAxis(true);
}
int TWindowContainer::getFreqSize(){
	if (!dataContainer) {
		return -1;
	}
	if (std::dynamic_pointer_cast<SeismicData>(dataContainer)) {
		return 0;
	}
	std::shared_ptr<RgbData> rd = std::dynamic_pointer_cast<RgbData>(dataContainer);
	if (rd) {
		return rd->getSize().f;
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
	return -2;
}

__fastcall TWindowContainer::~TWindowContainer(){
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
	window->resizeWindow(this->Width,this->Height);
	updateAxis(true);
}
void TWindowContainer::setViewParams(viewParams params){
	IRgbWindow* wd = dynamic_cast<IRgbWindow*>(window.get());
	FlatWindow* wd1 = dynamic_cast<FlatWindow*>(window.get());
	if (wd != nullptr) {
		wd->setColor(params.r,params.g,params.b);
		wd->setView(params.isR, params.isG, params.isB);
	}
	if (wd1 != nullptr) {
		wd1->setContrast(params.contrast);
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
				updateAxis(false);});
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
				updateAxis(false);});
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
	form -> Show();
	form -> initFromData(std::move(newData));
}

void TWindowContainer::setupFlatButtons(){
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
createWindow(newData);
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
createWindow(newData);
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
createWindow(newData);
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
void TWindowContainer::setupVolumButtons(){
//	addButton("Add Slice", [&](){
//		{
//		int num = 0;
//		TAbstractDialog* dialog = new TAbstractDialog(nullptr);
//		dialog->AddInput<int>("Номер", num);
//		if (dialog->Execute() && dialog->ContinuePressed) {
//			bool success = false;
//			try {
//					Traces tr = readTimeSliceRegular(fileName+L"s", num);
//					seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber);
//					glm::vec3 = glm::vec3(0.0f, 1.0f, 0.0f);
//					glm::vec3 = glm::vec3(0.0f, (float)sz.samples/2-num, 0.0f);
//					glm::vec2 = glm::vec2((float)sz.lines, (float)sz.traces);
//			}
//		}
//		delete dialog;
//		}
	//});
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
		setupFlatButtons();;
		break;
		case WindowType::VOL:
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
		winFlat->getSize(sx, sy);
		if (reEval) {
			axisY->createAxe(0, sy*winFlat->getDT(), sy*zoom*winFlat->getDT(), 3);
			auto seisCont = std::dynamic_pointer_cast<SeismicData>(dataContainer);
			if (!seisCont) {
			axis->createAxe(0, sx, sx*zoom, 0);
			}
			else if (seisCont->getType() == DataType::SWAN || seisCont->getType() == DataType::EN_SWAN) {
				float rx, ry;
				auto frequens = seisCont->getFreq();
				winFlat->getRatio(rx, ry);
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
				axis->createAxe(0, sx, sx*zoom, 0);
			}
		}
		float ox, oy;
		winFlat->getOffset(ox, oy);


		image1->Canvas->Brush->Color = clWhite;
		image1->Canvas->FillRect(Rect(0, 0, 50, viewPanel->Height));
		image1->Canvas->Draw(0, (-oy)*zoom*winFlat->getDT(), axisY->getBitmap());
		image2->Canvas->Brush->Color = clWhite;
		image2->Canvas->FillRect(Rect(0, 0, viewPanel->Width+50, 50));
		image2 ->Canvas->Draw((image1->Visible ? 50 : 0)-ox*zoom,0, axis->getBitmap());
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
