	//---------------------------------------------------------------------------

	#ifndef UnitSeismic3dH
	#define UnitSeismic3dH
	//---------------------------------------------------------------------------
	#include <System.Classes.hpp>
	#include <Vcl.Controls.hpp>
	#include <Vcl.StdCtrls.hpp>
	#include <Vcl.Forms.hpp>
	#include <Vcl.ExtCtrls.hpp>
	#include <Vcl.Dialogs.hpp>
	#include <Vcl.Menus.hpp>
	#include "Reader.h"
	#include <vector>
	#include <string>
	#include "Shaders.h"
	#include "ColorManager.h"
	#include "ViewportWindow.h"
	//---------------------------------------------------------------------------
	class TSeismic3d : public TForm
	{
	__published:	// IDE-managed Components
		TPanel *Panel1;
		TMainMenu *MainMenu1;
		TOpenDialog *OpenDialog1;
		TMenuItem *File1;
		TMenuItem *File2;
	TMenuItem *AddSlice;
	TMenuItem *AddInline;
	TMenuItem *AddCrossline;
	TPopupMenu *PopupMenu1;
	TMenuItem *DeletePlane1;
	TMenuItem *NewWindowPlane;
	TMenuItem *AddInline1;
	TMenuItem *AddCrossline1;
	TMenuItem *AddSlice1;
	TPanel *Panel2;
	TPanel *Panel3;
	TCheckBox *CheckBoxR;
	TPanel *Panel4;
	TLabel *LabelR;
	TScrollBar *ScrollBarR;
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
	TMenuItem *View2;
	TMenuItem *SetTransparency1;
		void __fastcall Panel1Resize(TObject *Sender);
	void __fastcall File2Click(TObject *Sender);
	void __fastcall AddClick(TObject *Sender);
	void __fastcall DeletePlane1Click(TObject *Sender);
	void __fastcall View2Click(TObject *Sender);
    void __fastcall ScrollBar1Change(TObject *Sender);
	void __fastcall CheckBoxRClick(TObject *Sender);
	void __fastcall SetTransparency1Click(TObject *Sender);
	private:	// User declarations
	GLuint seismicShader;
	GLuint rgbShader;
	std::shared_ptr<ColorManager> cm = std::make_shared<ColorManager>();
	std::shared_ptr<ColorManager> cm_rgb = std::make_shared<ColorManager>();
	std::wstring fileName = L"";
    std::wstring fileExt;
	size sz;
	int nf = 0;
    std::vector<float> frequens;
	std::vector<int> slices;
	std::vector<int> inlines = {200};
	std::vector<int> crosslines;
	public:		// User declarations
		void FormCreate();
		ViewportWindow viewport;
		__fastcall TSeismic3d(TComponent* Owner);
	};
	//---------------------------------------------------------------------------
	extern PACKAGE TSeismic3d *Seismic3d;
	//---------------------------------------------------------------------------
	#endif
