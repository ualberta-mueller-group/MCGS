#include "db_make_thermograph.h"
#include "SgBlackWhite.h"
#include "ThGraph.h"
#include "cgt_basics.h"
#include "database.h"
#include "throw_assert.h"
#include "sumgame.h"
#include "transposition.h"
#include <optional>
#include <memory> 

unsigned int max_thermograph_generation_depth = 0;

using namespace std;

#ifdef MCGS_USE_THERM



////////////////////////////////////////////////// helpers

namespace {
shared_ptr<ThGraph> load_from_db(database& db, const sumgame& sum)
{
    optional<db_entry_partisan> entry = db.get_partisan(sum);

    if (entry.has_value())
        return entry->thermograph;

    return {};
}

vector<shared_ptr<ThGraph>> get_option_graphs_for(database& db, sumgame& sum, bw player, unsigned int depth)
{
    assert(is_black_white(player));

    vector<shared_ptr<ThGraph>> option_graphs;

    assert_restore_sumgame ars1(sum);
    const bw restore_player = sum.to_play();

    sum.set_to_play(player);
    unique_ptr<sumgame_move_generator> gen(sum.create_sum_move_generator(player));

    while (*gen)
    {
        const sumgame_move sm = gen->gen_sum_move();
        ++(*gen);

        assert(sum.to_play() == player);
        sum.play_sum(sm, player);
        shared_ptr<ThGraph> option_graph = db_make_thermograph(db, sum, depth + 1);
        sum.undo_move();

        option_graph->Check();
        option_graphs.push_back(option_graph);
    }

    sum.set_to_play(restore_player);
    return option_graphs;
}


} // namespace

//////////////////////////////////////////////////
shared_ptr<ThGraph> db_make_thermograph(database& db, sumgame& sum, unsigned int depth)
{
    max_thermograph_generation_depth = max(max_thermograph_generation_depth, depth);

    // Check database
    {
        shared_ptr<ThGraph> graph_opt = load_from_db(db, sum);
        if (graph_opt)
            return graph_opt;
    }

    // Generate options
    SgBWArray<vector<ThGraph*>> option_graphs_raw;
    vector<ThGraph*>& option_graphs_raw_b = option_graphs_raw[SG_BLACK];
    vector<ThGraph*>& option_graphs_raw_w = option_graphs_raw[SG_WHITE];

    vector<shared_ptr<ThGraph>> option_graphs_b;
    vector<shared_ptr<ThGraph>> option_graphs_w;

    option_graphs_b = get_option_graphs_for(db, sum, BLACK, depth);
    option_graphs_w = get_option_graphs_for(db, sum, WHITE, depth);

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
#else
shared_ptr<ThGraph> db_make_thermograph(database& db, sumgame& sum, unsigned int depth)
{
    THROW_ASSERT(false);
}
#endif
