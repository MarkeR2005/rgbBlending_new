//---------------------------------------------------------------------------

#pragma hdrstop

#include "Structures.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
void Traces::init (int trNum, int spNum, int dT, float* dat)
{
    tracesNumber = trNum;
    samplesNumber = spNum;
    dt = dT;
	data = dat;
}
//--
void byteMap::init(int h, int w)
{
    height = h;
    width = w;
    std::vector<uint8_t> c(h*w, 0);
    color = c;
}
//--
void ArrayStats::init(float m, float l, float r)
{
    median = m;
    lower_bound = l;
    upper_bound = r;
}
