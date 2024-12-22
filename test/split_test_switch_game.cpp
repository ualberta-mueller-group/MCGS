#include "split_test_switch_game.h"
#include "cgt_switch.h"
#include "split_test_utils.h"

#include <memory>

using std::unique_ptr;


void test_switch_generic(int l, int r)
{
    switch_game pos(l, r);

    // Shouldn't split until move is played
    assert(!pos.split().has_value());

    // Play black move, check split
    {
        unique_ptr<move_generator> mgb(pos.create_move_generator(BLACK));
        assert(mgb);
        move mb = mgb->gen_move();
        pos.play(mb, BLACK);
        assert(pos.is_integer());
        assert(pos.value() == l);

        split_result srb = pos.split();

        assert(srb);
        assert(srb->size() == 1);

        integer_game* igb = dynamic_cast<integer_game*>(srb->back());
        assert(igb != nullptr);
        assert(igb->value() == l);

        for (game* g : *srb)
        {
            delete g;
        }
    }

    // undo move
    pos.undo_move();
    assert(!pos.split().has_value());

    // Play white move, check split
    {
        unique_ptr<move_generator> mgw(pos.create_move_generator(WHITE));
        assert(mgw);
        move mw = mgw->gen_move();
        pos.play(mw, WHITE);
        assert(pos.is_integer());
        assert(pos.value() == r);

        split_result srw = pos.split();

        assert(srw);
        assert(srw->size() == 1);

        integer_game* igw = dynamic_cast<integer_game*>(srw->back());
        assert(igw != nullptr);
        assert(igw->value() == r);

        for (game* g : *srw)
        {
            delete g;
        }
    }

}

void switch_game1()
{
    test_switch_generic(7, 5);

}

void switch_game2()
{
    test_switch_generic(400, -300);

}

void switch_game3()
{
    test_switch_generic(-21, -90);
}

void split_test_switch_game_all()
{
    switch_game1();
    switch_game2();
    switch_game3();
}
