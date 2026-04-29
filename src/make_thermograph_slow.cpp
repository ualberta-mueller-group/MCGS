#include "make_thermograph_slow.h"

#include <memory>
#include <unordered_map>

#include "SgBlackWhite.h"
#include "ThGraph.h"

#include "cgt_basics.h"
#include "sumgame.h"

using namespace std;

////////////////////////////////////////////////// helpers
namespace {
unordered_map<hash_t, shared_ptr<ThGraph>> therm_cache;

shared_ptr<ThGraph> get_thermograph_from_cache(sumgame& sum)
{
    const hash_t hash = sum.get_global_hash_for_player(EMPTY);
    auto it = therm_cache.find(hash);

    if (it != therm_cache.end())
        return it->second;

    shared_ptr<ThGraph> graph = make_thermograph_slow(sum);
    therm_cache[hash] = graph;

    return graph;
}

vector<shared_ptr<ThGraph>> get_option_graphs_for_player(sumgame& sum,
                                                         bw player)
{
    assert_restore_sumgame ars(sum);
    assert(is_black_white(player));

    const bw restore_player = sum.to_play();

    vector<shared_ptr<ThGraph>> option_graphs;

    sum.set_to_play(player);
    unique_ptr<sumgame_move_generator> gen(
        sum.create_sum_move_generator(player));

    while (*gen)
    {
        const sumgame_move sm = gen->gen_sum_move();
        ++(*gen);

        assert(sum.to_play() == player);
        sum.play_sum(sm, player);

        shared_ptr<ThGraph> option_graph = get_thermograph_from_cache(sum);

        sum.undo_move();

        option_graph->Check();
        option_graphs.push_back(option_graph);
    }

    sum.set_to_play(restore_player);
    return option_graphs;
}

} // namespace

//////////////////////////////////////////////////
shared_ptr<ThGraph> make_thermograph_slow(sumgame& sum)
{
    // Generate options
    SgBWArray<vector<ThGraph*>> option_graphs_raw;
    vector<ThGraph*>& option_graphs_raw_b = option_graphs_raw[SG_BLACK];
    vector<ThGraph*>& option_graphs_raw_w = option_graphs_raw[SG_WHITE];

    vector<shared_ptr<ThGraph>> option_graphs_b =
        get_option_graphs_for_player(sum, BLACK);
    vector<shared_ptr<ThGraph>> option_graphs_w =
        get_option_graphs_for_player(sum, WHITE);

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
