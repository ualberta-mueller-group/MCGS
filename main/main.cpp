/*
    main.cpp - main loop of MCGS
*/

#include <iostream>
#include "../cgt_basics.h"
#include "../game.h"
#include "../nim.h"

using std::cout;

int main()
{
    nim g("1 2 3");
    bool result = g.solve();
    cout << g << result << std::endl;
}
