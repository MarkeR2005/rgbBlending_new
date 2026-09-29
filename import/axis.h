//---------------------------------------------------------------------------

#include <vcl.h>
#include "AxisParameters.h"
#include <Math.hpp>
#include <math.h>
#include <vector>
//#include "svgExport.h"

#ifndef axisH
#define axisH

#define HORZ_AXE_TOP    0
#define HORZ_AXE_BOTTOM 1
#define VERT_AXE_RIGHT  2
#define VERT_AXE_LEFT   3
#define SWAN_AXE        4

#define LOGARIFMIC 0x10000

//#include "commonFunc.h"
//---------------------------------------------------------------------------
struct TAxeValue
{
UnicodeString sign;
TPoint signCoord;
TPoint pickCoord1;
TPoint pickCoord2;
};

struct TAxeInterval {
	double startValue;
	double endValue;
	int sizeInPixels;
	void init(double _st, double _end, int w) { startValue=_st; endValue=_end; sizeInPixels=w; }
};

struct TAxeParameters
{
TFont *axeFont;
double majorPicksInterval;
bool showMinorTicks;
Byte automatic;
TColor picksColor;//by gregory
TColor backColor;//by gregory
int majorPicksWidth;//by gregory
TAxeParameters() {
    axeFont=new TFont;
    majorPicksInterval=100;
    showMinorTicks=true;
    automatic=true; //0,1,2.  2-means narrow labels for HORZ axe
    picksColor=clBlack;
    backColor=clInfoBk;
    majorPicksWidth=1;
}
~TAxeParameters() {delete axeFont;}
TAxeParameters& operator =(TAxeParameters &source) {
     axeFont->Assign(source.axeFont);
     majorPicksInterval=source.majorPicksInterval;
     showMinorTicks=source.showMinorTicks;
     automatic=source.automatic;
     picksColor=source.picksColor;
     backColor=source.backColor;
     majorPicksWidth=source.majorPicksWidth;
     return *this;
}
};

class TAxis2
{

private:
        bool forPlanshet;
		DynamicArray < TAxeValue > axe;
		std::vector<int> smallPicksCoords;
		int shiftOfPicks2;
		double pixelsInOneSample, stepSample; //сколько пикселей в одной единице оси и главный шаг пиков в осевых единицах
        int makeVertAxe(double start, double end, int size, bool left); //возвращает ShiftOfPicks;
        int makeHorzAxe(DynamicArray < TAxeInterval > intervals, bool bottom);
        int makeSwanAxe(DynamicArray < TAxeInterval > intervals);
		int makeLogAxe(double start, double end, int size);
public:
        TAxeParameters *params;
        Graphics::TBitmap *axeBitmap;
        TAxis2(bool _forPlanshet=false);
        ~TAxis2();
        void createAxe(DynamicArray < TAxeInterval > intervals, int axeType, int roundPrec=2); //первичное округление порогов до какого знака?
        void createAxe(double start, double end, int size, int axeType, int roundPrec=2);
		int getShiftOfPicks()   {return shiftOfPicks2;}
		//TDPoint getMainParams(void) { return TDPoint(stepSample, pixelsInOneSample); }
		Graphics::TBitmap* getBitmap()     {return axeBitmap;}
        int getAxePicksCount()             {return axe.Length;}
        TAxeValue getAxeValue(int index)   {return axe[index];}
        bool changeParameters(TComponent *owner); //for changing this.params
		static bool changeParameters(TComponent *owner, TAxis2 *horzAxe, TAxis2 *vertAxe); //static method for seismicShow special to change both axis
//        String svgDraw(void);
};
#endif
