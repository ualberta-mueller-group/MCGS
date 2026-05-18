#include "db_make_bounds.h"

#include <memory>

#include "SgBlackWhite.h"
#include "ThGraph.h"
#include "ThScaffold.h"
#include "ThValue.h"
#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_dyadic_rational.h"
#include "fraction.h"
#include "thermograph_helpers.h"
#include "sumgame.h"

using namespace std;


//////////////////////////////////////////////////
std::shared_ptr<game_bounds> db_make_bounds(const database& db, sumgame& sum,
                                            const db_entry_partisan& entry)
{
    assert_restore_sumgame ars(sum);

    const std::shared_ptr<ThGraph>& therm = entry.thermograph;
    assert(therm);

    if (game_is_small_from_thermograph(*therm))
    {
        vector<bounds_options> options_vec;
        vector<game_bounds_ptr> bounds_vec;

        bound_t scale_min = -512;
        bound_t scale_max = 512;

        assert(entry.outcome != outcome_class::U);

        if (entry.outcome == outcome_class::L)
            scale_min = 0;
        if (entry.outcome == outcome_class::R)
            scale_max = 0;

        options_vec.emplace_back(BOUND_SCALE_UP, scale_min, scale_max, entry.outcome);
        bounds_vec = find_bounds(sum, options_vec);
        assert(options_vec.size() == 1 && bounds_vec.size() == 1);

        game_bounds_ptr bounds = bounds_vec.back();

        assert(bounds->both_valid());
        return bounds;
    }

    shared_ptr<game_bounds> bounds_therm(make_bounds_from_thermograph(*therm));
    assert(bounds_therm->both_valid());
    return bounds_therm;
}
