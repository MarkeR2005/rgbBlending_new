//---------------------------------------------------------------------------
#include "vcl.h"
#include "InterfacesWindow.h"
#include "InterfacesData.h"
#include "axis.h"
#include "Shaders.h"
#include "Structures.h"

#ifndef WindowContainerH
#define WindowContainerH
//---------------------------------------------------------------------------

enum class WindowType {
	NONE,
	SEIS,
	RGB,
	VOL
};
struct viewParams {
	bool isR, isG, isB;
	int r, g, b;
    float contrast;
};


class TWindowContainer : public TPanel
{
private:
	TMenuItem *NameInfo;
	TMenuItem *PosInfo;

	TImage* image1;
	TImage* image2;
    TImage* image3;
    TImage* image4;

	WindowType type = WindowType::NONE;
	TAxis2* axis = new TAxis2(false);
	TAxis2* axisY = new TAxis2(false);
    TAxis2* axisYR = new TAxis2(false);
	TPanel* viewPanel;
    TButton* updateHorizonsButton = nullptr;
    std::function<void(const std::vector<Horizon>&)> horizonsCallback;

	int posx_ = 0;
	int posy_ = 0;

    void getPos();

	void showPopupMenu(int button, int action, int mode);

    void useFunction(TObject* Sender);

	std::map<TObject*, std::function<void()>> func;

    std::function<void(int)> callback = [](int p){};
    std::function<void(int, int)> callbackDisp = [](int p, int s){};
protected:
	std::unique_ptr<IWindow> window;
	std::shared_ptr<ColorManager> colorManager = std::make_shared<ColorManager>();
	std::shared_ptr<IBaseData> dataContainer;
	std::wstring fileName;

	void __fastcall Resize(TObject* Sender);
    void __fastcall UpdateHorizonsClick(TObject* Sender);
	virtual void __fastcall Loaded();

	void setupCallbacks();
	void setupButtons();
	void setupFlatButtons();
	void setupSeisButtons();
	void setupRgbButtons();
	void setupVolumButtons();
	void createWindow(const std::shared_ptr<IBaseData>& newData);
    void createWindow(const std::shared_ptr<IBaseData>& newData, int pos);
public:
    void setTraceCallback(std::function<void(int)> callback_){callback = callback_;};
    void setDisplayCallback(std::function<void(int, int)> callback_){callbackDisp = callback_;};
    void setHorizonsCallback(std::function<void(const std::vector<Horizon>&)> cb){horizonsCallback = std::move(cb);}
    bool setHorizons(const std::vector<Horizon>& value);
    bool setCrosses(const std::vector<Cross>& value);
	static std::vector<TWindowContainer*> instances_;
	int getFreqSize();
    float getFreqByIndex(int idx);
    void setViewParams(viewParams params);
	TPopupMenu *FPopupMenu = new TPopupMenu(this);
	void updateAxis(bool reEval);
	static void __fastcall emitToAll();
    void setRatio();
	void __fastcall LockClick(TObject* Sender);
	__fastcall TWindowContainer(TComponent* Owner);
	__fastcall ~TWindowContainer();
	void initialize(const std::shared_ptr<IBaseData>& data);
	void initialize();
    void setFile(std::wstring fName) {fileName = fName;};
	void addButton(std::string caption, std::function<void()> function);
    void changePalette();
    void changeShader(GLuint shaderProgram);
	TMenuItem*	syncButton;
    void saveFile(std::wstring path);
	void saveScreenshot(System::UnicodeString path);
    void saveVolScreenshot(System::UnicodeString path);
    void SaveHorizon(const std::wstring& filename, const std::wstring& fileext);
	void LoadHorizon(const std::wstring& filename, const std::wstring& fileext);
    void checkAxis(bool left, bool top, bool right);
};

#endif
