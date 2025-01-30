#include "autotests.h"

#include <cstdio>
#include <iomanip>
#include <ios>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "cgt_basics.h"
#include "file_parser.h"
#include "sumgame.h"
#include <ratio>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <chrono>

using namespace std;

using filesystem::recursive_directory_iterator;

// CSV separator
inline constexpr const char* sep = ","; //NOLINT
inline constexpr const char newline = '\n'; //NOLINT

// convert game list to string
string human_readable_game_string(const vector<game*>& games)
{
    stringstream stream;

    const size_t N = games.size();
    for (size_t i = 0; i < N; i++)
    {
        const game* g = games[i];
        g->print(stream);

        if (i + 1 < N)
        {
            stream << " ";
        }
    }

    string game_string = stream.str();

    return game_string;
}

// Print column contents to CSV file
void append_field(ostream& os, const string& field, bool include_separator)
{
    // Remove leading/trailing whitespace, replace double quotes with 2 single quotes
    string sanitized_field;

    bool past_left_whitespace = false;

    const size_t N = field.size();
    for (size_t i = 0; i < N; i++)
    {
        const char& c = field[i];

        if (!past_left_whitespace && c == ' ')
        {
            continue;
        } else
        {
            past_left_whitespace = true;
        }

        if (c == '\"')
        {
            sanitized_field += "''";
        } else
        {
            sanitized_field.push_back(c);
        }
    }

    while (sanitized_field.size() > 0 && sanitized_field.back() == ' ')
    {
        sanitized_field.pop_back();
    }
    
    os << "\"" << sanitized_field << "\"";
    if (include_separator)
    {
        os << sep;
    }
}

// Print elapsed time to 2 decimal places
string format_duration(double duration)
{
    const char* format = "%.2f";

    int size = snprintf(nullptr, 0, format, duration) + 1;
    char buffer[size];

    int got_size = snprintf(buffer, size, format, duration);
    assert(size == got_size + 1);

    return string(buffer);
}

void run_autotests(const string& test_directory, const string& outfile_name, unsigned long long test_timeout)
{
    ofstream outfile(outfile_name); // CSV file

    if (!outfile.is_open())
    {
        throw ios_base::failure("Couldn't open file for writing: \"" + outfile_name +  "\"");
    }

    // print format as first row to file
    append_field(outfile, "File", true);
    append_field(outfile, "Case", true);
    append_field(outfile, "Games", true);
    append_field(outfile, "Player", true);
    append_field(outfile, "Expected Result", true);
    append_field(outfile, "Result", true);
    append_field(outfile, "Time (ms)", true);
    append_field(outfile, "Status", true);
    append_field(outfile, "Comments", true);
    append_field(outfile, "Input hash", false);
    outfile << newline;


    // iterate over autotests directory
    for (const filesystem::directory_entry& entry : recursive_directory_iterator(test_directory))
    {
        // Skip directories and non ".test" files
        if (!entry.is_regular_file())
        {
            continue;
        }

        const filesystem::path& file_path = entry.path();

        if (file_path.extension() != ".test")
        {
            continue;
        }

        const string file_name = file_path.string();

        cout << "New file: " << file_name << endl;

        // Path relative to test directory (this string is printed to CSV file)
        filesystem::path relative_file_path = filesystem::relative(file_path, filesystem::path(test_directory));

        // Open test file
        unique_ptr<file_parser> parser = unique_ptr<file_parser>(file_parser::from_file(file_name));
        game_case gc;

        int case_number = 0;
        while (parser->parse_chunk(gc))
        {
            cout << file_name << " " << case_number << endl;

            sumgame sum(gc.to_play);
            for (game* g : gc.games)
            {
                sum.add(g);
            }

            chrono::time_point start = chrono::high_resolution_clock::now();
            optional<solve_result> result = sum.solve_with_timeout(test_timeout);
            chrono::time_point end = chrono::high_resolution_clock::now();

            chrono::duration<double, std::milli> duration = end - start;

            string result_string = "???";

            if (result.has_value())
            {
                result_string = result.value().win ? test_result_to_string(TEST_RESULT_WIN)
                    : test_result_to_string(TEST_RESULT_LOSS);
            }

            string status_string = "TIMEOUT";
            if (result.has_value())
            {
                if (gc.expected_outcome == TEST_RESULT_UNSPECIFIED)
                {
                    status_string = "COMPLETED";
                } else
                {
                    status_string = (result.value().win == gc.expected_outcome) ? "PASS" : "FAIL";
                }
            }

            append_field(outfile, relative_file_path.string(), true);
            append_field(outfile, to_string(case_number), true);
            append_field(outfile, human_readable_game_string(gc.games), true);
            append_field(outfile, string(1, color_char(gc.to_play)), true);
            append_field(outfile, test_result_to_string(gc.expected_outcome), true);
            append_field(outfile, result_string, true);
            append_field(outfile, format_duration(duration.count()), true);
            append_field(outfile, status_string, true);
            append_field(outfile, gc.comments, true);
            append_field(outfile, gc.hash.get_string(), false);
            outfile << newline;

            gc.cleanup_games();
            case_number++;
        }

    }

    outfile.close();
}
