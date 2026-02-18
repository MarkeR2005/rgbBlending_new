//---------------------------------------------------------------------------
#include "axis.h"
#include "AxisParameters.h"
#pragma hdrstop


//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------

TAxis2::TAxis2(bool _forPlanshet)
{
forPlanshet=_forPlanshet;
params = new TAxeParameters();
params->axeFont->Color=clBlack;
axeBitmap = new Graphics::TBitmap;
axeBitmap->PixelFormat=pf24bit;
axeBitmap->Transparent=true;
axeBitmap->TransparentColor=params->backColor;
shiftOfPicks2=0;
}
//---------------------------------------------------------------------------

TAxis2::~TAxis2()
{
delete params;
delete axeBitmap;
}
//---------------------------------------------------------------------------

bool TAxis2::changeParameters(TComponent *owner, TAxis2 *horzAxe, TAxis2 *vertAxe) //static method for seismicShow special to change both axis
{
TAxisParametersForm *apf=new TAxisParametersForm(owner);
apf->init(horzAxe->params, vertAxe->params);
bool res = (apf->ShowModal()==mrOk);
if (res) {
    horzAxe->params->axeFont->Assign(apf->HExL->Font);
    horzAxe->params->automatic=apf->HAutoCB->Checked;
    horzAxe->params->showMinorTicks=apf->HMinorCB->Checked;
    horzAxe->params->majorPicksInterval=apf->HMajorTicksUD->Position;
    vertAxe->params->axeFont->Assign(apf->VExL->Font);
    vertAxe->params->automatic=apf->VAutoCB->Checked;
    vertAxe->params->showMinorTicks=apf->VMinorCB->Checked;
    vertAxe->params->majorPicksInterval=apf->VMajorTicksUD->Position;
}
delete apf;
return res;
}
//---------------------------------------------------------------------------

bool TAxis2::changeParameters(TComponent *owner)
{
TAxisParametersForm *apf=new TAxisParametersForm(owner);
apf->init(this->params);
bool res = (apf->ShowModal()==mrOk);
if (res) {
    this->params->axeFont->Assign(apf->HExL->Font);
    this->params->automatic=apf->HAutoCB->Checked;
    this->params->showMinorTicks=apf->HMinorCB->Checked;
    this->params->majorPicksInterval=apf->HMajorTicksUD->Position;
}
delete apf;
return res;
}
//---------------------------------------------------------------------------

void TAxis2::createAxe(double start, double end, int size, int axeType, int roundPrec)
{
TAxeInterval tai; tai.startValue=start; tai.endValue=end; tai.sizeInPixels=size;
DynamicArray < TAxeInterval > ti; ti.Length=1; ti[0]=tai;
createAxe(ti, axeType, roundPrec);
}
//---------------------------------------------------------------------------

void TAxis2::createAxe(DynamicArray < TAxeInterval > intervals, int axeType, int roundPrec)
{
bool log_scale = (axeType&LOGARIFMIC);
axeBitmap->Width=0; axeBitmap->Height=0;
axeBitmap->Canvas->Font->Assign(params->axeFont);
////Первое что надо сделать - это округлить пороговые значения
if (log_scale==false) {
	if (axeType==SWAN_AXE) roundPrec=0;
	for (int i=0; i<intervals.Length; ++i) {
		intervals[i].startValue= RoundTo(intervals[i].startValue, -roundPrec);
		intervals[i].endValue  = RoundTo(intervals[i].endValue, -roundPrec);
	}
}
///
if (log_scale) shiftOfPicks2 = makeLogAxe(intervals[0].startValue, intervals[0].endValue, intervals[0].sizeInPixels);
else {
 switch (axeType) {
  case HORZ_AXE_TOP:    shiftOfPicks2 = makeHorzAxe(intervals, false);break;
  case HORZ_AXE_BOTTOM: shiftOfPicks2 = makeHorzAxe(intervals, true);break;
  case VERT_AXE_RIGHT:  shiftOfPicks2 = makeVertAxe(intervals[0].startValue, intervals[0].endValue, intervals[0].sizeInPixels, false); break;
  case VERT_AXE_LEFT:   shiftOfPicks2 = makeVertAxe(intervals[0].startValue, intervals[0].endValue, intervals[0].sizeInPixels, true); break;
  case SWAN_AXE:        shiftOfPicks2 = makeSwanAxe(intervals);break;
 }
}
axeBitmap->TransparentColor=params->backColor;
}
//---------------------------------------------------------------------------

