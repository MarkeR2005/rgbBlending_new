#ifndef LegacyDataExportsH
#define LegacyDataExportsH

// Optional legacy data API. Its pointers refer to module-owned C++ classes;
// use Export.h alone for the window, callback, horizon and cross API.
#include "Export.h"
#include <string>
class SeismicData;
class RgbData;

extern "C" SeismicData* EXPORT readDataIBM(std::wstring filePath);
extern "C" RgbData* EXPORT readDataInternalFormat(std::wstring filePath, bool isRgb);
// type: 1 inline, 2 crossline, 3 slice
extern "C" SeismicData* EXPORT readDataIBMCube(std::wstring filePath, int type, int pos);
extern "C" int EXPORT getMaxTraceS(SeismicData* data);
extern "C" int EXPORT getMaxTraceR(RgbData* data);

#endif
