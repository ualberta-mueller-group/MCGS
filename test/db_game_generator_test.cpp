#include "db_game_generator_test.h"

#include <type_traits>
#include <cassert>
#include <exception>

#include "clobber_1xn.h"
#include "clobber.h"
#include "nogo_1xn.h"
#include "nogo.h"
#include "db_game_generator.h"
#include "grid_generator.h"
#include "custom_traits.h"
#include "gridlike_db_game_generator.h"
#include "strip.h"
#include "grid.h"

using namespace std;

namespace {

template <class Game_T>
void assert_generators_consistent(db_game_generator& game_gen, grid_generator& grid_gen)
{
    static_assert(std::is_base_of_v<strip, Game_T> || //
                  std::is_base_of_v<grid, Game_T>     //
                  );                                  //


    auto is_legal = [&]() -> bool
    {
        assert(grid_gen);

        if constexpr (!has_is_legal_v<Game_T>)
            return true;
        else
        {
            try
            {
                Game_T g(grid_gen.gen_board());
                return g.is_legal();
            }
            catch (exception& e)
            {
                return false;
            }
        }
    };

    auto advance_legal = [&](bool init) -> void
    {
        if (!grid_gen)
            return;

        if (!init)
            ++grid_gen;

        while (grid_gen && !is_legal())
            ++grid_gen;
    };


    advance_legal(true);

    while (game_gen)
    {
        assert(grid_gen);

        game* g = game_gen.gen_game();
        const string board_expected = grid_gen.gen_board();

        ++game_gen;
        advance_legal(false);

        Game_T* g_casted = dynamic_cast<Game_T*>(g);
        assert(g_casted != nullptr);

        const string& board_got = g_casted->board_as_string();

        assert(board_expected == board_got);

        delete g;
    }

    assert(!game_gen && !grid_gen);
}

void test_clobber_1xn()
{
    gridlike_db_game_generator<clobber_1xn, grid_generator_default> game_gen(3);
    grid_generator_default grid_gen(3);
    assert_generators_consistent<clobber_1xn>(game_gen, grid_gen);
}

void test_clobber()
{
    gridlike_db_game_generator<clobber, grid_generator_default> game_gen(2, 2);
    grid_generator_default grid_gen(2, 2);
    assert_generators_consistent<clobber>(game_gen, grid_gen);
}

void test_nogo_1xn()
{
    gridlike_db_game_generator<nogo_1xn, grid_generator_default> game_gen(3);
    grid_generator_default grid_gen(3);
    assert_generators_consistent<nogo_1xn>(game_gen, grid_gen);
}

void test_nogo()
{
    gridlike_db_game_generator<nogo, grid_generator_default> game_gen(3, 3);
    grid_generator_default grid_gen(3, 3);
    assert_generators_consistent<nogo>(game_gen, grid_gen);
}


} // namespace

void db_game_generator_test_all()
{
    test_clobber_1xn();
    test_clobber();
    test_nogo_1xn();
    test_nogo();
}
