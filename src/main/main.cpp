//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <iostream>
#include <sstream>
#include <string>

//#include "basic_player.h"
#include "cli_options.h"
#include "file_parser2.h"
#include "autotests.h"
#include "file_parser_new.h"
#include "mcgs_init.h"
#include "hashing.h"
#include "global_options.h"
#include "search_utils.h"
#include "clobber.h"

#include "sumgame.h"
#include "istream_tokenizer.h"
#include "throw_assert.h"
#include "gen_experiments.h"
//#include "winning_moves.h"
#include "test_file_parser2.h"
#include "basic_player.h"

using std::cout, std::endl, std::flush, std::string;

////////////////////////////////////////////////// forward declarations
namespace {
string get_games_string(const std::vector<game*>& games);

void run_test_from_main(std::shared_ptr<i_test_case> test_case,
                        const cli_options& opts);

} // namespace

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
        std::shared_ptr<file_parser2> parser = opts.parser;

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

    //if (opts.print_winning_moves)
    //{
    //    print_winning_moves_new(opts.parser);
    //    return 0;
    //}

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
        std::shared_ptr<file_parser2> parser = opts.parser;

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

////////////////////////////////////////////////// utility functions
namespace {
string get_games_string(const std::vector<game*>& games)
{
    if (games.empty())
        return "\t<no games specified>\n";

    std::stringstream stream;

    for (const game* g : games)
        stream << '\t' << *g << '\n';

    return stream.str();
}

void run_test_from_main(std::shared_ptr<i_test_case> test_case,
                        const cli_options& opts)
{
    const std::vector<game*>& games = test_case->get_games();
    const csv_row& row = test_case->get_csv_row();

    cout << "Test type: " << row.get_command_type_string() << endl;
    cout << get_games_string(games) << flush;
    cout << "Player: " << print_optional(row.player, "<N/A>") << endl;

    if (row.expected_result.has_value())
        cout << "Expected: " << row.expected_result.value() << endl;

    if (opts.dry_run)
        cout << "Not running search..." << endl;
    else
    {
        test_case->run(0);

        if (row.result.has_value())
            cout << "Got: " << row.result.value() << endl;

        cout << "Time (ms): " << row.get_time_ms_string() << endl;
        cout << "Status: " << row.get_status_string() << endl;
    }

    assert(row.comments.has_value());
    if (!row.comments->empty())
        cout << "\"" << row.comments.value() << "\"" << endl;
}

} // namespace
