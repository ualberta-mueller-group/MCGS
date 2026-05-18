#include "thermograph_helpers.h"

#include <cassert>
#include "ThGraph.h"
#include "ThValue.h"
#include "all_game_headers.h"
#include "amazons.h"
#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "make_thermograph_slow.h"
#include "safe_arithmetic.h"
#include "utilities.h"

using namespace std;

/*
    Get specified rational bound along BOUND_SCALE_DYADIC_RATIONAL, where the
    scale index represents a multiple of 1/8.

    For a game G and the returned left bound B, G <= B.
*/
bound_t read_rational_bound_from_thermograph(const ThGraph& graph, bool left)
{
    const ThValue& stop = left ? graph.LeftStop() : graph.RightStop();

    int p = stop.P();
    const int q = stop.Q();

    // Divide the stop by 1/8 by multiplying p by (8 == 2^3)
    const bool multiply_ok = safe_mul2_shift(p, 3);
    THROW_ASSERT(multiply_ok);

#warning TODO use safe numeric_cast from header
    bound_t scale_idx = static_cast<bound_t>(p / q);

    const bool is_precise = (p % q) == 0;
    const bool is_fuzzy = thermograph_bends_out_below_zero(graph, left);

    // Relax the bound if necessary
    if (!is_precise || is_fuzzy)
    {
        const bound_t relax_direction = left ? 1 : -1;
        const bool add_ok = safe_add_negatable(scale_idx, relax_direction);
        THROW_ASSERT(add_ok);
    }

    return scale_idx;
}

bool thermograph_bends_out_below_zero(const ThGraph& graph, bool left)
{
    const SgBlackWhite color = (left ? SG_BLACK : SG_WHITE);
    const ThScaffold* sc = graph.Sc(color);
    assert(sc != nullptr);

    const int dir = sc->DirectionTo(ThValue(0));

    assert(LOGICAL_IMPLIES(dir != 0, dir == left ? -1 : 1));

    return dir != 0;


    //ThPoint p1, p2;
    //int idx_ignored;
    //bool last_ignored;
    //sc->SegmentAt(ThValue(0), &p1, &p2, &idx_ignored, &last_ignored);

    //assert(p1.Temp() < p2.Temp());

    //return p1.Value() != stop;
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

game_bounds* make_bounds_from_thermograph(const ThGraph& graph)
{
    game_bounds* gb = new game_bounds(BOUND_SCALE_DYADIC_RATIONAL);

    const bound_t left_bound = read_rational_bound_from_thermograph(graph, true);
    const bound_t right_bound = read_rational_bound_from_thermograph(graph, false);

    const bool equality = left_bound == right_bound;

    const relation rel_left = equality ? REL_EQUAL : REL_GREATER;
    const relation rel_right = equality ? REL_EQUAL : REL_LESS;

    gb->set_upper(left_bound, rel_left);
    gb->set_lower(right_bound, rel_right);

    return gb;
}

