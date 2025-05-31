#include "order_test.h"
#include <iostream>

#include "order_test_utilities.h"
#include "clobber_1xn.h"
#include "grid_utils.h"

#include <string>
#include <cassert>

using namespace std;

void order_test_all()
{
    std::cout << __FILE__ << " TODO: move to game_test.h files?" << std::endl;

    vector<game*> games;

    for (grid_generator gen(4); gen; ++gen)
        games.push_back(new clobber_1xn(gen.gen_board()));

    order_test_impl(games);

    for (game* g : games)
        delete g;
}
