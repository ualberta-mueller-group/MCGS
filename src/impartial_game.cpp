//---------------------------------------------------------------------------
// Implementation of impartial_game
//---------------------------------------------------------------------------
#include "impartial_game.h"

#include "cgt_nimber.h"
#include "hashing.h"
#include "transposition.h"
#include <memory>
#include <cassert>
#include <set>

//---------------------------------------------------------------------------

namespace{

inline int search(game* subgame, impartial_tt& tt)
{
    const impartial_game* g =
        static_cast<const impartial_game*>(subgame);
    return g->search_impartial_game(tt);
}

inline void tt_store(impartial_tt& tt, impartial_game* g, 
                     int nim_value)
{
    const hash_t hash = g->get_local_hash();
    auto tt_it = tt.get_iterator(hash);
    tt_it.set_entry(impartial_ttable_entry(nim_value));
}

inline bool tt_lookup(impartial_tt& tt, impartial_game* g, 
                     int& nim_value)
{
    const hash_t hash = g->get_local_hash();
    auto tt_it = tt.get_iterator(hash);
    const bool is_valid = tt_it.entry_valid();
    if (is_valid)
        nim_value = tt_it.get_entry().nim_value;
    return is_valid;
}
} // namespace
//---------------------------------------------------------------------------

void impartial_game::set_solved(int nim_value)
{
    assert(! is_solved());
    assert(num_moves_played() == 0);
    _root_is_solved = true;
    _nim_value = nim_value;
}

int impartial_game::search_with_tt(int tt_size) const
{
    impartial_tt tt(tt_size, 0);
    return search_impartial_game(tt);
}

int impartial_game::search_impartial_game(impartial_tt& tt) const
{
    if (is_solved())
        return nim_value();

    assert_restore_game ar(*this);
    auto g = const_cast<impartial_game*>(this);
    int v;
    if (tt_lookup(tt, g, v))
        return v;

    std::unique_ptr<move_generator> mgp(g->create_move_generator());
    move_generator& mg = *mgp;

    // iterate over moves and solve after each move
    // compute mex
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
                int result = search(subgame, tt);
                nimber::add_nimber(move_nimber, result);
                delete subgame;
            }
        }
        else // no split, keep searching same subgame
        {
            move_nimber = g->search_impartial_game(tt);
        }

        nimbers.insert(move_nimber);
        g->undo_move();
    }
    int result = mex(nimbers);
    if (g->num_moves_played() == 0)
        g->set_solved(result);
    tt_store(tt, g, result);
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

