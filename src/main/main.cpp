//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <algorithm>
#include <iostream>
#include <string>
#include <memory>
#include <variant>

#include "ThGraph.h"
#include "amazons.h"
#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_up_star.h"
#include "cli_options.h"
#include "database.h"
#include "domineering.h"
#include "file_parser.h"
#include "autotests.h"
#include "global_database.h"
#include "make_thermograph_slow.h"
#include "sumgame_helpers.h"
#include "thermograph_helpers.h"
#include "print_moves.h"
#include "search_graph_debug.h"
#include "sumgame.h"
#include "test_case.h"
#include "mcgs_init.h"
#include "hashing.h"
#include "global_options.h"
#include "clobber.h"
#include "clobber_1xn.h"

#include "gen_experiments.h"
#include "basic_player.h"
#include "utils_for_main.h"

using std::cout, std::endl, std::flush, std::string;
using namespace std;

namespace {

void test_bounds_finder_optimization()
{
    vector<game*> games =
    {
        new up_star(6, false),
    };

    sumgame sum(BLACK);
    sum.add(games);

    cout << "Sum: ";
    sum.print_simple(cout);
    cout << endl;

    const outcome_class oc = get_sum_outcome(sum);

    vector<bounds_options> opts;
    opts.emplace_back(BOUND_SCALE_UP, -512, 512, oc);

    vector<game_bounds_ptr> bounds_vec;

    for (int i = 0; i < 100'000; i++)
    {
        bounds_vec = find_bounds(sum, opts);
        assert(bounds_vec.size() == 1);
    }

    game_bounds& bounds = *bounds_vec.back();
    cout << "Bounds: " << bounds << endl;

    sum.pop(games);
    for (game* g : games)
        delete g;
}


void print_thermograph_for_sum(sumgame& sum)
{
    assert_restore_sumgame ars(sum);

    shared_ptr<ThGraph> graph = make_thermograph_slow(sum);

    cout << "==============================" << endl;

    cout << "Game: ";
    sum.print_simple(cout);
    cout << endl;

    cout << "Thermograph: ";
    print_thermograph(cout, *graph);
    cout << endl;
}

void print_option_thermographs_for_player(sumgame& sum, bw player)
{
    assert_restore_sumgame ars1(sum);
    assert(is_black_white(player));

    const bw restore_player = sum.to_play();
    sum.set_to_play(player);

    unique_ptr<sumgame_move_generator> gen(sum.create_sum_move_generator(player));

    while (*gen)
    {
        assert_restore_sumgame ars2(sum);

        const sumgame_move sm = gen->gen_sum_move();
        ++(*gen);

        assert(sum.to_play() == player);
        sum.play_sum(sm, player);

        print_thermograph_for_sum(sum);

        sum.undo_move();
    }

    sum.set_to_play(restore_player);
}

//void do_thermograph_stuff()
//{
//    sumgame sum(BLACK);
//    amazons g(".#.|#XO");
//    sum.add(&g);
//
//    print_thermograph_for_sum(sum);
//
//    print_option_thermographs_for_player(sum, BLACK);
//    print_option_thermographs_for_player(sum, WHITE);
//
//    sum.pop(&g);
//}
} // namespace

////////////////////////////////////////////////// main function
int main(int argc, char** argv)
{
    mcgs_init_1();

    cli_options opts = parse_args(argc, (const char**) argv, false);

    // i.e. ./MCGS --help
    if (opts.should_exit)
        return 0;

    //test_bounds_finder_optimization();
    //return 0;

    if (opts.db_dump_file_name.has_value())
    {
        THROW_ASSERT(global::use_db(),
                     "Error: --dump-db specified, but not using DB.");

        database& db = get_global_database();
        db.dump_to_file(opts.db_dump_file_name.value());
    }

    if (opts.use_player)
    {
        std::shared_ptr<file_parser> parser = opts.parser;

        if (parser.get() != nullptr)
            play_games(*parser, opts.play_log_name);

        return 0;
    }


    if (opts.run_tests)
    {
        run_autotests(opts.test_directory, opts.outfile_name,
                      opts.test_timeout);
        return 0;
    }

    if (opts.parser)
    {
        switch (opts.print_moves_action)
        {
            case PRINT_MOVES_ACTION_NONE:
                break;
            case PRINT_MOVES_ACTION_WINNING:
            {
                print_winning_moves_by_chunk(cout, opts.parser);
                return 0;
            }
            case PRINT_MOVES_ACTION_SUBGAME:
            {
                print_subgame_moves_by_chunk(cout, opts.parser);
                return 0;
            }
            case PRINT_MOVES_ACTION_SUM:
            {
                print_sum_moves_by_chunk(cout, opts.parser);
                return 0;
            }
        }
    }
    
    // Don't uncomment?
    //if (opts.run_tests_stdin)
    //{
    //    run_autotests_stdin(opts.outfile_name, opts.test_timeout);
    //    return 0;
    //}

    if (opts.gen_experiments)
    {
        gen_experiments();
        return 0;
    }

    // Run sums from input
    if (opts.parser)
    {
        bool first_case = true;
        std::shared_ptr<file_parser> parser = opts.parser;

        while (parser->parse_chunk())
        {
            const int n_test_cases = parser->n_test_cases();

            for (int test_number = 0; test_number < n_test_cases; test_number++)
            {

                std::shared_ptr<i_test_case> test_case =
                    parser->get_test_case(test_number);

                if (!first_case)
                    cout << endl;

                run_test_from_main(test_case, opts);

                first_case = false;
            }
        }
    }

    if (opts.search_graph_verify_dir.has_value())
        sgraph::annotate_graphs(*opts.search_graph_verify_dir);

    if (random_table::did_resize_warning())
        random_table::print_resize_warning();


    return 0;
}
