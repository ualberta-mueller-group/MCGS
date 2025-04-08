//---------------------------------------------------------------------------
// Implementation of impartial_game
//---------------------------------------------------------------------------
#include "impartial_game.h"

#include <iostream>
#include <string>
#include <sstream>
#include "cgt_basics.h"
#include "cgt_nimber.h"
//---------------------------------------------------------------------------

namespace{
inline int search(game* subgame)
{
    const impartial_game* g =
        static_cast<const impartial_game*>(subgame);
    return g->search_impartial_game();
}
} // namespace
//---------------------------------------------------------------------------

void impartial_game::set_solved(int nimber)
{
    assert(! is_solved());
    assert(num_moves_played() == 0);
    _root_is_solved = true;
    _nimber = nimber;
}

int impartial_game::search_impartial_game() const
{
    if (is_solved())
        return nimber();
        
    assert_restore_game ar(*this);
    // iterate over moves
    // solve resulting games after each move, XOR
    // compute mex
    auto g = const_cast<impartial_game*>(this);
    std::unique_ptr<move_generator> mgp(g->create_move_generator());
    move_generator& mg = *mgp;

    std::set<int> nimbers;
    for (; mg; ++mg)
    {
        assert_restore_game arm(*this);
        move m = mg.gen_move();
        g->play(m);
        int move_nimber = 0;
        split_result sr = g->split();
        if (sr) // split found a sum
        {
            // search new games in sr, nim-add them
            for (game* subgame: *sr)
            {
                int result = search(subgame);
                nimber::add_nimber(move_nimber, result);
                delete subgame;
            }
        }
        else // no split, keep searching same subgame
        {
            move_nimber = g->search_impartial_game();
        }

        nimbers.insert(move_nimber);
        g->undo_move();
    }
    int result = mex(nimbers);
    if (g->num_moves_played() == 0)
        g->set_solved(result);
    return result;
}

int impartial_game::mex(const std::set<int>& nimbers)
{
    // find smallest missing number in sorted set 0, 1, 2, ...
    int i = 0;
    for (auto it = nimbers.begin(); it != nimbers.end(); ++it)
    {
        if (*it != i)
            return i;
        ++i;
    }
    return i;
}

