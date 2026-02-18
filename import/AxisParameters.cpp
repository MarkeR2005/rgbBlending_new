//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "AxisParameters.h"
//#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//#pragma link "AlexButton"
#pragma resource "*.dfm"
TAxisParametersForm *AxisParametersForm;
//---------------------------------------------------------------------------

__fastcall TAxisParametersForm::TAxisParametersForm(TComponent* Owner)
        : TForm(Owner)
{
Color=clYellow;
Panel1->Color=clYellow;
Panel2->Color=clBlue;
HAutoCBClick(this);
VAutoCBClick(this);
}
//---------------------------------------------------------------------------

void TAxisParametersForm::init(TAxeParameters *p1, TAxeParameters *p2)
{
HExL->Font->Assign(p1->axeFont);
HAutoCB->Checked=p1->automatic;
HMinorCB->Checked=p1->showMinorTicks;
HMajorTicksUD->Position=p1->majorPicksInterval;
if (p2) {
    ClientWidth=585;
    VGB->Enabled=true;
    VExL->Font->Assign(p2->axeFont);
    VAutoCB->Checked=p2->automatic;
    VMinorCB->Checked=p2->showMinorTicks;
    VMajorTicksUD->Position=p2->majorPicksInterval;
} else {
    HGB->Caption=EmptyStr;
    ClientWidth=310;
}
}
//---------------------------------------------------------------------------

void __fastcall TAxisParametersForm::FormKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
if (Key==VK_RETURN) OKSB->Click();
if (Key==VK_ESCAPE) CancelSB->Click();
}
//---------------------------------------------------------------------------

void __fastcall TAxisParametersForm::HFSBClick(TObject *Sender)
{
FontDialog1->Font->Assign(HExL->Font);
if (FontDialog1->Execute()) HExL->Font->Assign(FontDialog1->Font);
}
//---------------------------------------------------------------------------

void __fastcall TAxisParametersForm::VFSBClick(TObject *Sender)
{
FontDialog1->Font->Assign(VExL->Font);
if (FontDialog1->Execute()) VExL->Font->Assign(FontDialog1->Font);
}
//---------------------------------------------------------------------------

void __fastcall TAxisParametersForm::HAutoCBClick(TObject *Sender)
{
HLE->Enabled=!HAutoCB->Checked;
HMajorTicksUD->Enabled=!HAutoCB->Checked;
HMinorCB->Enabled=!HAutoCB->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TAxisParametersForm::VAutoCBClick(TObject *Sender)
{
VLE->Enabled=!VAutoCB->Checked;
VMajorTicksUD->Enabled=!VAutoCB->Checked;
VMinorCB->Enabled=!VAutoCB->Checked;
}
//---------------------------------------------------------------------------

