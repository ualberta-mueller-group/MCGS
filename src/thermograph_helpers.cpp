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

hash_t get_thermograph_hash(const ThGraph& graph)
{
    local_hash hash;

    const ThScaffold* sc_arr[2] = {graph.Sc(SG_BLACK), graph.Sc(SG_WHITE)};
    assert(sc_arr[0] != nullptr && sc_arr[1] != nullptr);

    hash.toggle_value(0, sc_arr[0]->NuPoints());
    hash.toggle_value(1, sc_arr[1]->NuPoints());

    int hash_position = 2;

    for (int sc_idx = 0; sc_idx < 2; sc_idx++)
    {
        const ThScaffold* sc = sc_arr[sc_idx];
        const int sc_len = sc->NuPoints();

        // Index in [1, len]
        for (int point_idx = 1; point_idx <= sc_len; point_idx++)
        {
            const ThPoint* point = sc->NthPoint(point_idx);
            assert(point != nullptr);

            const ThValue& value = point->Value();
            const ThValue& temp = point->Temp();

            hash.toggle_value(hash_position + 0, value.P());
            hash.toggle_value(hash_position + 1, value.Q());
            hash.toggle_value(hash_position + 2, temp.P());
            hash.toggle_value(hash_position + 3, temp.Q());

            hash_position += 4;
        }
    }

    return hash.get_value();
}

bool game_is_small_from_thermograph(const ThGraph& graph)
{
    const ThValue& left_stop = graph.LeftStop();
    const ThValue& right_stop = graph.RightStop();

    if (!left_stop.IsZero() || !right_stop.IsZero())
        return false;

    // Left bends?
    if (thermograph_bends_out_below_zero(graph, true))
        return true;

    // Right bends?
    if (thermograph_bends_out_below_zero(graph, false))
        return true;

    return false;
}

