#include "sumgame_test_nogo.h"
#include "nogo.h"
#include <iostream>
#include "test_utilities.h"

void sumgame_test_nogo_all()
{
    assert_inverse_sum_zero(new nogo("X.|.X"));
    assert_inverse_sum_zero(new nogo("..|.."));

    assert_sum_outcomes(true, false,
    {
        new nogo("X.|.X"),
        new nogo("..|.."),
    });

    assert_sum_outcomes(false, true,
    {
        new nogo("O.|.O"),
        new nogo("..|.."),
    });


    assert_sum_outcomes(false, false,
    {
        new nogo(".|."),
        new nogo("..|.."),
    });
}
