#include "sumgame_test_clobber_1xn.h"
#include "clobber_1xn.h"
#include <cassert>
#include "test_utilities.h"
#include <string>
#include <memory>

using std::string;

namespace {

void test_1_clobber(const string& s, bool res_b, bool res_w)
{
    clobber_1xn g(s);
    test_one_game(g, res_b, res_w);
}

void test_2_clobber(const string& s1, const string& s2, bool res_b, bool res_w)
{
    clobber_1xn g1(s1);
    clobber_1xn g2(s2);
    test_two_games(g1, g2, res_b, res_w);
}

void test_3_clobber(const string& s1, const string& s2, const string& s3,
                    bool res_b, bool res_w)
{
    clobber_1xn g1(s1);
    clobber_1xn g2(s2);
    clobber_1xn g3(s3);
    test_three_games(g1, g2, g3, res_b, res_w);
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

void test_inverses()
{
    auto games = {"X", "XO", "OXO", "XOOOOO", "XOXOXO"};
    for (auto gs : games)
    {
        clobber_1xn g(gs);
        std::unique_ptr<game> inv(g.inverse());
        test_inverse(g, *inv);
    }
}

} // namespace

void sumgame_test_clobber_1xn_all()
{
    sum_1();
    sum_2();
    sum_3();
    sum_4();
    sum_5();
    sum_6();
    test_inverses();
}
