/*
    main_test.cpp - main loop of MCGS unit test
*/

#include "nim_test.h"
#include "clobber_1xn_test.h"

int main()
{
    nim_test_all();
    clobber_1xn_test_all();
}
