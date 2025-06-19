//---------------------------------------------------------------------------
// Implementation of the algorithm for solving impartial games
// from Lemoine and Viennot's "Nimbers are inevitable".
//---------------------------------------------------------------------------
#pragma once

#include "hashing.h"
#include "transposition.h"
#include "impartial_game.h"

// The solver interface is analogous to impartial_game.h, except:
// - This implementation uses functions in a namespace,
//   not methods of impartial_game
// - The type of transposition table (tt) is different
// TODO: Also allow impartial_sumgame to use this algorithm for 
// solving each subgame
// TODO: check out the improvements in Beling's implementation in
// https://github.com/beling/impartial-games/blob/main/igs/src/solver/lvb.rs
// vs.
// https://github.com/beling/impartial-games/blob/main/igs/src/solver/lv.rs

namespace lemoine_viennot{

//---------------------------------------------------------------------------
// Transposition table for impartial games
//---------------------------------------------------------------------------
struct lv_ttable_entry
{
    bool value;

    lv_ttable_entry() : value(false) {}

    lv_ttable_entry(bool v) : value(v) {}
};

typedef ttable<lv_ttable_entry> lemoine_viennot_tt;
//---------------------------------------------------------------------------
// Table of hash codes for nimbers
//---------------------------------------------------------------------------

class nimber_hashcode
{
public:
    static const hash_t& get(int nimber);
private:
    static std::vector<hash_t> _codes;
    static void init_codes(int max_nimber);
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

int search_with_tt(const impartial_game& g, int tt_size = 24);
int search_impartial_game(const impartial_game& g, lemoine_viennot_tt& tt);
    
} // namespace lemoine_viennot
