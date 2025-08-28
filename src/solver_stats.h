/*
    Solver statistics tracking. Must call reset_stats() after each
    sumgame::solve
*/
#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>

//////////////////////////////////////////////////
struct solver_stats
{
    void reset();

    uint64_t node_count;

    uint64_t tt_hits;
    uint64_t tt_misses;

    uint64_t db_hits;
    uint64_t db_misses;

    uint64_t search_depth;

    size_t n_subgames;
};

//////////////////////////////////////////////////
namespace stats {
// TODO make these methods instead of global functions...

// NOLINTBEGIN(readability-identifier-naming)
extern solver_stats __global_stats;

// NOLINTEND(readability-identifier-naming)

inline void reset_stats()
{
    __global_stats.reset();
}

inline const solver_stats& get_global_stats()
{
    return __global_stats;
}

inline void inc_node_count()
{
    __global_stats.node_count++;
}

inline void tt_access(bool hit)
{
    if (hit)
        __global_stats.tt_hits++;
    else
        __global_stats.tt_misses++;
}

inline void db_access(bool hit)
{
    if (hit)
        __global_stats.db_hits++;
    else
        __global_stats.db_misses++;
}

inline void update_search_depth(const uint64_t& current_depth)
{
    __global_stats.search_depth =
        std::max(__global_stats.search_depth, current_depth);
}

inline void set_n_subgames(const size_t& n_subgames)
{
    __global_stats.n_subgames = n_subgames;
}

} // namespace stats
