//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "cgt_basics.h"
#include "alternating_move_game.h"
#include "cgt_move.h"
#include "clobber_1xn.h"
#include "nim.h"
#include "nogo_1xn.h"
#include "elephants.h"
#include "file_parser.h"

using std::cout, std::endl;

int main(int argc, char** argv)
{

    file_parser::init_game_parsers();



    //parse("general.test");
    //std::stringstream stream(argv[1]);

    file_parser p = file_parser::from_file("simple.test");

    game_case gc;
    while (p.parse_chunk(gc))
    {
        cout << "GOT CASE" << endl;
        gc.cleanup_games();
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