int TAxis2::makeLogAxe(double start, double end, int size)
{ 
///пока что это как VERT_AXE_RIGHT
double tmp,tmp2;
bool rev=false;
if (start>end) {
double temp = start;
start = end;
end = temp;
 rev=true;
 }
if (start<=0) start=1e-6;
if (end<=0) end=1e-5;

start = log10(start);
end = log10(end);

int firstPick, lastPick;
firstPick=Ceil(start);
lastPick=Floor(end);
if (firstPick>lastPick) return 0;
///
double pixelsBetweenPicks = (double)(size-1)/(end-start);
int shiftOfPicks = axeBitmap->Canvas->TextHeight(UnicodeString(pow(10,firstPick)))/2-((double)firstPick-start)*pixelsBetweenPicks;  //firstPickCoord - half of firstPickLabel-height
if (shiftOfPicks<0) shiftOfPicks=0;
int shiftOfPicks_e=axeBitmap->Canvas->TextHeight(UnicodeString(pow(10,lastPick)))/2-((double)end-lastPick)*pixelsBetweenPicks;
//int shiftOfPicks_e= ((double)lastPick-start)*pixelsBetweenPicks+axeBitmap->Canvas->TextHeight(UnicodeString(pow(10,firstPick)))/2 - size;
if (shiftOfPicks_e<0) shiftOfPicks_e=0;

axeBitmap->Width = Max(axeBitmap->Canvas->TextWidth(UnicodeString(pow(10,start))),axeBitmap->Canvas->TextWidth(UnicodeString(pow(10,end))))+20;
axeBitmap->Height = size+shiftOfPicks+shiftOfPicks_e;
  
axe.Length=0;

if (firstPick!=start) {  ///small pre-picks
        tmp = ((double)firstPick-start)*pixelsBetweenPicks;
        //small pick at FirstPick - (1-log10(j))*pixelsBetweenPicks     //until coord>0
        for (int j=2; j<=9; ++j) {
                double y = tmp - (1.0-log10(j))*pixelsBetweenPicks;
                if (y<0) continue;
                axe.Length+=(10-j);
                for (int k=j; k<=9; ++k) {
                        y = shiftOfPicks + tmp - (1.0-log10(k))*pixelsBetweenPicks;  if (rev) y=axeBitmap->Height-y;
                        axe[k-j].pickCoord1=TPoint(0,y);
                        axe[k-j].pickCoord2=TPoint(4,y);
						axe[k-j].sign=EmptyStr;
                        axe[k-j].signCoord.x=7;
                        axe[k-j].signCoord.y=y-axeBitmap->Canvas->TextHeight(axe[k-j].sign)/2;
				}
                break;
        }
}
///main picks stream
int m=axe.Length;
axe.Length+=(lastPick-firstPick+1)+(lastPick-firstPick)*8;
for (int i=firstPick; i<lastPick; ++i) {
        //Pick at shiftOfPicks + ((double)i-start)*pixelsBetweenPicks
        tmp = shiftOfPicks + ((double)i-start)*pixelsBetweenPicks;
        tmp2 = tmp; if (rev) tmp2=axeBitmap->Height-tmp2;
        axe[m].pickCoord1=TPoint(0,tmp2);
        axe[m].pickCoord2=TPoint(7,tmp2);
        axe[m].sign=UnicodeString(pow(10,i));
        axe[m].signCoord.x=10;
        axe[m].signCoord.y=tmp2-axeBitmap->Canvas->TextHeight(axe[m].sign)/2;
        ++m;
        for (int j=2; j<=9; ++j) { //small picks
                //small pick at BigPick+ log10(j)*pixelsBetweenPicks
                double y = tmp + log10(j)*pixelsBetweenPicks;  if (rev) y=axeBitmap->Height-y;
                axe[m].pickCoord1=TPoint(0,y);
                axe[m].pickCoord2=TPoint(4,y);
				axe[m].sign=EmptyStr;
                axe[m].signCoord.x=7;
                axe[m].signCoord.y=y-axeBitmap->Canvas->TextHeight(axe[m].sign)/2;
                ++m;
        }
}
////lastPick and small post-picks
tmp = shiftOfPicks + ((double)lastPick-start)*pixelsBetweenPicks;
tmp2=tmp; if (rev) tmp2=axeBitmap->Height-tmp2;
axe[m].pickCoord1=TPoint(0,tmp2);
axe[m].pickCoord2=TPoint(7,tmp2);
axe[m].sign=UnicodeString(pow(10,lastPick));
axe[m].signCoord.x=10;
axe[m].signCoord.y=tmp2-axeBitmap->Canvas->TextHeight(axe[m].sign)/2;
++m;
if (lastPick!=end) {
        //small pick at LastPick + log10(j)*pixelsBetweenPicks     //until coord<size
        for (int j=9; j>=2; --j) {
                double y = tmp + log10(j)*pixelsBetweenPicks;
                if (y>=size+shiftOfPicks) continue;
                axe.Length+=(j-1);
                for (int k=2; k<=j; ++k) {
                        y = tmp + log10(k)*pixelsBetweenPicks; if (rev) y=axeBitmap->Height-y;
                        axe[m].pickCoord1=TPoint(0,y);
                        axe[m].pickCoord2=TPoint(4,y);
						axe[m].sign=EmptyStr;
                        axe[m].signCoord.x=7;
                        axe[m].signCoord.y=y-axeBitmap->Canvas->TextHeight(axe[m].sign)/2;
                        ++m;
                }
                break;
        }
}
//Отрисовка оси
axeBitmap->Canvas->Pen->Color=params->backColor;
axeBitmap->Canvas->Brush->Color=params->backColor;
axeBitmap->Canvas->Pen->Width=params->majorPicksWidth;
axeBitmap->Canvas->Rectangle(TRect(0,0,axeBitmap->Width,axeBitmap->Height));
axeBitmap->Canvas->Pen->Color=params->picksColor;
for (int i=0; i<axe.Length; ++i) {
        axeBitmap->Canvas->MoveTo(axe[i].pickCoord1.x, axe[i].pickCoord1.y);
        axeBitmap->Canvas->LineTo(axe[i].pickCoord2.x, axe[i].pickCoord2.y);
        axeBitmap->Canvas->TextOut(axe[i].signCoord.x,axe[i].signCoord.y,axe[i].sign);
}
if (rev) {
        DynamicArray < TAxeValue > tmpAxe = axe.Copy();
        for (int i=0; i<axe.Length; ++i) axe[i]=tmpAxe[axe.Length-i-1];
}

if (rev) return shiftOfPicks_e;
return shiftOfPicks;
}
//---------------------------------------------------------------------------

