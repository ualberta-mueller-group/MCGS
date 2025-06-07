#include "cgt_nimber_test.h"
#include "cgt_nimber.h"

#include <cassert>
#include "test_utilities.h"

namespace {
namespace cgt_nimber_test {

void test1()
{
    nimber g(0);
    test_one_game(g, false, false);
}

void test2()
{
    nimber g(3);
    test_one_game(g, true, true);
}

} // namespace cgt_nimber_test

namespace cgt_nimber_move_generator_test {

void test1()
{
    nimber g(0);
    assert_num_moves(g, BLACK, 0);
    assert_num_moves(g, WHITE, 0);
}

void test2()
{
    nimber g(12);
    assert_num_moves(g, BLACK, 12);
    assert_num_moves(g, WHITE, 12);
}

} // namespace cgt_nimber_move_generator_test
} // namespace

//---------------------------------------------------------------------------

void cgt_nimber_test_all()
{
    cgt_nimber_test::test1();
    cgt_nimber_test::test2();
    cgt_nimber_move_generator_test::test1();
    cgt_nimber_move_generator_test::test2();
}

//---------------------------------------------------------------------------
