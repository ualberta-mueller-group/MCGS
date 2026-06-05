#include "thermograph_helpers_test.h"

#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "cgt_up_star.h"
#include "grid_generator.h"
#include "grid_hash.h"
#include "sumgame.h"
#include "sumgame_helpers.h"
#include "thermograph_builder_no_db.h"
#include "thermograph_helpers.h"
#include "all_game_headers.h"

using namespace std;

namespace {
////////////////////////////////////////////////// Helpers
/*
    Ensures `read_rational_bound_from_thermograph(...)` is maximally precise.
*/
void test_bound_helper_imprecision_impl(
    sumgame& sum, int p, int q, bool star,
    thermograph_builder_no_db& therm_builder, vector<game*>& game_vec)
{
    assert(game_vec.empty());
    assert(sum.num_total_games() == 0);

    game_vec.push_back(new dyadic_rational(p, q));

    if (star)
        game_vec.push_back(new up_star(0, true));

    sum.add(game_vec);

    shared_ptr<ThGraph> graph;
    {
        assert_restore_sumgame ars(sum);
        graph = therm_builder.build_thermograph(sum);
        assert(graph);
        graph->Check();
    }

    const bound_t upper = read_rational_bound_from_thermograph(*graph, true);
    const bound_t lower = read_rational_bound_from_thermograph(*graph, false);

    // Bounds correct?
    assert(sum_rel_scale_game(sum, REL_LESS_OR_EQUAL,
                              BOUND_SCALE_DYADIC_RATIONAL, upper));
    assert(sum_rel_scale_game(sum, REL_GREATER_OR_EQUAL,
                              BOUND_SCALE_DYADIC_RATIONAL, lower));

    // Bounds tight?
    assert(!sum_rel_scale_game(sum, REL_LESS_OR_EQUAL,
                               BOUND_SCALE_DYADIC_RATIONAL, upper - 1));
    assert(!sum_rel_scale_game(sum, REL_GREATER_OR_EQUAL,
                               BOUND_SCALE_DYADIC_RATIONAL, lower + 1));

    sum.pop(game_vec);
    for (game* g : game_vec)
        delete g;
    game_vec.clear();
}

////////////////////////////////////////////////// Main test functions

// get_thermograph_hash
void test_hash_helper()
{
    sumgame sum(BLACK);
    thermograph_builder_no_db thgraph_builder;

    grid_generator gen({3, 3}, {BLACK, WHITE}, true, EMPTY, false,
                       grid_hash_mask<clobber>());

    unordered_map<hash_t, shared_ptr<ThGraph>> hash_to_graph;

    uint64_t n_games = 0;
    while (gen)
    {
        assert(sum.num_total_games() == 0);

        clobber g(gen.gen_board(), gen.get_shape());
        sum.add(&g);

        n_games++;
        ++gen;

        shared_ptr<ThGraph> graph;
        {
            assert_restore_sumgame ars(sum);
            graph = thgraph_builder.build_thermograph(sum);
        }

        const hash_t hash = get_thermograph_hash(*graph);

        const auto existing_element = hash_to_graph.find(hash);
        if (existing_element != hash_to_graph.end())
            assert(*existing_element->second == *graph);
        else
        {
            const auto inserted_element = hash_to_graph.emplace(hash, graph);
            assert(inserted_element.second);
        }

        sum.pop(&g);
    }

    assert(n_games >= 1000);
}

// thermograph_bends_out_below_zero
// game_is_small_from_thermograph
void test_basic_helpers()
{
    /*
       - game
       - left bends, right bends
       - is small
    */
    typedef tuple<game*, pair<bool, bool>, bool> test_case_t;

    // clang-tidy off
    vector<test_case_t> test_cases
    {
        {
            new nogo_1xn("...."),         // 0
            {false, false},
            false,
        },
        {
            new nogo_1xn("......"),       // +-1
            {true, true},
            false,
        },
        {
            new nogo_1xn("..X...O...X."), // hot N position
            {true, true},
            false,
        },
        {
            new dyadic_rational(3, 8),    // 3/8
            {false, false},
            false,
        },
        {
            new nogo_1xn("........."),    // hot N position
            {false, false},
            false,
        },
        {
            new nogo_1xn("....X.O..."),   // hot N position
            {true, false},
            false,
        },
        {
            new clobber_1xn("XOOX"),   // 0
            {false, false},
            false,
        },
        {
            new clobber_1xn("XXO"),   // ^
            {true, false},
            true,
        },
        {
            new clobber_1xn("OOX"),   // v
            {false, true},
            true,
        },
        {
            new clobber_1xn("XO"),   // *
            {true, true},
            true,
        },
    };
    // clang-tidy on

    sumgame sum(BLACK);
    thermograph_builder_no_db thgraph_builder;

    for (test_case_t& test_case : test_cases)
    {
        game* g = get<0>(test_case);
        const pair<bool, bool>& exp_bends_outwards = get<1>(test_case);
        const bool& exp_is_small = get<2>(test_case);

        assert(sum.num_total_games() == 0);
        sum.add(g);

        shared_ptr<ThGraph> graph;
        {
            assert_restore_sumgame ars(sum);
            graph = thgraph_builder.build_thermograph(sum);
        }

        assert(thermograph_bends_out_below_zero(*graph, true) ==
               exp_bends_outwards.first);

        assert(thermograph_bends_out_below_zero(*graph, false) ==
               exp_bends_outwards.second);

        assert(game_is_small_from_thermograph(*graph) == exp_is_small);

        sum.pop(g);
        delete g;
    }
}

// read_rational_bound_from_thermograph
// make_rational_bounds_from_thermograph
void test_bound_helpers()
{
    // clang-tidy off
    vector<game*> test_cases
    {
        new nogo_1xn("...."),         // 0
        new nogo_1xn("......"),       // +-1
        new nogo_1xn("..X...O...X."), // hot N position
        new dyadic_rational(3, 8),    // 3/8
        new nogo_1xn("........."),    // hot N position
        new nogo_1xn("....X.O..."),   // hot N position
    };
    // clang-tidy on

    sumgame sum(BLACK);
    thermograph_builder_no_db thgraph_builder;

    for (game* g : test_cases)
    {
        assert(sum.num_total_games() == 0);
        sum.add(g);

        vector<bounds_options> opts;
        opts.push_back({BOUND_SCALE_DYADIC_RATIONAL, -64, 64});
        vector<game_bounds_ptr> exp_bounds_vec;
        {
            assert_restore_sumgame ars(sum);
            exp_bounds_vec = find_bounds(sum, opts);
            assert(exp_bounds_vec.size() == 1);
        }

        const game_bounds& exp_bounds = *exp_bounds_vec.back();
        assert(exp_bounds.both_valid());

        shared_ptr<ThGraph> graph;
        {
            assert_restore_sumgame ars(sum);
            graph = thgraph_builder.build_thermograph(sum);
        }

        const bound_t upper_from_graph =
            read_rational_bound_from_thermograph(*graph, true);
        const bound_t lower_from_graph =
            read_rational_bound_from_thermograph(*graph, false);

        assert(sum_rel_scale_game(sum, REL_LESS_OR_EQUAL,
                                  BOUND_SCALE_DYADIC_RATIONAL,
                                  upper_from_graph));

        assert(sum_rel_scale_game(sum, REL_GREATER_OR_EQUAL,
                                  BOUND_SCALE_DYADIC_RATIONAL,
                                  lower_from_graph));

        unique_ptr<game_bounds> bounds_from_thermograph(
            make_rational_bounds_from_thermograph(*graph));

        assert(*bounds_from_thermograph == exp_bounds);

        sum.pop(g);
        delete g;
    }
}



void test_bound_helper_imprecision(bool extra_tests)
{
    sumgame sum(BLACK);
    vector<game*> games;
    thermograph_builder_no_db therm_builder;

    const int TOP_RADIUS = extra_tests ? 4096 : 256;
    const int BOTTOM = extra_tests ? 128 : 64;

    for (int top = -TOP_RADIUS; top <= TOP_RADIUS; top++)
    {
        test_bound_helper_imprecision_impl(sum, top, BOTTOM, false,
                                           therm_builder, games);

        test_bound_helper_imprecision_impl(sum, top, BOTTOM, true,
                                           therm_builder, games);
    }
}

} // namespace

////////////////////////////////////////////////// Exported functions
void thermograph_helpers_test_all(bool extra_tests)
{
    test_hash_helper();
    test_basic_helpers();
    test_bound_helpers();
    test_bound_helper_imprecision(extra_tests);
}
