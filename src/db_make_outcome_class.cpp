#include "db_make_outcome_class.h"

#include "cgt_basics.h"
#include "database.h"
#include "solver_stats.h"
#include "sumgame.h"
#include "thermograph_helpers.h"

outcome_class db_make_outcome_class(const database& db,
                                    const db_entry_partisan& entry)
{
    const std::shared_ptr<const ThGraph> therm =
        db.get_graph_from_id(entry.thermograph_id);
    assert(therm);

    const ThValue& left_stop = therm->LeftStop();
    const ThValue& right_stop = therm->RightStop();

    if (right_stop.IsPositive())
        return outcome_class::L;
    if (left_stop.IsNegative())
        return outcome_class::R;

    const bool left_bends = thermograph_bends_out_below_zero(*therm, true);
    const bool right_bends = thermograph_bends_out_below_zero(*therm, false);

    const bool zero_touches_left = (left_stop.IsZero() && !left_bends);
    const bool zero_touches_right = (right_stop.IsZero() && !right_bends);

    if (zero_touches_left && zero_touches_right) // 11
        return outcome_class::P;
    if (!zero_touches_left && zero_touches_right) // 01
        return outcome_class::L;
    if (zero_touches_left && !zero_touches_right) // 10
        return outcome_class::R;
    if (!zero_touches_left && !zero_touches_right) // 00
        return outcome_class::N;

    assert(false);
}
