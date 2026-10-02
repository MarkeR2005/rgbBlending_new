//---------------------------------------------------------------------------

#ifndef ExportH
#define ExportH

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <vcl.h>
#ifdef MAKEDLL
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT __declspec(dllimport)
#endif
//----------------------------------------------------------------

extern "C" void init();
//extern "C" void EXPORT setMoveCallback(TFormUniversal* form, void (*callback)(int));
extern "C" TForm* __cdecl EXPORT CreateRgbBlendingForm(System::UnicodeString path, std::function<void(int)> callback, std::function<void(int, int, std::string)> callbackDisplay);
extern "C" TForm* __cdecl EXPORT CreateRgbBlendingFormCube(System::UnicodeString path);
// Public STL types; no definitions from Structures.h are needed by the host.
// Each pair contains (name, ordinates) or (name, trace index), respectively.
// Negative or nonfinite ordinates mark gaps. Setters copy the lists before return.
typedef std::vector<std::pair<std::wstring, std::vector<float>>> RgbHorizons;
// One 0xRRGGBB color per horizon, in the same order. Missing entries use negative mode.
typedef std::vector<std::uint32_t> RgbHorizonColors;
typedef std::vector<std::pair<std::wstring, int>> RgbCrosses;

// The list passed to a callback is valid for the duration of the call.
// A null callback removes the registration.
typedef void (__cdecl *RgbHorizonsCallback)(const RgbHorizons& horizons, void* context);

// The form must be a live TFormUniversal returned by CreateRgbBlendingForm.
// An empty vector clears the list.
extern "C" bool __cdecl EXPORT setTraceCallback(TForm* f, std::function<void(int)> callback);
extern "C" bool __cdecl EXPORT setDisplayCallback(TForm* f, std::function<void(int, int, std::string)> callback);
extern "C" bool __cdecl EXPORT setHorizonsCallback(TForm* f, RgbHorizonsCallback callback, void* context);
extern "C" bool __cdecl EXPORT setHorizons(TForm* f, const RgbHorizons& horizons,
    const RgbHorizonColors& colors = RgbHorizonColors());
extern "C" bool __cdecl EXPORT setCrosses(TForm* f, const RgbCrosses& crosses);

// Function pointer signatures for LoadLibrary/GetProcAddress clients.
// Use the same C++Builder toolchain and C++ runtime on both sides of this API.
typedef TForm* (__cdecl *CreateRgbBlendingFormFn)(System::UnicodeString,
    std::function<void(int)>, std::function<void(int, int, std::string)>);
typedef TForm* (__cdecl *CreateRgbBlendingFormCubeFn)(System::UnicodeString);
typedef bool (__cdecl *SetTraceCallbackFn)(TForm*, std::function<void(int)>);
typedef bool (__cdecl *SetDisplayCallbackFn)(TForm*, std::function<void(int, int, std::string)>);
typedef bool (__cdecl *SetHorizonsCallbackFn)(TForm*, RgbHorizonsCallback, void*);
// GetProcAddress callers pass an empty vector when no colors are supplied.
typedef bool (__cdecl *SetHorizonsFn)(TForm*, const RgbHorizons&, const RgbHorizonColors&);
typedef bool (__cdecl *SetCrossesFn)(TForm*, const RgbCrosses&);
//-----------
#endif
