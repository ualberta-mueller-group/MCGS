#include "db_make_thermograph.h"

#include <memory>

#include "SgBlackWhite.h"
#include "ThGraph.h"

#include "cgt_basics.h"
#include "database.h"
#include "sumgame.h"

// Should be 0 or 1
unsigned int max_thermograph_generation_depth = 0;

using namespace std;

#ifdef MCGS_USE_THERM

////////////////////////////////////////////////// helpers
namespace {
shared_ptr<ThGraph> get_thermograph_from_db(database& db, sumgame& sum,
                                                   unsigned int depth,
                                                   bool silent)
{
    const db_entry_partisan* entry = db.get_partisan_ptr(sum);

    if (entry == nullptr)
    {
        db.generate_entry_single_partisan(sum, depth + 1, silent);
        entry = db.get_partisan_ptr(sum);
    }

    THROW_ASSERT(entry != nullptr);
    THROW_ASSERT(entry->thermograph);

    return entry->thermograph;
}

vector<shared_ptr<ThGraph>> get_option_graphs_for(database& db, sumgame& sum,
                                                  bw player, unsigned int depth, bool silent)
{
    assert_restore_sumgame ars(sum);
    assert(is_black_white(player));

    const bw restore_player = sum.to_play();

    vector<shared_ptr<ThGraph>> option_graphs;

    sum.set_to_play(player);
    unique_ptr<sumgame_move_generator> gen(
        sum.create_sum_move_generator(player));

    const int n_active_before = sum.num_active_games();
    optional<sumgame> sum2_opt;

    while (*gen)
    {
        const sumgame_move sm = gen->gen_sum_move();
        ++(*gen);

        assert(sum.to_play() == player);
        sum.play_sum(sm, player);

        if (n_active_before == 1 && sum.num_active_games() >= 2)
        {
            const int n_games = sum.num_total_games();
            if (!sum2_opt.has_value())
                sum2_opt.emplace(BLACK);

            sumgame& sum2 = sum2_opt.value();

            for (int i = 0; i < n_games; i++)
            {
                game* gi = sum.subgame(i);
                if (!gi->is_active())
                    continue;

                assert(sum2.num_total_games() == 0);
                sum2.add(gi);
                db.generate_entry_single_partisan(sum2, depth + 1, silent);
                sum2.pop(gi);
            }
        }

        shared_ptr<ThGraph> option_graph =
            get_thermograph_from_db(db, sum, depth, silent);

        sum.undo_move();

        option_graph->Check();
        option_graphs.push_back(option_graph);
    }

    sum.set_to_play(restore_player);
    return option_graphs;
}

shared_ptr<ThGraph> db_make_thermograph_impl(database& db, sumgame& sum,
                                             unsigned int depth, bool silent)
{
    max_thermograph_generation_depth =
        max(max_thermograph_generation_depth, depth);

    // Generate options
    SgBWArray<vector<ThGraph*>> option_graphs_raw;
    vector<ThGraph*>& option_graphs_raw_b = option_graphs_raw[SG_BLACK];
    vector<ThGraph*>& option_graphs_raw_w = option_graphs_raw[SG_WHITE];

    vector<shared_ptr<ThGraph>> option_graphs_b =
        get_option_graphs_for(db, sum, BLACK, depth, silent);
    vector<shared_ptr<ThGraph>> option_graphs_w =
        get_option_graphs_for(db, sum, WHITE, depth, silent);

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
#endif

} // namespace

//////////////////////////////////////////////////
std::shared_ptr<ThGraph> db_make_thermograph(database& db, sumgame& sum,
                                             unsigned int depth, bool silent)
{
#ifdef MCGS_USE_THERM
    return db_make_thermograph_impl(db, sum, depth, silent);
#else
    assert(false);
#endif
}
