//---------------------------------------------------------------------------

#ifndef ChooseFormH
#define ChooseFormH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <VirtualTrees.hpp>
#include "VirtualTrees.AncestorVCL.hpp"
#include "VirtualTrees.BaseAncestorVCL.hpp"
#include "VirtualTrees.BaseTree.hpp"
#include <Vcl.ExtCtrls.hpp>

struct TRGBFileData {
    UnicodeString FullPath;
    UnicodeString FileName;
};
//---------------------------------------------------------------------------
class TChoose : public TForm
{
__published:	// IDE-managed Components
	TVirtualStringTree *VST;
	TButton *New;
	TButton *ViewSeis;
	TButton *Close;
	TPanel *Panel1;
//void __fastcall VSTGetText(TVirtualStringTree *Sender, TVirtualNode *Node,
//		TVirtualColumn *Column, TVSTTextType TextType, UnicodeString &CellText);
//    void __fastcall VSTFreeNode(TVirtualStringTree *Sender, TVirtualNode *Node);
	void __fastcall VSTClick(TObject *Sender);
	void __fastcall VSTGetText(TBaseVirtualTree *Sender, PVirtualNode Node, TColumnIndex Column,
          TVstTextType TextType, UnicodeString &CellText);
	void __fastcall VSTFreeNode(TBaseVirtualTree *Sender, PVirtualNode Node);
	void __fastcall CloseClick(TObject *Sender);
	void __fastcall ViewSeisClick(TObject *Sender);
	void __fastcall NewClick(TObject *Sender);
private:	// User declarations
	void LoadRGBFiles(const UnicodeString& path);
	void ShowFileDescription(const UnicodeString& filePath);
    UnicodeString chosen;
    UnicodeString curPath;
public:		// User declarations
	__fastcall TChoose(TComponent* Owner);
    void SetRGBPath(const UnicodeString& path);
    bool Execute();
    UnicodeString getChosen(){return chosen;};
    int execType = 0;
};
//---------------------------------------------------------------------------
extern PACKAGE TChoose *Choose;
//---------------------------------------------------------------------------
#endif
