#include "sbhsolver_test.h"
#include "gridlike_db_game_generator.h"
#include "nogo.h"
#include "sumgame.h"
#include <memory>
#include <vector>

using namespace std;

void nogo_sbhsolver_test()
{
    gridlike_db_game_generator<nogo> gen(3, 3);
    assert(gen && gen.get_shape() == int_pair(0, 0));
    ++gen;
    assert(gen && gen.get_shape() != int_pair(0, 0));

    sumgame s(BLACK);

    while (gen)
    {
        // Generate game
        unique_ptr<game> g(gen.gen_game());
        int_pair shape = gen.get_shape();

        ++gen;
        nogo* g_nogo = dynamic_cast<nogo*>(g.get());
        assert(g_nogo != nullptr);

        //const vector<int>& immortal = g_nogo->immortal();
        //cout << immortal.size() << endl;
        //assert(immortal.empty());

        // Check for split
        split_result sr = g->split();
        if (!(sr.has_value() && sr->size() > 1))
            continue;

        for (game* sg : *sr)
            delete sg;

        // Get outcomes
        assert(s.num_total_games() == 0);
        s.add(g.get());

        s.set_to_play(BLACK);
        bool black_win = s.solve();

        s.set_to_play(WHITE);
        bool white_win = s.solve();

        cout << shape << " ";
        cout << *g << " ";
        cout << "B:" << black_win << " ";
        cout << "W:" << white_win << endl;


        s.pop(g.get());
        assert(s.num_total_games() == 0);
    }

}
