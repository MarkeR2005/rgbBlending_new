//---------------------------------------------------------------------------
#include "Structures.h"
#include <vector>
#include <vcl.h>
#include "RgbData.h"
#ifndef ReaderH
#define ReaderH
//---------------------------------------------------------------------------
Traces readInlineRegular(std::wstring fName, int l);
Traces readIBM(std::wstring fName);
//--
Traces readTimeSliceRegular(std::wstring fName, int timeSample);
//--
Traces readCrosslineRegular(std::wstring fName, int crosslineNumber);
//--
struct size{
	int samples;
	int traces;
	int lines;
};
struct size4{
	int samples;
	int traces;
	int lines;
    int filters;
};
size readSize(std::wstring fName);
size4 readSizeRGB(std::wstring fName, std::vector<float>& freqOut);


std::shared_ptr<RgbData> readCrosslineRGB(std::wstring fName, int trace);
std::shared_ptr<RgbData> readInlineRGB(std::wstring fName, int line);
std::shared_ptr<RgbData> readTimeSliceRGB(std::wstring fileName, int timeSample);
#endif
