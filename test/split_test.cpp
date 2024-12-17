#include "split_test.h"

#include "split_test_elephants.h"
#include "split_test_clobber_1xn.h"
#include "split_test_dyadic_rational.h"
#include "split_test_integer_game.h"
#include "split_test_switch_game.h"
#include "split_test_nogo_1xn.h"

void split_test_all()
{
    // TODO do we need to test nim and other games that don't split?
    split_test_elephants_all();
    split_test_clobber_1xn_all();
    split_test_dyadic_rational_all();
    split_test_integer_game_all();
    split_test_switch_game_all();
    split_test_nogo_1xn_all();
}
