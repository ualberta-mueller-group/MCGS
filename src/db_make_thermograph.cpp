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

unsigned int max_thermograph_generation_depth = 0;

////////////////////////////////////////////////// helpers

namespace {
optional<ThGraph*> load_from_db(database& db, const sumgame& sum)
{
    optional<db_entry_partisan> entry = db.get_partisan(sum);

    if (entry.has_value())
        return new ThGraph(entry->thermograph);

    return {};
}

vector<ThGraph*> get_option_graphs_for(database& db, sumgame& sum, bw player, unsigned int depth)
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

        assert(sum.to_play() == player);
        sum.play_sum(sm, player);
        ThGraph* option_graph = db_make_thermograph(db, sum, depth + 1);
        sum.undo_move();

        option_graph->Check();
        option_graphs.push_back(option_graph);
    }

    sum.set_to_play(restore_player);
    return option_graphs;
}


} // namespace

//////////////////////////////////////////////////
ThGraph* db_make_thermograph(database& db, sumgame& sum, unsigned int depth)
{
    max_thermograph_generation_depth = max(max_thermograph_generation_depth, depth);

    // Check database
    {
        optional<ThGraph*> graph_opt = load_from_db(db, sum);
        if (graph_opt.has_value())
            return *graph_opt;
    }

    // Generate options
    SgBWArray<vector<ThGraph*>> option_graphs;
    vector<ThGraph*>& option_graphs_b = option_graphs[SG_BLACK];
    vector<ThGraph*>& option_graphs_w = option_graphs[SG_WHITE];

    option_graphs_b = get_option_graphs_for(db, sum, BLACK, depth);
    option_graphs_w = get_option_graphs_for(db, sum, WHITE, depth);

    ThGraph* graph = ThGraph::MakeGraphFromOptions(option_graphs);
    graph->Check();

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
