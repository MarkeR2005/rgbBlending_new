//---------------------------------------------------------------------------
#include "vcl.h"
#include "InterfacesWindow.h"
#include "InterfacesData.h"
#include "axis.h"

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

	WindowType type = WindowType::NONE;
	TAxis2* axis = new TAxis2(false);
	TAxis2* axisY = new TAxis2(false);
	TPanel* viewPanel;

	int posx_ = 0;
	int posy_ = 0;

    void getPos();

	void showPopupMenu(int button, int action, int mode);

    void useFunction(TObject* Sender);

	std::map<TObject*, std::function<void()>> func;
protected:
	std::unique_ptr<IWindow> window;
	std::shared_ptr<ColorManager> colorManager = std::make_shared<ColorManager>();
	std::shared_ptr<IBaseData> dataContainer;
	std::wstring fileName;

	void __fastcall Resize(TObject* Sender);
	virtual void __fastcall Loaded();

	void setupCallbacks();
	void setupButtons();
	void setupFlatButtons();
	void setupSeisButtons();
//	void setupRgbButtons();
	void setupVolumButtons();
	void createWindow(const std::shared_ptr<IBaseData>& newData);
public:
	static std::vector<TWindowContainer*> instances_;
	int getFreqSize();
    float getFreqByIndex(int idx);
    void setViewParams(viewParams params);
	TPopupMenu *FPopupMenu = new TPopupMenu(this);
	void updateAxis(bool reEval);
	static void __fastcall emitToAll();
	void __fastcall LockClick(TObject* Sender);
	__fastcall TWindowContainer(TComponent* Owner);
	__fastcall ~TWindowContainer();
	void initialize(const std::shared_ptr<IBaseData>& data);
	void initialize();
    void setFile(std::wstring fName) {fileName = fName;};
	void addButton(std::string caption, std::function<void()> function);
	TMenuItem*	syncButton;
    void saveFile(std::wstring path);
};

#endif
