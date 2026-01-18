#include "solver_stats.h"

#include <cassert>
#include <unordered_set>
#include <iostream>
#include <optional>
#include <vector>

#include "cgt_basics.h"
#include "global_options.h"
#include "hashing.h"
#include "sumgame.h"

// Global solver_stats object
solver_stats stats::__global_stats;


////////////////////////////////////////////////// solver_stats methods
void solver_stats::reset()
{
    // TT accesses
    tt_hits = 0;
    tt_misses = 0;

    // DB accesses
    db_hits = 0;
    db_misses = 0;

    // Nodes
    search_node_count = 0;
    max_search_depth = 0;
    if (global::count_sums())
        search_node_hashes = std::unordered_set<hash_t>();

    // Subgames
    max_subgame_count = 0;

    // Initial node values
    has_initial_values = false;
    initial_subgame_count.reset();
}

#ifdef PRINT_FIELD
#error Macro PRINT_FIELD already defined...
#endif

#ifdef PRINT_FIELD_OPTIONAL
#error Macro PRINT_FIELD_OPTIONAL already defined...
#endif

#define PRINT_FIELD(field) \
    ostr << "\n" #field " " << (field); \
    static_assert(true)

#define PRINT_FIELD_OPTIONAL(field, expr) \
    if (field.has_value()) \
        ostr << "\n" #expr " " << (expr); \
    static_assert(true)

void solver_stats::print_search_statistics(std::ostream& ostr) const
{
    ostr << "\nSearch statistics:";

    PRINT_FIELD(tt_hits);
    PRINT_FIELD(tt_misses);

    PRINT_FIELD(db_hits);
    PRINT_FIELD(db_misses);

    PRINT_FIELD(search_node_count);
    PRINT_FIELD_OPTIONAL(search_node_hashes, search_node_hashes->size());
    PRINT_FIELD(max_search_depth);

    PRINT_FIELD_OPTIONAL(initial_subgame_count, *initial_subgame_count);
    PRINT_FIELD(max_subgame_count);

    ostr << std::endl;

    //ostr << "\nSearch statistics:\nnode_count " << node_count
    //     << "\ntt_hits " << tt_hits
    //     << "\ntt_misses " << tt_misses
    //     << "\ndb_hits " << db_hits
    //     << "\ndb_misses " << db_misses
    //     << "\nsearch_depth " << search_depth
    //     << "\nn_subgames " << n_subgames
    //     // << "\nsum_hashes " << sum_hashes.size() TODO if has_value()...
    //     << std::endl;
}

#undef PRINT_FIELD
#undef PRINT_FIELD_OPTIONAL

std::optional<double> solver_stats::get_tt_hit_rate() const
{
    const uint64_t total = tt_hits + tt_misses;

    if (total == 0)
        return {};

    return (double) tt_hits / (double) total;
}

std::optional<double> solver_stats::get_db_hit_rate() const
{
    const uint64_t total = db_hits + db_misses;

    if (total == 0)
        return {};

    return (double) db_hits / (double) total;
}

////////////////////////////////////////////////// Stats/reporting functions
namespace stats {
namespace {
std::optional<global_hash> hash_helper;

hash_t get_node_hash(const std::vector<game*>& games, ebw to_play)
{
assert(hash_helper.has_value());
return hash_helper->get_global_hash_value(games, to_play);
}

hash_t get_node_hash(const game* g, ebw to_play)
{
assert(hash_helper.has_value());
return hash_helper->get_global_hash_value(g, to_play);
}

} // namespace 

global_hash& get_global_hash_helper()
{
    assert(hash_helper.has_value());
    return hash_helper.value();
}

void __report_search_node_initial(size_t initial_subgame_count)
{
    solver_stats& stats = __global_stats;

    assert(!stats.has_initial_values);
    stats.has_initial_values = true;

    stats.initial_subgame_count = initial_subgame_count;
}

void __count_search_node_hash(const sumgame& sum, ebw to_play)
{
    solver_stats& stats = __global_stats;
    assert(stats.search_node_hashes.has_value() && global::count_sums());

    const hash_t node_hash = get_node_hash(sum.subgames(), to_play);
    stats.search_node_hashes->insert(node_hash);
}

void __count_search_node_hash(const std::vector<game*>& games, ebw to_play)
{
    solver_stats& stats = __global_stats;
    assert(stats.search_node_hashes.has_value() && global::count_sums());

    const hash_t node_hash = get_node_hash(games, to_play);
    stats.search_node_hashes->insert(node_hash);
}

void __count_search_node_hash(const game* g, ebw to_play)
{
    solver_stats& stats = __global_stats;
    assert(stats.search_node_hashes.has_value() && global::count_sums());

    const hash_t node_hash = get_node_hash(g, to_play);
    stats.search_node_hashes->insert(node_hash);
}

void __count_search_node_hash(hash_t node_hash)
{
    solver_stats& stats = __global_stats;
    assert(stats.search_node_hashes.has_value() && global::count_sums());

    stats.search_node_hashes->insert(node_hash);
}

} // namespace stats

////////////////////////////////////////////////// Init global hash
namespace mcgs_init {
void init_solver_stats()
{
assert(!stats::hash_helper.has_value());
stats::hash_helper.emplace();
}
} // namespace mcgs_init
