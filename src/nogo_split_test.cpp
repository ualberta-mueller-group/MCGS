#include "sbhsolver_test.h"
#include "gridlike_db_game_generator.h"
#include "nogo.h"
#include "sumgame.h"
#include <memory>
#include <vector>

using namespace std;

void nogo_sbhsolver_test()
{
    sumgame s(BLACK);
    gridlike_db_game_generator<nogo> gen(4, 4);

    uint64_t game_number = 0;
    while (gen)
    {
        unique_ptr<game> g(gen.gen_game());
        ++gen;

        nogo* g_nogo = dynamic_cast<nogo*>(g.get());
        THROW_ASSERT(g_nogo != nullptr);

        split_result sr = g_nogo->split();

        if (!sr.has_value())
            continue;

        cout << "Game number " << game_number << ":" << endl;
        cout << *g << endl << endl;
        game_number++;

        unique_ptr<game> g_inv(g->inverse());

        THROW_ASSERT(s.num_total_games() == 0);

        s.add(g_inv.get());

        for (game* sg : *sr)
            s.add(sg);

        s.set_to_play(BLACK);
        bool b_win = s.solve();

        s.set_to_play(WHITE);
        bool w_win = s.solve();

        THROW_ASSERT(b_win == false);
        THROW_ASSERT(w_win == false);

        while (!sr->empty())
        {
            game* sg = sr->back();
            sr->pop_back();
            s.pop(sg);
            delete sg;
        }
        s.pop(g_inv.get());
    }

    THROW_ASSERT(s.num_total_games() == 0);
}
