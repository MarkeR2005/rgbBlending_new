//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitLoading.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TLoading *Loading;
//---------------------------------------------------------------------------
__fastcall TLoading::TLoading(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TLoading::setDuration(int iterations){
ProgressBar1->Max = iterations;
}
void TLoading::update(int step){
ProgressBar1->StepBy(step);
if (ProgressBar1->Position == ProgressBar1->Max) {
this->Free();
}
Application->ProcessMessages();
}