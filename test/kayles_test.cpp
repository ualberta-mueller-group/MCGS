//---------------------------------------------------------------------------
// Unit tests for the game of Kayles
//---------------------------------------------------------------------------
#include "kayles_test.h"
#include "kayles.h"

#include <cassert>
#include "cgt_move.h"
#include "test_utilities.h"

namespace {

void kayles_test_values()
{
    for(int i = 0; i < 100; ++i)
    {
        kayles k(i);
        const int nim_value = k.search_with_tt();
        assert(kayles::static_result(i) == nim_value);
    }
}

void kayles_test_move_generator()
{
    for(int i = 0; i < 20; ++i)
    {
        kayles k(i);
        assert_num_moves(k, BLACK, i);
    }
}

void test_play_undo(int n)
{
    kayles g(n);
    std::unique_ptr<move_generator> 
        mgp(g.create_move_generator());
    move_generator& mg(*mgp);
    for (; mg; ++mg)
    {
        assert_restore_game ar(g);
        move m = mg.gen_move();
        g.play(m);
        g.undo_move();
        assert(g.value() == n);
        assert(! g.is_split());
    }
}

void kayles_test_play_undo()
{
    test_play_undo(1);
    test_play_undo(2);
    test_play_undo(3);
    test_play_undo(4);
    test_play_undo(5);
    test_play_undo(10);
    test_play_undo(55);
}
} // namespace

void kayles_test_all()
{
    kayles_test_move_generator();
    kayles_test_play_undo();
    kayles_test_values();
}
