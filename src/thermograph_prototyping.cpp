#include "thermograph_prototyping.h"
#include "SgBWArray.h"
#include "SgBlackWhite.h"
#include "ThGraph.h"
#include "ThScaffold.h"
#include "cgt_basics.h"
#include "domineering.h"

#include <iostream>

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {


//////////////////////////////////////// forward declarations
void make_option_graphs_for(game& g, bw player, vector<ThGraph*>& options);
ThGraph* make_graph(game& g);

//////////////////////////////////////// implementations

void make_option_graphs_for(game& g, bw player, vector<ThGraph*>& options)
{
    assert(is_black_white(player));

    unique_ptr<move_generator> gen(g.create_move_generator(player));

    while (*gen)
    {
        const ::move m = gen->gen_move();
        ++(*gen);

        g.play(m, player);
        ThGraph* graph = make_graph(g);
        graph->Check();
        g.undo_move();

        options.push_back(graph);
    }
}

ThGraph* make_graph(game& g)
{
    SgBWArray<vector<ThGraph*>> graphs;
    vector<ThGraph*>& graphs_b = graphs[SG_BLACK];
    vector<ThGraph*>& graphs_w = graphs[SG_WHITE];

    make_option_graphs_for(g, BLACK, graphs_b);
    make_option_graphs_for(g, WHITE, graphs_w);

    ThGraph* graph = ThGraph::MakeGraphFromOptions(graphs);

    graph->Check();
    return graph;
}

void print_thermograph(ThGraph* th)
{
    assert(th != nullptr);

    const ThScaffold* sc_b = th->Sc(SG_BLACK);
    const ThScaffold* sc_w = th->Sc(SG_WHITE);
    assert(sc_b != nullptr);
    assert(sc_w != nullptr);

    for (ThPointIterator it_b(*sc_b); it_b; ++it_b)
    {
        const ThPoint* point = *it_b;
        assert(point != nullptr);

        cout << '(';
        cout << point->Value();
        cout << ", ";
        cout << point->Temp();
        cout << ") ";
    }

    cout << " | ";

    for (ThPointIterator it_w(*sc_w); it_w; ++it_w)
    {
        const ThPoint* point = *it_w;
        assert(point != nullptr);

        cout << '(';
        cout << point->Value();
        cout << ", ";
        cout << point->Temp();
        cout << ") ";
    }

    cout << endl;
}

} // namespace

//////////////////////////////////////////////////
void thermograph_prototyping()
{
    domineering g("...|...");

    ThGraph* graph = make_graph(g);
    print_thermograph(graph);

    delete graph;
}

