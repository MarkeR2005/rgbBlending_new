//---------------------------------------------------------------------------

#ifndef UnitSimpleDialogH
#define UnitSimpleDialogH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSimpleDialog : public TForm
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TLabel *Description;
	TPanel *Panel2;
	TButton *Continue;
	TButton *Delete;
	TButton *Cancel;
	void __fastcall ContinueClick(TObject *Sender);
	void __fastcall CancelClick(TObject *Sender);
	void __fastcall DeleteClick(TObject *Sender);
private:	// User declarations


public:		// User declarations
	__fastcall TSimpleDialog(TComponent* Owner);
    bool Execute();
    bool shouldDelete = false;
    void setDescription(System::UnicodeString desc){Description->Caption = desc;};
};
//---------------------------------------------------------------------------
extern PACKAGE TSimpleDialog *SimpleDialog;
//---------------------------------------------------------------------------
#endif
