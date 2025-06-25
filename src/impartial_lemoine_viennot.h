//---------------------------------------------------------------------------
// Implementation of the algorithm for solving impartial games
// from Lemoine and Viennot's "Nimbers are inevitable".
//---------------------------------------------------------------------------
#pragma once

#include "hashing.h"
#include "transposition.h"
#include "impartial_game.h"

// Approach - the Lemoine - Viennot algorithms:
// - The tt stores the win/loss results of sums G + *n, where
//   G is an impartial game position, and *n is a nimber.
// - If G + *n is a loss then G = *n
// - If G + *n is a win then G != *n
// - Special case: P + *0 = P
// - Algorithm 1:
// - If G is splittable, use Algorithm 2: compute nimbers of most subgames and return nim-sum
// - If G is not splittable
//     - search all position options Pi + *n and all nimber options P + *i, i<n.
//     - nimber
//     - If all options are winning, G + *n is a loss
// - Algorithm 2: G = G1 + ... Gk, compute boolean outcome of G + *n
//     - sort by expected difficulty, leave hardest game Gk for last
//     - Compute nimber ni = Gi for all i<n, compute nim sum 
//       *nprime = n1 + ... + n_{k-1} + n
//     - Check Gk + *nprime
// - Algorithm 3: Compute nimber of G:
//     - Try (G + *n) for n = 0, 1, ... until a loss is found.
//         If G + *n is a loss, then G + *n = 0, and G = *n

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

// Compute n such that g = *n. "Algorithm 3" in Lemoine and Viennot.
int search_impartial_game(const impartial_game& g, lemoine_viennot_tt& tt);

// Boolean solver for g + *n. "Algorithm 1" in Lemoine and Viennot
bool search_g_plus_nimber(const impartial_game& g, int n, 
                          lemoine_viennot_tt& tt);

// Boolean solver for sum(g_i) + *n. "Algorithm 2" in Lemoine and Viennot
bool search_sum_plus_nimber(split_result& subgames, int n,                                                     
                            lemoine_viennot_tt& tt);

} // namespace lemoine_viennot
