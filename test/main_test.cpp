/*
    main_test.cpp - main loop of MCGS unit test
*/

#include "cgt_game_test.h"
#include "cgt_integer_game_test.h"
#include "cgt_move_test.h"
#include "clobber_1xn_test.h"
#include "nim_random_test.h"
#include "nim_test.h"

// TODO make a "do slow tests" build target?
const bool DO_SLOWER_TESTS = false;

int main()
{
    cgt_game_test_all();
    cgt_integer_game_test_all();
    cgt_move_test_all();
    clobber_1xn_test_all();
    nim_test_all();
    if (DO_SLOWER_TESTS)
        nim_random_test(); // takes about a second.
}
