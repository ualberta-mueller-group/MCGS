//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <iostream>
#include <string>
#include <memory>

//#include "basic_player.h"
#include "cli_options.h"
#include "file_parser.h"
#include "autotests.h"
#include "print_moves.h"
#include "test_case.h"
#include "mcgs_init.h"
#include "hashing.h"
#include "global_options.h"
#include "clobber.h"

#include "gen_experiments.h"
//#include "winning_moves.h"
#include "basic_player.h"
#include "utils_for_main.h"

using std::cout, std::endl, std::flush, std::string;

////////////////////////////////////////////////// main function
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
                print_winning_moves_by_chunk(opts.parser);
                return 0;
            }
            case PRINT_MOVES_ACTION_SUBGAME:
            {
                print_subgame_moves_by_chunk(opts.parser);
                return 0;
            }
            case PRINT_MOVES_ACTION_SUM:
            {
                print_sum_moves_by_chunk(opts.parser);
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

    if (random_table::did_resize_warning())
        random_table::print_resize_warning();


    return 0;
}