int TAxis2::makeVertAxe(double start, double end, int size, bool left)
{
int halfSignHeight=axeBitmap->Canvas->TextHeight(UnicodeString(start))/2;
int pixelsBetweenPiks = halfSignHeight*4;
if (pixelsBetweenPiks==0) pixelsBetweenPiks=50;
double div = (double)(fabs(end-start)); if (div==0) div=1;
pixelsInOneSample = (double)(size-1) / div;
double startSample=start;
stepSample=(double)size/(double)pixelsBetweenPiks;
if (stepSample<1) stepSample=1;
stepSample = fabs((end-start)/stepSample);

//Авто-подбор шага по дискретам
double roundSteps[26] = {0.1, 0.5, 1, 2, 5, 10, 20, 50, 100, 200, 250, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000, 10000000, 20000000, 50000000, 100000000};
if (params->automatic)
{
        for (int i=0; i<26; i++)
        {
         if ((roundSteps[i]*pixelsInOneSample)<pixelsBetweenPiks) {continue;}
		 stepSample=roundSteps[i]; break;
        }
} else stepSample=params->majorPicksInterval; //Если автомат не нужен то задаем вручную
//Нахождение первого нужного дискрета
if (stepSample==0) stepSample=1;
double tempF = start/stepSample;
int tempI;
if (start>end) {
        tempI = ceil(tempF);
        if (tempF-(double)tempI!=0) startSample = (double)tempI*stepSample - stepSample;
} else {
        tempI = floor(tempF);
        if (tempF-(double)tempI!=0) startSample = (double)tempI*stepSample + stepSample;
}
//Подсчет количества пиков
int picsCount=fabs(end-startSample)/stepSample + 3;   //исправлено гришей. было +2
axe.Length=picsCount;
int shiftOfPicks = halfSignHeight;
axe[0].pickCoord1.x=0; axe[0].pickCoord1.y=shiftOfPicks;
axe[0].pickCoord2.x=7; axe[0].pickCoord2.y=axe[0].pickCoord1.y;
axe[0].signCoord.x=10; axe[0].signCoord.y=axe[0].pickCoord1.y-halfSignHeight;
axe[0].sign =  FloatToStrF(start,ffGeneral,8,2);
for (int i=1; i<picsCount-1; i++)     //исправлено гришей. было (i) cnfkj (i-1)
{
        axe[i].pickCoord1.x = 0;
		axe[i].pickCoord1.y = (fabs(startSample-start)+stepSample*(double)(i-1))*pixelsInOneSample+shiftOfPicks;
        axe[i].pickCoord2.x = 7;
        axe[i].pickCoord2.y = axe[i].pickCoord1.y;
        axe[i].signCoord.x  = 10;
        axe[i].signCoord.y  = axe[i].pickCoord1.y - halfSignHeight;
        if (start>end) axe[i].sign=UnicodeString(startSample-stepSample*(double)(i-1));
        else           axe[i].sign=UnicodeString(startSample+stepSample*(double)(i-1));
}
axe[picsCount-1].pickCoord1.x=0; axe[picsCount-1].pickCoord1.y=shiftOfPicks+size-1;
axe[picsCount-1].pickCoord2.x=7; axe[picsCount-1].pickCoord2.y=axe[picsCount-1].pickCoord1.y;
axe[picsCount-1].signCoord.x=10; axe[picsCount-1].signCoord.y=axe[picsCount-1].pickCoord1.y-halfSignHeight;
axe[picsCount-1].sign = FloatToStrF(end,ffGeneral,8,2);
axeBitmap->Width = 2+Max(axeBitmap->Canvas->TextWidth(String(start)), axeBitmap->Canvas->TextWidth(String(end))) + 10;
axeBitmap->Height = size+halfSignHeight*2;
//Отрисовка оси
axeBitmap->Canvas->Pen->Color=params->backColor;
axeBitmap->Canvas->Brush->Color=params->backColor;
axeBitmap->Canvas->Pen->Width=params->majorPicksWidth;
axeBitmap->Canvas->Rectangle(TRect(0,0,axeBitmap->Width,axeBitmap->Height));
axeBitmap->Canvas->Pen->Color=params->picksColor;

if (left) {
  for (int i=0; i<axe.Length; i++) {
    axeBitmap->Canvas->MoveTo(axeBitmap->Width-axe[i].pickCoord1.x, axe[i].pickCoord1.y);
    axeBitmap->Canvas->LineTo(axeBitmap->Width-axe[i].pickCoord2.x, axe[i].pickCoord2.y);
    axeBitmap->Canvas->TextOut(axeBitmap->Width - axe[i].signCoord.x - axeBitmap->Canvas->TextWidth(axe[i].sign),axe[i].signCoord.y,axe[i].sign);
  }
} else {
  for (int i=0; i<axe.Length; i++) {
    axeBitmap->Canvas->MoveTo(axe[i].pickCoord1.x, axe[i].pickCoord1.y);
    axeBitmap->Canvas->LineTo(axe[i].pickCoord2.x, axe[i].pickCoord2.y);
    axeBitmap->Canvas->TextOut(axe[i].signCoord.x,axe[i].signCoord.y,axe[i].sign);
  }
}
//Отрисовка маленьких пиков // всего 4 маленьких пика
smallPicksCoords.resize(0);
if ( ((params->automatic)&&(pixelsBetweenPiks>14)) || (!params->automatic && params->showMinorTicks) )
if (stepSample>=5) {
		double picksStep = stepSample/5;
        int picksBefore = fabs(startSample - start) / picksStep;
        int picksAfter;
		if (start>end) picksAfter  = fabs(end - (startSample-stepSample*(picsCount-2))) / picksStep;
		else picksAfter  = (end - (startSample+stepSample*(picsCount-1))) / picksStep;
        int picksBetween= 5*(picsCount-1);//+1;
        int picksCount = picksBefore+picksAfter+picksBetween;
		smallPicksCoords.resize(picksCount);
        double startPick;
        if (start>end) startPick = startSample + picksStep*picksBefore;
        else startPick = startSample - picksStep*picksBefore;
        for (int i=0; i<picksCount; i++) {
                int yCoord;
                if (start>end) yCoord = (fabs(startPick-start)+picksStep*i)*pixelsInOneSample+shiftOfPicks;
                else yCoord = ((startPick-start)+picksStep*i)*pixelsInOneSample+shiftOfPicks;
				smallPicksCoords[i]=yCoord;
                if (left) {
                  axeBitmap->Canvas->MoveTo(axeBitmap->Width-0, yCoord);
                  axeBitmap->Canvas->LineTo(axeBitmap->Width-4, yCoord);
                } else {
                  axeBitmap->Canvas->MoveTo(0, yCoord);
                  axeBitmap->Canvas->LineTo(4, yCoord);
                }
        }
}
/////////////////////////////////////////////////////
return shiftOfPicks;
}
//---------------------------------------------------------------------------

