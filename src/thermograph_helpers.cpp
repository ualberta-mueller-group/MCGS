#include "thermograph_helpers.h"

#include <cassert>
#include "ThGraph.h"

bool thermograph_bends_out_below_zero(const ThGraph& graph, bool left)
{
    const ThValue& stop = (left ? graph.LeftStop() : graph.RightStop());

    const SgBlackWhite color = (left ? SG_BLACK : SG_WHITE);
    const ThScaffold* sc = graph.Sc(color);
    assert(sc != nullptr);

    ThPoint p1, p2;
    int idx_ignored;
    bool last_ignored;
    sc->SegmentAt(ThValue(0), &p1, &p2, &idx_ignored, &last_ignored);

    assert(p1.Temp() < p2.Temp());

    return p1.Value() != stop;
}

