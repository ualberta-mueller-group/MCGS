//---------------------------------------------------------------------------
// Implementation of impartial_game
//---------------------------------------------------------------------------
#include "impartial_game.h"

#include <cassert>
#include <memory>
#include <set>

#include "cgt_nimber.h"
#include "global_options.h"
#include "hashing.h"
#include "impartial_lemoine_viennot.h"
#include "solver_stats.h"
#include "transposition.h"

//---------------------------------------------------------------------------

namespace {

inline int search(game* subgame, impartial_tt& tt, const bool& over_time)
{
    const impartial_game* g = static_cast<const impartial_game*>(subgame);
    assert(g == dynamic_cast<const impartial_game*>(subgame));
    return g->search_impartial_game_cancellable(tt, over_time);
}

inline void tt_store(impartial_tt& tt, impartial_game* g, int nim_value)
{
    const hash_t hash = g->get_local_hash();
    auto tt_result = tt.search(hash);
    tt_result.set_entry(impartial_ttable_entry(nim_value));
}

inline bool tt_lookup(impartial_tt& tt, impartial_game* g, int& nim_value)
{
    const hash_t hash = g->get_local_hash();
    auto tt_result = tt.search(hash);
    const bool is_valid = tt_result.entry_valid();
    if (is_valid)
        nim_value = tt_result.get_entry().nim_value;
    return is_valid;
}
} // namespace

//---------------------------------------------------------------------------

void impartial_game::set_solved(int nim_value)
{
    assert(!is_solved());
    assert(num_moves_played() == 0);
    _root_is_solved = true;
    _nim_value = nim_value;
}

int impartial_game::search_with_tt(int tt_size) const
{
    if (global::alt_imp_search.get())
    {
        return lemoine_viennot::search_with_tt(*this);
    }
    else
    {
        impartial_tt tt(tt_size, 0);
        return search_impartial_game(tt);
    }
}

int impartial_game::search_impartial_game(impartial_tt& tt) const
{
    const int result = search_impartial_game_cancellable(tt, false);
    assert(result >= 0);
    return result;
}

int impartial_game::search_impartial_game_cancellable(
    impartial_tt& tt, const bool& over_time) const
{
    if (over_time)
        return -1;

    // TODO increment before or after is_solved()?
    stats::inc_node_count();

    if (is_solved())
        return nim_value();

    assert_restore_game ar(*this);
    auto g = const_cast<impartial_game*>(this);
    int v;
    if (tt_lookup(tt, g, v))
    {
        if (g->num_moves_played() == 0)
            g->set_solved(v);
        return v;
    }

    std::unique_ptr<move_generator> mgp(g->create_move_generator());
    move_generator& mg = *mgp;

    // iterate over moves and solve after each move
    // compute mex
    std::set<int> nimbers;
    for (; mg; ++mg)
    {
        if (over_time)
            return -1;

        assert_restore_game arm(*this);
        move m = mg.gen_move();
        g->play(m);
        int move_nimber = 0;
        split_result sr = g->split();

        if (sr) // split found a sum
        {
            // search new games in sr, nim-add them
            for (game* subgame : *sr)
            {
                // No need for subgame->undo_normalize() -- it will be deleted
                subgame->normalize();
                int result = search(subgame, tt, over_time);

                if (over_time)
                    break;
                assert(result >= 0);

                nimber::add_nimber(move_nimber, result);
            }

            for (game* subgame : *sr)
                delete subgame;

            if (over_time)
            {
                // g was not normalized, don't call undo_normalize()
                g->undo_move();
                return -1;
            }
        }
        else // no split, keep searching same subgame
        {
            g->normalize();
            move_nimber = g->search_impartial_game_cancellable(tt, over_time);
            g->undo_normalize();

            if (over_time)
            {
                g->undo_move();
                return -1;
            }

            assert(move_nimber >= 0);
        }

        assert(move_nimber >= 0);
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
        assert(*it >= 0);

        if (*it != i)
            return i;
        ++i;
    }
    return i;
}
