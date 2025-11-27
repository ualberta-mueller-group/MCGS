//---------------------------------------------------------------------------
// Unit tests for impartial_game_wrapper
//---------------------------------------------------------------------------
#include "impartial_game_wrapper_test.h"
#include "impartial_game_wrapper.h"

#include <cassert>
#include <memory>
#include <string>
#include "cgt_move.h"
#include "clobber_1xn.h"
#include "nogo_1xn.h"
#include "test_utilities.h"

using std::string;

namespace {

void test_num_moves(const string& s, int num_moves)
{
    clobber_1xn c(s);
    impartial_game_wrapper g(&c);
    assert_num_moves(g, BLACK, num_moves);
}

void impartial_game_wrapper_test_move_generator()
{
    test_num_moves("OX", 2);
    test_num_moves("XOXO", 6);
    test_num_moves(".X.X", 0);
}

void test_nim_value(const string& s, int nim_value)
{
    clobber_1xn c(s);
    impartial_game_wrapper g(&c);
    const int v = g.search_with_tt();
    assert(v == nim_value);
}

void impartial_game_wrapper_test_values()
{
    // Results 1-42 except 17, 21, 39, 40 from: J. Dai and X. Chen,
    // Impartial Clobber Solver,
    // CMPUT 655 project report, University of Alberta 2022
    static int expected_result[] = {
        0,
        1, 3, 0, 2, 0, 2, 0, 3, 1, 4, //  1-10
        6, 1, 0, 1, 3, 7, 8, 3, 1, 0, // 11-20
        8, 4, 0, 1, 3, 0, 4, 0, 2, 0, // 21-30
        3, 1, 4, 6, 1, 0, 1, 3,11, 8, // 31-40
        3, 1,11, 8,12,11, 1, 3, 0, 6  // 41-50
    };

    int limit = 50;
    for (int i = 0; i <= limit; ++i)
        test_nim_value(clobber_1xn::xo(i), expected_result[i]);
}

void test_play_undo(const string& s)
{
    clobber_1xn c(s);
    impartial_game_wrapper g(&c);
    std::unique_ptr<move_generator> mgp(g.create_move_generator());
    move_generator& mg(*mgp);
    for (; mg; ++mg)
    {
        assert_restore_game arc(c);
        assert_restore_game arg(g);
        move m = mg.gen_move();
        g.play(m);
        g.undo_move();
    }
}

void impartial_game_wrapper_test_play_undo()
{
    test_play_undo("");
    test_play_undo("X");
    test_play_undo(".X");
    test_play_undo(".X.X");
    test_play_undo("OX");
    test_play_undo("XO");
    test_play_undo("XOXO");
    test_play_undo("XOXOXO");
}

void test_nogo(string s, int nim_value)
{
    nogo_1xn c(s);
    impartial_game_wrapper g(&c);
    const int v = g.search_with_tt();
    assert(v == nim_value);
}

void impartial_game_wrapper_test_nogo()
{
    static int expected[] = // computed with MCGS
        {
            0,
            0, 1, 0, 1, 2, 0, 1, 0, 1, 2, //  1-10
            3, 1, 0, 3, 1, 0, 2, 2, 3     // 11-19
        };

    for (int i = 0; i < 11; ++i)
    {
        string s(i, '.');
        test_nogo(s, expected[i]);
    }
}

} // namespace

void impartial_game_wrapper_test_all()
{
    impartial_game_wrapper_test_move_generator();
    impartial_game_wrapper_test_play_undo();
    impartial_game_wrapper_test_values();
    impartial_game_wrapper_test_nogo();
}
