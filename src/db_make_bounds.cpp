#include "db_make_bounds.h"

#include <memory>

#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "fraction.h"
#include "sumgame.h"

#define RATIONAL_EPSILON_DENOMINATOR 1024

#ifdef MCGS_USE_THERM

using namespace std;

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
std::shared_ptr<game_bounds> db_make_bounds(sumgame& sum)
{
    assert_restore_sumgame ars(sum);

    vector<bounds_options> options_vec;
    vector<game_bounds_ptr> bounds_vec;

    const bool is_small = sum_is_small(sum);

    if (is_small)
    {
        options_vec.emplace_back(BOUND_SCALE_UP, -64, 64);
        bounds_vec = find_bounds(sum, options_vec);
        assert(options_vec.size() == 1 && bounds_vec.size() == 1);

        game_bounds_ptr bounds = bounds_vec.back();

        if (bounds->both_valid())
            return bounds;
    }

    options_vec.clear();
    options_vec.emplace_back(BOUND_SCALE_DYADIC_RATIONAL, -64, 64);
    bounds_vec = find_bounds(sum, options_vec);
    assert(options_vec.size() == 1 && bounds_vec.size() == 1);

    game_bounds_ptr bounds = bounds_vec.back();

    if (bounds->both_valid())
        return bounds;

    THROW_ASSERT(false, "Bounds not found for game");
    return nullptr;
}
#else
std::shared_ptr<game_bounds> db_make_bounds(sumgame& sum)
{
    assert(false);
}
#endif
