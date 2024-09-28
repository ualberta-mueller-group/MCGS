/*
    main_test.cpp - main loop of MCGS unit test
*/

#include "cgt_move_test.h"
#include "clobber_1xn_test.h"
#include "nim_random_test.h"
#include "nim_test.h"

int main()
{
    cgt_move_test_all();
    clobber_1xn_test_all();
    nim_test_all();
    nim_random_test();
}
