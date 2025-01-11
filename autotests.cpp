#include "autotests.h"

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

void run_autotests()
{
    ofstream outfile("out.txt");

    // print format to file

    outfile << "file name" << sep;
    outfile << "case number" << sep;
    outfile << "human readable games" << sep;
    outfile << "to play" << sep;
    outfile << "expected value" << sep;
    outfile << "found value" << sep;
    outfile << "time (ms)" << sep;
    outfile << "outcome (pass, fail, timeout)";
    outfile << newline;


    if (!outfile.is_open())
    {
        throw ios_base::failure("Couldn't open file out.txt");
    }

    // iterate over autotests directory
    for (const filesystem::directory_entry& entry : recursive_directory_iterator("test/input/autotests"))
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

            sumgame sum(gc.to_play);
            for (game* g : gc.games)
            {
                sum.add(g);
            }


            chrono::time_point start = chrono::high_resolution_clock::now();
            bool win = sum.solve();
            chrono::time_point end = chrono::high_resolution_clock::now();

            chrono::duration<double, std::milli> duration = end - start;

            string win_string = win ? test_outcome_to_string(TEST_OUTCOME_WIN)
                : test_outcome_to_string(TEST_OUTCOME_LOSS);



            outfile << file_name << sep;
            outfile << case_number << sep;
            outfile << human_readable_game_string(gc.games) << sep;
            outfile << color_char(gc.to_play) << sep;
            outfile << test_outcome_to_string(gc.expected_outcome) << sep;
            outfile << win_string << sep;
            outfile << duration.count() << sep;
            outfile << "TODO test outcome" << newline;

            gc.cleanup_games();
            break;





            case_number++;
            gc.cleanup_games();
        }




        delete parser;


    }

    outfile.close();
}
