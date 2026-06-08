#include "sumgame_helpers_test.h"

#include <tuple>
#include <array>
#include <vector>
#include <set>
#include <cassert>

#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_integer_game.h"
#include "sumgame.h"
#include "sumgame_helpers.h"
#include "domineering.h"
#include "clobber_1xn.h"
#include "utilities.h"

using namespace std;

////////////////////////////////////////////////// Helpers
namespace {

inline static constexpr std::array<relation, 7> RELATIONS_ALL {
    REL_EQUAL,            //
    REL_FUZZY,            //
    REL_LESS_OR_EQUAL,    //
    REL_LESS,             //
    REL_GREATER_OR_EQUAL, //
    REL_GREATER,          //
    REL_UNKNOWN,          //
};

////////////////////////////////////////////////// Main test logic
// get_sum_outcome(...)
void test_get_sum_outcome()
{
    typedef tuple<game*, outcome_class> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases
    {
        {new domineering("..|.."), outcome_class::N},
        {new domineering(".|."), outcome_class::L},
        {new domineering(".."), outcome_class::R},
        {new domineering("."), outcome_class::P},
    };
    // clang-format on

    sumgame sum(BLACK);

    for (test_case_t& test_case : test_cases)
    {
        game* g = get<0>(test_case);
        const outcome_class exp_outcome = get<1>(test_case);

        assert(sum.num_total_games() == 0);
        sum.add(g);

        {
            assert_restore_sumgame ars(sum);
            const outcome_class outcome = get_sum_outcome(sum);
            assert(outcome == exp_outcome);
        }

        sum.pop(g);
        delete g;
    }
}

// sum_rel_zero(...)
void test_sum_rel_zero()
{
    typedef tuple<game*, set<relation>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases
    {
        {
            new domineering("..|.."),
            {
                REL_FUZZY,
            },
        },
        {
            new domineering(".|."),
            {
                REL_GREATER,
                REL_GREATER_OR_EQUAL,
            },
        },
        {
            new domineering(".."),
            {
                REL_LESS,
                REL_LESS_OR_EQUAL,
            },
        },
        {
            new domineering("."),
            {
                REL_EQUAL,
                REL_LESS_OR_EQUAL,
                REL_GREATER_OR_EQUAL,
            },
        },

    };
    // clang-format on

    sumgame sum(BLACK);

    for (test_case_t& test_case : test_cases)
    {
        game* g = get<0>(test_case);
        const set<relation>& exp_relations = get<1>(test_case);

        assert(sum.num_total_games() == 0);
        sum.add(g);

        for (const relation rel : RELATIONS_ALL)
        {
            if (rel == REL_UNKNOWN)
                continue;

            assert_restore_sumgame ars(sum);
            const bool rel_is_expected =
                exp_relations.find(rel) != exp_relations.end();

            const bool is_rel = sum_rel_zero(sum, rel);

            assert(logical_iff(rel_is_expected, is_rel));
        }

        sum.pop(g);
        delete g;
    }

}

// sum_rel_scale_game(...)
void test_sum_rel_scale_game()
{
    typedef tuple<game*, bound_scale, bound_t, set<relation>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases
    {
        {
            new clobber_1xn("XXO"),
            BOUND_SCALE_UP,
            1,
            {
                REL_EQUAL,
                REL_GREATER_OR_EQUAL,
                REL_LESS_OR_EQUAL,
            },
        },
        {
            new clobber_1xn("XXO"),
            BOUND_SCALE_UP_STAR,
            1,
            {
                REL_FUZZY,
            },
        },
        {
            new integer_game(3),
            BOUND_SCALE_DYADIC_RATIONAL,
            6,
            {
                REL_GREATER,
                REL_GREATER_OR_EQUAL,
            },
        },
        {
            new clobber_1xn("OOX"),
            BOUND_SCALE_DYADIC_RATIONAL,
            0,
            {
                REL_LESS_OR_EQUAL,
                REL_LESS,
            },
        },
    };
    // clang-format on

    sumgame sum(BLACK);

    for (test_case_t& test_case : test_cases)
    {
        game* g = get<0>(test_case);
        const bound_scale scale = get<1>(test_case);
        const bound_t scale_idx = get<2>(test_case);
        const set<relation>& exp_relations = get<3>(test_case);

        assert(sum.num_total_games() == 0);
        sum.add(g);

        for (const relation rel : RELATIONS_ALL)
        {
            if (rel == REL_UNKNOWN)
                continue;

            assert_restore_sumgame ars(sum);
            const bool rel_is_expected =
                exp_relations.find(rel) != exp_relations.end();

            const bool is_rel = sum_rel_scale_game(sum, rel, scale, scale_idx);

            assert(logical_iff(rel_is_expected, is_rel));
        }

        sum.pop(g);
        delete g;
    }
}

// sum_rel_game(...)
// sum_rel_sum(...)
void test_sum_rel_game_or_sum()
{
    typedef tuple<game*, game*, set<relation>> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases
    {
        {
            new clobber_1xn("XXXXO"),
            new clobber_1xn("XXO"),
            {
                REL_GREATER_OR_EQUAL,
                REL_GREATER,
            },
        },
        {
            new clobber_1xn("XOXOXO"),
            new clobber_1xn(""),
            {
                REL_EQUAL,
                REL_GREATER_OR_EQUAL,
                REL_LESS_OR_EQUAL
            },
        },
        {
            new clobber_1xn("XXO"),
            new clobber_1xn("XXOXO"),
            {
                REL_FUZZY,
            },
        },
        {
            new clobber_1xn("XXO"),
            new clobber_1xn("XXO.XXO"),
            {
                REL_LESS_OR_EQUAL,
                REL_LESS,
            },
        },
    };
    // clang-format on

    sumgame sum1(BLACK);
    sumgame sum2(BLACK);

    for (test_case_t& test_case : test_cases)
    {
        game* g1 = get<0>(test_case);
        game* g2 = get<1>(test_case);
        const set<relation>& exp_relations = get<2>(test_case);

        assert(sum1.num_total_games() == 0);
        sum1.add(g1);

        for (const relation rel : RELATIONS_ALL)
        {
            if (rel == REL_UNKNOWN)
                continue;

            const bool rel_is_expected =
                exp_relations.find(rel) != exp_relations.end();

            bool sum_rel_game_result;
            {
                assert_restore_sumgame ars1(sum1);
                assert_restore_game arg2(*g2);
                sum_rel_game_result = sum_rel_game(sum1, rel, *g2);
            }

            assert(sum2.num_total_games() == 0);
            sum2.add(g2);

            bool sum_rel_sum_result;
            {
                assert_restore_sumgame ars1(sum1);
                assert_restore_sumgame ars2(sum2);
                sum_rel_sum_result = sum_rel_sum(sum1, rel, sum2);
            }

            sum2.pop(g2);

            assert(sum_rel_game_result == sum_rel_sum_result);
            assert(logical_iff(rel_is_expected, sum_rel_game_result));
        }

        sum1.pop(g1);
        delete g1;
        delete g2;
    }


}

} // namespace

////////////////////////////////////////////////// Exported functions
void sumgame_helpers_test_all()
{
    test_get_sum_outcome();
    test_sum_rel_zero();
    test_sum_rel_scale_game();
    test_sum_rel_game_or_sum();
}
