#include "db_make_thermograph.h"
#include "SgBlackWhite.h"
#include "ThGraph.h"
#include "cgt_basics.h"
#include "database.h"
#include "throw_assert.h"
#include "sumgame.h"
#include "transposition.h"
#include <optional>

#ifdef MCGS_USE_THERM

using namespace std;

struct ttable_therm_entry
{
    ThGraph thermograph;
};

typedef ttable<ttable_therm_entry> ttable_therm;

////////////////////////////////////////////////// helpers

namespace {
optional<ttable_therm> tt_opt;

inline ttable_therm& get_tt()
{
    if (tt_opt.has_value()) [[likely]]
        return *tt_opt;

    tt_opt.emplace(2, 0);
    return *tt_opt;
}

inline hash_t get_sum_hash(const sumgame& sum)
{
    return sum.get_global_hash_for_player(EMPTY);
}

optional<ThGraph*> load_from_local_cache(sumgame& sum)
{
    const hash_t hash = get_sum_hash(sum);

    ttable_therm& tt = get_tt();
    ttable_therm::search_result result = tt.search(hash);

    if (!result.entry_valid())
        return {};

    const ttable_therm_entry& entry = result.get_entry();
    return new ThGraph(entry.thermograph);
}

void store_to_local_cache(sumgame& sum, const ThGraph* graph)
{
    const hash_t hash = get_sum_hash(sum);

    ttable_therm& tt = get_tt();
    ttable_therm::search_result result = tt.search(hash);

    assert(!result.entry_valid());
    result.init_entry();

    ttable_therm_entry& entry = result.get_entry();
    entry.thermograph = *graph;
}

optional<ThGraph*> load_from_db(database& db, const sumgame& sum)
{
    optional<db_entry_partisan> entry = db.get_partisan(sum);

    if (entry.has_value())
        return new ThGraph(entry->thermograph);

    return {};
}

vector<ThGraph*> get_option_graphs_for(database& db, sumgame& sum, bw player)
{
    assert(is_black_white(player));

    vector<ThGraph*> option_graphs;

    assert_restore_sumgame ars1(sum);
    const bw restore_player = sum.to_play();

    sum.set_to_play(player);
    unique_ptr<sumgame_move_generator> gen(sum.create_sum_move_generator(player));

    while (*gen)
    {
        const sumgame_move sm = gen->gen_sum_move();
        ++(*gen);

        sum.play_sum(sm, player);
        ThGraph* option_graph = db_make_thermograph(db, sum);
        sum.undo_move();

        option_graph->Check();
        option_graphs.push_back(option_graph);
    }

    sum.set_to_play(restore_player);
    return option_graphs;
}


} // namespace

//////////////////////////////////////////////////
ThGraph* db_make_thermograph(database& db, sumgame& sum)
{
    // Check database
    {
        optional<ThGraph*> graph_opt = load_from_db(db, sum);
        if (graph_opt.has_value())
            return *graph_opt;
    }

    // Check local cache
    {
        optional<ThGraph*> graph_opt = load_from_local_cache(sum);
        if (graph_opt.has_value())
            return *graph_opt;
    }

    // Generate options
    SgBWArray<vector<ThGraph*>> option_graphs;
    vector<ThGraph*>& option_graphs_b = option_graphs[SG_BLACK];
    vector<ThGraph*>& option_graphs_w = option_graphs[SG_WHITE];

    option_graphs_b = get_option_graphs_for(db, sum, BLACK);
    option_graphs_w = get_option_graphs_for(db, sum, WHITE);

    ThGraph* graph = ThGraph::MakeGraphFromOptions(option_graphs);
    graph->Check();

    store_to_local_cache(sum, graph);

    for (ThGraph* option : option_graphs_b)
        delete option;
    for (ThGraph* option : option_graphs_w)
        delete option;

    return graph;
}
#else
ThGraph* db_make_thermograph(database& db, sumgame& sum)
{
    THROW_ASSERT(false);
}
#endif
