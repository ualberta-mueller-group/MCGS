#include "sumgame_test_up_star.h"
#include "cgt_up_star.h"
#include <cassert>
#include "test_utilities.h"
#include <memory>

namespace {

void test_up_stars()
{
    up_star zero(0, false);
    test_zero_1(zero);
    up_star star(0, true);
    test_one_game(star, true, true);
    {
        up_star star(0, true);
        up_star star2(0, true);
        test_zero_2(star, star2);
    }
    up_star up1(1, false);
    test_one_game(up1, true, false);
    up_star up1s(1, true);
    test_one_game(up1s, true, true);
    up_star down1(-1, false);
    test_one_game(down1, false, true);
    up_star down1s(-1, true);
    test_one_game(down1s, true, true);
    {
        up_star up1(1, false);
        up_star down1(-1, false);
        test_zero_2(up1, down1);
    }
    {
        up_star up1s(1, true);
        up_star down1s(-1, true);
        test_zero_2(up1s, down1s);
    }
    {
        up_star up1(1, false);
        up_star up1s(1, true);
        up_star down2s(-2, true);
        test_zero_3(up1, up1s, down2s);
    }
}

void test_inverses()
{
    auto games = {0, 1, -1, 2, -3, 16, 8};
    for (auto up : games)
    {
        up_star g1(up, false);
        std::unique_ptr<game> inv1(g1.inverse());
        test_inverse(g1, *inv1);

        up_star g2(up, true);
        std::unique_ptr<game> inv2(g2.inverse());
        test_inverse(g2, *inv2);
    }
}

} // namespace

void sumgame_test_up_star_all()
{
    test_up_stars();
    test_inverses();
}
