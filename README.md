# rgbBlending_new

Windows VCL/OpenGL module for seismic and RGB views. The existing cube/3D renderer uses its original texture array path.

## 2D RGB memory use

The 2D RGB window keeps the source `RgbData` in RAM. It converts only the three selected frequency filters through `ColorManager`, combines them into one RGB buffer and uploads a single `GL_RGB8` texture. Changing channel indices rebuilds that texture immediately for cached filters, or after a 250 ms pause in scrollbar input for uncached filters; changing visibility, contrast, zoom or pan uses the existing texture. Converted 8-bit filters are reused from an LRU cache capped at 128 MiB and eight filters. Repeated indices are converted once. The RGB staging buffer is reused across updates. A cold, uncached filter still requires conversion of its full image. The full frequency stack is no longer allocated as a 2D GPU texture array. The OpenGL maximum single texture dimension still applies.

The 2D RGB, highlight, horizon and cross-label shaders are embedded in the module, so opening a 2D RGB window does not depend on deploying shader files. The original `shaders` directory is still needed for other render paths. Missing file-based shaders now report their filename rather than an opaque GLSL error.

## Horizons and crosses

`Structures.h` defines `Horizon { std::wstring name; std::vector<float> points; }` and `Cross { std::wstring name; int x; }`. Each horizon ordinate corresponds to a trace index; negative or nonfinite ordinates leave a gap. Cross `x` is a trace index. Ordinate values use the same units as the existing horizon editor (sample position multiplied by the window's `dT`).

`CreateRgbBlendingForm(path, callback, callbackDisplay)` keeps its export name, but its display callback now takes a third `std::string` parameter. Rebuild dynamic clients with the updated `Export.h`. This header does not include `Structures.h` or expose the internal `Horizon`/`Cross` classes. `RgbHorizons` and `RgbCrosses` are public aliases for vectors of `(name, values)` pairs. The old data-object exports remain available to callers that explicitly include `LegacyDataExports.h`.

For a client that loads the DLL with `GetProcAddress`, include only `Export.h` and use its function pointer typedefs. For example:

```cpp
#include <Windows.h>
#include "Export.h"

void __cdecl horizonsUpdated(const RgbHorizons& items, void* context) {
    // Copy items if they are needed after this callback returns.
}

void openView(const wchar_t* dllPath, const wchar_t* dataPath, void* userContext) {
    HMODULE dll = LoadLibraryW(dllPath);
    if (!dll) return;

    auto create = reinterpret_cast<CreateRgbBlendingFormFn>(
        GetProcAddress(dll, "CreateRgbBlendingForm"));
    auto putHorizons = reinterpret_cast<SetHorizonsFn>(
        GetProcAddress(dll, "setHorizons"));
    auto putCrosses = reinterpret_cast<SetCrossesFn>(
        GetProcAddress(dll, "setCrosses"));
    auto onHorizons = reinterpret_cast<SetHorizonsCallbackFn>(
        GetProcAddress(dll, "setHorizonsCallback"));
    if (!create || !putHorizons || !putCrosses || !onHorizons) {
        FreeLibrary(dll);
        return;
    }

    TForm* form = create(System::UnicodeString(dataPath),
        [](int trace) { /* map callback */ },
        [](int trace, int sample, std::string info) { /* display callback */ });
    if (!form) {
        FreeLibrary(dll);
        return;
    }

    RgbHorizons horizons = {{L"Top", {120.0f, 121.0f, 123.0f}}};
    RgbCrosses crosses = {{L"Line 42", 42}};
    RgbHorizonColors colors = {0xFF8040}; // red, green, blue; 0xRRGGBB
    putHorizons(form, horizons, colors);
    // To use the default negative mode: putHorizons(form, horizons, {});
    putCrosses(form, crosses);
    onHorizons(form, horizonsUpdated, userContext);
    auto setDisplay = reinterpret_cast<SetDisplayCallbackFn>(
        GetProcAddress(dll, "setDisplayCallback"));
    if (setDisplay) setDisplay(form, [](int trace, int sampleTime, std::string info) {
        /* update the information display */
    });
    form->Show();
    // Keep dll loaded while form and callbacks are in use.
}
```

An empty vector clears the corresponding list. The **Обновить горизонты** button calls `horizonsUpdated` with a snapshot of all named horizons; edits do not invoke it automatically. `setTraceCallback` and `setDisplayCallback` can be loaded the same way if callbacks need to change after creation. The display callback signature is `void(int trace, int sampleTime, std::string info)`; its third argument is UTF-8. After the mouse stays still for 200 ms, it receives `Ampl: N` for a seismic pixel (the 8-bit index minus 127, giving −127 through 128), `R:N, G:N, B:N` for the currently selected RGB bytes (0 through 255), or `Hor: name` / `Cross: name` over a visible overlay. The map trace callback keeps its original frequency. A `false` setter result indicates an invalid form or unsupported view. Since this interface passes VCL and STL types across the DLL boundary, the host and DLL must use compatible C++Builder toolchains and runtimes; keep the DLL loaded for the lifetime of the window.

The context menu's **Edit horizon (draw with mouse)** action opens a tree of named horizons with an entry for a new horizon. Selecting one starts the existing drag/interpolation interaction. Shift+left drag erases picks close to the cursor. Press the arrow keys to pan the section by 32 screen pixels per step while drawing (also available when the form has focus). Select the action again to leave editing mode. The new horizon name is entered in the field below the tree. Each name follows the leftmost portion of its horizon currently visible in the viewport, including a segment clipped at the viewport edge. The context menu also offers per-horizon color and contrast outline choices. `setHorizons` takes a third `RgbHorizonColors` argument with values in `0xRRGGBB` order; the direct export defaults it to an empty vector. For `GetProcAddress` calls pass `{}` explicitly to use negative mode. If the color list is shorter than the horizon list, remaining horizons use negative mode; surplus colors are ignored. Every call replaces the palette of the supplied horizons. Any negative or nonfinite ordinate means a missing point; strokes, labels and hover stop at gaps, while isolated valid points remain visible as dots. The legacy text, CSV and binary horizon import/export commands still read/write concatenated point arrays; imported horizons receive default names.

Use **View → Horizons (H)** or **View → Crosses (C)**, or press **H**/**C** while the OpenGL view or the VCL form has focus. The checked menu items follow the current visibility. Horizons invert pixels once at intersections and add a black outer stroke so they remain legible on midgray backgrounds. Crosses and their labels are black on seismic sections and white on RGB sections.
