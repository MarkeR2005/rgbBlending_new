//---------------------------------------------------------------------------

#ifndef UnitFormUniversalH
#define UnitFormUniversalH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "SeismicWindow.h"
#include <Vcl.Menus.hpp>
#include "SeismicData.h"
#include "ColorManager.h"
#include <Vcl.Dialogs.hpp>
#include <Vcl.ExtCtrls.hpp>
#include "WindowContainer.h"
#include "axis.h"
//---------------------------------------------------------------------------
class TFormUniversal : public TForm
{
__published:	// IDE-managed Components
	TMainMenu *MainMenu1;
	TMenuItem *File1;

	TOpenDialog *OpenDialog1;
	TMenuItem *SaveAs;
	TSaveDialog *SaveDialog1;
	TMenuItem *OpenAs;
	TScrollBar *ScrollBar1;
	TPanel *Panel1;
	TMenuItem *SaveAsHor;
	TMenuItem *OpenAsHor;
	TSaveDialog *SaveDialog2;
	TPanel *Panel2;
	TScrollBar *ScrollBarR;
	TPanel *Panel3;
	TCheckBox *CheckBoxR;
	TLabel *LabelR;
	TPanel *Panel4;
	TPanel *Panel5;
	TCheckBox *CheckBoxB;
	TPanel *Panel6;
	TLabel *LabelB;
	TScrollBar *ScrollBarB;
	TPanel *Panel7;
	TCheckBox *CheckBoxG;
	TPanel *Panel8;
	TLabel *LabelG;
	TScrollBar *ScrollBarG;
	TMenuItem *View1;
	TMenuItem *ContrastBar;
	TMenuItem *ControlPanel;
	TMenuItem *LeftAxe;
	TMenuItem *TopAxe;
	TMenuItem *Changepalette1;
	TMenuItem *Changeshader1;
	TMenuItem *SetRatio1;
	TMenuItem *SaveScreenShot1;
	TMenuItem *RightAxe;
	TMenuItem *ShowHorizons;
	TMenuItem *ShowCrosses;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall OpenAsClick(TObject *Sender);
	void __fastcall ScrollBar1Change(TObject *Sender);
	void __fastcall SaveAsF(TObject *Sender);
	void __fastcall OpenAsHorF(TObject *Sender);
	void __fastcall SaveAsHorF(TObject *Sender);
	void __fastcall ContrastBarClick(TObject *Sender);
	void __fastcall ControlPanelClick(TObject *Sender);
	void __fastcall CheckBoxBClick(TObject *Sender);
	void __fastcall ScrollBarRChange(TObject *Sender);
	void __fastcall LeftAxeClick(TObject *Sender);
	void __fastcall TopAxeClick(TObject *Sender);
	void __fastcall Changepalette1Click(TObject *Sender);
	void __fastcall Changeshader1Click(TObject *Sender);
	void __fastcall SetRatio1Click(TObject *Sender);
	void __fastcall SaveScreenShot1Click(TObject *Sender);
	void __fastcall RightAxeClick(TObject *Sender);
	void __fastcall ShowHorizonsClick(TObject *Sender);
	void __fastcall ShowCrossesClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
private:	// User declarations
    System::UnicodeString path = "";
    TTimer* overlayEventsTimer = nullptr;
    void __fastcall PollOverlayEvents(TObject *Sender);
public:		// User declarations
	TWindowContainer* windowContainer = nullptr;
    System::UnicodeString getPath() {return path;};
	__fastcall TFormUniversal(TComponent* Owner, System::UnicodeString path);
	void __fastcall initFromData(std::shared_ptr<IBaseData> data){
		windowContainer->initialize(data);
		int fr_size = windowContainer->getFreqSize();
		if (fr_size < 0) {
			throw "Bad data";
		}
		ScrollBarR->Max = std::max(fr_size-1, 0);
		ScrollBarG->Max = std::max(fr_size-1, 0);
		ScrollBarB->Max = std::max(fr_size-1, 0);
	}
    void __fastcall initForCube(System::UnicodeString path){
    windowContainer->initialize();
    windowContainer->setFile(path.c_str());
    int fr_size = windowContainer->getFreqSize();
		if (fr_size < 0) {
			throw "Bad data";
		}
		ScrollBarR->Max = std::max(fr_size-1, 0);
		ScrollBarG->Max = std::max(fr_size-1, 0);
		ScrollBarB->Max = std::max(fr_size-1, 0);
    };
    bool setHorizons(const std::vector<Horizon>& value){return windowContainer->setHorizons(value);}
    bool setCrosses(const std::vector<Cross>& value){return windowContainer->setCrosses(value);}
    bool setHorizonsCallback(std::function<void(const std::vector<Horizon>&)> cb){
        return windowContainer->setHorizonsCallback(std::move(cb));
    }
    void setTraceCallback(std::function<void(int)> callback){
        windowContainer->setTraceCallback(std::move(callback));
    }
    void setDisplayCallback(std::function<void(int, int)> callback){
        windowContainer->setDisplayCallback(callback);
    }
};
//---------------------------------------------------------------------------
extern PACKAGE TFormUniversal *FormUniversal;
//---------------------------------------------------------------------------
#endif
