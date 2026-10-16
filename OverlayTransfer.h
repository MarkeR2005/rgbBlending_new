#ifndef OverlayTransferH
#define OverlayTransferH

#include "Structures.h"
#include <algorithm>
#include <cmath>

// The X coordinate of a SWAN image is frequency, while its Y coordinate
// still represents the samples of one trace in the source section.
enum class OverlayMapping { SameGrid, Trace, Spectrum };

struct TransferredOverlays {
    std::vector<Horizon> horizons;
    std::vector<Cross> crosses;
};

inline TransferredOverlays transferOverlays(
    const std::vector<Horizon>& sourceHorizons,
    const std::vector<Cross>& sourceCrosses,
    int sourceWidth, int targetWidth, OverlayMapping mapping, int trace = 0)
{
    TransferredOverlays result;
    if (sourceWidth <= 0 || targetWidth <= 0) return result;
    if (mapping == OverlayMapping::SameGrid && sourceWidth != targetWidth)
        return result;
    if (mapping != OverlayMapping::SameGrid && (trace < 0 || trace >= sourceWidth))
        return result;

    result.horizons.reserve(sourceHorizons.size());
    for (const auto& original : sourceHorizons) {
        Horizon horizon = original; // preserves color, contrast and name
        horizon.points.assign(static_cast<size_t>(targetWidth), -1.0f);
        if (mapping == OverlayMapping::SameGrid) {
            for (int x = 0; x < targetWidth && static_cast<size_t>(x) < original.points.size(); ++x)
                if (std::isfinite(original.points[x]) && original.points[x] >= 0)
                    horizon.points[x] = original.points[x];
        } else if (static_cast<size_t>(trace) < original.points.size()) {
            const float ordinate = original.points[trace];
            if (std::isfinite(ordinate) && ordinate >= 0)
                std::fill(horizon.points.begin(), horizon.points.end(), ordinate);
        }
        result.horizons.push_back(std::move(horizon));
    }
    if (mapping != OverlayMapping::Spectrum) {
        for (auto cross : sourceCrosses) {
            if (mapping == OverlayMapping::Trace) {
                if (cross.x != trace) continue;
                cross.x = 0;
            } else if (cross.x < 0 || cross.x >= targetWidth) {
                continue;
            }
            result.crosses.push_back(std::move(cross));
        }
    }
    return result;
}

#endif
