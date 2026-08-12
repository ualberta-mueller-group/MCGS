#include "pitm_test.h"

#include <vector>
#include <array>
#include <cassert>

#include "global_options.h"
#include "game.h"
#include "all_game_headers.h"
#include "impartial_game_wrapper.h"

using namespace std;

////////////////////////////////////////////////// Helpers
namespace {

vector<::move> make_moves(move_generator* mg)
{
    vector<::move> moves;

    for (; *mg; ++(*mg))
        moves.push_back(mg->gen_move());

    delete mg;
    return moves;
}

void pitm_test_impl()
{
    vector<game*> games
    {
        new clobber_1xn("XOXOXOXXOXOOO"),
        new amazons(".X.|...|.O."),
        new domineering("....|....|...."),

        new kayles(9),

        new impartial_game_wrapper(new clobber_1xn("XOXOXOXXOXOOO"), true),
        new impartial_game_wrapper(new amazons(".X.|...|.O."), true),
        new impartial_game_wrapper(new domineering("....|....|...."), true),
    };

    constexpr array<bw, 2> COLORS = {BLACK, WHITE};

    for (game* g : games)
    {
        // `game` BLACK/WHITE
        for (const bw color : COLORS)
        {
            const vector<::move> moves_basic = make_moves(
                g->create_move_generator(color, MOVE_GENERATOR_TYPE_BASIC));

            const vector<::move> moves_pitm = make_moves(
                g->create_move_generator(color, MOVE_GENERATOR_TYPE_PITM));

            global::pitm.set(false);
            const vector<::move> moves_basic2 =
                make_moves(g->create_move_generator(color));

            global::pitm.set(true);
            const vector<::move> moves_pitm2 =
                make_moves(g->create_move_generator(color));

            assert(moves_basic != moves_pitm);
            assert(moves_basic == moves_basic2);
            assert(moves_pitm == moves_pitm2);
        }

        // `impartial_game`
        impartial_game* g_imp = dynamic_cast<impartial_game*>(g);

        if (g_imp != nullptr)
        {
            const vector<::move> moves_basic = make_moves(
                g_imp->create_move_generator(MOVE_GENERATOR_TYPE_BASIC));

            const vector<::move> moves_pitm = make_moves(
                g_imp->create_move_generator(MOVE_GENERATOR_TYPE_PITM));

            global::pitm.set(false);
            const vector<::move> moves_basic2 =
                make_moves(g_imp->create_move_generator());

            global::pitm.set(true);
            const vector<::move> moves_pitm2 =
                make_moves(g_imp->create_move_generator());

            assert(moves_basic != moves_pitm);
            assert(moves_basic == moves_basic2);
            assert(moves_pitm == moves_pitm2);
        }

        delete g;
    }
}

} // namespace

////////////////////////////////////////////////// Exported functions
void pitm_test_all()
{
    // Save global options
    const bool restore_pitm = global::pitm();

    const bool restore_imp_wrapper_alternate_color =
        global::imp_wrapper_alternate_color();

    // Run tests
    global::imp_wrapper_alternate_color.set(false);
    pitm_test_impl();

    global::imp_wrapper_alternate_color.set(true);
    pitm_test_impl();

    // Restore global options
    global::pitm.set(restore_pitm);

    global::imp_wrapper_alternate_color.set(
        restore_imp_wrapper_alternate_color);
}
