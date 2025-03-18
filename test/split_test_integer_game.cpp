#include "split_test_integer_game.h"
#include "cgt_integer_game.h"
#include "split_test_utils.h"
#include <memory>

using std::unique_ptr;

void integer_game1()
{
    integer_game pos(21);
    assert_no_split(&pos);
}

void integer_game2()
{
    integer_game pos(-5);
    assert_no_split(&pos);
}

void integer_game3()
{
    integer_game pos(0);
    assert_no_split(&pos);
}

void integer_game4()
{
    integer_game pos(6);
    assert_no_split(&pos);

    unique_ptr<move_generator> mg =
        unique_ptr<move_generator>(pos.create_move_generator(BLACK));

    assert(*mg);
    move m = mg->gen_move();
    pos.play(m, BLACK);

    assert(pos.value() == 5);
    assert_no_split(&pos);
}

void split_test_integer_game_all()
{
    integer_game1();
    integer_game2();
    integer_game3();
    integer_game4();
}
