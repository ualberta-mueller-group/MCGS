//---------------------------------------------------------------------------
// main_test.cpp - main loop of MCGS unit test
// Imports all unit tests
//---------------------------------------------------------------------------

#include "cgt_basics_test.h"
#include "cgt_dyadic_rational_test.h"
#include "cgt_integer_game_test.h"
#include "cgt_move_test.h"
#include "cgt_nimber_test.h"
#include "cgt_switch_test.h"
#include "cgt_up_star_test.h"
#include "clobber_1xn_test.h"
#include "nim_random_test.h"
#include "nim_test.h"
#include "nogo_1xn_test.h"
#include "sumgame_test.h"
#include "elephants_test.h"

#include "test_split.h"




// TODO make a "do slow tests" build target?
const bool DO_SLOWER_TESTS = false;

int main()
{

    test_split();
    return 0;

    cgt_basics_test_all();
    cgt_dyadic_rational_test_all();
    cgt_integer_game_test_all();
    cgt_move_test_all();
    cgt_nimber_test_all();
    cgt_switch_test_all();
    cgt_up_star_test_all();

    if (DO_SLOWER_TESTS)
    {
        clobber_1xn_test_all();
        nim_test_all();
        nim_random_test(); // takes about a second.
        nogo_1xn_test_all();
        elephants_test_all(); // takes several seconds
    }
    sumgame_test_all();
}
