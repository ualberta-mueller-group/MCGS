#include "autotests.h"

#include <cstdio>
#include <ios>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "file_parser.h"
#include "search_utils.h"
#include "throw_assert.h"
#include <vector>
#include <sstream>
#include "hashing.h"
#include <string>
#include "game.h"
#include <cassert>
#include <memory>

using namespace std;

using filesystem::recursive_directory_iterator;

// CSV separator
inline constexpr const char* SEP = ",";
inline constexpr const char NEWLINE = '\n';

//////////////////////////////////////// helper functions
namespace {
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
    // Remove leading/trailing whitespace, replace double quotes with 2 single
    // quotes
    string sanitized_field;

    bool past_left_whitespace = false;

    const size_t N = field.size();
    for (size_t i = 0; i < N; i++)
    {
        const char& c = field[i];

        if (!past_left_whitespace && c == ' ')
        {
            continue;
        }
        else
        {
            past_left_whitespace = true;
        }

        if (c == '\"')
        {
            sanitized_field += "''";
        }
        else
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
        os << SEP;
    }
}
} // namespace

//////////////////////////////////////// exported functions
void run_autotests(const string& test_directory, const string& outfile_name,
                   unsigned long long test_timeout)
{
    THROW_ASSERT(test_directory.size() > 0);

    ofstream outfile(outfile_name); // CSV file

    if (!outfile.is_open())
    {
        throw ios_base::failure("Couldn't open file for writing: \"" +
                                outfile_name + "\"");
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
    outfile << NEWLINE;

    // iterate over autotests directory
    for (const filesystem::directory_entry& entry :
         recursive_directory_iterator(test_directory))
    {
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
            filesystem::relative(file_path, filesystem::path(test_directory));

        // Open test file
        unique_ptr<file_parser> parser =
            unique_ptr<file_parser>(file_parser::from_file(file_name));

        game_case gc;
        int case_number = 0;

        while (parser->parse_chunk(gc))
        {
            cout << file_name << " " << case_number << endl;

            search_result sr = gc.run(test_timeout);

            append_field(outfile, relative_file_path.string(), true);
            append_field(outfile, to_string(case_number), true);
            append_field(outfile, human_readable_game_string(gc.games), true);
            append_field(outfile, sr.player_str(), true);
            append_field(outfile, gc.expected_value.str(), true);
            append_field(outfile, sr.value_str(), true);
            append_field(outfile, sr.duration_str(), true);
            append_field(outfile, sr.status_str(), true);
            append_field(outfile, gc.comments, true);
            append_field(outfile, gc.hash.get_string(), false);
            outfile << NEWLINE;

            gc.cleanup_games();
            case_number++;
        }
    }

    if (random_table::did_resize_warning())
        random_table::print_resize_warning();

    outfile.close();
}
