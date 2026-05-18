#include "db_make_bounds.h"

#include <memory>

#include "SgBlackWhite.h"
#include "ThGraph.h"
#include "ThScaffold.h"
#include "ThValue.h"
#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "fraction.h"
#include "thermograph_helpers.h"
#include "sumgame.h"


using namespace std;

#ifdef MCGS_USE_THERM

#define RATIONAL_EPSILON_DENOMINATOR 1

////////////////////////////////////////////////// helpers
namespace {
inline unique_ptr<dyadic_rational> get_epsilon(bool positive)
{
    const int p = positive ? 1 : -1;
    return make_unique<dyadic_rational>(p, RATIONAL_EPSILON_DENOMINATOR);
}

bool sum_is_positive(sumgame& sum)
{
    assert_restore_sumgame ars(sum);
    const bw restore_player = sum.to_play();

    bool is_maybe_positive = true;

    {
        sum.set_to_play(BLACK);
        const bool black_wins = sum.solve();
        is_maybe_positive = black_wins;
    }

    if (is_maybe_positive)
    {
        sum.set_to_play(WHITE);
        const bool white_wins = sum.solve();
        is_maybe_positive = !white_wins;
    }

    sum.set_to_play(restore_player);
    return is_maybe_positive;
}

bool sum_is_negative(sumgame& sum)
{
    assert_restore_sumgame ars(sum);
    const bw restore_player = sum.to_play();

    bool is_maybe_negative = true;

    {
        sum.set_to_play(WHITE);
        const bool white_wins = sum.solve();
        is_maybe_negative = white_wins;
    }

    if (is_maybe_negative)
    {
        sum.set_to_play(BLACK);
        const bool black_wins = sum.solve();
        is_maybe_negative = !black_wins;
    }

    sum.set_to_play(restore_player);
    return is_maybe_negative;
}

/*
    True IFF for epsilon := 1 / RATIONAL_EPSILON_DENOMINATOR,
        -epsilon < sum < epsilon
*/
bool sum_is_small(sumgame& sum)
{
    assert_restore_sumgame ars(sum);
    const bw restore_player = sum.to_play();

    bool is_maybe_small = true;

    /*
        Check: -epsilon < sum
            IFF: 0 < sum + epsilon
            IFF: sum + epsilon > 0
    */
    {
        unique_ptr<dyadic_rational> epsilon_unique = get_epsilon(true);
        dyadic_rational* epsilon = epsilon_unique.get();
        sum.add(epsilon);

        is_maybe_small = sum_is_positive(sum);

        sum.pop(epsilon);
    }

    /*
        Check: sum < epsilon
            IFF: sum - epsilon < 0
    */
    if (is_maybe_small)
    {
        unique_ptr<dyadic_rational> epsilon_unique = get_epsilon(false);
        dyadic_rational* epsilon = epsilon_unique.get();
        sum.add(epsilon);

        is_maybe_small = sum_is_negative(sum);

        sum.pop(epsilon);
    }

    sum.set_to_play(restore_player);
    return is_maybe_small;
}



} // namespace

//////////////////////////////////////////////////
std::shared_ptr<game_bounds> db_make_bounds(const database& db, sumgame& sum,
                                            const db_entry_partisan& entry)
{
    assert_restore_sumgame ars(sum);

    vector<bounds_options> options_vec;
    vector<game_bounds_ptr> bounds_vec;

    const std::shared_ptr<const ThGraph> therm = entry.thermograph;
    assert(therm);

    const bool is_small = game_is_small_from_thermograph(*therm);

    if (is_small)
    {
        bound_t scale_min = -512;
        bound_t scale_max = 512;

        assert(entry.outcome != outcome_class::U);

        if (entry.outcome == outcome_class::L)
            scale_min = 0;
        if (entry.outcome == outcome_class::R)
            scale_max = 0;

        options_vec.emplace_back(BOUND_SCALE_UP, scale_min, scale_max);
        bounds_vec = find_bounds(sum, options_vec);
        assert(options_vec.size() == 1 && bounds_vec.size() == 1);

        game_bounds_ptr bounds = bounds_vec.back();

        assert(bounds->both_valid());
        if (bounds->both_valid())
            return bounds;
    }

    //options_vec.clear();
    //options_vec.emplace_back(BOUND_SCALE_DYADIC_RATIONAL, -512, 512);
    //bounds_vec = find_bounds(sum, options_vec);
    //assert(options_vec.size() == 1 && bounds_vec.size() == 1);

    //game_bounds_ptr bounds = bounds_vec.back();

#if 1
    {
        //cout << "START" << endl;
        //cout << sum << endl;

        //cout << "Bounds (EXP): " << *bounds << endl;

        // Read bounds from thermograph

        //print_thermograph(cout, th);
        //cout << endl;

        //th.Check();

        const ThValue& left_stop = therm->LeftStop();
        const ThValue& right_stop = therm->RightStop();

        const bool left_bends = thermograph_bends_out_below_zero(*therm, true);
        const bool right_bends = thermograph_bends_out_below_zero(*therm, false);

        // Need to convert to MCGS scale (each scale ordinal is 1/8th)
        fraction left_frac(left_stop.P() * 8, left_stop.Q());
        left_frac.simplify();
        int left_int = left_frac.get_integral_part();
        left_frac.remove_integral_part();
        const bool left_is_exact = (left_frac.top() == 0);
        assert(left_is_exact);

        fraction right_frac(right_stop.P() * 8, right_stop.Q());
        right_frac.simplify();
        int right_int = right_frac.get_integral_part();
        right_frac.remove_integral_part();
        const bool right_is_exact = (right_frac.top() == 0);
        assert(right_is_exact);

        //cout << "Bendy-ness: " << left_bends << " " << right_bends << endl;

        if (left_bends)
            left_int++;

        if (right_bends)
            right_int--;

        const bool is_equal = (left_int == right_int) && !(left_bends || right_bends);

        //cout << "Setting bounds: ";
        //cout << "Lower: " << (is_equal ? "=" : "<") << right_int;
        //cout << " ";
        //cout << "Upper: " << left_int << (is_equal ? "=" : ">");
        //cout << endl;

        game_bounds_ptr bounds_therm(new game_bounds(BOUND_SCALE_DYADIC_RATIONAL));
        bounds_therm->set_lower(right_int, is_equal ? REL_EQUAL : REL_LESS);
        bounds_therm->set_upper(left_int, is_equal ? REL_EQUAL : REL_GREATER);

        //cout << *bounds << " " << *bounds_therm << endl;

        //if (*bounds_therm != *bounds)
        //    assert(false);

        //assert(*bounds == *bounds_therm);
        return bounds_therm;
    }
#endif

    //if (bounds->both_valid())
    //    return bounds;


    THROW_ASSERT(false, "Bounds not found for game");
    return nullptr;
}
#else
std::shared_ptr<game_bounds> db_make_bounds(sumgame& sum)
{
    assert(false);
}
#endif
