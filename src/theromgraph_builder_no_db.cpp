#include "solver_stats.h"
#include "thermograph_builder_no_db.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cassert>

#include "SgBlackWhite.h"
#include "ThGraph.h"
#include "game.h"
#include "SgBWArray.h"

#include "cgt_basics.h"
#include "sumgame.h"
#include "database.h"
#include "timeout_token.h"
#include "utilities.h"

using namespace std;

static thermograph_builder_no_db global_instance;

shared_ptr<const ThGraph> thermograph_builder_no_db::build_thermograph(
    sumgame& sum, const database* db_nullable)
{
    shared_ptr<const ThGraph> graph =
        build_thermograph_with_timeout(sum, 0, db_nullable);

    assert(graph.get() != nullptr);
    return graph;
}

shared_ptr<const ThGraph> thermograph_builder_no_db::
    build_thermograph_with_timeout(sumgame& sum, unsigned long long timeout,
                                   const database* db_nullable)
{
    timeout_source src;
    src.start_timeout(timeout);

    timeout_token timeout_tok = src.get_timeout_token();

    shared_ptr<const ThGraph> graph = _get_thermograph_from_cache(
        sum, timeout_tok, db_nullable, INITIAL_SEARCH_DEPTH);

    assert(
        LOGICAL_IMPLIES(!timeout_tok.stop_requested(), graph.get() != nullptr));

    src.cancel_timeout();
    return graph;
}

void thermograph_builder_no_db::clear()
{
    _therm_cache.clear();
}

thermograph_builder_no_db& thermograph_builder_no_db::get_global_instance()
{
    return global_instance;
}

shared_ptr<ThGraph> thermograph_builder_no_db::_build_thermograph_from_options(
    sumgame& sum, const timeout_token& timeout_tok, const database* db_nullable,
    uint64_t depth)
{
    if (timeout_tok.stop_requested())
        return nullptr;

    // Generate options
    SgBWArray<vector<ThGraph*>> option_graphs_raw;
    vector<ThGraph*>& option_graphs_raw_b = option_graphs_raw[SG_BLACK];
    vector<ThGraph*>& option_graphs_raw_w = option_graphs_raw[SG_WHITE];

    vector<shared_ptr<ThGraph>> option_graphs_b = _get_option_graphs_for_player(
        sum, BLACK, timeout_tok, db_nullable, depth);
    vector<shared_ptr<ThGraph>> option_graphs_w = _get_option_graphs_for_player(
        sum, WHITE, timeout_tok, db_nullable, depth);

    if (timeout_tok.stop_requested())
        return nullptr;

    for (const shared_ptr<ThGraph>& option_shared : option_graphs_b)
    {
        ThGraph* option = option_shared.get();
        assert(option != nullptr);
        option_graphs_raw_b.push_back(option);
    }

    for (const shared_ptr<ThGraph>& option_shared : option_graphs_w)
    {
        ThGraph* option = option_shared.get();
        assert(option != nullptr);
        option_graphs_raw_w.push_back(option);
    }

    shared_ptr<ThGraph> graph(ThGraph::MakeGraphFromOptions(option_graphs_raw));
    graph->Check();

    return graph;
}

shared_ptr<ThGraph> thermograph_builder_no_db::_get_thermograph_from_cache(
    sumgame& sum, const timeout_token& timeout_tok, const database* db_nullable,
    uint64_t depth)
{
    if (timeout_tok.stop_requested())
        return nullptr;

    stats::report_search_node(sum, EMPTY, depth);

    // Check non-DB cache. (It seems faster to check here first -- the database
    // is unchanging from our context)
    const hash_t hash = sum.get_global_hash_for_player(EMPTY);
    auto it = _therm_cache.find(hash);

    const bool tt_hit = (it != _therm_cache.end());
    stats::report_tt_access(tt_hit);

    if (tt_hit)
        return it->second;

    // Check database
    if (db_nullable != nullptr)
    {
        const db_entry_partisan* entry = db_nullable->get_partisan_ptr(sum);

        const bool db_hit = entry != nullptr && entry->thermograph;
        stats::report_db_access(db_hit);

        if (db_hit)
            return entry->thermograph;
    }

    // Not found; build from options
    shared_ptr<ThGraph> graph =
        _build_thermograph_from_options(sum, timeout_tok, db_nullable, depth + 1);

    if (timeout_tok.stop_requested())
        return nullptr;

    assert(graph.get() != nullptr);
    _therm_cache[hash] = graph;

    return graph;
}

vector<shared_ptr<ThGraph>> thermograph_builder_no_db::
    _get_option_graphs_for_player(sumgame& sum, bw player,
                                  const timeout_token& timeout_tok,
                                  const database* db_nullable, uint64_t depth)
{
    vector<shared_ptr<ThGraph>> option_graphs;

    if (timeout_tok.stop_requested())
        return option_graphs;

    assert_restore_sumgame ars(sum);
    assert(is_black_white(player));

    const bw restore_player = sum.to_play();

    sum.set_to_play(player);
    unique_ptr<sumgame_move_generator> gen(
        sum.create_sum_move_generator(player));

    while (*gen)
    {
        if (timeout_tok.stop_requested())
            break;

        const sumgame_move sm = gen->gen_sum_move();
        ++(*gen);

        assert(sum.to_play() == player);
        sum.play_sum(sm, player);

        shared_ptr<ThGraph> option_graph =
            _get_thermograph_from_cache(sum, timeout_tok, db_nullable, depth);

        sum.undo_move();

        if (option_graph)
        {
            option_graph->Check();
            option_graphs.push_back(option_graph);
        }
        else
            assert(timeout_tok.stop_requested());
    }

    sum.set_to_play(restore_player);
    return option_graphs;
}
