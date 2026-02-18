//---------------------------------------------------------------------------

#ifndef EventsH
#define EventsH
//---------------------------------------------------------------------------
#pragma once
#include <string>

struct UpdateAllViews {};
struct ContrastChanged { float value; };
struct DataLoaded { std::wstring filename; };
struct CursorMoved { int x, y; };
#endif
