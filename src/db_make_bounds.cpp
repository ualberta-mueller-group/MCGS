/*
    TODO scale indices should be clearer, and there should be a better way
    to get i.e. integers

    TODO make a `cast_int<Int_From, Int_To>(Int_From)` function
*/
#include "db_make_bounds.h"

#include <algorithm>
#include <limits>
#include <tuple>

#include "bounds.h"
#include "cgt_dyadic_rational.h"
#include "database.h"
#include "fraction.h"
#include "sumgame.h"
#include "utilities.h"

using namespace std;


namespace {


inline pair<bound_scale, bound_t> get_bound_idx_int(int32_t val)
{
    return {BOUND_SCALE_DYADIC_RATIONAL, val * 8};
}

bool check_if_small(sumgame& sum)
{
    assert_restore_sumgame ars(sum);

    const bw restore_player = sum.to_play();
    bool in_range = true;

    /*
        -epsilon < sum
        0 < sum + epsilon
        sum + epsilon > 0
    */
    {
        unique_ptr<dyadic_rational> epsilon_unique(new dyadic_rational(1, 1024));
        dyadic_rational* epsilon = epsilon_unique.get();

        sum.add(epsilon);

        sum.set_to_play(BLACK);
        const bool black_wins = sum.solve();
        in_range = black_wins;

        if (in_range)
        {
            sum.set_to_play(WHITE);
            const bool white_wins = sum.solve();
            in_range = !white_wins;
        }

        sum.pop(epsilon);
    }

    /*
       sum < epsilon
       sum - epsilon < 0
    */
    if (in_range)
    {
        unique_ptr<dyadic_rational> epsilon_unique(new dyadic_rational(-1, 1024));
        dyadic_rational* epsilon = epsilon_unique.get();

        sum.add(epsilon);

        sum.set_to_play(BLACK);
        const bool black_wins = sum.solve();
        in_range = !black_wins;

        if (in_range)
        {
            sum.set_to_play(WHITE);
            const bool white_wins = sum.solve();
            in_range = white_wins;
        }

        sum.pop(epsilon);
    }

    sum.set_to_play(restore_player);
    return in_range;
}

} // namespace

//////////////////////////////////////////////////
optional<tuple<bound_scale, game_bounds_ptr>> db_make_bounds(database& db, sumgame& sum)
{
    assert_restore_sumgame ars(sum);

    vector<bounds_options> options_vec;
    vector<game_bounds_ptr> bounds_vec;

    const bool is_small = check_if_small(sum);

    if (is_small)
    {
        options_vec.emplace_back(BOUND_SCALE_UP, -64, 64);
        bounds_vec = find_bounds(sum, options_vec);
        assert(options_vec.size() == 1 && bounds_vec.size() == 1);

        game_bounds_ptr bounds = bounds_vec.back();

        if (bounds->both_valid())
            return tuple<bound_scale, game_bounds_ptr>(BOUND_SCALE_UP, bounds);
    }

    options_vec.clear();
    options_vec.emplace_back(BOUND_SCALE_DYADIC_RATIONAL, -64, 64);
    bounds_vec = find_bounds(sum, options_vec);
    assert(options_vec.size() == 1 && bounds_vec.size() == 1);

    game_bounds_ptr bounds = bounds_vec.back();

    if (bounds->both_valid())
        return tuple<bound_scale, game_bounds_ptr>(BOUND_SCALE_DYADIC_RATIONAL,
                                                   bounds);

    return {};
}
