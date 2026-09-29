# rgbBlending_new

Windows VCL/OpenGL module for seismic and RGB views. The existing cube/3D renderer uses its original texture array path.

## 2D RGB memory use

The 2D RGB window keeps the source `RgbData` in RAM. It converts only the three selected frequency filters through `ColorManager`, combines them into one RGB buffer and uploads a single `GL_RGB8` texture. Changing channel indices rebuilds that texture immediately for cached filters, or after a 250 ms pause in scrollbar input for uncached filters; changing visibility, contrast, zoom or pan uses the existing texture. Converted 8-bit filters are reused from an LRU cache capped at 128 MiB and eight filters. Repeated indices are converted once. The RGB staging buffer is reused across updates. A cold, uncached filter still requires conversion of its full image. The full frequency stack is no longer allocated as a 2D GPU texture array. The OpenGL maximum single texture dimension still applies.

The 2D RGB, highlight, horizon and cross-label shaders are embedded in the module, so opening a 2D RGB window does not depend on deploying shader files. The original `shaders` directory is still needed for other render paths. Missing file-based shaders now report their filename rather than an opaque GLSL error.

## Horizons and crosses

`Structures.h` defines `Horizon { std::wstring name; std::vector<float> points; }` and `Cross { std::wstring name; int x; }`. Each horizon ordinate corresponds to a trace index; negative or nonfinite ordinates leave a gap. Cross `x` is a trace index. Ordinate values use the same units as the existing horizon editor (sample position multiplied by the window's `dT`).

The existing `CreateRgbBlendingForm(path, callback, callbackDisplay)` entry point remains available. `Export.h` has no dependency on `Structures.h` or the internal `Horizon`/`Cross` classes. The old data-object exports remain available to callers that explicitly include `LegacyDataExports.h`.

For an initialized 2D form, the exported functions accept public pointer-and-count views. Inputs are copied before the setter returns; passing `nullptr, 0` clears a list:

```cpp
void __cdecl horizonsUpdated(const RgbHorizonView* items, int count, void* context) {
    // Copy names and point arrays here if they are needed after this call.
}

void configureView(TForm* form, void* userContext) {
    float picked[] = {120.0f, 121.0f, 123.0f};
    RgbHorizonView horizons[] = {{L"Top", picked, 3}};
    RgbCrossView crosses[] = {{L"Line 42", 42}};
    setHorizons(form, horizons, 1);
    setCrosses(form, crosses, 1);
    setHorizonsCallback(form, horizonsUpdated, userContext);
}
```

The **Обновить горизонты** button calls that callback with a temporary snapshot of all named horizons; edits do not invoke it automatically. `setTraceCallback` and `setDisplayCallback` remain available for the existing `std::function` based integration. A `false` setter result indicates an invalid form, unsupported view, or invalid array argument.

The context menu's **Edit horizon (draw with mouse)** action opens a tree of named horizons with an entry for a new horizon. Selecting one starts the existing drag/interpolation interaction. Select the action again to leave editing mode. The new horizon name is entered in the field below the tree. Each horizon name appears just above its first valid (leftmost) point in the same negative drawing style as the lines. The legacy text, CSV and binary horizon import/export commands still read/write concatenated point arrays; imported horizons receive default names.

With the OpenGL view focused, press **H** to show/hide horizons and **C** to show/hide crosses. Both layers invert the pixels beneath them. At their intersections, two inversions can cancel.
