/*
    main.cpp - main loop of MCGS
*/

#include "cgt_basics.h"
#include "game.h"
#include "nim.h"

int main()
{
    game g = nim([1,2,3]);
    bool result = g.solve();
    cout << nim << result << endl;
}
