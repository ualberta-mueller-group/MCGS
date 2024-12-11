//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <iostream>
#include "cgt_basics.h"
#include "alternating_move_game.h"
#include "cgt_move.h"
#include "clobber_1xn.h"
#include "nim.h"
#include "nogo_1xn.h"
#include "temp.h"
#include "elephants_rhinos.h";

using std::cout, std::endl;

int main()
{
    std::cout << "\"SOME_VALUE\" is " << SOME_VALUE << std::endl;


    const char * strs[] = {
        "XX...XO.X",
        "X..X.O..O.O",
        "X.O",
        "X..X..X.....O..OOO",
    };

    /*
    for (int i = 0; i < sizeof(strs) / sizeof(strs[0]); i++)
    {
        elephants_rhinos pos(strs[i]);
        cout << pos << endl;

        move_generator* mg = pos.create_move_generator(BLACK);

        while (*mg)
        {
            move m = mg->gen_move();

            int from = cgt_move::first(m);
            int to = cgt_move::second(m);

            cout << from << " -> " << to << endl;
            ++(*mg);
        }

        delete mg;
        cout << endl;

    }
    */

    for (int i = 0; i < sizeof(strs) / sizeof(strs[0]); i++)
    {
        elephants_rhinos pos(strs[i]);
        cout << pos << endl;
        alternating_move_game g(pos, BLACK);
        bool result = g.solve();

        cout << "Solve elephants_rhinos: " << result << endl;

        cout << endl;
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
}
