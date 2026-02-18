//---------------------------------------------------------------------------
#include "WindowPanel.h"

#ifndef RgbPanelH
#define RgbPanelH
//---------------------------------------------------------------------------
class TRgbPanel : public TWindowPanel
{
private:
	TMenuItem *SmoothX;
	TMenuItem *SmoothT;
	TMenuItem *SmoothF;
	TMenuItem *GetFilter;
	TMenuItem *GetTrace;
	TMenuItem *GetSwan;
	TMenuItem *GetStat;
    TMenuItem *Mute;

	struct ColorControl {
        TCheckBox* CheckBox;
        TScrollBar* ScrollBar;  // Изменено на TScrollBar
        TLabel* Label;
        UnicodeString Name;
    };

    std::vector<ColorControl> ColorControls;

protected:
public:
	void setData(std::shared_ptr<RgbData> data);
	__fastcall TRgbPanel(TComponent* Owner);
	__fastcall ~TRgbPanel();

	__fastcall void applySmoothingX(TObject* Sender);
	__fastcall void applySmoothingT(TObject* Sender);
	__fastcall void applySmoothingF(TObject* Sender);
	__fastcall void getTraceF(TObject* Sender);
	__fastcall void getSwanF(TObject* Sender);
	__fastcall void getFilterF(TObject* Sender);
	__fastcall void GetStatF(TObject* Sender);
    	__fastcall void MuteF(TObject* Sender);
	void CheckBoxClick(bool r, bool g, bool b);
	point3F ScrollBarChange(int r, int g, int b);

	RgbData* getContainer(){return static_cast<RgbData*>(container.get());};
	RgbWindow* getWindow(){return static_cast<RgbWindow*>(window.get());};
};
#endif
