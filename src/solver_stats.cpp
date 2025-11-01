#include "solver_stats.h"

#include <unordered_set>
#include <cassert>

#include "global_options.h"
#include "hashing.h"
#include "sumgame.h"

void solver_stats::reset()
{
    node_count = 0;

    tt_hits = 0;
    tt_misses = 0;

    db_hits = 0;
    db_misses = 0;

    search_depth = 0;

    n_subgames = 0;

    if (global::count_sums())
        sum_hashes = std::unordered_set<hash_t>();
}

namespace stats {
solver_stats __global_stats;

void count_sum(const sumgame& sum)
{
    if (!global::count_sums())
        return;

    assert(__global_stats.sum_hashes.has_value());
    __global_stats.sum_hashes->insert(sum.get_global_hash());
}

} // namespace stats
