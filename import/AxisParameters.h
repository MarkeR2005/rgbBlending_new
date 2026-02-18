//---------------------------------------------------------------------------

#ifndef AxisParametersH
#define AxisParametersH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//#include <AlexButton.hpp>

#include "axis.h"
#include <Vcl.Mask.hpp>
//---------------------------------------------------------------------------
class TAxisParametersForm : public TForm
{
__published:	// IDE-managed Components
        TFontDialog *FontDialog1;
        TPanel *Panel2;
		TPanel *Panel1;
	TGroupBox *VGB;
	TCheckBox *VAutoCB;
    TLabeledEdit *VLE;
    TUpDown *VMajorTicksUD;
    TCheckBox *VMinorCB;
    TLabel *VExL;
    TGroupBox *HGB;
    TLabel *HExL;
    TCheckBox *HAutoCB;
    TLabeledEdit *HLE;
    TUpDown *HMajorTicksUD;
	TCheckBox *HMinorCB;
	TButton *CancelSB;
	TButton *OKSB;
	TButton *HFSB;
	TButton *VFSB;
        void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall HFSBClick(TObject *Sender);
    void __fastcall VFSBClick(TObject *Sender);
	void __fastcall HAutoCBClick(TObject *Sender);
	void __fastcall VAutoCBClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TAxisParametersForm(TComponent* Owner);
        void init(TAxeParameters *p1, TAxeParameters *p2=NULL);
};
//---------------------------------------------------------------------------
extern PACKAGE TAxisParametersForm *AxisParametersForm;
//---------------------------------------------------------------------------
#endif
