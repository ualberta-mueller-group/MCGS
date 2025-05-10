//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <iostream>
#include <string>
#include "cgt_basics.h"
#include "cli_options.h"
#include "file_parser.h"
#include "sumgame.h"
#include "autotests.h"
#include <chrono>
#include <ratio>
#include "mcgs_init.h"
#include "hashing.h"
#include "global_options.h"


using std::cout, std::endl, std::string;

int main(int argc, char** argv)
{
    cli_options opts = parse_args(argc, (const char**) argv, false);

    // i.e. ./MCGS --help
    if (opts.should_exit)
    {
        return 0;
    }

    mcgs_init_all(opts);

    if (opts.run_tests)
    {
        run_autotests(opts.test_directory, opts.outfile_name,
                      opts.test_timeout);
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
            }
            else
            {
                cout << endl;
            }

            sumgame sum(gc.run_command.player);

            for (game* g : gc.games)
            {
                cout << "\t" << *g << endl;
                sum.add(g);
            }
            if (gc.games.size() == 0)
            {
                cout << "\t" << "<no games specified>" << endl;
            }

            cout << "Player: " << color_char(gc.run_command.player) << endl;
            cout << "Expected: " << test_result_to_string(gc.run_command.expected_outcome)
                 << endl;

            if (opts.dry_run)
            {
                cout << "Not running search..." << endl;
            }
            else
            {
                std::chrono::time_point start =
                    std::chrono::high_resolution_clock::now();
                bool result = sum.solve(); // TODO allow timeout here?
                std::chrono::time_point end =
                    std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration =
                    end - start;

                cout << "Got: " << test_result_to_string((test_result) result)
                     << endl;
                cout << "Time (ms): " << duration.count() << endl;

                cout << "Status: ";
                if (gc.run_command.expected_outcome == TEST_RESULT_UNSPECIFIED)
                {
                    cout << "COMPLETED";
                }
                else
                {
                    cout << ((gc.run_command.expected_outcome == result) ? "PASS" : "FAIL");
                }
                cout << endl;
            }
            if (gc.comments.size() > 0)
            {
                cout << "\"" << gc.comments << "\"" << endl;
            }

            gc.cleanup_games();
        }
    }


    if (random_table::did_resize_warning())
        random_table::print_resize_warning();

    return 0;
}
