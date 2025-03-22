#include "sumgame_test_mixed.h"
#include "all_game_headers.h"
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "cgt_switch.h"
#include "cgt_up_star.h"
#include "clobber_1xn.h"
#include "elephants.h"
#include "nogo_1xn.h"
#include "test_utilities.h"

namespace {
void test_mixed1()
{
    // * + * == 0
    assert_sum_outcomes(false, false,
                        {
                            new clobber_1xn("XO"),
                            new elephants("X.O"),
                        });
}

void test_mixed2()
{

    // ** + * == *
    assert_sum_outcomes(true, true,
                        {
                            new clobber_1xn("XO.XO"),
                            new elephants("X.O"),
                        });
}

void test_mixed3()
{

    // 5 + -5 == 0
    assert_sum_outcomes(false, false,
                        {
                            new elephants("X....."),
                            new integer_game(-5),
                        });
}

void test_mixed4()
{

    // -3 + -3 + 6 == 0
    assert_sum_outcomes(false, false,
                        {
                            new nogo_1xn("O.O.O.O.X"),
                            new elephants("XO...O"),
                            new integer_game(6),
                        });
}

void test_mixed5()
{
    // {-10 | -12} + * < 0
    assert_sum_outcomes(false, true,
                        {
                            new elephants("X.O...O....O"),
                            new up_star(0, true),
                        });
}

void test_mixed6()
{

    // {-10 | -12} + ^* + * + v + {12 | 10} == 0
    assert_sum_outcomes(false, false,
                        {
                            new elephants("X.O...O....O"),
                            new up_star(1, true),
                            new nogo_1xn("X..O"),
                            new clobber_1xn("OOX"),
                            new switch_game(12, 10),
                        });
}

void test_mixed7()
{
    // {-10 | -12} + ^* + * + {12 | 10} == ^ > 0
    assert_sum_outcomes(true, false,
                        {
                            new elephants("X.O...O....O"),
                            new up_star(1, true),
                            new nogo_1xn("X..O"),
                            new switch_game(12, 10),
                        });
}

void test_mixed8()
{
    // (2 + -4) + 2 == 0
    assert_sum_outcomes(false, false,
                        {
                            new nogo_1xn("X.X.XX.OX.OO.O.O.O.O"),
                            new integer_game(2),
                        });
}

void test_mixed9()
{
    // * + *4 + *4 + * == 0
    assert_sum_outcomes(false, false,
                        {
                            new nimber(1),
                            new nimber(4),
                            new nimber(4),
                            new clobber_1xn("XO"),
                        });
}

void test_mixed10()
{
    // {1 | 2} - (3/2) == + or - 1/2 --> N position
    assert_sum_outcomes(true, true,
                        {
                            new switch_game(2, 1),
                            new dyadic_rational(-3, 2),
                        });
}
} // namespace

void sumgame_test_mixed_all()
{
    test_mixed1();
    test_mixed2();
    test_mixed3();
    test_mixed4();
    test_mixed5();
    test_mixed6();
    test_mixed7();
    test_mixed8();
    test_mixed9();
    test_mixed10();
}
