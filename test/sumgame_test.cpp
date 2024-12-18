#include "sumgame.h"
#include "sumgame_test_integer.h"
#include "sumgame_test_rational.h"
#include "sumgame_test_nimber.h"
#include "sumgame_test_clobber.h"
#include "sumgame_test_nogo.h"
#include "sumgame_test_elephants.h"
#include "sumgame_test_up_star.h"
#include "sumgame_test_switch.h"

void sumgame_test_all()
{
    sumgame_test_integer_all();
    sumgame_test_rational_all();
    sumgame_test_nimber_all();
    sumgame_test_clobber_all();
    sumgame_test_nogo_all();
    sumgame_test_elephants_all();
    sumgame_test_up_star_all();
    sumgame_test_switch_all();
}
