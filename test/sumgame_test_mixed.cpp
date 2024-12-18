#include "sumgame_test_mixed.h"
#include "cgt_switch.h"
#include "clobber_1xn.h"
#include "test_utilities.h"

#include "all_game_headers.h"
#include <tuple>





#include <iostream>


void test_mixed1()
{
    // * + * == 0
    assert_sum_outcomes(false, false,
        make_factory<clobber_1xn>("XO"),
        make_factory<elephants>("X.O")
    );
}

void test_mixed2()
{
    // ** + * == *
    assert_sum_outcomes(true, true,
        make_factory<clobber_1xn>("XO.XO"),
        make_factory<elephants>("X.O")
    );
}

void test_mixed3()
{

}

void test_mixed4()
{

}

void test_mixed5()
{

}

void test_mixed6()
{

}

void test_mixed7()
{

}

void test_mixed8()
{

}

void test_mixed9()
{

}

void test_mixed10()
{

}

void test_mixed11()
{

}

void test_mixed12()
{

}

void test_mixed13()
{

}

void test_mixed14()
{

}

void test_mixed15()
{

}

void test_mixed16()
{

}

void test_mixed17()
{

}

void test_mixed18()
{

}

void test_mixed19()
{

}

void test_mixed20()
{

}

void test_mixed21()
{

}

void test_mixed22()
{

}

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
    test_mixed11();
    test_mixed12();
    test_mixed13();
    test_mixed14();
    test_mixed15();
    test_mixed16();
    test_mixed17();
    test_mixed18();
    test_mixed19();
    test_mixed20();
    test_mixed21();
    test_mixed22();
}
