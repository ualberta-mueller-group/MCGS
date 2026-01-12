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
#include "file_parser.h"
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
        unique_ptr<file_parser> parser =
            unique_ptr<file_parser>(file_parser::from_file(file_name));


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
                write_csv_field_strings(outfile, row_fields); // flushes stream

                file_test_idx++;
            }
        }
    }

    if (random_table::did_resize_warning())
        random_table::print_resize_warning();

    outfile.close();
}

