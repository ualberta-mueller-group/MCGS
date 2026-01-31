#include "thermograph_prototyping.h"
#include "SgBWArray.h"
#include "SgBlackWhite.h"
#include "ThGraph.h"
#include "ThPoint.h"
#include "ThScaffold.h"
#include "cgt_basics.h"
#include "serializer.h"
#include "all_game_headers.h"

#include <iostream>

using namespace std;
////////////////////////////////////////////////// serialization
template <>
struct serializer<ThValue>
{
    inline static void save(obuffer& os, const ThValue& value)
    {
        os.write_i32(value.P());
        os.write_i32(value.Q());
    }

    inline static ThValue load(ibuffer& is)
    {
        const int32_t p = is.read_i32();
        const int32_t q = is.read_i32();
        return ThValue(p, q);
    }
};

template <>
struct serializer<ThPoint>
{
    inline static void save(obuffer& os, const ThPoint& point)
    {
        const ThValue& value = point.Value();
        const ThValue& temp = point.Temp();

        serializer<ThValue>::save(os, value);
        serializer<ThValue>::save(os, temp);
    }

    inline static ThPoint load(ibuffer& is)
    {
        ThValue value = serializer<ThValue>::load(is);
        ThValue temp = serializer<ThValue>::load(is);
        return ThPoint(value, temp);
    }
};

template <>
struct serializer<ThScaffold>
{
    inline static void save(obuffer& os, const ThScaffold& sc)
    {
        const int32_t n_points = sc.NuPoints();
        os.write_i32(n_points);

        for (int32_t i = 1; i <= n_points; i++)
        {
            const ThPoint* point = sc.NthPoint(i);
            assert(point != nullptr);

            serializer<ThPoint>::save(os, *point);
        }
    }

    inline static ThScaffold load(ibuffer& is)
    {
        ThScaffold sc;

        const int32_t n_points = is.read_i32();

        for (int32_t i = 1; i <= n_points; i++)
            sc.AppendPoint(serializer<ThPoint>::load(is));

        return sc;
    }
};

template <>
struct serializer<ThGraph>
{
    inline static void save(obuffer& os, const ThGraph& graph)
    {
        const ThScaffold* sc_black = graph.Sc(SG_BLACK);
        const ThScaffold* sc_white = graph.Sc(SG_WHITE);
        assert(sc_black != nullptr && sc_white != nullptr);

        serializer<ThScaffold>::save(os, *sc_black);
        serializer<ThScaffold>::save(os, *sc_white);
    }

    inline static ThGraph load(ibuffer& is)
    {
        ThScaffold sc_black = serializer<ThScaffold>::load(is);
        ThScaffold sc_white = serializer<ThScaffold>::load(is);

        return ThGraph(sc_black, sc_white);
    }
};

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

    for (ThGraph* subgraph : graphs_b)
        delete subgraph;
    for (ThGraph* subgraph : graphs_w)
        delete subgraph;

    graph->Check();
    return graph;
}

void print_thermograph(ThGraph* th)
{
    assert(th != nullptr);

    cout << "MEAN: " << th->MastValue() << " ";
    cout << "TEMP: " << th->Temperature() << " ";

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

    vector<game*> games;
    games.push_back(new domineering("...|..."));
    games.push_back(new amazons("X.O|..."));
    games.push_back(new domineering("...|.#.|..."));

    {
        obuffer os("test.bin");
        os.write_u64(games.size());

        for (game* g : games)
        {
            ThGraph* graph = make_graph(*g);

            cout << *g << endl;
            print_thermograph(graph);
            cout << endl;

            serializer<ThGraph>::save(os, *graph);
            delete graph;
            delete g;
        }
    }

    cout << "========================================" << endl;

    {
        ibuffer is("test.bin");

        const uint64_t n_graphs = is.read_u64();
        for (uint64_t i = 0; i < n_graphs; i++)
        {
            ThGraph graph = serializer<ThGraph>::load(is);

            print_thermograph(&graph);
            cout << endl;
        }

    }




}

