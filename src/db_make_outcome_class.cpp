#include "db_make_outcome_class.h"

#include "cgt_basics.h"
#include "database.h"
#include "solver_stats.h"
#include "sumgame.h"
#include "thermograph_helpers.h"

namespace {

inline outcome_class make_outcome_from_search(sumgame& sum)
{
    const bw restore_player = sum.to_play();

    sum.set_to_play(BLACK);
    const bool black_wins = sum.solve();

    sum.set_to_play(WHITE);
    const bool white_wins = sum.solve();

    sum.set_to_play(restore_player);

    return bools_to_outcome_class(black_wins, white_wins);
}

#ifdef MCGS_USE_THERM
inline outcome_class make_outcome_from_thermograph(
    const db_entry_partisan& entry)
{
    assert(entry.thermograph);
    const ThGraph& th = *entry.thermograph;

    const ThValue& left_stop = th.LeftStop();
    const ThValue& right_stop = th.RightStop();

    if (right_stop.IsPositive())
        return outcome_class::L;
    if (left_stop.IsNegative())
        return outcome_class::R;

    const bool left_bends = thermograph_bends_out_below_zero(th, true);
    const bool right_bends = thermograph_bends_out_below_zero(th, false);

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
#endif

} // namespace

//////////////////////////////////////////////////
outcome_class db_make_outcome_class(sumgame& sum,
                                    const db_entry_partisan& entry)
{
    //const outcome_class oc_search = make_outcome_from_search(sum);
    const outcome_class oc_therm = make_outcome_from_thermograph(entry);

    //assert(oc_search == oc_therm);

    return oc_therm;
}
