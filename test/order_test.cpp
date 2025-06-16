#include "order_test.h"

#include <cassert>

#include "order_test_strips_and_grids.h"
#include "order_test_basic_cgt.h"

void order_test_all()
{
    order_test_strips_and_grids_all();
    order_test_basic_cgt_all();
}
