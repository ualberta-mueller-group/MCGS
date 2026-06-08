#include "game_bounds_test.h"

#include <cassert>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>
#include <optional>

#include "bounds.h"
#include "cgt_basics.h"
#include "bounds_finder.h"
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "cgt_up_star.h"
#include "sumgame.h"
#include "sumgame_helpers.h"
#include "all_game_headers.h"
#include "utilities.h"

using namespace std;

namespace {
////////////////////////////////////////////////// Helpers
void test_find_initial_interval_impl(sumgame& sum, bound_scale scale,
                                     bool use_outcome_hint)
{
    assert_restore_sumgame ars(sum);
    restore_sumgame_player restore(sum);

    bounds_finder bf;

    bounds_options opt(scale, -64, 64);
    assert(opt.outcome_hint == outcome_class::U);

    if (use_outcome_hint)
    {
        opt.outcome_hint = get_sum_outcome(sum);
        assert(opt.outcome_hint != outcome_class::U);
    }

    unique_ptr<game_bounds> bounds = make_unique<game_bounds>(scale);

    const search_region initial_interval =
        bf.find_initial_interval(sum, opt, bounds.get());

    // Validate initial interval
    assert(initial_interval.valid());
    assert(sum_rel_scale_game(sum, REL_GREATER_OR_EQUAL, scale,
                              initial_interval.low));
    assert(sum_rel_scale_game(sum, REL_LESS_OR_EQUAL, scale,
                              initial_interval.high));

    // Validate bounds
    assert(logical_iff(scale != BOUND_SCALE_UP_STAR, bounds->both_valid()));
    if (scale != BOUND_SCALE_UP_STAR)
    {
        assert(sum_rel_scale_game(sum,
                                  flip_relation(bounds->get_lower_relation()),
                                  scale, bounds->get_lower()));
        assert(sum_rel_scale_game(sum,
                                  flip_relation(bounds->get_upper_relation()),
                                  scale, bounds->get_upper()));
    }

    // Validate fuzzy interval
    const bounds_finder::fuzzy_interval_t& fuzzy_interval =
        bf.get_fuzzy_interval();
    if (fuzzy_interval.has_value())
    {
        assert(fuzzy_interval->first <= fuzzy_interval->second);
        for (bound_t i = fuzzy_interval->first; i <= fuzzy_interval->second;
             i++)
            assert(sum_rel_scale_game(sum, REL_FUZZY, scale, i));
    }
}

////////////////////////////////////////////////// Main test functions
void test_constructor()
{
    game_bounds gb(BOUND_SCALE_UP);

    assert(!gb.lower_valid());
    assert(!gb.upper_valid());
    assert(!gb.both_valid());
}

void test_setters_and_getters()
{
    game_bounds gb(BOUND_SCALE_UP);

    assert(!gb.lower_valid());
    assert(!gb.upper_valid());
    assert(!gb.both_valid());

    gb.set_lower(-5, REL_LESS_OR_EQUAL);

    assert(gb.lower_valid());
    assert(!gb.upper_valid());
    assert(!gb.both_valid());

    gb.set_upper(5, REL_GREATER_OR_EQUAL);

    assert(gb.lower_valid());
    assert(gb.upper_valid());
    assert(gb.both_valid());

    assert(gb.get_lower() == -5);
    assert(gb.get_lower_relation() == REL_LESS_OR_EQUAL);

    assert(gb.get_upper() == 5);
    assert(gb.get_upper_relation() == REL_GREATER_OR_EQUAL);
}

void test_midpoint()
{
    game_bounds gb(BOUND_SCALE_UP);
    gb.set_lower(-3, REL_LESS);
    gb.set_upper(7, REL_GREATER_OR_EQUAL);

    assert(gb.get_midpoint() == 2);
}

void test_invalidate()
{
    game_bounds gb(BOUND_SCALE_UP);
    gb.set_lower(-6, REL_LESS);
    gb.set_upper(-4, REL_GREATER_OR_EQUAL);

    assert(gb.both_valid());

    gb.invalidate_lower();

    assert(!gb.lower_valid());
    assert(gb.upper_valid());
    assert(!gb.both_valid());

    gb.set_lower(-6, REL_LESS);
    gb.invalidate_upper();

    assert(gb.lower_valid());
    assert(!gb.upper_valid());
    assert(!gb.both_valid());

    gb.set_upper(-4, REL_GREATER_OR_EQUAL);

    assert(gb.both_valid());
    gb.invalidate_both();
    assert(!gb.both_valid());
}


void test_clip_using_fuzzy_interval()
{
    /*
       - Initial search region
       - Fuzzy interval
       - Resulting search region(s)
    */
    // clang-format off
    typedef tuple<
        search_region,
        pair<bound_t, bound_t>,
        optional<search_region>,
        optional<search_region>
    >
    test_case_t;

    /*
        Graphically represented in comments, using '[' and ']' to denote
        initial region and '{' and '}' to denote fuzzy interval.
    */
    const vector<test_case_t> test_cases
    {
        //// No overlap
        // {}[]
        {
            {-10, 10},
            {-12, -11},
            {{-10, 10}},
            {},
        },

        // []{}
        {
            {-10, 10},
            {11, 12},
            {{-10, 10}},
            {},
        },

        // [] ... {}
        {
            {-10, 10},
            {20, 23},
            {{-10, 10}},
            {},
        },


        //// Partial overlap
        // {[} ... ]
        {
            {-10, 10},
            {-11, -10},
            {{-9, 10}},
            {},
        },

        // [ ... {]}
        {
            {-10, 10},
            {10, 11},
            {{-10, 9}},
            {},
        },

        // { ... [ ... } ... ]
        {
            {-10, 10},
            {-20, 5},
            {{6, 10}},
            {},
        },

        //// Complete overlap
        // [ ... { . } .. ]
        {
            {-20, 20},
            {-2, 6},
            {{-20, -3}},
            {{7, 20}},
        },


        // {[ .. ]}
        {
            {-10, 10},
            {-10, 10},
            {},
            {},
        },

        {
            {-10, 10},
            {-11, 11},
            {},
            {},
        },

        // {[]}
        {
            {8, 8},
            {8, 8},
            {},
            {},
        },
    };

    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const search_region& input_region = get<0>(test_case);
        const pair<bound_t, bound_t>& fuzzy_interval = get<1>(test_case);
        const optional<search_region>& exp_region_1 = get<2>(test_case);
        const optional<search_region>& exp_region_2 = get<3>(test_case);

        search_region sr = input_region;
        vector<search_region> regions_next;

        bounds_finder::clip_using_fuzzy_interval(sr, regions_next, fuzzy_interval);

        assert(logical_iff(exp_region_1.has_value(), sr.valid()));
        if (exp_region_1.has_value())
            assert(*exp_region_1 == sr);

        assert(logical_iff(exp_region_2.has_value(), !regions_next.empty()));
        if (exp_region_2.has_value())
        {
            assert(regions_next.size() == 1);
            assert(*exp_region_2 == regions_next.back());
        }
    }
}

