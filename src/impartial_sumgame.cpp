//---------------------------------------------------------------------------
// Implementation of impartial sumgame search
//---------------------------------------------------------------------------
#include "impartial_sumgame.h"

#include "cgt_basics.h"
#include "cgt_nimber.h"
#include "game.h"
#include "impartial_game.h"

int search_sumgame(const sumgame& s)
{
    assert_restore_alternating_game ar(s);
    int sum_nim_value = 0;
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
            result = ig->search_impartial_game();
            ig->set_solved(result);
        }
        nimber::add_nimber(sum_nim_value, result);
    }
    return sum_nim_value;
}
