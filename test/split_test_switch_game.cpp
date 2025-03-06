#include "split_test_switch_game.h"
#include "cgt_switch.h"
#include "split_test_utils.h"

#include <memory>

using std::unique_ptr;


void test_switch_generic(const fraction& l, const fraction& r)
{
    switch_game pos(l, r);

    // Shouldn't split until move is played
    assert(!pos.split().has_value());

    // Play black move, check split
    {
        unique_ptr<move_generator> mgp(pos.create_move_generator(BLACK));
        move_generator& gen = *mgp;

        move mb = gen.gen_move();
        pos.play(mb, BLACK);
        assert(pos.is_rational());
        assert(pos.value() == l);

        split_result sr = pos.split();

        assert(sr);
        assert(sr->size() == 1);

        dyadic_rational* dr = dynamic_cast<dyadic_rational*>(sr->back());
        assert(dr != nullptr);
        assert(dr->get_fraction() == l);

        for (game* g : *sr)
        {
            delete g;
        }
    }

    // undo move
    pos.undo_move();
    assert(!pos.split().has_value());

    // Play white move, check split
    {
        unique_ptr<move_generator> mgp(pos.create_move_generator(WHITE));
        move_generator& gen = *mgp;

        move mw = gen.gen_move();
        pos.play(mw, WHITE);
        assert(pos.is_rational());
        assert(pos.value() == r);

        split_result sr = pos.split();

        assert(sr);
        assert(sr->size() == 1);

        dyadic_rational* dr = dynamic_cast<dyadic_rational*>(sr->back());
        assert(dr != nullptr);
        assert(dr->get_fraction() == r);

        for (game* g : *sr)
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

void switch_game4()
{
    test_switch_generic(fraction(21, 4), fraction(201, 512));
}

void split_test_switch_game_all()
{
    switch_game1();
    switch_game2();
    switch_game3();
    switch_game4();
}
