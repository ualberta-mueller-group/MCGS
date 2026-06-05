#include "utils_for_main.h"

#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <cassert>
#include <cstdint>

#include "cli_options.h"
#include "game.h"
#include "csv_row.h"
#include "test_case.h"
#include "test_filter.h"
#include "utilities.h"
#include "file_parser.h"

using namespace std;


string get_games_string(const vector<game*>& games)
{
    if (games.empty())
        return "\t<no games specified>\n";

    stringstream stream;

    for (const game* g : games)
        stream << '\t' << *g << '\n';

    return stream.str();
}

void run_test_from_main(shared_ptr<i_test_case> test_case,
                        const cli_options& opts)
{
    const vector<game*>& games = test_case->get_games();
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

void run_tests_from_main(std::shared_ptr<file_parser> parser,
                         const cli_options& opts, test_filter_enum filter_type)
{
    bool first_case = true;

    uint64_t n_tests_filtered = 0;

    while (parser->parse_chunk())
    {
        const int n_test_cases = parser->n_test_cases();

        for (int test_number = 0; test_number < n_test_cases; test_number++)
        {
            std::shared_ptr<i_test_case> test_case =
                parser->get_test_case(test_number);

            if (!test_filter_permits_test_case(filter_type, *test_case))
            {
                n_tests_filtered++;
                continue;
            }

            if (!first_case)
                cout << endl;

            run_test_from_main(test_case, opts);

            first_case = false;
        }
    }

    if (n_tests_filtered > 0)
        cout << n_tests_filtered << " skipped by the test filter" << endl;
}
