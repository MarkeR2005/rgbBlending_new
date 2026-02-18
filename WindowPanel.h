#include "BaseWindow.h"
#include "SeismicWindow.h"
#include "RgbWindow.h"

#include "IBaseData.h"
#include "SeismicData.h"
#include "RgbData.h"

#include <memory>
#include <Vcl.ExtCtrls.hpp>
#include "axis.h"

#ifndef WindowPanelH
#define WindowPanelH
//---------------------------------------------------------------------------
enum class PanelType{
	SEIS,
	RGB
};
enum class PanelData{
	BASE,
	TRACE,
	SWAN
};

class TWindowPanel : public TPanel
{
private:
	static std::vector<TWindowPanel*> instances_;
	TMenuItem *Draw;
	TMenuItem *Clear;
	void showPopupMenu(int screenX, int screenY);

	bool isDrawing = false;

	void __fastcall LockClick(TObject* Sender);
    void __fastcall onNameClick(TObject* Sender);
	void __fastcall DrawClick(TObject* Sender);
	void __fastcall ClearClick(TObject* Sender);
    void __fastcall updateAxis(bool reeval);
	void setPos (int _posx, int _posy);

	TAxis2* axis = new TAxis2(false);
	TAxis2* axisY = new TAxis2(false);
    TPanel* viewPanel;

protected:
	TMenuItem *NameInfo;
	TMenuItem *PosInfo;
	TMenuItem *Update;
	TMenuItem *Lock;
	bool isEnergy = false;
	int posx = 0, posy = 0;

	std::unique_ptr<BaseWindow> window = std::make_unique<BaseWindow>();
	// = std::make_unique<BaseWindow>();
	void setWindow(std::unique_ptr<BaseWindow> newWindow);

	std::shared_ptr<IBaseData> container;
	//Foundation

	PanelType type;
	PanelData dataType = PanelData::BASE;

	void __fastcall Resize(TObject* Sender);
	virtual void __fastcall Loaded();

public:
	static void emitToAll();
	__fastcall void UpdateClick(TObject* Sender);
	PanelType getType(){return type;};
	void setType(PanelData data_type){dataType=data_type;};
	__fastcall TWindowPanel(TComponent* Owner);
    __fastcall ~TWindowPanel();
	//--
	TPopupMenu *FPopupMenu = new TPopupMenu(this);
	//--
	TImage* image1;
	TImage* image2;
	void changeContrast(float c);
	void LoadHorizon(const std::wstring& filename, const std::wstring& fileext);
	void SaveHorizon(const std::wstring& filename, const std::wstring& fileext);
    std::vector<float> frequens = {};
};
#endif

