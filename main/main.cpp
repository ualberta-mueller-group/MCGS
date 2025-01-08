//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <cstdio>
#include <iostream>
#include <string>
#include "cgt_basics.h"
#include "alternating_move_game.h"
#include "cli_options.h"
#include "file_parser.h"
#include "sumgame.h"
#include "cli_options.h"

#include "all_game_headers.h"

using std::cout, std::endl, std::string;

int main(int argc, const char** argv)
{
    cli_options opts = parse_cli_args(argc, argv);

    if (opts.should_exit)
    {
        return 0;
    }

    // Run sums from input
    if (opts.parser)
    {
        game_case gc;

        while (opts.parser->parse_chunk(gc))
        {
            cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
            cout << "TEST CASE" << endl;
            cout << "Player: " << color_char(gc.to_play) << endl;
            cout << "Expected outcome: " << test_outcome_to_string(gc.expected_outcome) << endl;
            cout << endl;

            sumgame sum(gc.to_play);

            for (game* g : gc.games)
            {
                cout << *g << endl;
                sum.add(g);
            }
            cout << endl;


            if (opts.dry_run)
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
