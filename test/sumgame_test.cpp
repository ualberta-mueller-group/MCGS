#include "sumgame_test.h"

#include "sumgame_test_integer.h"
#include "sumgame_test_rational.h"
#include "sumgame_test_nimber.h"
#include "sumgame_random_test_nimber.h"
#include "sumgame_test_clobber_1xn.h"
#include "sumgame_test_clobber.h"
#include "sumgame_test_nogo_1xn.h"
#include "sumgame_test_elephants.h"
#include "sumgame_test_up_star.h"
#include "sumgame_test_switch.h"
#include "sumgame_test_mixed.h"

void sumgame_test_all()
{
    sumgame_test_integer_all();
    sumgame_test_rational_all();
    sumgame_test_nimber_all();
    sumgame_random_test_nimber();
    sumgame_test_clobber_1xn_all();
    sumgame_test_clobber_all();
    sumgame_test_nogo_1xn_all();
    sumgame_test_elephants_all();
    sumgame_test_up_star_all();
    sumgame_test_switch_all();
    sumgame_test_mixed_all();
}
