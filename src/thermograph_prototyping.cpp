#include "thermograph_prototyping.h"

#include <iostream>
#include <vector>
#include <memory>

#include "SgBWArray.h"
#include "SgBlackWhite.h"
#include "ThGraph.h"
#include "cgt_basics.h"
#include "clobber_1xn.h"
#include "game.h"
#include "global_database.h"
#include "global_options.h"
#include "grid_generator.h"
#include "gridlike_db_game_generator.h"
#include "all_game_headers.h"

//#define DO_THERMOGRAPH_CHECK

using namespace std;


////////////////////////////////////////////////// helpers
namespace {

//////////////////////////////////////// declarations
ThGraph* make_graph(game* g);
vector<ThGraph*> make_option_graphs_for(game* g, bw player);

//////////////////////////////////////// implementations
vector<ThGraph*> make_option_graphs_for(game* g, bw player)
{
    assert(is_black_white(player));
    vector<ThGraph*> graphs;

    unique_ptr<move_generator> gen(g->create_move_generator(player));

    while (*gen)
    {
        const ::move m = gen->gen_move();
        ++(*gen);

        g->play(m, player);
        graphs.push_back(make_graph(g));
        g->undo_move();
    }

    return graphs;
}

ThGraph* make_graph(game* g)
{
    SgBWArray<vector<ThGraph*>> option_graphs;
    vector<ThGraph*>& options_b = option_graphs[SG_BLACK];
    vector<ThGraph*>& options_w = option_graphs[SG_WHITE];

    options_b = make_option_graphs_for(g, BLACK);
    options_w = make_option_graphs_for(g, WHITE);

    ThGraph* graph = ThGraph::MakeGraphFromOptions(option_graphs);
    graph->Check();

    for (ThGraph* option : options_b)
        delete option;
    for (ThGraph* option : options_w)
        delete option;

    return graph;
}

} // namespace

//////////////////////////////////////////////////
void thermograph_prototyping()
{
    if (!global::use_db())
        return;

#ifdef DO_THERMOGRAPH_CHECK
    database& db = get_global_database();

    grid_generator gg({1, 6}, {EMPTY, BLACK, WHITE}, true);

    while (gg)
    {
        clobber_1xn g(gg.gen_board());
        ++gg;

        cout << g << " ... " << flush;

        optional<db_entry_partisan> entry = db.get_partisan(g);
        if (!entry.has_value())
        {
            cout << "NOT PRESENT" << endl;
            continue;
        }

        ThGraph* graph = make_graph(&g);

        const bool same = *graph == entry->thermograph;

        if (same)
            cout << "OK" << endl;
        else
            cout << "ERROR" << endl;

        assert(same);
        delete graph;
    }
#else
    cout << "Not checking DB entry thermographs: "
            "DO_THERMOGRAPH_CHECK not defined..."
         << endl;

#endif
}

