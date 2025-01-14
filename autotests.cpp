#include "autotests.h"

#include <iomanip>
#include <ios>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "cgt_basics.h"
#include "file_parser.h"
#include "misc_constants.h"
#include "sumgame.h"
#include <ratio>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <chrono>

using namespace std;


using filesystem::recursive_directory_iterator;

/*
   file name
   case number
   human readable games
   to play

   expected value
   found value

   time (ms)

   outcome (pass, fail, timeout)

   included comments
*/


// separator
constexpr const char* sep = ",";

string human_readable_game_string(const vector<game *>& games)
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

    // TODO replace separator maybe?
    if (game_string.find(sep) != string::npos)
    {
        cerr << "human_readable_game_string() output contains separator string" << endl;
        exit(-1);
    }

    return game_string;
}

void append_field(ostream& os, const string& field, bool include_separator)
{
    // Remove leading/trailing whitespace, replace quotes
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


void run_autotests(const string& test_directory, const string& outfile_name, unsigned long long test_timeout)
{
    ofstream outfile(outfile_name);

    if (!outfile.is_open())
    {
        throw ios_base::failure("Couldn't open file for writing: \"" + outfile_name +  "\"");
    }


    // print format to file
    append_field(outfile, "File", true);
    append_field(outfile, "Case", true);
    append_field(outfile, "Games", true);
    append_field(outfile, "Player", true);
    append_field(outfile, "Expected", true);
    append_field(outfile, "Got", true);
    append_field(outfile, "Time (ms)", true);
    append_field(outfile, "Outcome", true);
    append_field(outfile, "Comments", false);
    outfile << newline;


    // iterate over autotests directory
    for (const filesystem::directory_entry& entry : recursive_directory_iterator(test_directory))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        const filesystem::path& file_path = entry.path();

        if (file_path.extension() != ".test")
        {
            continue;

        }

        cout << entry << endl;

        const string file_name = file_path.string();

        // Open test
        file_parser* parser = file_parser::from_file(file_name);
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

            string win_string = test_outcome_to_string(TEST_OUTCOME_UNKNOWN);

            if (result)
            {
                win_string = result.value().win ? test_outcome_to_string(TEST_OUTCOME_WIN)
                    : test_outcome_to_string(TEST_OUTCOME_LOSS);
            }

            string outcome_string = "TIMEOUT";
            if (result)
            {
                outcome_string = (result.value().win == gc.expected_outcome) ? "PASS" : "FAIL";
            }



            append_field(outfile, file_name, true);
            append_field(outfile, to_string(case_number), true);
            append_field(outfile, human_readable_game_string(gc.games), true);
            append_field(outfile, string(1, color_char(gc.to_play)), true);
            append_field(outfile, test_outcome_to_string(gc.expected_outcome), true);
            append_field(outfile, win_string, true);
            append_field(outfile, to_string(duration.count()), true);
            append_field(outfile, outcome_string, true);
            append_field(outfile, gc.comments, false);
            outfile << newline;


            gc.cleanup_games();


            case_number++;
            gc.cleanup_games();
        }




        delete parser;


    }

    outfile.close();
}
