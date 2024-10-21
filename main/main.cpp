//---------------------------------------------------------------------------
// main.cpp - main loop of MCGS
//---------------------------------------------------------------------------

#include <iostream>
#include "cgt_basics.h"
#include "game.h"
#include "clobber_1xn.h"
#include "nim.h"
#include "nogo_1xn.h"

using std::cout;

int main()
{
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
