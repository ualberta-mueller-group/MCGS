#include "sumgame_test_elephants.h"
#include "cgt_basics.h"
#include "elephants.h"
#include "sumgame.h"
#include "test_utilities.h"

#include <iostream>
using std::cout, std::endl;

using std::string;

void elephants1()
{
    // 1* (this game is positive)
    assert_sum_outcomes(true, false, 
        string_spec<elephants>("X.O.X.X")
    );
}

void elephants2()
{
    // {* | -1*} (this game is negative, not 0)
    assert_sum_outcomes(false, true, 
        string_spec<elephants>("X.O.X.X"),
        string_spec<elephants>("O.X.O.O")
    );

}

void elephants3()
{
    // 0
    assert_sum_outcomes(false, false, 
        string_spec<elephants>("X.O.X.X"),
        string_spec<elephants>("O.O.X.O")
    );

}

void elephants4()
{
    assert_inverse_sum_zero(string_spec<elephants>("X.O.X.X"));
}

void elephants5()
{
    assert_inverse_sum_zero(string_spec<elephants>("O....X"));
    assert_inverse_sum_zero(string_spec<elephants>("X....O"));
    assert_inverse_sum_zero(string_spec<elephants>("X.X.O.O"));
}

void elephants6()
{
    // * + * == 0
    assert_sum_outcomes(false, false, 
        string_spec<elephants>("X.O"),
        string_spec<elephants>("X.O")
    );

}


void sumgame_test_elephants_all()
{
    elephants1();
    elephants2();
    elephants3();
    elephants4();
    elephants5();
    elephants6();
}
