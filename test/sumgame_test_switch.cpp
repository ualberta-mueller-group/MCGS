#include "sumgame_test_switch.h"
#include "test_utilities.h"
#include "cgt_switch.h"

void switch1()
{
    // positive
    assert_sum_outcomes(true, false, {
        new switch_game(5, 3),
    });
}

void switch2()
{

    // negative
    assert_sum_outcomes(false, true, {
        new switch_game(-5, -7),
    });
}

void switch3()
{
    // zero
    assert_sum_outcomes(false, false, {
        new switch_game(30, 10),
        new switch_game(-10, -30),
    });
}

void switch4()
{
    // positive
    assert_sum_outcomes(true, false, {
        new switch_game(5, 4),
        new switch_game(2, -1),
        new switch_game(-1, -3),
        new switch_game(1, -1),
    });
}

void switch5()
{
    // N position
    assert_sum_outcomes(true, true, {
        new switch_game(10, -10),
    });
}

void switch6()
{
   
    // N position
    assert_sum_outcomes(true, true, {
        new switch_game(5, -4),
        new switch_game(1, -1),
        new switch_game(20, 5),
        new switch_game(-10, -20),
    });
}

void sumgame_test_switch_all()
{
    switch1();
    switch2();
    switch3();
    switch4();
    switch5();
    switch6();
}
