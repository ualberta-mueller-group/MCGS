#include "split_test_dyadic_rational.h"
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"

#include <memory>

using std::unique_ptr;



void dyadic_rational_split1()
{
    dyadic_rational pos(35, 8);

    assert(!pos.split().has_value());

    int to_play = BLACK;
    {
        unique_ptr<move_generator> mg1 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg1);
        move m1 = mg1->gen_move();
        pos.play(m1, to_play);
    }
    // 34/8 == 17/4
    assert(!pos.split().has_value());
    {
        unique_ptr<move_generator> mg2 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg2);
        move m2 = mg2->gen_move();
        pos.play(m2, to_play);
    }
    // 16/4 == 4
    split_result sr = pos.split();

    assert(sr.has_value() && sr->size() == 1);

    integer_game *ig = dynamic_cast<integer_game*>(sr->back());
    assert(ig != nullptr);

    assert(ig->value() == 4);

    for (game* g : *sr)
    {
        delete g;
    }


}

void dyadic_rational_split2()
{
    dyadic_rational pos(37, 16);

    assert(!pos.split().has_value());

    int to_play = WHITE;

    {
        unique_ptr<move_generator> mg1 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg1);
        move m1 = mg1->gen_move();
        pos.play(m1, to_play);
    }

    // 38/16 == 19 / 8
    assert(!pos.split().has_value());
    {
        unique_ptr<move_generator> mg2 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg2);
        move m2 = mg2->gen_move();
        pos.play(m2, to_play);
    }
    // 20 / 8 == 10 / 4 == 5 / 2
    assert(!pos.split().has_value());
    {
        unique_ptr<move_generator> mg3 = unique_ptr<move_generator>(pos.create_move_generator(to_play));
        assert(*mg3);
        move m3 = mg3->gen_move();
        pos.play(m3, to_play);
    }

    // 6/2 == 3
    split_result sr = pos.split();

    assert(sr.has_value() && sr->size() == 1);

    integer_game *ig = dynamic_cast<integer_game*>(sr->back());
    assert(ig != nullptr);

    assert(ig->value() == 3);

    for (game* g : *sr)
    {
        delete g;
    }
}

void dyadic_rational_split3()
{
    dyadic_rational pos(16, 2);

    split_result sr = pos.split();

    assert(sr.has_value());
    assert(sr->size() == 1);
    integer_game* ig = dynamic_cast<integer_game*>(sr->back());
    assert(ig != nullptr);

    assert(ig->value() == 8);
}

void split_test_dyadic_rational_all()
{
    dyadic_rational_split1();
    dyadic_rational_split2();
    dyadic_rational_split3();

}
