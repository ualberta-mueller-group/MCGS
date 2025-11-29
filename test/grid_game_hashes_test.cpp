#include "grid_game_hashes_test.h"

#include <type_traits>

#include "amazons.h"
#include "nogo.h"
#include "clobber.h"
#include "domineering.h"
#include "fission.h"
//#include "sheep.h"

using namespace std;

namespace {
////////////////////////////////////////////////// helpers
template <class Grid_Game_T>
bool game_hashes_equal(const string& board1, const string& board2)
{
    static_assert(std::is_base_of_v<grid, Grid_Game_T> && //
                  !std::is_abstract_v<Grid_Game_T>        //
    );

    Grid_Game_T g1(board1);
    Grid_Game_T g2(board2);

    const grid& g1_grid = g1;
    const grid& g2_grid = g2;

    return g1_grid.get_local_hash() == g2_grid.get_local_hash();
}

////////////////////////////////////////////////// main test functions

void test_amazons()
{
    assert(game_hashes_equal<amazons>("", ""));
    assert(game_hashes_equal<amazons>("...|...|...", "...|...|..."));

    assert(game_hashes_equal<amazons>("...|X..|...", "...|..X|..."));
    assert(game_hashes_equal<amazons>(".X.|...|...", "...|...|.X."));
    assert(game_hashes_equal<amazons>("X..|...|...", "...|...|..X"));

    assert(game_hashes_equal<amazons>("...|...", "..|..|.."));
    assert(game_hashes_equal<amazons>("...|.X.", "..|X.|.."));
    assert(game_hashes_equal<amazons>(".X.|...|...", "...|..X|..."));
}

void test_nogo()
{
    assert(game_hashes_equal<nogo>("", ""));
    assert(game_hashes_equal<nogo>("...|...|...", "...|...|..."));

    assert(game_hashes_equal<nogo>("...|X..|...", "...|..X|..."));
    assert(game_hashes_equal<nogo>(".X.|...|...", "...|...|.X."));
    assert(game_hashes_equal<nogo>("X..|...|...", "...|...|..X"));

    assert(game_hashes_equal<nogo>("...|...", "..|..|.."));
    assert(game_hashes_equal<nogo>("...|.X.", "..|X.|.."));
    assert(game_hashes_equal<nogo>(".X.|...|...", "...|..X|..."));
}


void test_clobber()
{
    assert(game_hashes_equal<clobber>("", ""));
    assert(game_hashes_equal<clobber>("XO", "OX"));
    assert(game_hashes_equal<clobber>("XOX|OXO", "XO|OX|XO"));
}

void test_domineering()
{
    assert(game_hashes_equal<domineering>("", ""));
    assert(game_hashes_equal<domineering>("...|...|...", "...|...|..."));

    assert(game_hashes_equal<domineering>("...|#..|...", "...|..#|..."));
    assert(game_hashes_equal<domineering>(".#.|...|...", "...|...|.#."));
    assert(game_hashes_equal<domineering>("#..|...|...", "...|...|..#"));

    assert(!game_hashes_equal<domineering>("...|...", "..|..|.."));
    assert(!game_hashes_equal<domineering>(".#.|...|...", "...|..#|..."));
}

void test_fission()
{
    assert(game_hashes_equal<fission>("", ""));
    assert(game_hashes_equal<fission>("...|...|...", "...|...|..."));

    assert(game_hashes_equal<fission>("...|X..|...", "...|..X|..."));
    assert(game_hashes_equal<fission>(".X.|...|...", "...|...|.X."));
    assert(game_hashes_equal<fission>("X..|...|...", "...|...|..X"));

    assert(!game_hashes_equal<fission>("...|...", "..|..|.."));
    assert(!game_hashes_equal<fission>(".X.|...|...", "...|..X|..."));
}


} // namespace

//////////////////////////////////////////////////
void grid_game_hashes_test_all()
{
    test_amazons();
    test_nogo();
    test_clobber();
    test_domineering();
    test_fission();
}
