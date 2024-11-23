#include "sumgame.h"
#include "clobber_1xn.h"
#include <iostream>
#include <cassert>
#include "test_utilities.h"

using std::string;

namespace {

void test_1_clobber(const string& s, bool resB, bool resW)
{
    clobber_1xn g(s);
    test_one_game(g, resB, resW);
}

void test_2_clobber(const string& s1, const string& s2, 
                    bool resB, bool resW)
{
    clobber_1xn g1(s1);
    clobber_1xn g2(s2);
    test_two_games(g1, g2, resB, resW);
}

void test_3_clobber(const string& s1, const string& s2, 
                    const string& s3, bool resB, bool resW)
{
    clobber_1xn g1(s1);
    clobber_1xn g2(s2);
    clobber_1xn g3(s3);
    test_three_games(g1, g2, g3, resB, resW);
}

void sum_1()
{
    test_1_clobber("XO", true, true);
    test_2_clobber("XO", "X", true, true);
}

void sum_2()
{
    test_2_clobber("XO", "OX", false, false);
}

void sum_3()
{
    test_3_clobber("XO", "XO", "XO", true, true);
}

void sum_4()
{
    test_2_clobber("XOX", "OXO", false, false);
}

void sum_5()
{
    test_2_clobber("XO", "XOXOXO", true, true);
}

void sum_6()
{
    test_2_clobber("XOXOXO", "OXOXOX", false, false);
}
} // namespace

void sumgame_test_clobber_all()
{
    sum_1();
    sum_2();
    sum_3();
    sum_4();
    sum_5();
    sum_6();
}
