//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <iostream>
#include <string>

#include "basic_player.h"
#include "cli_options.h"
#include "file_parser.h"
#include "autotests.h"
#include "mcgs_init.h"
#include "hashing.h"
#include "global_options.h"
#include "search_utils.h"
#include "clobber.h"

#include "sumgame.h"
#include "throw_assert.h"
#include "gen_experiments.h"

using std::cout, std::endl, std::string;

int main(int argc, char** argv)
{
    mcgs_init_1();

    cli_options opts = parse_args(argc, (const char**) argv, false);

    // i.e. ./MCGS --help
    if (opts.should_exit)
        return 0;

    mcgs_init_2(opts);

    if (opts.use_player)
    {
        file_parser* parser = opts.parser.get();
        THROW_ASSERT(parser != nullptr, "No games specified for player");

        play_games(*parser, opts.play_log_name);
        return 0;
    }


    if (opts.run_tests)
    {
        run_autotests(opts.test_directory, opts.outfile_name,
                      opts.test_timeout);
        return 0;
    }

    /* TODO enable in version 1.5
    if (opts.print_winning_moves)
    {
        //print_winning_moves_impl(opts.parser);
        print_winning_moves_new(opts.parser);
        return 0;
    }
    */

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
        game_case gc;
        bool first_case = true;

        while (opts.parser->parse_chunk(gc))
        {
            if (!first_case)
                cout << endl;

            for (game* g : gc.games)
                cout << "\t" << *g << endl;

            if (gc.games.size() == 0)
                cout << "\t" << "<no games specified>" << endl;

            cout << "Player: " << player_name_bw_imp(gc.to_play) << endl;
            cout << "Expected: " << gc.expected_value.str() << endl;

            if (opts.dry_run)
                cout << "Not running search..." << endl;
            else
            {
                if (global::clear_tt() && !first_case)
                    sumgame::reset_ttable();

                search_result sr = gc.run(0);
                cout << "Got: " << sr.value_str() << endl;
                cout << "Time (ms): " << sr.duration_str() << endl;
                cout << "Status: " << sr.status_str() << endl;
            }

            if (gc.comments.size() > 0)
                cout << "\"" << gc.comments << "\"" << endl;

            gc.cleanup_games();
            first_case = false;
        }
    }

    if (random_table::did_resize_warning())
        random_table::print_resize_warning();


    return 0;
}
