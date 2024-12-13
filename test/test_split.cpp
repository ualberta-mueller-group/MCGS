#include "test_split.h"
#include <iostream>
#include "clobber_1xn.h"
#include "sumgame.h"

using std::cout, std::endl;

void test_split()
{
    clobber_1xn c1("XOX");
    clobber_1xn c2("XOXO");
    clobber_1xn c3("OXOX");

    sumgame game(BLACK);

    game.add(&c1);
    game.add(&c2);
    game.add(&c3);


    game.solve();







}
