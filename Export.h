//---------------------------------------------------------------------------

#ifndef ExportH
#define ExportH

#include <functional>
#include <vcl.h>
#ifdef MAKEDLL
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT __declspec(dllimport)
#endif
//----------------------------------------------------------------

extern class IBaseData;
extern class SeismicData;
extern class RgbData;

//Reads data from IBM profile
extern "C" SeismicData* EXPORT readDataIBM(std::wstring filePath);
//Reads data from internal formats
extern "C" RgbData* EXPORT readDataInternalFormat(std::wstring filePath, bool isRgb);
//Reads data from IBM cube
//Types 1 - inline | 2 - crossline | 3 - slice
extern "C" SeismicData* EXPORT readDataIBMCube(std::wstring filePath, int type, int pos);



extern "C" int EXPORT getMaxTraceS(SeismicData* data);
extern "C" int EXPORT getMaxTraceR(RgbData* data);

extern "C" void init();
//extern "C" void EXPORT setMoveCallback(TFormUniversal* form, void (*callback)(int));
extern "C" TForm* EXPORT CreateRgbBlendingForm(System::UnicodeString path, std::function<void(int)> callback, std::function<void(int, int)> callbackDisplay);
extern "C" TForm* EXPORT CreateRgbBlendingFormCube(System::UnicodeString path);
//-----------
#endif