int TAxis2::makeHorzAxe(DynamicArray < TAxeInterval > intervals, bool bottom)
{
/*////Первое что надо сделать - это округлить до второго знака пороговые значения
for (int i=0; i<intervals.Length; ++i) {
		intervals[i].startValue = RoundTo(intervals[i].startValue, -2);
		intervals[i].endValue = RoundTo(intervals[i].endValue, -2);
}*/
//выяснение размеров битмапа
int shiftOfPicks_s = axeBitmap->Canvas->TextWidth(UnicodeString(intervals[0].startValue))/2;
int shiftOfPicks_e = axeBitmap->Canvas->TextWidth(UnicodeString(intervals[intervals.High].endValue))/2;
int shiftOfPicks = Max(shiftOfPicks_s, shiftOfPicks_e);
int bmpWidth=shiftOfPicks+shiftOfPicks_e;
for (int i=0; i<intervals.Length; i++) bmpWidth+=intervals[i].sizeInPixels;
int bmpHeight = axeBitmap->Canvas->TextHeight(UnicodeString(intervals[0].startValue))+10;
axeBitmap->Canvas->Pen->Color=params->backColor;
axeBitmap->Canvas->Brush->Color=params->backColor;
axeBitmap->Canvas->Pen->Width=params->majorPicksWidth;

if (!forPlanshet) {
	axeBitmap->Width=bmpWidth; axeBitmap->Height=bmpHeight;
	axeBitmap->Canvas->Rectangle(0,0,bmpWidth,bmpHeight);
} else shiftOfPicks=0; //чтобы не вычитать его потом
/////////////////////////////////////////////////////////
int shiftOfInterval=0;
int narrowLabels=params->automatic; if (narrowLabels==0) narrowLabels=1;
for (int i=0; i<intervals.Length; i++) {
	int sL = String(intervals[i].startValue).Length();
	int eL = String(intervals[i].endValue).Length();
	int pixelsBetweenPicks;
	if (sL>eL) pixelsBetweenPicks = axeBitmap->Canvas->TextWidth(String(intervals[i].startValue))*2/narrowLabels;
	else pixelsBetweenPicks = axeBitmap->Canvas->TextWidth(String(intervals[i].endValue))*2/narrowLabels;
	if (pixelsBetweenPicks==0) pixelsBetweenPicks=50;
	double start = intervals[i].startValue;
	double end   = intervals[i].endValue;
	int size  = intervals[i].sizeInPixels;
	double div = fabs(end-start); if (div==0) div=1;
	pixelsInOneSample = (double)size / div;
	double startSample=start;
	stepSample=fabs((end-start)/2);
	if (fabs(end-start)>1E9) return 0;//NEW
	//Авто-подбор шага по дискретам
	double roundSteps[30] = { 0.05, 0.1, 0.5, 1, 2, 5, 10, 20, 50, 100, 200, 250, 500, 1000, 2000, 5000, 10000, 20000, 50000,
							 100000, 200000, 500000, 1000000, 10000000, 20000000, 50000000, 100000000, 200000000, 500000000, 1000000000 };
	if (params->automatic) {
		for (int i=0; i<30; i++) {
			if ((roundSteps[i]*pixelsInOneSample)<pixelsBetweenPicks) continue;
			stepSample=roundSteps[i]; break;
		}
	} else stepSample=params->majorPicksInterval; //Если автомат не нужен то задаем вручную
	//Нахождение первого нужного дискрета
	if (stepSample==0) stepSample=1;
	double tempF = start/stepSample;
	int tempI;
	if (start>end) {
		tempI = ceil(tempF);
		if (tempF-(double)tempI!=0) startSample = (double)tempI*stepSample - stepSample;
	} else {
		tempI = floor(tempF);
		if (tempF-(double)tempI!=0) startSample = (double)tempI*stepSample + stepSample;
	}
	//Подсчет количества пиков
	int picsCount=fabs(end-startSample)/stepSample + 1;
	axe.Length=picsCount;
	for (int j=0; j<picsCount; j++) {
		axe[j].pickCoord1.x = (fabs(startSample-start)+stepSample*(double)(j))*pixelsInOneSample+shiftOfPicks+shiftOfInterval;

		if (bottom) axe[j].pickCoord1.y=0;
		else axe[j].pickCoord1.y = bmpHeight;

		axe[j].pickCoord2.x = axe[j].pickCoord1.x;

		if (bottom) axe[j].pickCoord2.y=7;
		else axe[j].pickCoord2.y = bmpHeight-7;

		if (start>end) axe[j].sign=UnicodeString(startSample-stepSample*(double)(j));
		else           axe[j].sign=UnicodeString(startSample+stepSample*(double)(j));

		axe[j].signCoord.x  = axe[j].pickCoord1.x - axeBitmap->Canvas->TextWidth(axe[j].sign)/2;

		if (bottom) axe[j].signCoord.y  = bmpHeight-axeBitmap->Canvas->TextHeight(axe[j].sign);
		else        axe[j].signCoord.y  = 0;
	}
	if (forPlanshet) break;
	//Отрисовка оси
	axeBitmap->Canvas->Pen->Color=params->picksColor;
	//Отрисовка крайних пиков и подписей// А также граничных Больших пиков интервалов
	if (intervals.Length==1) {
		int firstValueWidth = axeBitmap->Canvas->TextWidth(UnicodeString(start));
		int firstValueRightMargin = shiftOfPicks+shiftOfInterval+firstValueWidth/2;
		if (firstValueRightMargin>=axe[0].signCoord.x&&!forPlanshet)
				axe[0].sign=EmptyStr; //убираем первую подпись
		int lastValueWidth = axeBitmap->Canvas->TextWidth(UnicodeString(end));
		int lastValueLeftMargin = shiftOfPicks+shiftOfInterval+size-1-lastValueWidth/2;
		if (lastValueLeftMargin<=(axe[axe.High].signCoord.x+axeBitmap->Canvas->TextWidth(axe[axe.High].sign))&&!forPlanshet)
				axe[axe.High].sign=EmptyStr; //убираем последнюю подпись
		if (bottom) {
          axeBitmap->Canvas->MoveTo(shiftOfPicks+shiftOfInterval, 0);
          axeBitmap->Canvas->LineTo(shiftOfPicks+shiftOfInterval, 7);
          axeBitmap->Canvas->TextOut(shiftOfInterval+shiftOfPicks-firstValueWidth/2,bmpHeight-axeBitmap->Canvas->TextHeight(UnicodeString(start)),UnicodeString(start));
          axeBitmap->Canvas->MoveTo(shiftOfPicks+shiftOfInterval+size-1, 0);
          axeBitmap->Canvas->LineTo(shiftOfPicks+shiftOfInterval+size-1, 7);
          axeBitmap->Canvas->TextOut(lastValueLeftMargin,bmpHeight-axeBitmap->Canvas->TextHeight(UnicodeString(end)),UnicodeString(end));
        } else {
		  axeBitmap->Canvas->MoveTo(shiftOfPicks+shiftOfInterval, bmpHeight-7);
          axeBitmap->Canvas->LineTo(shiftOfPicks+shiftOfInterval, bmpHeight);
          axeBitmap->Canvas->TextOut(shiftOfInterval+shiftOfPicks-firstValueWidth/2,0,UnicodeString(start));
		  axeBitmap->Canvas->MoveTo(shiftOfPicks+shiftOfInterval+size-1, bmpHeight-7);
          axeBitmap->Canvas->LineTo(shiftOfPicks+shiftOfInterval+size-1, bmpHeight);
          axeBitmap->Canvas->TextOut(lastValueLeftMargin,0,UnicodeString(end));
        }
	} else {
        if (i>0) {
                if (bottom) {
                  axeBitmap->Canvas->MoveTo(shiftOfPicks+shiftOfInterval, 0);
                  axeBitmap->Canvas->LineTo(shiftOfPicks+shiftOfInterval, 14);
                } else {
				  axeBitmap->Canvas->MoveTo(shiftOfPicks+shiftOfInterval, bmpHeight-14);
				  axeBitmap->Canvas->LineTo(shiftOfPicks+shiftOfInterval, bmpHeight);
                }
        }
	}
	/////////отрисовка
	for (int j=0; j<axe.Length; j++) {
		axeBitmap->Canvas->MoveTo(axe[j].pickCoord1.x, axe[j].pickCoord1.y);
		axeBitmap->Canvas->LineTo(axe[j].pickCoord2.x, axe[j].pickCoord2.y);
		//Надо чтобы правый край последней подписи не вылезал за интервал
		bool draw=true;
		if (intervals.Length>1) {
				int testDrawLeftMargin = axe[j].signCoord.x;
				int testDrawRightMargin = axe[j].signCoord.x+axeBitmap->Canvas->TextWidth(axe[j].sign);
				if (testDrawLeftMargin < (shiftOfPicks+shiftOfInterval)) draw=false;
				if (testDrawRightMargin> (shiftOfPicks+shiftOfInterval+size)) draw=false;
		}
		if (draw) axeBitmap->Canvas->TextOut(axe[j].signCoord.x,axe[j].signCoord.y,axe[j].sign);
	}
	//Отрисовка маленьких пиков // всего 4 маленьких пика
	if ( ((params->automatic)&&(pixelsBetweenPicks>14)) || (!params->automatic && params->showMinorTicks) )
	if (stepSample>=5) {
        int picksStep = stepSample/5;
        int picksBefore = abs(startSample - start) / picksStep;
        int picksAfter;
        if (start>end) picksAfter  = abs(end - (startSample-stepSample*(picsCount-1))) / picksStep;
		else picksAfter  = (end - (startSample+stepSample*(picsCount-1))) / picksStep;
        int picksBetween= 5*(picsCount-1)+1;
        int picksCount = picksBefore+picksAfter+picksBetween;
        int startPick;
        if (start>end) startPick = startSample + picksStep*picksBefore;
		else startPick = startSample - picksStep*picksBefore;
		for (int j=0; j<picksCount; j++) {
				int xCoord;
				if (start>end) xCoord = (abs(startPick-start)+picksStep*j)*pixelsInOneSample+shiftOfPicks+shiftOfInterval;
				else xCoord = ((startPick-start)+picksStep*j)*pixelsInOneSample+shiftOfPicks+shiftOfInterval;
				if (bottom) {
				  axeBitmap->Canvas->MoveTo(xCoord, 0);
				  axeBitmap->Canvas->LineTo(xCoord, 4);
				} else {
				  axeBitmap->Canvas->MoveTo(xCoord, bmpHeight);
				  axeBitmap->Canvas->LineTo(xCoord, bmpHeight-4);
				}
		}
	}
	/////////////////////////////////////////////////////
	shiftOfInterval+=intervals[i].sizeInPixels;
}
return shiftOfPicks;
}
//---------------------------------------------------------------------------

