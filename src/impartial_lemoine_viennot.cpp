//---------------------------------------------------------------------------
// Implementation of Lemoine - Viennot algorithm for impartial games
//---------------------------------------------------------------------------
#include "impartial_lemoine_viennot.h"

#include <algorithm>
#include <memory>
#include <cassert>
#include <optional>
#include <vector>
#include "cgt_nimber.h"
#include "database.h"
#include "global_database.h"
#include "global_options.h"
#include "hashing.h"
#include "impartial_game.h"
#include "solver_stats.h"
#include "timeout_token.h"
#include "transposition.h"

const int NO_DB_RESULT = -1;

namespace {

    int db_lookup(const impartial_game& g)
    {
        if (global::use_db())
        {
            database& db = get_global_database();
            std::optional<db_entry_impartial> entry = db.get_impartial(g);
            stats::report_db_access(entry.has_value());
            if (entry.has_value())
                return entry.value().nim_value;
        }
        return NO_DB_RESULT;
    }

    inline bool compare_complexity_score(const game* a, const game* b)
    {
        return a->complexity_score() < b->complexity_score();
    }
    
    game* find_hardest(const std::vector<game*>& games)
    {
        if (global::use_complexity_score())
        {
            auto hardest = std::max_element(games.begin(), games.end(),
                                            compare_complexity_score);
            return *hardest;
        }
        else
        {
            return games.back();
        }
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
    stats::report_tt_access(is_valid);
    return is_valid;
}

int search_with_tt(const impartial_game& g, int tt_size)
{
    timeout_source src;
    timeout_token timeout_tok = src.get_timeout_token();
    src.start_timeout(0);

    lv_bool_tt tt(tt_size, 0);
    return search_impartial_game(g, tt, timeout_tok);
}

// Compute n such that g = *n. "Algorithm 3" in Lemoine and Viennot.
int search_impartial_game(const impartial_game& g, lv_bool_tt& tt, const timeout_token& timeout_tok)
{
    const int db_result = db_lookup(g);
    if (db_result != NO_DB_RESULT)
        return db_result;

    int n = 0;
    for ( ; ; ++n)
        if (timeout_tok.stop_requested() || ! search_g_plus_nimber(g, n, tt, timeout_tok))
            break;
    return n;
}

// check in tt if g+ *i = loss, so g = *i for any i<n. 
// In that case, no further search is needed
// to prove that g + *n  = *i + *n != *0 is a win
// It is very likely more efficient to store nimbers as well, 
// in a second hash table. Especially for games equal to large nimbers.
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
                          lv_bool_tt& tt, const timeout_token& timeout_tok)
{
    bool result;
    if (tt_lookup(tt, &g, n, result))
        return result;
    const int db_result = db_lookup(g);
    if (db_result != NO_DB_RESULT)
        return db_result != n;
    if (pre_search_probe(g, n, tt))
        return true;
    if (timeout_tok.stop_requested())
        return false; // return value does not matter?

    //stats::inc_node_count();
    stats::report_search_node(&g, EMPTY, 0);
    /*
        TODO: depth?

        TODO: Count node before or after TT/DB lookup? Sumgame solve functions
              count before

        TODO: the node hash inserted into "solver_stats::search_node_hashes"
              from this call is wrong. Possible solution: 
              "report_search_node_verbose()" which forces caller to compute all
              applicable fields (and only applicable fields i.e. no node hash
              if the CLI option for it was not enabled)
    */


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
            const bool move_result = search_sum_plus_nimber(sr, n, tt, timeout_tok);
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
            const bool move_result = search_g_plus_nimber(*g_nonconst, n, tt, timeout_tok);
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
        const bool move_result = search_g_plus_nimber(g, i, tt, timeout_tok);
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
inline bool search_game_nimber(game *g, int nimber, lv_bool_tt& tt, const timeout_token& timeout_tok)
{
    const impartial_game* gi =
       static_cast<const impartial_game*>(g);
    return search_g_plus_nimber(*gi, nimber, tt, timeout_tok);
    
}

// Boolean solver for sum(g_i) + *n. "Algorithm 2" in Lemoine and Viennot
bool search_sum_plus_nimber(const split_result& subgames, int n,
                            lv_bool_tt& tt, const timeout_token& timeout_tok)
{
    assert(subgames);    
    if (subgames->size() == 0)
    {
        return n != 0;
    }
    else if (subgames->size() == 1)
    {
        game* subgame = subgames->front();
        return search_game_nimber(subgame, n, tt, timeout_tok);
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
            const int subgame_nimber = search_impartial_game(*g, tt, timeout_tok);
            if (timeout_tok.stop_requested())
                return false;
            nimber::add_nimber(nim_sum, subgame_nimber);
        }
    }
    return search_game_nimber(hardest, nim_sum, tt, timeout_tok);
}

} // namespace lemoine_viennot
