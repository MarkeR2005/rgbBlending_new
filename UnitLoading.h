//---------------------------------------------------------------------------

#ifndef UnitLoadingH
#define UnitLoadingH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------
class TLoading : public TForm
{
__published:	// IDE-managed Components
	TProgressBar *ProgressBar1;
private:	// User declarations
public:		// User declarations
	__fastcall TLoading(TComponent* Owner);
	void setDuration(int iterations);
	void update(int step);
};
//---------------------------------------------------------------------------
extern PACKAGE TLoading *Loading;
//---------------------------------------------------------------------------
#endif
