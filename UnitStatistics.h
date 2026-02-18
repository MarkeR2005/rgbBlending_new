//---------------------------------------------------------------------------

#ifndef UnitStatisticsH
#define UnitStatisticsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include "Structures.h"
//---------------------------------------------------------------------------
class TUnitStatistic : public TForm
{
__published:	// IDE-managed Components
	TScrollBox *ScrollBox1;
	TPanel *Panel3;
	TImage *Image1;
    void __fastcall Draw(TObject* Sender);
private:	// User declarations
	std::vector<point2F> data;
public:		// User declarations
	__fastcall TUnitStatistic(TComponent* Owner);
	void SetData(std::vector<point2F> _data);
};
//---------------------------------------------------------------------------
extern PACKAGE TUnitStatistic *UnitStatistic;
//---------------------------------------------------------------------------
#endif
