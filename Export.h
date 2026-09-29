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

extern "C" void init();
//extern "C" void EXPORT setMoveCallback(TFormUniversal* form, void (*callback)(int));
extern "C" TForm* EXPORT CreateRgbBlendingForm(System::UnicodeString path, std::function<void(int)> callback, std::function<void(int, int)> callbackDisplay);
extern "C" TForm* EXPORT CreateRgbBlendingFormCube(System::UnicodeString path);
// Public transfer types. Pointers supplied to setters are copied before return.
// Negative/nonfinite point values mark gaps; x is a trace index.
struct RgbHorizonView {
    const wchar_t* name;       // nullptr means an empty name
    const float* points;       // nullptr is allowed only when pointCount == 0
    int pointCount;
};
struct RgbCrossView {
    const wchar_t* name;       // nullptr means an empty name
    int x;
};
// The callback receives temporary views. Copy names and points if they are needed
// after the callback returns. A null callback removes the registration.
typedef void (__cdecl *RgbHorizonsCallback)(const RgbHorizonView* horizons, int count, void* context);

// The form must be a live TFormUniversal returned by CreateRgbBlendingForm.
// Setter inputs are validated; count == 0 clears a list and permits nullptr.
extern "C" bool EXPORT setTraceCallback(TForm* f, std::function<void(int)> callback);
extern "C" bool EXPORT setDisplayCallback(TForm* f, std::function<void(int, int)> callback);
extern "C" bool EXPORT setHorizonsCallback(TForm* f, RgbHorizonsCallback callback, void* context);
extern "C" bool EXPORT setHorizons(TForm* f, const RgbHorizonView* horizons, int count);
extern "C" bool EXPORT setCrosses(TForm* f, const RgbCrossView* crosses, int count);
//-----------
#endif
