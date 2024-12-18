#include "sumgame_test_switch.h"
#include "test_utilities.h"
#include "cgt_switch.h"

void switch1()
{
    assert_sum_outcomes(true, false, 
        int2_spec<switch_game>(5, 3)
    );

}

void switch2()
{

}

void switch3()
{

}

void switch4()
{

}

void switch5()
{

}

void switch6()
{

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
