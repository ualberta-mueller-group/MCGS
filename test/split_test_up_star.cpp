#include "split_test_up_star.h"
#include "cgt_up_star.h"
#include "split_test_utils.h"

namespace {
void up_star1()
{
    up_star g(0, true);
    assert_no_split(&g);
}

void up_star2()
{
    up_star g(5, true);
    assert_no_split(&g);
}

void up_star3()
{
    up_star g(-4, true);
    assert_no_split(&g);
}

void up_star4()
{
    up_star g(0, false);
    assert_no_split(&g);
}

void up_star5()
{
    up_star g(20, false);
    assert_no_split(&g);
}

void up_star6()
{
    up_star g(-13, false);
    assert_no_split(&g);
}

} // namespace

void split_test_up_star_all()
{
    up_star1();
    up_star2();
    up_star3();
    up_star4();
    up_star5();
    up_star6();
}
