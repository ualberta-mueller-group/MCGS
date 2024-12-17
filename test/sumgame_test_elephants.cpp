#include "sumgame_test_elephants.h"
#include "cgt_basics.h"
#include "elephants.h"
#include "sumgame.h"
#include "test_utilities.h"

#include <iostream>
using std::cout, std::endl;

using std::string;



//////////////////////////////////////////////////////////// tests

void elephants1()
{
    // 1* (this game is positive)
    assert_sum_outcome(true, false, 
        game_spec<elephants>("X.O.X.X")
    );
}

void elephants2()
{
    // {* | -1*} (this game is negative, not 0)
    assert_sum_outcome(false, true, 
        game_spec<elephants>("X.O.X.X"),
        game_spec<elephants>("O.X.O.O")
    );

}

void elephants3()
{
    // 0
    assert_sum_outcome(false, false, 
        game_spec<elephants>("X.O.X.X"),
        game_spec<elephants>("O.O.X.O")
    );

}

void elephants4()
{
    assert_correct_inverse<elephants>("X.O.X.X");
}

void elephants5()
{
    assert_correct_inverse<elephants>("O....X");
    assert_correct_inverse<elephants>("X....O");
    assert_correct_inverse<elephants>("X.X.O.O");
}

void elephants6()
{
    // * + * == 0
    assert_sum_outcome(false, false, 
        game_spec<elephants>("X.O"),
        game_spec<elephants>("X.O")
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
