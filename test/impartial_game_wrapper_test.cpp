//---------------------------------------------------------------------------
// Unit tests for impartial_game_wrapper
//---------------------------------------------------------------------------
#include "impartial_game_wrapper_test.h"
#include "impartial_game_wrapper.h"

#include <cassert>
#include "cgt_move.h"
#include "clobber_1xn.h"
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
    const int v = g.search_impartial_game();
    assert(v == nim_value);
}

string xoxo(int n)
{
    string result;
    for (int i = 0; i < n; ++i)
        result += "XO";
    return result;
}

void impartial_game_wrapper_test_values()
{
    static int Dai_Chen_result[] =  // -1 means they could not solve it
    { 0, 
      1, 3, 0, 2, 0, 2,  0, 3,  1,  4,   //  1-10
      6, 1, 0, 1, 3, 7, -1, 3,  1,  0,   // 11-20
     -1, 4, 0, 1, 3, 0,  4, 0,  2,  0,   // 21-30
      3, 1, 4, 6, 1, 0,  1, 3, -1, -1,   // 31-40
      3, 1                               // 41-42
     };
    
    // Fast up to 6, 7 takes a few seconds, 
    // 8 not solved in 5 minutes without a cache/table
    int limit = 7; 
    for (int i = 0; i < limit; ++i)
        test_nim_value(xoxo(i), Dai_Chen_result[i]);
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
        
} // namespace

void impartial_game_wrapper_test_all()
{
    impartial_game_wrapper_test_move_generator();
    impartial_game_wrapper_test_play_undo();
    impartial_game_wrapper_test_values();
}
