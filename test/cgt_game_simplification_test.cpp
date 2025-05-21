#include "cgt_game_simplification_test.h"
#include <climits>
#include <functional>
#include "cgt_dyadic_rational.h"
#include "cgt_game_simplification.h"
#include "game.h"
#include "sumgame.h"
#include <memory>
#include "sumgame_change_record.h"
#include "sumgame_map_view.h"
#include "game_compare_utils.h"
#include "all_game_headers.h"
#include <vector>
#include <tuple>
#include <cassert>

using namespace std;
using compare_games_by_print::sumgame_same_games;

typedef vector<shared_ptr<game>> game_vec;
typedef tuple<game_vec, game_vec> test_case_t; // sumgame games, expected games
typedef function<void(sumgame_map_view&)> simplify_function;

namespace {
void do_single_test(test_case_t& test_case, simplify_function& fn)
{
    game_vec& sumgame_games = get<0>(test_case);
    game_vec& expected_result_games = get<1>(test_case);

    sumgame sum(BLACK);
    for (shared_ptr<game>& g : sumgame_games)
        sum.add(g.get());

    sumgame_impl::change_record cr;
    sumgame_map_view map_view(sum, cr);

    fn(map_view);

    assert(sumgame_same_games(sum, expected_result_games));
    cr.undo_simplify_basic(sum);
}

void do_all_tests(vector<test_case_t>& test_cases, simplify_function& fn)
{
    for (test_case_t& test : test_cases)
        do_single_test(test, fn);
}

void nimber_test()
{
    // clang-format off
    vector<test_case_t> test_cases
    {
        {{ // INPUT
                make_shared<nimber>(1),
                make_shared<nimber>(2),
                make_shared<nimber>(4),
        },{ // EXPECTED
                make_shared<nimber>(7),
        },}, // END

        {{ // INPUT
            make_shared<nimber>(1),
        },{ // EXPECTED
            make_shared<up_star>(0, true),
        },}, // END

        {{ // INPUT
            make_shared<nimber>(0),
        },{ // EXPECTED
        },}, // END

        {{ // INPUT
            make_shared<nimber>(2),
        },{ // EXPECTED
            make_shared<nimber>(2),
        },}, // END

        {{ // INPUT
            make_shared<nimber>(7),
        },{ // EXPECTED
            make_shared<nimber>(7),
        },}, // END

        {{ // INPUT
            make_shared<nimber>(1),
            make_shared<nimber>(2),
            make_shared<nimber>(7),
            make_shared<integer_game>(3),
        },{ // EXPECTED
            make_shared<nimber>(4),
            make_shared<integer_game>(3),
        },}, // END
    };
    // clang-format on

    simplify_function f = [](sumgame_map_view& map_view)
    {
        simplify_basic_nimber(map_view);
    };
    do_all_tests(test_cases, f);
}

void switch_test()
{
    // clang-format off
    vector<test_case_t> test_cases
    {
        {{ // INPUT
                make_shared<switch_game>(6, 3),
        },{ // EXPECTED
                make_shared<switch_game>(6, 3),
        },}, // END

        {{ // INPUT
                make_shared<switch_game>(6, 3),
                make_shared<integer_game>(0),
        },{ // EXPECTED
                make_shared<switch_game>(fraction(3, 2), fraction(-3, 2)),
                make_shared<dyadic_rational>(9, 2),
                make_shared<integer_game>(0),
        },}, // END

        {{ // INPUT
                make_shared<switch_game>(4, 4),
        },{ // EXPECTED
                make_shared<up_star>(0, true),
                make_shared<dyadic_rational>(4, 1),
        },}, // END

        {{ // INPUT
                make_shared<switch_game>(3, 21),
        },{ // EXPECTED
                make_shared<dyadic_rational>(4),
        },}, // END

        {{ // INPUT
                make_shared<switch_game>(fraction(91, 16), fraction(372, 64)),
        },{ // EXPECTED
                make_shared<dyadic_rational>(92, 16),
        },}, // END

        {{ // INPUT
                make_shared<switch_game>(fraction(-47, 8), fraction(437, 2)),
        },{ // EXPECTED
        },}, // END

        {{ // INPUT
                make_shared<switch_game>(fraction(-1425, 128), fraction(-177, 16)),
        },{ // EXPECTED
                make_shared<dyadic_rational>(fraction(-89, 8)),
        },}, // END
    };
    // clang-format on

    simplify_function f = [](sumgame_map_view& map_view)
    {
        simplify_basic_switch(map_view);
    };
    do_all_tests(test_cases, f);
}

void up_star_test()
{
    // clang-format off
    vector<test_case_t> test_cases
    {
        {{ // INPUT
                make_shared<up_star>(5, true),
                make_shared<up_star>(3, false),
                make_shared<up_star>(-7, true),
                make_shared<up_star>(-3, true),
        },{ // EXPECTED
                make_shared<up_star>(-2, true),
        },}, // END

        {{ // INPUT
                make_shared<up_star>(6, false),
        },{ // EXPECTED
                make_shared<up_star>(6, false),
        },}, // END

        {{ // INPUT
                make_shared<up_star>(0, false),
        },{ // EXPECTED
        },}, // END

        {{ // INPUT
                make_shared<up_star>(INT_MAX, false),
                make_shared<up_star>(6, false),
                make_shared<up_star>(INT_MIN + 1, false),
        },{ // EXPECTED
                make_shared<up_star>(6, false),
        },}, // END
    };
    // clang-format on

    simplify_function f = [](sumgame_map_view& map_view)
    {
        simplify_basic_up_star(map_view);
    };
    do_all_tests(test_cases, f);
}

void integers_rationals_test()
{
    // clang-format off
    vector<test_case_t> test_cases
    {
        {{ // INPUT
            make_shared<integer_game>(6),
            make_shared<dyadic_rational>(fraction(21, 4)),
            make_shared<integer_game>(3),
            make_shared<dyadic_rational>(fraction(43, 2)),
            make_shared<integer_game>(-2),
            make_shared<dyadic_rational>(fraction(-101, 16)),
        },{ // EXPECTED
            make_shared<dyadic_rational>(fraction(439, 16)),
        },}, // END

        {{ // INPUT
            make_shared<integer_game>(6),
            make_shared<dyadic_rational>(fraction(21, 4)),
        },{ // EXPECTED
            make_shared<dyadic_rational>(fraction(45, 4)),
        },}, // END

        {{ // INPUT
            make_shared<integer_game>(23),
        },{ // EXPECTED
            make_shared<integer_game>(23),
        },}, // END

        {{ // INPUT
            make_shared<dyadic_rational>(fraction(21, 4)),
        },{ // EXPECTED
            make_shared<dyadic_rational>(fraction(21, 4)),
        },}, // END

        {{ // INPUT
            make_shared<dyadic_rational>(fraction(0, 4)),
            make_shared<integer_game>(0),
        },{ // EXPECTED
        },}, // END

        {{ // INPUT
            make_shared<dyadic_rational>(fraction(0, 4)),
            make_shared<integer_game>(0),
        },{ // EXPECTED
        },}, // END
    };
    // clang-format on

    simplify_function f = [](sumgame_map_view& map_view)
    {
        simplify_basic_integers_rationals(map_view);
    };
    do_all_tests(test_cases, f);
}

void all_test()
{
    // clang-format off
    vector<test_case_t> test_cases
    {
        {{ // INPUT
        },{ // EXPECTED
        },}, // END

        {{ // INPUT

                make_shared<integer_game>(0),
                make_shared<switch_game>(fraction(5), fraction(-6)),
                make_shared<switch_game>(fraction(3), fraction(-7)),
                make_shared<switch_game>(fraction(61, 4), fraction(-131, 8)),
        },{ // EXPECTED
                make_shared<switch_game>(fraction(11, 2), fraction(-11, 2)),
                make_shared<switch_game>(fraction(5), fraction(-5)),
                make_shared<switch_game>(fraction(253, 16), fraction(-253, 16)),
                make_shared<dyadic_rational>(fraction(-49, 16)),
        },}, // END

        {{ // INPUT
            make_shared<integer_game>(0),
            make_shared<switch_game>(fraction(5), fraction(5)),
            make_shared<switch_game>(fraction(-7), fraction(3)),
            make_shared<switch_game>(fraction(-21, 8), fraction(9, 4)),
            make_shared<switch_game>(fraction(945, 32), fraction(41)),
            make_shared<switch_game>(fraction(-423, 128), fraction(-823, 256)),
        },{ // EXPECTED
            make_shared<up_star>(0, true),
            make_shared<dyadic_rational>(fraction(127, 4)),
        },}, // END

        {{ // INPUT
            make_shared<nimber>(7),
            make_shared<nimber>(3),
            make_shared<integer_game>(-7),
            make_shared<nimber>(13),
            make_shared<nimber>(16),
            make_shared<nimber>(14),
            make_shared<switch_game>(fraction(21, 8), fraction(-10, 2)),
            make_shared<switch_game>(fraction(-33, 2), fraction(35, 4)),
            make_shared<nimber>(6),
            make_shared<integer_game>(3),
            make_shared<switch_game>(fraction(19, 4), fraction(37, 2)),
            make_shared<up_star>(21, true),
            make_shared<dyadic_rational>(-20, 32),
            make_shared<up_star>(-18, false),
            make_shared<dyadic_rational>(-20, 4),
            make_shared<switch_game>(fraction(48, 1), fraction(-12, 1)),
            make_shared<switch_game>(fraction(-33, 2), fraction(19, 2)),
            make_shared<nimber>(16),
        },{ // EXPECTED
            make_shared<switch_game>(fraction(61, 16), fraction(-61, 16)),
            make_shared<switch_game>(fraction(30), fraction(-30)),
            make_shared<up_star>(3, false),
            make_shared<dyadic_rational>(195, 16),
        },}, // END
    };
    // clang-format on

    simplify_function f = [](sumgame_map_view& map_view)
    {
        simplify_basic_all(map_view);
    };
    do_all_tests(test_cases, f);
}

} // namespace

void cgt_game_simplification_test_all()
{
    nimber_test();
    switch_test();
    up_star_test();
    integers_rationals_test();
    all_test();
}
