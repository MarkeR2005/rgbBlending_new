//---------------------------------------------------------------------------
#include "WindowPanel.h"

#ifndef SeismicPanelH
#define SeismicPanelH
//---------------------------------------------------------------------------
class TSeismicPanel : public TWindowPanel
{
private:
	TMenuItem *SmoothX;
	TMenuItem *SmoothT;
	TMenuItem *ToEnergy;
	TMenuItem *GetTrace;
	TMenuItem *GetSwan;
    TMenuItem *GetSwanRGB;
	TMenuItem *ToRgb;
    TMenuItem *GetStat;
protected:
public:

	__fastcall void applySmoothingX(TObject* Sender);
	__fastcall void applySmoothingT(TObject* Sender);
	__fastcall void getTraceF(TObject* Sender);
	__fastcall void getSwanF(TObject* Sender);
    __fastcall void getSwanRGBF(TObject* Sender);
	__fastcall void toEnergyF(TObject* Sender);
	__fastcall void toRgbF(TObject* Sender);
	__fastcall void GetStatF(TObject* Sender);
	void setData(std::shared_ptr<SeismicData> data);
	__fastcall TSeismicPanel(TComponent* Owner);
	__fastcall ~TSeismicPanel();
	SeismicData* getContainer(){return static_cast<SeismicData*>(container.get());};
	SeismicWindow* getWindow(){return static_cast<SeismicWindow*>(window.get());};
};
#endif
