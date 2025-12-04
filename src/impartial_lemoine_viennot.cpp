//---------------------------------------------------------------------------
// Implementation of Lemoine - Viennot algorithm for impartial games
//---------------------------------------------------------------------------
#include "impartial_lemoine_viennot.h"

#include <vector>
#include <optional>
#include "cgt_nimber.h"
#include "hashing.h"
#include "impartial_game.h"
#include "transposition.h"

namespace {
    // TODO implement some subgame complexity heuristic?
    game* find_hardest(const std::vector<game*>& subgames)
    {
        return subgames.back();
    }

} // namespace

namespace lemoine_viennot{

std::vector<hash_t> nimber_hashcode::_codes;

void nimber_hashcode::init_codes(int max_nimber)
{
    _codes.reserve(max_nimber + 1);
    global_hash hash;
    for (int i = 0; i <= max_nimber; ++i)
    {
        const nimber g(i);
        hash.reset();
        hash.set_to_play(BLACK); // TODO why needed?
        hash.add_subgame(0, &g);
        _codes.push_back(hash.get_value());
    }
}
//---------------------------------------------------------------------------

inline hash_t combined_hash(const impartial_game* g, int nim_value)
{
    return g->get_local_hash() ^ nimber_hashcode::get(nim_value);
}

inline void tt_store(lv_bool_tt& tt,
                     const impartial_game* g,
                     int nim_value,
                     bool result)
{
    const hash_t hash = combined_hash(g, nim_value);
    auto tt_result = tt.search(hash);
    tt_result.set_entry(lv_bool_entry(result));
}

inline bool tt_lookup(lv_bool_tt& tt,
                      const impartial_game* g,
                      int nim_value, 
                      bool& result)
{
    const hash_t hash = combined_hash(g, nim_value);
    auto tt_result = tt.search(hash);
    const bool is_valid = tt_result.entry_valid();
    if (is_valid)
    {
        result = tt_result.get_entry().value;
    }
    return is_valid;
}

int search_with_tt(const impartial_game& g, int tt_size)
{
    lv_bool_tt tt(tt_size, 0);
    return search_impartial_game(g, tt);
}

// Compute n such that g = *n. "Algorithm 3" in Lemoine and Viennot.
int search_impartial_game(const impartial_game& g, lv_bool_tt& tt)
{
    int n = 0;
    for ( ; ; ++n)
        if (! search_g_plus_nimber(g, n, tt))
            break;
    return n;
}

// check in tt if g = *i for i<n. In that case, no search is needed
// to prove that g + *n  = *i + *n != *0 is a win
bool pre_search_probe(const impartial_game& g, int n, lv_bool_tt& tt)
{
    for (int i = 0; i < n; ++i)
    {
        bool result;
        if (tt_lookup(tt, &g, i, result) && !result)
        {
            tt_store(tt, &g, n, true);
            return true;
        }
    }
    return false;
}

// Boolean solver for g + *n. "Algorithm 1" in Lemoine and Viennot
bool search_g_plus_nimber(const impartial_game& g, int n,
                          lv_bool_tt& tt)
{
    bool result;
    if (tt_lookup(tt, &g, n, result))
        return result;
    if (pre_search_probe(g, n, tt))
        return true;

    // Part A: search all position options Gi + *n 
    // If any option is a loss, then G + *n is a win
    std::unique_ptr<move_generator> mgp(g.create_move_generator());
    for (move_generator& mg = *mgp; mg; ++mg)
    {
        assert_restore_game arm(g);
        auto g_nonconst = const_cast<impartial_game*>(&g);
        move m = mg.gen_move();
        g_nonconst->play(m);
        split_result sr = g_nonconst->split();
        if (sr) // split found a sum
        {
            const bool move_result = search_sum_plus_nimber(sr, n, tt);
            for (game* subgame : *sr)
               delete subgame;

            // g_nonconst was not normalized, don't call undo_normalize()

            if (! move_result)
            {
                g_nonconst->undo_move();
                tt_store(tt, g_nonconst, n, true);
                return true;
            }
        }
        else // no split, solve same subgame
        {
            g_nonconst->normalize();
            const bool move_result = search_g_plus_nimber(*g_nonconst, n, tt);
            g_nonconst->undo_normalize();
            if (! move_result)
            {
               g_nonconst->undo_move();
               tt_store(tt, g_nonconst, n, true);
               return true;
            }
        }
        g_nonconst->undo_move();
    }

    // Part B: search all nimber options G + *i, i<n.
    for(int i = 0; i < n; ++i)
    {
        const bool move_result = search_g_plus_nimber(g, i, tt);
        if (! move_result)
        {
            tt_store(tt, &g, n, true);
            return true;
        }
    }

    // Final result when g + *n is a loss - store and return
    tt_store(tt, &g, n, false);
    return false;
}

// Helper function that casts game to impartial, then solves.
inline bool search_game_nimber(game *g, int nimber, lv_bool_tt& tt)
{
    const impartial_game* gi =
       static_cast<const impartial_game*>(g);
    return search_g_plus_nimber(*gi, nimber, tt);
    
}

// Boolean solver for sum(g_i) + *n. "Algorithm 2" in Lemoine and Viennot
bool search_sum_plus_nimber(const split_result& subgames, int n,
                            lv_bool_tt& tt)
{
    assert(subgames);    
    if (subgames->size() == 0)
    {
        return n != 0;
    }
    else if (subgames->size() == 1)
    {
        game* subgame = subgames->front();
        return search_game_nimber(subgame, n, tt);
    }
    
    game* hardest = find_hardest(*subgames);
    int nim_sum = n;
    for (game* subgame : *subgames)
    {
        if (subgame != hardest)
        {
            const impartial_game* g = 
               static_cast<const impartial_game*>(subgame);
        // TODO? g->normalize();
            const int subgame_nimber = search_impartial_game(*g, tt);
            nimber::add_nimber(nim_sum, subgame_nimber);
        }
    }
    return search_game_nimber(hardest, nim_sum, tt);
}

} // namespace lemoine_viennot