int TAxis2::makeSwanAxe(DynamicArray < TAxeInterval > intervals)
{
////Первое что надо сделать - это округлить до целых значений пороговые частоты
//intervals[0].startValue = RoundTo(intervals[0].startValue, 0);
//intervals[0].endValue = RoundTo(intervals[0].endValue, 0);
int swanFilterNumber = intervals[1].startValue;
//int traceBase = intervals[1].endValue;
bool simmetric = intervals[1].sizeInPixels;
int sc=1; if (simmetric) sc=2;
//выяснение размеров битмапа
int shiftOfPicks = axeBitmap->Canvas->TextWidth(UnicodeString(intervals[0].startValue))/2;;
int bmpWidth=shiftOfPicks;
for (int i=0; i<sc; i++) bmpWidth+=intervals[0].sizeInPixels;
if (!simmetric) bmpWidth+=axeBitmap->Canvas->TextWidth(UnicodeString(intervals[0].endValue))/2;
else bmpWidth+=axeBitmap->Canvas->TextWidth(UnicodeString(intervals[0].startValue))/2;
int bmpHeight = axeBitmap->Canvas->TextHeight(UnicodeString(intervals[0].startValue))+10;
axeBitmap->Width=bmpWidth; axeBitmap->Height=bmpHeight;
axeBitmap->Canvas->Pen->Color=params->backColor;
axeBitmap->Canvas->Brush->Color=params->backColor;
axeBitmap->Canvas->Pen->Width=params->majorPicksWidth;
axeBitmap->Canvas->Rectangle(TRect(0,0,axeBitmap->Width,axeBitmap->Height));
/////////////////////////////////////////////////////////
//сначала нужно найти первый пик который будет отстоять на 2 пикселя от максимальной частоты
//его частота обрубается до целого числа и таким образом мы имеем шаг по частоте.
//А теперь идём с этим шагом начиная с миимальной частоты и расчитвыаем положение каждого следующего тика.
//Ну а затем рисуем всё это.
//Этот метод содержит изъян, но пока других вариантов не придумали.

double fn   = Min(intervals[0].startValue,intervals[0].endValue);
double fk   = Max(intervals[0].startValue,intervals[0].endValue);
double koeff = exp((1.0/(double)(swanFilterNumber-1))*log(fk*1.0/fn));
double pixelsInOneTrace = (double)(intervals[0].sizeInPixels)/(double)(swanFilterNumber);
int fr = (double)(intervals[0].sizeInPixels-3) / pixelsInOneTrace; //если intervals[0].sizeInPixels-1 - это последний пик, то нам надо -2 пикселя

double nearestPickFreq = fn;
for (int i=0; i<fr; ++i) nearestPickFreq*=koeff;
int freqStep = (int)fk - (int)nearestPickFreq; if (freqStep==0) freqStep=1;

axe.Length = (fk-fn)/freqStep - 1; //не включает края
int prevCoord = axeBitmap->Canvas->TextWidth(fn);
int lastCoord = shiftOfPicks+intervals[0].sizeInPixels-1-axeBitmap->Canvas->TextWidth(UnicodeString(fk))/2;
for (int i=0; i<axe.Length; ++i) {
        int j=0;
        double freq = fn + (i+1)*freqStep;
        axe[i].sign=UnicodeString(freq);
        while (freq>fn) {freq/=koeff; ++j;}
        if (freq*koeff-fn < fn-freq) j--;
        axe[i].pickCoord1.x = shiftOfPicks + pixelsInOneTrace*(double)j + pixelsInOneTrace/2;
        axe[i].pickCoord1.y = axeBitmap->Height;
        axe[i].pickCoord2.y = axeBitmap->Height - 7;
        axe[i].signCoord.x = axe[i].pickCoord1.x - axeBitmap->Canvas->TextWidth(axe[i].sign)/2;
        axe[i].signCoord.y = 0;
        bool hide=false;
		if (axe[i].signCoord.x-prevCoord < 10) {hide=true;axe[i].sign=EmptyStr;}
        if (!hide) prevCoord = axe[i].pickCoord1.x+axeBitmap->Canvas->TextWidth(axe[i].sign)/2;
        if (lastCoord - prevCoord < 10) {hide=true;axe[i].sign=EmptyStr;}
        if (hide) axe[i].pickCoord2.y+=2;

        if (intervals[0].startValue>intervals[0].endValue) {
                axe[i].pickCoord1.x = intervals[0].sizeInPixels-1 - axe[i].pickCoord1.x + 2*shiftOfPicks;
                axe[i].signCoord.x = axe[i].pickCoord1.x - axeBitmap->Canvas->TextWidth(axe[i].sign)/2;
        }
        axe[i].pickCoord2.x = axe[i].pickCoord1.x;
        //
}

//Отрисовка оси
axeBitmap->Canvas->Pen->Color=params->picksColor;
//Отрисовка крайних пиков и подписей
axeBitmap->Canvas->MoveTo(shiftOfPicks, axeBitmap->Height);
axeBitmap->Canvas->LineTo(shiftOfPicks, axeBitmap->Height-10);
axeBitmap->Canvas->TextOut(0, 0, UnicodeString(intervals[0].startValue));
axeBitmap->Canvas->MoveTo(shiftOfPicks+intervals[0].sizeInPixels-1, axeBitmap->Height);
axeBitmap->Canvas->LineTo(shiftOfPicks+intervals[0].sizeInPixels-1, axeBitmap->Height-10);
axeBitmap->Canvas->TextOut(shiftOfPicks+intervals[0].sizeInPixels-1-axeBitmap->Canvas->TextWidth(UnicodeString(intervals[0].endValue))/2, 0, UnicodeString(intervals[0].endValue));
if (simmetric) {
        axeBitmap->Canvas->MoveTo(shiftOfPicks+2*intervals[0].sizeInPixels-1, axeBitmap->Height);
        axeBitmap->Canvas->LineTo(shiftOfPicks+2*intervals[0].sizeInPixels-1, axeBitmap->Height-10);
		axeBitmap->Canvas->TextOut(shiftOfPicks+2*intervals[0].sizeInPixels-1-axeBitmap->Canvas->TextWidth(UnicodeString(intervals[0].startValue))/2, 0, UnicodeString(intervals[0].startValue));
}
/////////отрисовка
for (int i=0; i<axe.Length; i++)
{
        axeBitmap->Canvas->MoveTo(axe[i].pickCoord1.x, axe[i].pickCoord1.y);
        axeBitmap->Canvas->LineTo(axe[i].pickCoord2.x, axe[i].pickCoord2.y);
        axeBitmap->Canvas->TextOut(axe[i].signCoord.x,axe[i].signCoord.y,axe[i].sign);
}
if (simmetric) {
for (int i=0; i<axe.Length; i++)
{
        axe[i].pickCoord1.x = 2*intervals[0].sizeInPixels-1 - axe[i].pickCoord1.x + 2*shiftOfPicks;
        axeBitmap->Canvas->MoveTo(axe[i].pickCoord1.x, axe[i].pickCoord1.y);
        axeBitmap->Canvas->LineTo(axe[i].pickCoord1.x, axe[i].pickCoord2.y);
        axeBitmap->Canvas->TextOut(axe[i].pickCoord1.x - axeBitmap->Canvas->TextWidth(axe[i].sign)/2,axe[i].signCoord.y,axe[i].sign);
}
}
return shiftOfPicks;
}
//---------------------------------------------------------------------------

