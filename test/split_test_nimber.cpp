#include "split_test_nimber.h"
#include "cgt_nimber.h"
#include "split_test_utils.h"

namespace {
void nimber1()
{
    nimber g(5);
    assert_no_split(&g);
}

void nimber2()
{
    nimber g(0);
    assert_no_split(&g);
}

void nimber3()
{
    nimber g(21);
    assert_no_split(&g);
}

} // namespace

void split_test_nimber_all()
{
    nimber1();
    nimber2();
    nimber3();
}
