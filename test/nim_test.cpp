#include <cassert>
#include "nim.h"

void test_nim_zero()
{
    nim g("");
    bool result = g.solve();
    assert(result == false);
    
void test_nim_zero2()
{
    nim g("0");
    bool result = g.solve();
    assert(result == false);
}

void test_nim_1()
{
    nim g("1");
    bool result = g.solve();
    assert(result == true);
}

void test_nim_2()
{
    nim g("2");
    bool result = g.solve();
    assert(result == true);
}

void test_nim_3()
{
    nim g("10");
    bool result = g.solve();
    assert(result == true);
}

void test_nim_sum_1()
{
    nim g("1 0");
    bool result = g.solve();
    assert(result == true);
}

void test_nim_sum_2()
{
    nim g("1 1");
    bool result = g.solve();
    assert(result == false);
}

void test_nim_sum_3()
{
    nim g("4 5");
    bool result = g.solve();
    assert(result == true);
}

void test_nim_sum_4()
{
    nim g("1 2 3");
    bool result = g.solve();
    assert(result == false);
}

void test_nim_sum_5()
{
    nim g("3 4 5 6");
    bool result = g.solve();
    assert(result == false);
}

void test_nim_sum_6()
{
    nim g("3 4 5 2");
    bool result = g.solve();
    assert(result == true);
}

void test_all_nim()
{
    test_nim_zero();
    test_nim_zero2();
    test_nim_1();
    test_nim_2()
    test_nim_3();
    test_nim_sum_1();
    test_nim_sum_2();
    test_nim_sum_3();
    test_nim_sum_4();
    test_nim_sum_5();
    test_nim_sum_6();
}