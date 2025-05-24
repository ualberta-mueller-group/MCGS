#include "sumgame_test_clobber.h"
#include "clobber.h"
#include "test/test_utilities.h"

void sumgame_test_clobber_all()
{
    assert_inverse_sum_zero(new clobber("X|X|O"));
    assert_inverse_sum_zero(new clobber("XO|.X"));

    assert_sum_outcomes(false, true,
    {
        new clobber("OX|XO|OX|OX"),
        new clobber("XO|OO"),
    });

    assert_sum_outcomes(true, false,
    {
        new clobber("XXO|...|XO."),
        new clobber("X|O"),
    });
}
