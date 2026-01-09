#include "solver_stats.h"

#include <cassert>
#include <unordered_set>

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

void solver_stats::print_search_statistics(std::ostream& ostr)
{
    ostr << "\nSearch statistics:\nnode_count " << node_count
         << "\ntt_hits " << tt_hits
         << "\ntt_misses " << tt_misses
         << "\ndb_hits " << db_hits
         << "\ndb_misses " << db_misses
         << "\nsearch_depth " << search_depth
         << "\nn_subgames " << n_subgames
         // << "\nsum_hashes " << sum_hashes.size() TODO if has_value()...
         << std::endl;
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

void print_search_statistics(std::ostream& ostr)
{
    __global_stats.print_search_statistics(ostr);
}

} // namespace stats
