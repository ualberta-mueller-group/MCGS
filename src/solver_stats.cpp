#include "solver_stats.h"

void solver_stats::reset()
{
    node_count = 0;

    tt_hits = 0;
    tt_misses = 0;

    db_hits = 0;
    db_misses = 0;

    search_depth = 0;

    n_subgames = 0;
}

namespace stats {
solver_stats __global_stats;
} // namespace stats
