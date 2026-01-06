#include "autotests.h"

#include <string>
#include <sstream>
#include <cstdio>
#include <ios>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cassert>
#include <memory>
#include <algorithm>

#include "cgt_basics.h"
#include "csv_row.h"
#include "solver_stats.h"
#include "file_parser2.h"
#include "global_options.h"
#include "search_utils.h"
#include "test_case.h"
#include "throw_assert.h"
#include "utilities.h"
#include "sumgame.h"
#include "hashing.h"
#include "game.h"
#include "file_iterator.h"

//////////////////////////////////////////////////
using namespace std;

// CSV separator
inline constexpr const char NEWLINE = '\n';


//////////////////////////////////////// exported functions
void run_autotests(const string& root_test_directory, const string& outfile_name,
                   unsigned long long test_timeout)
{
    THROW_ASSERT(root_test_directory.size() > 0);

    // Prepare output CSV file
    ofstream outfile(outfile_name); // .csv file

    if (!outfile.is_open())
    {
        throw ios_base::failure("Couldn't open file for writing: \"" +
                                outfile_name + "\"");
    }
    

    // Write CSV header
    vector<string> header_fields = csv_row::get_header_field_strings();
    write_csv_field_strings(outfile, header_fields);

    // iterate over all files in root test directory
    for (file_iterator_alphabetical iter(root_test_directory); iter; ++iter)
    {
        const filesystem::directory_entry& entry = iter.gen_entry();
        assert(!entry.is_directory());

        // Skip directories and non ".test" files
        if (!entry.is_regular_file())
            continue;

        const filesystem::path& file_path = entry.path();

        if (file_path.extension() != ".test")
            continue;

        const string file_name = file_path.string();

        cout << "New file: " << file_name << endl;

        // Path relative to test directory (this string is printed to CSV file)
        filesystem::path relative_file_path =
            filesystem::relative(file_path, filesystem::path(root_test_directory));

        // Open test file
        unique_ptr<file_parser2> parser =
            unique_ptr<file_parser2>(file_parser2::from_file(file_name));


        int file_test_idx = 0;

        while (parser->parse_chunk())
        {
            const int n_chunk_tests = parser->n_test_cases();
            for (int chunk_test_idx = 0; chunk_test_idx < n_chunk_tests; chunk_test_idx++)
            {
                cout << file_name << " " << file_test_idx << endl;
                shared_ptr<i_test_case> test_case = parser->get_test_case(chunk_test_idx);

                // Populate csv row fields
                csv_row& row = test_case->get_csv_row();
                row.fill_autotest_fields(relative_file_path.string(), file_test_idx);

                // Run the test
                test_case->run(test_timeout);

                // Write CSV row
                vector<string> row_fields = row.get_row_field_strings();
                assert(row_fields.size() == header_fields.size());
                write_csv_field_strings(outfile, row_fields);

                file_test_idx++;
            }
        }
    }

    if (random_table::did_resize_warning())
        random_table::print_resize_warning();

    outfile.close();
}

//void run_autotests_stdin(const string& outfile_name,
//                         unsigned long long test_timeout)
//{
//    assert(global::clear_tt());
//
//    bool first_case = true;
//
//    ofstream outfile(outfile_name); // CSV file
//
//    if (!outfile.is_open())
//    {
//        throw ios_base::failure("Couldn't open file for writing: \"" +
//                                outfile_name + "\"");
//    }
//
//    // print format as first row to file
//    append_field(outfile, "File", true);
//    append_field(outfile, "Case", true);
//    append_field(outfile, "Games", true);
//    append_field(outfile, "Player", true);
//    append_field(outfile, "Expected Result", true);
//    append_field(outfile, "Result", true);
//    append_field(outfile, "Time (ms)", true);
//    append_field(outfile, "Status", true);
//    append_field(outfile, "Comments", true);
//
//    append_field(outfile, "Node Count", true);
//    append_field(outfile, "TT Hits", true);
//    append_field(outfile, "TT Misses", true);
//    append_field(outfile, "DB Hits", true);
//    append_field(outfile, "DB Misses", true);
//    append_field(outfile, "Max Depth", true);
//    append_field(outfile, "# Subgames", true);
//
//    append_field(outfile, "Input hash", false);
//    outfile << NEWLINE;
//
//    print_ready_signal(); // READY
//
//    unique_ptr<file_parser> parser(file_parser::from_stdin());
//
//    game_case gc;
//    uint64_t case_number = 0;
//
//    while (parser->parse_chunk(gc))
//    {
//        if (case_number % 20 == 0)
//            outfile.flush();
//
//        if (global::clear_tt() && !first_case)
//            sumgame::reset_ttable();
//
//        stats::reset_stats();
//        search_result sr = gc.run(test_timeout);
//        first_case = false;
//
//        const solver_stats& st = stats::get_global_stats();
//
//        append_field(outfile, "stdin", true);
//        append_field(outfile, to_string(case_number), true);
//        append_field(outfile, human_readable_game_string(gc.games), true);
//        append_field(outfile, sr.player_str(), true);
//        append_field(outfile, gc.expected_value.str(), true);
//        append_field(outfile, sr.value_str(), true);
//        append_field(outfile, sr.duration_str(), true);
//        append_field(outfile, sr.status_str(), true);
//        append_field(outfile, gc.comments, true);
//
//        append_field(outfile, to_string(st.node_count), true);   //
//        append_field(outfile, to_string(st.tt_hits), true);      //
//        append_field(outfile, to_string(st.tt_misses), true);    //
//        append_field(outfile, to_string(st.db_hits), true);      //
//        append_field(outfile, to_string(st.db_misses), true);    //
//        append_field(outfile, to_string(st.search_depth), true); //
//        append_field(outfile, to_string(st.n_subgames), true);   //
//
//        append_field(outfile, gc.hash.get_string(), false);
//        outfile << NEWLINE;
//
//        gc.cleanup_games();
//        case_number++;
//
//        print_ready_signal(); // READY
//    }
//
//    if (random_table::did_resize_warning())
//        cerr << "TABLE RESIZE" << endl;
//
//    outfile.close();
//}
