#include "sumgame.h"
#include "sumgame_test_integer.h"
#include "sumgame_test_rational.h"
#include "sumgame_test_nimber.h"

void sumgame_test_all()
{
    sumgame_test_integer_all();
    sumgame_test_rational_all();
    sumgame_test_nimber_all();
}
