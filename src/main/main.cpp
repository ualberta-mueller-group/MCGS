//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include "cgt_basics.h"
#include "cli_options.h"
#include "file_parser.h"
#include "sumgame.h"
#include "throw_assert.h"
#include "cli_options.h"
#include "autotests.h"
#include <chrono>

using std::cout, std::endl, std::string;

//#include "hashing.h"
//#include "hashing2.h"
//#include "hashing3.h"
#include "hashing.h"
#include "hash_eval.h"
int main(int argc, char** argv)
{


    hash_eval_all();
    //test_hashing_final();
    return 0;
    /*

    int test_no = 0;
    int test_no_count = 0;

    for (int i = 0; i < argc; i++)
    {
        char* arg = argv[i];

        if (strcmp(arg, "-1") == 0)
        {
            test_no = 1;
            test_no_count++;
        }

        if (strcmp(arg, "-2") == 0)
        {
            test_no = 2;
            test_no_count++;
        }

        if (strcmp(arg, "-3") == 0)
        {
            test_no = 3;
            test_no_count++;
        }

        if (strcmp(arg, "-4") == 0)
        {
            test_no = 4;
            test_no_count++;
        }
    }

    THROW_ASSERT(test_no >= 1 || test_no <= 4);
    THROW_ASSERT(test_no_count == 1);

    switch (test_no)
    {
        case 1:
            test_hashing1();
            return 0;

        case 2:
            test_hashing2();
            return 0;

        case 3:
            test_hashing3();
            return 0;

        case 4:
            test_hashing_final();
            return 0;

        default:
            THROW_ASSERT(false);
    }

    return 0;
    */

    cli_options opts = parse_cli_args(argc, (const char**) argv, false);

    // i.e. ./MCGS --help
    if (opts.should_exit)
    {
        return 0;
    }

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

            sumgame sum(gc.to_play);

            for (game* g : gc.games)
            {
                cout << "\t" << *g << endl;
                sum.add(g);
            }
            if (gc.games.size() == 0)
            {
                cout << "\t" << "<no games specified>" << endl;
            }

            cout << "Player: " << color_char(gc.to_play) << endl;
            cout << "Expected: " << test_result_to_string(gc.expected_outcome)
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
                if (gc.expected_outcome == TEST_RESULT_UNSPECIFIED)
                {
                    cout << "COMPLETED";
                }
                else
                {
                    cout << ((gc.expected_outcome == result) ? "PASS" : "FAIL");
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
}
