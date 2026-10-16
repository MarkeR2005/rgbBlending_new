#include "../OverlayTransfer.h"
#include <cassert>
#include <limits>

int main()
{
    Horizon colored;
    colored.name = L"Layer";
    colored.points = {10, -1, 30, std::numeric_limits<float>::quiet_NaN()};
    colored.useColor = true;
    colored.red = 12;
    std::vector<Horizon> horizons{colored};
    std::vector<Cross> crosses{{L"First", 0}, {L"Third", 2}};

    auto same = transferOverlays(horizons, crosses, 4, 4, OverlayMapping::SameGrid);
    assert(same.horizons.size() == 1 && same.horizons[0].points.size() == 4);
    assert(same.horizons[0].points[0] == 10);
    assert(same.horizons[0].points[1] == -1 && same.horizons[0].points[3] == -1);
    assert(same.horizons[0].useColor && same.horizons[0].red == 12);
    assert(same.crosses.size() == 2 && same.crosses[1].x == 2);

    auto trace = transferOverlays(horizons, crosses, 4, 1, OverlayMapping::Trace, 2);
    assert(trace.horizons[0].points == std::vector<float>{30});
    assert(trace.crosses.size() == 1 && trace.crosses[0].x == 0);

    auto gap = transferOverlays(horizons, crosses, 4, 1, OverlayMapping::Trace, 1);
    assert(gap.horizons[0].points == std::vector<float>{-1});
    assert(gap.crosses.empty());

    auto spectrum = transferOverlays(horizons, crosses, 4, 3, OverlayMapping::Spectrum, 2);
    assert((spectrum.horizons[0].points == std::vector<float>{30, 30, 30}));
    assert(spectrum.crosses.empty());
    assert(transferOverlays(horizons, crosses, 4, 5, OverlayMapping::SameGrid).horizons.empty());
    assert(transferOverlays(horizons, crosses, 4, 1, OverlayMapping::Trace, 9).horizons.empty());
}
