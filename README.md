# rgbBlending_new

Windows VCL/OpenGL module for seismic and RGB views. The existing cube/3D renderer uses its original texture array path.

## 2D RGB memory use

The 2D RGB window keeps the source `RgbData` in RAM. It converts only the three selected frequency filters through `ColorManager`, combines them into one RGB buffer and uploads a single `GL_RGB8` texture. Changing channel indices rebuilds that texture; changing visibility, contrast, zoom or pan uses the existing texture. Repeated indices are converted once. The full frequency stack is no longer allocated as a 2D GPU texture array. The OpenGL maximum single texture dimension still applies.

Keep the `shaders` directory alongside the application executable; it now also needs `rgb_composite.frag`, `cross_label.vert` and `cross_label.frag`.

## Horizons and crosses

`Structures.h` defines `Horizon { std::wstring name; std::vector<float> points; }` and `Cross { std::wstring name; int x; }`. Each horizon ordinate corresponds to a trace index; negative or nonfinite ordinates leave a gap. Cross `x` is a trace index. Ordinate values use the same units as the existing horizon editor (sample position multiplied by the window's `dT`).

The existing `CreateRgbBlendingForm(path, callback, callbackDisplay)` entry point remains available. For an initialized 2D form, the exported functions `setHorizons(form, horizons)` and `setCrosses(form, crosses)` replace the corresponding lists. `setTraceCallback(form, callback)`, `setDisplayCallback(form, callback)` and `setHorizonsCallback(form, callback)` register callbacks on a specific form and return `false` for a form of the wrong type. The **Обновить горизонты** button sends a snapshot of all named horizons through the registered horizons callback; edits do not invoke it automatically.

The context menu's **Edit horizon (draw with mouse)** action asks for the zero-based horizon number and then uses the existing drag/interpolation interaction. Select the action again to leave editing mode. If the requested number does not exist, blank horizons are created through that number. The legacy text, CSV and binary horizon import/export commands still read/write concatenated point arrays; imported horizons receive default names.

With the OpenGL view focused, press **H** to show/hide horizons and **C** to show/hide crosses. Both layers invert the pixels beneath them. At their intersections, two inversions can cancel.
