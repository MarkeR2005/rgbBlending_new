//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitSimpleDialog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSimpleDialog *SimpleDialog;
//---------------------------------------------------------------------------
__fastcall TSimpleDialog::TSimpleDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TSimpleDialog::ContinueClick(TObject *Sender)
{
    ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TSimpleDialog::CancelClick(TObject *Sender)
{
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
void __fastcall TSimpleDialog::DeleteClick(TObject *Sender)
{
    shouldDelete = true;
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
bool TSimpleDialog::Execute()
{
    return (ShowModal() == mrOk);
}