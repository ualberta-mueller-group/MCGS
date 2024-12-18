#include "sumgame_test_mixed.h"
#include "cgt_integer_game.h"
#include "cgt_switch.h"
#include "cgt_up_star.h"
#include "clobber_1xn.h"
#include "elephants.h"
#include "nogo_1xn.h"
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
    // 5 + -5 == 0
    assert_sum_outcomes(false, false,
        make_factory<elephants>("X....."),
        make_factory<integer_game>(-5)
    );
}

void test_mixed4()
{
    // -3 + -3 + 6 == 0
    assert_sum_outcomes(false, false,
        make_factory<nogo_1xn>("O.O.O.O.X"),
        make_factory<elephants>("XO...O"),
        make_factory<integer_game>(6)
    );
}

void test_mixed5()
{
    // {-10 | -12} + * < 0
    assert_sum_outcomes(false, true,
        make_factory<elephants>("X.O...O....O"),
        make_factory<up_star>(0, true)
    );
}



void test_mixed6()
{
    // {-10 | -12} + ^* + * + v + {12 | 10} == 0
    assert_sum_outcomes(false, false,
        make_factory<elephants>("X.O...O....O"),
        make_factory<up_star>(1, true),
        make_factory<nogo_1xn>("X..O"),
        make_factory<clobber_1xn>("OOX"),
        make_factory<switch_game>(12, 10)
    );
}

void test_mixed7()
{
    // {-10 | -12} + ^* + * + {12 | 10} == ^ > 0
    assert_sum_outcomes(true, false,
        make_factory<elephants>("X.O...O....O"),
        make_factory<up_star>(1, true),
        make_factory<nogo_1xn>("X..O"),
        make_factory<switch_game>(12, 10)
    );
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
