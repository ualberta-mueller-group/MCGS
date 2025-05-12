//---------------------------------------------------------------------------
// Implementation of impartial sumgame search
//---------------------------------------------------------------------------
#include "impartial_sumgame.h"

#include "cgt_nimber.h"
#include "game.h"
#include "impartial_game.h"
#include "sumgame.h"
#include "alternating_move_game.h"

int search_impartial_sumgame(const sumgame& s)
{
    assert_restore_alternating_game ar(s);
    int sum_nim_value = 0;

    impartial_tt tt(24, 0);

    for (game* g : s.subgames())
    {
        if (! g->is_active())
            continue;
        auto ig = static_cast<impartial_game*>(g);
        int result = 0;
        if (ig->is_solved())
            result = ig->nim_value();
        else
        {
            result = ig->search_impartial_game(tt);
            assert(ig->num_moves_played() > 0 || ig->is_solved());
        }
        nimber::add_nimber(sum_nim_value, result);
    }
    return sum_nim_value;
}
