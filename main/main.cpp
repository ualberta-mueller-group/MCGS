//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include "cgt_basics.h"
#include "alternating_move_game.h"
#include "cli_options.h"
#include "file_parser.h"
#include "sumgame.h"
#include "cli_options.h"
#include "autotests.h"
#include <chrono>

#include "all_game_headers.h"

using std::cout, std::endl, std::string;

int main(int argc, char** argv)
{
    cli_options opts = parse_cli_args(argc, (const char**) argv, true);

    if (opts.should_exit)
    {
        return 0;
    }

    if (opts.run_tests)
    {
        run_autotests(opts.test_directory, opts.outfile_name, opts.test_timeout);
        return 0;
    }


    // Run sums from input
    if (opts.parser)
    {
        game_case gc;
        bool first_case = true;

        while (opts.parser->parse_chunk(gc))
        {
            if (first_case)
            {
                first_case = false;
            } else
            {
                cout << endl;
            }


            sumgame sum(gc.to_play);

            for (game* g : gc.games)
            {
                cout << "\t" << *g << endl;
                sum.add(g);
            }

            cout << "Player: " << color_char(gc.to_play) << endl;
            cout << "Expected: " << test_outcome_to_string(gc.expected_outcome) << endl;

            if (opts.dry_run)
            {
                cout << "Not running games..." << endl;
            } else
            {
                std::chrono::time_point start = std::chrono::high_resolution_clock::now();
                bool result = sum.solve();
                std::chrono::time_point end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = end - start;

                cout << "Got: " << test_outcome_to_string((test_outcome) result) << endl;
                cout << "Time (ms): " << duration.count() << endl;
                
                cout << "Test outcome: ";
                if (gc.expected_outcome == TEST_OUTCOME_UNSPECIFIED)
                {
                    cout << "COMPLETED";
                } else
                {
                    cout << ((gc.expected_outcome == result) ? "PASS" : "FAIL");
                }
                cout << endl;

                if (gc.comments.size() > 0)
                {
                    cout << "\"" << gc.comments << "\"" << endl;
                }

            }
            gc.cleanup_games();
        }
    }
}
