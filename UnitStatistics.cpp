//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitStatistics.h"
#include <algorithm>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TUnitStatistic *UnitStatistic;
//---------------------------------------------------------------------------
__fastcall TUnitStatistic::TUnitStatistic(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TUnitStatistic::SetData(std::vector<point2F> _data){
data = _data;
Draw(this);
}
void __fastcall TUnitStatistic::Draw(TObject* Sender){
	int neededWidth = std::max(800, ClientWidth - 20); // Минимум 800px или по размеру формы
	int neededHeight = std::max(600, ClientHeight - 20);
	Image1->Width = neededWidth;
	Image1->Height = neededHeight;
	Image1->Repaint();
	Image1->Canvas->Brush->Color = clWhite;
	Image1->Canvas->FillRect(Rect(0, 0, Image1->Width, Image1->Height));
	std::vector<TPoint> points;
	float sz = data.size();
	if (sz == 0) {
		return;
	}
	float binWidth = static_cast<float>(Image1->Width) / (sz);
	float norm = 1.0f;
    for (int i = 0; i < sz; ++i)
	{
		norm = std::max(norm, abs(data[i].y));
	}
	int e_max = 0;
	for (int i = 0; i < sz; ++i)
	{
			int x = i * binWidth;
			int y = Image1->Height - ((data[i].y/norm+1.0f)/2.0f * Image1->Height);
			points.push_back(TPoint(x, y));
//			if (Swanebm.color[i+nf*swan.getFreq()]>e_max) {
//				e_max=Swanebm.color[i+nf*swan.getFreq()];
//			}
	}
	if (points.size() > 1)
	{
		Image1->Canvas->Pen->Color = clRed;
		Image1->Canvas->Pen->Width = 2;
		Image1->Canvas->MoveTo(points[0].x, points[0].y);

		for (size_t i = 1; i < points.size(); ++i)
		{
			Image1->Canvas->LineTo(points[i].x, points[i].y);
		}
	}
	Image1->Canvas->Pen->Color = clGreen;
//	Image1->Canvas->MoveTo(0,Image1->Height -  e_max/sqrt(2)* Image1->Height / 255);
//	Image1->Canvas->LineTo(Image1->Width, Image1->Height -  e_max/sqrt(2)* Image1->Height / 255);
	Image1->Canvas->Pen->Color = clBlack;
	Image1->Canvas->MoveTo(0,Image1->Height / 2);
	Image1->Canvas->LineTo(Image1->Width, Image1->Height /2);
	Image1->Canvas->MoveTo(Image1->Width / 2, 0);
	Image1->Canvas->LineTo(Image1->Width / 2, Image1->Height);
//	for (int i = 0; i < sz; ++i)
//	{
//			Image1->Canvas->MoveTo(i * binWidth, 0);
//			Image1->Canvas->LineTo(i * binWidth, Image1->Height);
//	}
}