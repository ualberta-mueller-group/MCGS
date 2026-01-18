/*
    Solver statistics tracking.
    
    Must call reset_stats() before every test case. This should be done
    in i_test_case::run()
*/
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <ostream>
#include <unordered_set>

#include "global_options.h"
#include "hashing.h"
#include "sumgame.h"
#include "utilities.h"

constexpr uint64_t INITIAL_SEARCH_DEPTH = 0;

////////////////////////////////////////////////// struct solver_stats
struct solver_stats
{
    solver_stats() { reset(); }

    void reset();
    void print_search_statistics(std::ostream& ostr) const;

    std::optional<double> get_tt_hit_rate() const;
    std::optional<double> get_db_hit_rate() const;

    // TT accesses
    uint64_t tt_hits;
    uint64_t tt_misses;

    // DB accesses
    uint64_t db_hits;
    uint64_t db_misses;

    // Nodes
    uint64_t search_node_count;
    std::optional<std::unordered_set<hash_t>> search_node_hashes;
    uint64_t max_search_depth;

    // Subgames
    size_t max_subgame_count;

    // Initial node values
    bool has_initial_values;
    std::optional<size_t> initial_subgame_count;
};

// Global solver_stats object
namespace stats {
extern solver_stats __global_stats; // NOLINT(readability-identifier-naming)
} // namespace stats

////////////////////////////////////////////////// Stats/reporting functions
//////////////////////////////////////// Declarations
namespace stats {

// Get/print/reset
const solver_stats& get_global_stats();
void print_global_stats(std::ostream& ostr);
void reset_global_stats();

// Report TT/DB access
void report_tt_access(bool hit);
void report_db_access(bool hit);

// Report per-node
void report_search_node(const sumgame& sum, ebw to_play, uint64_t depth);
void report_search_node(const std::vector<game*>& games, ebw to_play,
                        uint64_t depth);
void report_search_node(const game* g, ebw to_play, uint64_t depth);

/*
    NOTE: node_hash MUST be present IFF global::count_sums() is true. An
    exception is thrown when either incorrectly omitted OR included.

    Use the other report_search_node() functions where possible.
*/
void report_search_node_verbose(const std::optional<hash_t>& node_hash,
                                size_t subgame_count, ebw to_play,
                                uint64_t depth);

global_hash& get_global_hash_helper();

} // namespace stats

//////////////////////////////////////// Implementations
namespace stats {
// Implementation details
// NOLINTBEGIN(readability-identifier-naming)
inline void __report_search_node_common(size_t subgame_count, uint64_t depth)
{
    __global_stats.search_node_count++;

    __global_stats.max_search_depth =
        std::max(__global_stats.max_search_depth, depth);

    __global_stats.max_subgame_count =
        std::max(__global_stats.max_subgame_count, subgame_count);
}

void __report_search_node_initial(size_t initial_subgame_count);

void __count_search_node_hash(const sumgame& sum, ebw to_play);
void __count_search_node_hash(const std::vector<game*>& games, ebw to_play);
void __count_search_node_hash(const game* g, ebw to_play);
void __count_search_node_hash(hash_t node_hash);
// NOLINTEND(readability-identifier-naming)

inline const solver_stats& get_global_stats()
{
    return __global_stats;
}

inline void print_global_stats(std::ostream& ostr)
{
    __global_stats.print_search_statistics(ostr);
}

inline void reset_global_stats()
{
    __global_stats.reset();
}

inline void report_tt_access(bool hit)
{
    if (hit)
        __global_stats.tt_hits++;
    else
        __global_stats.tt_misses++;
}

inline void report_db_access(bool hit)
{
    if (hit)
        __global_stats.db_hits++;
    else
        __global_stats.db_misses++;
}

inline void report_search_node(const sumgame& sum, ebw to_play, uint64_t depth)
{
    const int n_active = sum.num_active_games();
    __report_search_node_common(n_active, depth);

    if (global::count_sums())
        __count_search_node_hash(sum, to_play);

    if (!__global_stats.has_initial_values) [[unlikely]]
        __report_search_node_initial(n_active);
}

inline void report_search_node(const std::vector<game*>& games, ebw to_play,
                        uint64_t depth)
{
    size_t n_active = 0;
    for (const game* g : games)
        if (g->is_active())
            n_active++;

    __report_search_node_common(n_active, depth);

    if (global::count_sums())
        __count_search_node_hash(games, to_play);

    if (!__global_stats.has_initial_values) [[unlikely]]
        __report_search_node_initial(n_active);
}

inline void report_search_node(const game* g, ebw to_play, uint64_t depth)
{
    assert(g->is_active());

    __report_search_node_common(1, depth);

    if (global::count_sums())
        __count_search_node_hash(g, to_play);

    if (!__global_stats.has_initial_values) [[unlikely]]
        __report_search_node_initial(1);
}

inline void report_search_node_verbose(const std::optional<hash_t>& node_hash,
                                       size_t subgame_count, ebw to_play,
                                       uint64_t depth)
{
    assert(logical_iff(node_hash.has_value(), global::count_sums()));

    __report_search_node_common(subgame_count, depth);

    if (global::count_sums())
        __count_search_node_hash(node_hash.value());

    if (!__global_stats.has_initial_values) [[unlikely]]
        __report_search_node_initial(subgame_count);
}

} // namespace stats

////////////////////////////////////////////////// init global_hash
namespace mcgs_init {
void init_solver_stats();
} // namespace mcgs_init
