//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "UnitFileSelection.h"
#include "UnitSeismic3d.h"
#include <string>
#include <cwchar>
#include "Reader.h"
#include "UnitLoading.h"
#include "UnitFormUniversal.h"
//#include "UnitFormUniversal.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFileSelection *FileSelection;
//---------------------------------------------------------------------------
__fastcall TFileSelection::TFileSelection(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFileSelection::Button2Click(TObject *Sender)
{
	TSeismic3d *Seis = new TSeismic3d(this);
	Seis->Show();
	Seis->FormCreate();
}
//---------------------------------------------------------------------------

void __fastcall TFileSelection::Button3Click(TObject *Sender)
{
	TFormUniversal *test = new TFormUniversal(Application->MainForm);
	test->Show();
//TForm1 *test = new TForm1(Application->MainForm);
//test->Show();
//test->load();
}
//---------------------------------------------------------------------------

void __fastcall TFileSelection::Button1Click(TObject *Sender)
{
//	TLoading* load = new TLoading(Application->MainForm);
//	load->setDuration(500);
//	load->Show();
//	for (int i = 0; i < 501; i++) {
//	Traces tr = readInlineRegular(L"Z:\\projects\\握我咽闻_涛信\\cubes\\CUBE-2a_LAST_涛信\\Cycle_1.ibm", i);
//	Traces tr2 = readInlineRegular(L"Z:\\projects\\握我咽闻_涛信\\cubes\\CUBE-2a_LAST_涛信\\cube2_cut_ara.ibm", i);
//
//	std::shared_ptr<SeismicData> seismicData;
//	seismicData = std::make_shared<SeismicData>(tr.data, tr.dt, tr.samplesNumber, tr.tracesNumber, "cycle" + std::to_string(i));
//	std::shared_ptr<SeismicData> seismicData1;
//	seismicData1 = std::make_shared<SeismicData>(tr2.data, tr2.dt, tr2.samplesNumber, tr2.tracesNumber, "base" + std::to_string(i));
//	seismicData->saveFile(L"C:\\Users\\belousov\\Desktop\\1111\\3\\pr\\cycle_" + std::to_wstring(i) + L".sd");
//	seismicData1->saveFile(L"C:\\Users\\belousov\\Desktop\\1111\\3\\pr\\base_" + std::to_wstring(i) + L".sd");
//	load->update(1);
//	}
}
//---------------------------------------------------------------------------

