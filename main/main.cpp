//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <cstdio>
#include <iostream>
#include <string>
#include "cgt_basics.h"
#include "alternating_move_game.h"
#include "clobber_1xn.h"
#include "nim.h"
#include "nogo_1xn.h"
#include "elephants.h"
#include "file_parser.h"
#include "sumgame.h"

using std::cout, std::endl, std::string;

/*

*/

int main(int argc, char** argv)
{
    // TODO clean this all up and move it somewhere else. Store options in some other file

    bool skip_running = false;

    // Parse args
    vector<string> args;
    for (int i = 0; i < argc; i++)
    {
        args.push_back(string(argv[i]));
    }

    file_parser* parser = nullptr;

    int arg_idx = 0;
    for (arg_idx = 1; arg_idx < argc; arg_idx++) // skip "./MCGS"
    {
        const string& arg = args[arg_idx];
        const string& arg_next = (arg_idx + 1) < argc ? args[arg_idx + 1] : "";

        if (arg == "--stdin" && parser == nullptr)
        {
            cout << "Reading game input from stdin" << endl;
            parser = file_parser::from_stdin();
            continue;
        }

        if (arg == "--file")
        {
            arg_idx++; // consume the file name

            if (arg_next.size() == 0)
            {
                cout << "Error: got --file but no file path" << endl;
                return -1;
            }

            if (parser == nullptr)
            {
            cout << "Reading game input from file: \"" << arg_next << "\"" << endl;
                parser = file_parser::from_file(arg_next);
            }

            continue;
        }

        if (arg == "-h" || arg == "--help")
        {
            cout << "TODO: print useful help message" << endl;
            continue;
        }

        if (arg == "--skip-running")
        {
            skip_running = true;
            continue;
        }

        if (arg.size() > 0 && arg.front() != '-')
        {
            // the rest of args is input to the parser

            // for now it should be quoted, so there should only be one arg at this point...
            assert(arg_idx == argc - 1);

            break;
        }

        cout << "Error: unrecognized flag (TODO print help message)" << endl;
        return -1;
    }

    if (parser == nullptr)
    {
        cout << "Reading game input from args" << endl;
        const string& input = arg_idx < argc ? args[arg_idx] : "";

        parser = file_parser::from_string(input);
    }

    {
        game_case gc;
        while (parser->parse_chunk(gc))
        {

            cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
            cout << "TEST CASE" << endl;
            cout << "Player: " << gc.to_play << endl;
            cout << "Expected outcome: " << gc.expected_outcome << endl;

            sumgame sum(gc.to_play);

            for (game* g : gc.games)
            {
                cout << *g << endl;
                sum.add(g);
            }


            if (skip_running)
            {
                cout << "Not running games..." << endl;
            } else
            {
                bool result = sum.solve();
                cout << "Result: " << result << endl;
            }
            cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
            gc.cleanup_games();
        }
    }

    {
        nim pos("1 2 3");
        alternating_move_game g(pos, BLACK);
        bool result = g.solve();
        cout << "Solve nim " << pos << ", result " << result << std::endl;
    }

    {
        clobber_1xn pos("XOXOXO");
        alternating_move_game g(pos, BLACK);
        bool result = g.solve();
        cout << "Solve clobber_1xn " << pos << ", result " << result << std::endl;
    }

    {
        clobber_1xn pos("XXOXOXOOX");
        alternating_move_game g(pos, BLACK);
        bool result = g.solve();
        cout << "Solve clobber_1xn " << pos << ", result " << result << std::endl;
    }

    {
        nogo_1xn pos("....");
        alternating_move_game g(pos, BLACK);
        bool result = g.solve();
        cout << "Solve nogo_1xn " << pos << ", result " << result << std::endl;
    }

    {
        elephants pos("X..X.O..O.O");
        alternating_move_game g(pos, BLACK);
        bool result = g.solve();
        cout << "Solve elephants " << pos << ", result " << result << endl;
        
    }
}
