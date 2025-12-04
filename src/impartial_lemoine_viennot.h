//---------------------------------------------------------------------------
// Implementation of the algorithm for solving impartial games
// from Lemoine and Viennot's "Nimbers are inevitable".
//---------------------------------------------------------------------------
#pragma once

#include "hashing.h"
#include "transposition.h"
#include "impartial_game.h"
// See development-notes.md, 
// Impartial Games - the Lemoine - Viennot algorithms

namespace lemoine_viennot{

//---------------------------------------------------------------------------
// Transposition table - boolean for impartial game + nimber
//---------------------------------------------------------------------------
struct lv_bool_entry
{
    bool value;

    lv_bool_entry() : value(false) {}

    lv_bool_entry(bool v) : value(v) {}
};

typedef ttable<lv_bool_entry> lv_bool_tt;
//---------------------------------------------------------------------------
// Transposition table - nimber for impartial game
//---------------------------------------------------------------------------
struct lv_nimber_entry
{
    int nim_value;

    lv_nimber_entry() : nim_value(-1) {}

    lv_nimber_entry(int v) : nim_value(v) {}
};

typedef ttable<lv_nimber_entry> lv_nimber_tt;
//---------------------------------------------------------------------------
// Table of hash codes for nimbers
//---------------------------------------------------------------------------

class nimber_hashcode
{
public:
    static const hash_t& get(int nimber);
    static void init_codes(int max_nimber);
private:
    static std::vector<hash_t> _codes;
};

inline const hash_t& nimber_hashcode::get(int nimber)
{
    assert(nimber >= 0);
    assert(nimber < _codes.size());
    return _codes[nimber];
}

//---------------------------------------------------------------------------
// Search algorithms
//---------------------------------------------------------------------------

// Create a tt with size 2^tt_size, then search_g_plus_nimber
int search_with_tt(const impartial_game& g, int tt_size = 24);

// Boolean solver for g + *n. "Algorithm 1" in Lemoine and Viennot
bool search_g_plus_nimber(const impartial_game& g, int n, 
                          lv_bool_tt& tt);

// Boolean solver for sum(g_i) + *n. "Algorithm 2" in Lemoine and Viennot
bool search_sum_plus_nimber(const split_result& subgames, int n,                                                     
                            lv_bool_tt& tt);

// Compute n such that g = *n. "Algorithm 3" in Lemoine and Viennot.
int search_impartial_game(const impartial_game& g, lv_bool_tt& tt);

} // namespace lemoine_viennot
