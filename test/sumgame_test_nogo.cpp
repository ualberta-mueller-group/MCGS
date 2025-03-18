#include "nogo_1xn.h"
#include <cassert>
#include "test_utilities.h"

using std::string;

namespace {
void test_1_nogo(const string& s, bool res_b, bool res_w)
{
    nogo_1xn g(s);
    test_one_game(g, res_b, res_w);
}

void test_2_nogo(const string& s1, const string& s2, bool res_b, bool res_w)
{
    nogo_1xn g1(s1);
    nogo_1xn g2(s2);
    test_two_games(g1, g2, res_b, res_w);
}

void test_3_nogo(const string& s1, const string& s2, const string& s3,
                 bool res_b, bool res_w)
{
    nogo_1xn g1(s1);
    nogo_1xn g2(s2);
    nogo_1xn g3(s3);
    test_three_games(g1, g2, g3, res_b, res_w);
}

void sum_1()
{
    test_1_nogo("..", true, true);
    test_2_nogo("..", ".", true, true);
}

void sum_2()
{
    test_2_nogo("..", "..", false, false);
    test_2_nogo("X..", "..O", false, false);
}

void sum_3()
{
    test_3_nogo("..", "..", "..", true, true);
}

void sum_4()
{
    test_2_nogo("X..O", "X..", false, false);
}

void sum_5()
{
    test_2_nogo("..", "...", true, true);
}

void sum_6()
{
    test_2_nogo("...", "...", false, false);
}

void test_inverses()
{
    auto games = {".", "..", "..O", "X.X..", "X..O.O"};
    for (auto gs : games)
    {
        nogo_1xn g(gs);
        std::unique_ptr<game> inv(g.inverse());
        test_inverse(g, *inv);
    }
}

} // namespace

void sumgame_test_nogo_all()
{
    sum_1();
    sum_2();
    sum_3();
    sum_4();
    sum_5();
    sum_6();
    test_inverses();
}
