/*
    Filters for determining whether or not a test case is compatible with a
    specified solver.

    MCGS filter always returns true

    Use `--filter-tests` CLI option to use filters.
*/
#pragma once

#include "test_case.h"

enum test_filter_enum
{
    TEST_FILTER_MCGS = 0,
    TEST_FILTER_SEGCLOBBER,
};

// true IFF keep test, false IFF skip test
bool test_filter_permits_test_case(test_filter_enum filter_type,
                                   const i_test_case& tc);
