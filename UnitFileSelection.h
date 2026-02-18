//---------------------------------------------------------------------------

#ifndef UnitFileSelectionH
#define UnitFileSelectionH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
#include <windows.h>
#include <commdlg.h>
#include <Vcl.Dialogs.hpp>

//---------------------------------------------------------------------------
class TFileSelection : public TForm
{
__published:	// IDE-managed Components
	TButton *Button2;
	TButton *Button3;
	TButton *Button1;
	//--
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	//--



private:	// User declarations



public:		// User declarations
	__fastcall TFileSelection(TComponent* Owner);
	//--
};
//---------------------------------------------------------------------------
extern PACKAGE TFileSelection *FileSelection;
//---------------------------------------------------------------------------
#endif
