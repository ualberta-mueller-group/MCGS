/*
    main.cpp - main loop of MCGS
*/

#include <iostream>
#include "cgt_basics.h"
#include "game.h"
#include "clobber_1xn.h"
#include "nim.h"

using std::cout;

int main()
{
    {
        nim g("1 2 3");
        bool result = g.solve();
        cout << "Solve nim " << g << ", result " << result << std::endl;
    }

    {
        clobber_1xn g("XOXOXO");
        bool result = g.solve();
        cout << "Solve clobber_1xn " << g << ", result " << result << std::endl;
    }

    {
        clobber_1xn g("XXOXOXOOX");
        bool result = g.solve();
        cout << "Solve clobber_1xn " << g << ", result " << result << std::endl;
    }
}
