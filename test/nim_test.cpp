//---------------------------------------------------------------------------
// Unit tests for the game of nim
//---------------------------------------------------------------------------
#include "nim.h"

#include <cassert>
#include <iostream>
#include "test_utilities.h"

using std::cout;
using std::endl;

void nim_test_zero()
{
    nim g("");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == false);
}

void nim_test_zero2()
{
    nim g("0");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == false);
}

void nim_test_1()
{
    nim g("1");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == true);
}

void nim_test_2()
{
    nim g("2");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == true);
}

void nim_test_3()
{
    nim g("10");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == true);
}

void nim_test_sum_1()
{
    nim g("1 0");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == true);
}

void nim_test_sum_2()
{
    nim g("1 1");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == false);
}

void nim_test_sum_3()
{
    nim g("4 5");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == true);
}

void nim_test_sum_4()
{
    nim g("1 2 3");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == false);
}

void nim_test_sum_5()
{
    nim g("3 4 5 6");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == true);
}

void nim_test_sum_6()
{
    nim g("3 4 5 2");
    bool result = g.solve();
    assert(result == g.static_solve());
    assert(result == false);
}


void nim_move_generator_test_1()
{
    nim g("2 0 1");
    assert(g.heaps().size() == 2); // removed 0 heap so heaps = [2,1]
    
    std::unique_ptr<move_generator>mgp(g.create_mg());
    move_generator& mg(*mgp);
    assert(mg);
    assert_move(mg, 0, 1);
    ++mg;
    assert(mg);
    assert_move(mg, 0, 2);
    ++mg;
    assert(mg);
    assert_move(mg, 1, 1);
    ++mg;
    assert(!mg);
}

void nim_test_all()
{
    nim_test_zero();
    nim_test_zero2();
    nim_move_generator_test_1();
    nim_test_1();
    nim_test_2();
    nim_test_3();
    nim_test_sum_1();
    nim_test_sum_2();
    nim_test_sum_3();
    nim_test_sum_4();
    nim_test_sum_5();
    nim_test_sum_6();
}