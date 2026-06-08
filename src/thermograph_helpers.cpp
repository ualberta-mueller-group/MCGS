#include "thermograph_helpers.h"

#include <cassert>
#include <ostream>

#include "ThGraph.h"
#include "ThValue.h"
#include "SgBlackWhite.h"
#include "ThScaffold.h"
#include "ThPoint.h"
#include "all_game_headers.h"
#include "amazons.h"
#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "integral_conversion.h"
#include "safe_arithmetic.h"
#include "utilities.h"
#include "throw_assert.h"

using namespace std;

void print_thermograph(ostream& os, const ThGraph& graph)
{
    os << "Left Scaffold: " << *graph.Sc(SG_BLACK);
    os << " Right Scaffold: " << *graph.Sc(SG_WHITE);
}

bound_t read_rational_bound_from_thermograph(const ThGraph& graph, bool left)
{
    const ThValue& stop = left ? graph.LeftStop() : graph.RightStop();

    int p = stop.P();
    int q = stop.Q();

    if (q < 0)
    {
        const bool negate_p_success = safe_negate(p);
        const bool negate_q_success = safe_negate(q);
        THROW_ASSERT(negate_p_success && negate_q_success);
    }

    // Divide the stop by 1/8 by multiplying p by (8 == 2^3)
    const bool multiply_ok = safe_mul2_shift(p, 3);
    THROW_ASSERT(multiply_ok);

    assert(q > 0);
    const int quotient = p / q;
    const int remainder = p % q;

    bound_t scale_idx = integral_cast_checked<bound_t>(quotient);

    assert(                            //
        LOGICAL_IMPLIES(               //
            remainder != 0,            //
            (remainder < 0) == (p < 0) //
            ));                        //

    const bool is_precise = (remainder == 0);
    const bool is_fuzzy = thermograph_bends_out_below_zero(graph, left);

    // Relax the bound if necessary
    if (!is_precise || is_fuzzy)
    {
        bound_t truncate_direction = 0;
        if (remainder != 0)
            truncate_direction = (remainder < 0) ? 1 : -1;

        const bound_t relax_direction = left ? 1 : -1;

        if (relax_direction != truncate_direction)
        {
            const bool add_ok = safe_add_negatable(scale_idx, relax_direction);
            THROW_ASSERT(add_ok);
        }
    }

    return scale_idx;
}

//bound_t read_rational_bound_from_thermograph(const ThGraph& graph, bool left)
//{
//    const ThValue& stop = left ? graph.LeftStop() : graph.RightStop();
//
//    int p = stop.P();
//    const int q = stop.Q();
//
//    // Divide the stop by 1/8 by multiplying p by (8 == 2^3)
//    const bool multiply_ok = safe_mul2_shift(p, 3);
//    THROW_ASSERT(multiply_ok);
//
//    bound_t scale_idx = integral_cast_checked<bound_t>(p / q);
//
//    const bool is_precise = (p % q) == 0;
//    const bool is_fuzzy = thermograph_bends_out_below_zero(graph, left);
//
//    // Relax the bound if necessary
//    if (!is_precise || is_fuzzy)
//    {
//        const bound_t relax_direction = left ? 1 : -1;
//        const bool add_ok = safe_add_negatable(scale_idx, relax_direction);
//        THROW_ASSERT(add_ok);
//    }
//
//    return scale_idx;
//}

bool thermograph_bends_out_below_zero(const ThGraph& graph, bool left)
{
    const SgBlackWhite color = (left ? SG_BLACK : SG_WHITE);
    const ThScaffold* sc = graph.Sc(color);
    assert(sc != nullptr);

    const int dir = sc->DirectionTo(ThValue(0));
    const int in_dir = left ? -1 : 1;

    // TODO not a reasonable assumption in general?
    assert(LOGICAL_IMPLIES( //
        dir != 0,           //
        dir == in_dir       //
        ));                 //

    return dir == in_dir;
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

    // Is 0
    return false;
}

game_bounds* make_rational_bounds_from_thermograph(const ThGraph& graph)
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