//String TAxis2::svgDraw(void)
//{ //это рассчитано на вертикальный скважинный планшет, т.е. тип оси VERT_AXE_LEFT
//String res;
//int ii=axe.Length;
//if (ii==0) return res;
////надо ли рисовать делать прямоугольник фона?
////res+="<path fill=\"RGB("+String(GetRValue(params->backColor))+CommaStr+String(GetGValue(params->backColor))+CommaStr+String(GetBValue(params->backColor))+")\" d=\"M0 0h"+String(axeBitmap->Width)+"v"+String(axeBitmap->Height)+"H0z\" />\r\n";
//res+=L"<path fill=\"none\" stroke-width=\""+String(params->majorPicksWidth)+"\" stroke="+svgColor(params->picksColor)+"\r\nd=\"";
//for (int i=0; i<ii; i++) {
//	res+=L"M"+String(axeBitmap->Width-7)+SpaceStr+String(axe[i].pickCoord1.y)+L"h7";
//	if (i%30==0) res+=NewLineStr;
//}
//int jj=smallPicksCoords.Length;
//for (int i=0; i<jj; ++i) {
//	res+=L"M"+String(axeBitmap->Width-4)+SpaceStr+String(smallPicksCoords[i])+L"h4";
//	if (i%30==0) res+=NewLineStr;
//}
//res+=L"\"/>\r\n";
//res+=L"<g text-anchor=\"end\" dominant-baseline=\"alphabetic\" fill="+svgColor(params->axeFont->Color);
////res+=L"<text text-anchor=\"end\" dominant-baseline=\"text-before-edge\" fill="+svgColor(params->axeFont->Color);
//res+=L" font-size=\""+String(abs(params->axeFont->Height))+"\""+(params->axeFont->Style.Contains(fsBold) ? String(" font-weight=\"bold\"") : EmptyStr);
//res+=L" font-family=\""+params->axeFont->Name+"\">\r\n";
//axeBitmap->Canvas->Font->Assign(params->axeFont);
//TPoint fd=getFontDescent(axeBitmap->Canvas->Handle);
//fd.y-=fd.x;
//for (int i=0; i<ii; i++) {
//	res+="<text x=\""+String(axeBitmap->Width-10)+"\" y=\""+String(axe[i].signCoord.y+fd.y)+"\">"+axe[i].sign+"</text>";
//	if (i%30==0) res+=NewLineStr;
//}
////res+=L"</text>\r\n";
//res+=L"</g>\r\n";
//return res;
//}

//---------------------------------------------------------------------------

