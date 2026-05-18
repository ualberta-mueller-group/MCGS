#include "test_filter.h"

#include <cassert>
#include <vector>

#include "game.h"
#include "test_case.h"
#include "test_case_enums.h"

#include "clobber_1xn.h"

////////////////////////////////////////////////// Filter implementations
namespace {
bool segclobber_filter(const i_test_case& tc)
{
    if (tc.get_command_type() != COMMAND_TYPE_SOLVE_BW)
        return false;

    const std::vector<game*>& games = tc.get_games();

    for (const game* g : games)
    {
        if (g->game_type() != game_type<clobber_1xn>())
            return false;
    }

    return true;
}
} // namespace

//////////////////////////////////////////////////
bool test_filter_permits_test_case(test_filter_enum filter_type,
                                   const i_test_case& tc)
{
    switch (filter_type)
    {
        case TEST_FILTER_MCGS:
            return true;
        case TEST_FILTER_SEGCLOBBER:
            return segclobber_filter(tc);
        default:
            assert(false);
    }
}
