//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <iostream>
#include <string>
#include <memory>

#include "cli_options.h"
#include "convert_to_ctl.h"
#include "file_parser.h"
#include "autotests.h"
#include "print_moves.h"
#include "test_case.h"
#include "mcgs_init.h"
#include "hashing.h"
#include "global_options.h"
#include "clobber.h"

#include "gen_experiments.h"
#include "basic_player.h"
#include "test_filter.h"
#include "utils_for_main.h"

using std::cout, std::endl, std::flush, std::string;


////////////////////////////////////////////////// main function
int main(int argc, char** argv)
{
    THROW_ASSERT(argc >= 1);
    mcgs_init_1(argv[0]);

    cli_options opts = parse_args(argc, (const char**) argv, false);

    // i.e. ./MCGS --help
    if (opts.should_exit)
        return 0;

    mcgs_init_2(opts);

    if (opts.use_player)
    {
        std::shared_ptr<file_parser> parser = opts.parser;

        // TODO should this use the test filter too?
        if (parser.get() != nullptr)
            play_games(*parser, opts.play_log_name);

        return 0;
    }

    if (opts.run_tests)
    {
        if (opts.lib_ctl_output_dir.has_value())
        {
            convert_tests_to_ctl_format(opts.test_directory,
                                        *opts.lib_ctl_output_dir,
                                        opts.test_filter_type);
        }
        else
        {
            run_autotests(opts.test_directory, opts.outfile_name,
                          opts.test_timeout, opts.test_filter_type);
        }

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
                print_subgame_moves_by_chunk(cout, opts.parser,
                                             opts.format_moves_as_options);
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
    // if (opts.run_tests_stdin)
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
        if (opts.lib_ctl_output_dir.has_value())
            convert_tests_to_ctl_format(opts.parser, *opts.lib_ctl_output_dir,
                                        opts.test_filter_type);
        else
            run_tests_from_main(opts.parser, opts, opts.test_filter_type);
    }

    if (random_table::did_resize_warning())
        random_table::print_resize_warning();


    return 0;
}