void test_find_initial_interval()
{
    sumgame sum(BLACK);

    typedef tuple<game*, bool> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases
    {
        {new clobber_1xn("XOXO"), true},
        {new clobber_1xn("XXO"), true},
        {new clobber_1xn("XOXOXO"), true},
        {new clobber_1xn("XXOXO"), true},

        {new up_star(4, true), true},
        {new up_star(21, false), true},
        {new up_star(0, true), true},
        {new up_star(0, false), true},
        {new up_star(-3, true), true},
        {new up_star(-7, false), true},

        {new domineering("...|..."), false},
        {new domineering(".#.|.#.|..."), false},

        {new nogo_1xn("....."), true},
        {new nogo_1xn("X..O..X..O."), false},
    };
    // clang-format on

    for (test_case_t& test_case : test_cases)
    {
        game* g = get<0>(test_case);
        const bool use_infinitesimal_scales = get<1>(test_case);

        assert(sum.num_total_games() == 0);
        sum.add(g);

        for (const bound_scale scale : BOUND_SCALES_ALL)
        {
            if (!use_infinitesimal_scales && scale_is_infinitesimal(scale))
                continue;

            test_find_initial_interval_impl(sum, scale, false);
            test_find_initial_interval_impl(sum, scale, true);
        }

        sum.pop(g);
        delete g;
    }
}

} // namespace

void game_bounds_test_all()
{
    test_constructor();
    test_setters_and_getters();
    test_midpoint();
    test_invalidate();

    test_clip_using_fuzzy_interval();
    test_find_initial_interval();
}
