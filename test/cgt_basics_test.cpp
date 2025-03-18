#include "cgt_basics.h"

namespace cgt_basics_test {

void test_colors()
{
    assert(BLACK == 0);
    assert(WHITE == 1);
    assert(EMPTY == 2);
    assert(BORDER == 3);

    assert(LEFT == 0);
    assert(RIGHT == 1);
}

void test_is_black_white()
{
    assert(is_black_white(BLACK));
    assert(is_black_white(WHITE));
    assert(!is_black_white(EMPTY));
    assert(!is_black_white(BORDER));
}

void test_is_empty_black_white()
{
    assert(is_empty_black_white(BLACK));
    assert(is_empty_black_white(WHITE));
    assert(is_empty_black_white(EMPTY));
    assert(!is_empty_black_white(BORDER));
}

void test_opponent()
{
    assert(WHITE == opponent(BLACK));
    assert(BLACK == opponent(WHITE));
}

void test_in_range()
{
    assert(in_range(5, 1, 10));
    assert(in_range(1, 1, 10));
    assert(in_range(9, 1, 10));
    assert(!in_range(10, 1, 10));
    assert(!in_range(0, 1, 10));
    assert(!in_range(1000, 1, 10));
}

void test_color_char()
{
    assert(COLOR_CODE[BLACK] == 'B');
    assert(COLOR_CODE[WHITE] == 'W');
    assert(COLOR_CODE[EMPTY] == '.');
    assert(COLOR_CODE[BORDER] == 'X');

    assert(color_char(BLACK) == 'B');
    assert(color_char(WHITE) == 'W');
    assert(color_char(EMPTY) == '.');
    assert(color_char(BORDER) == 'X');

    assert(char_to_color('B') == BLACK);
    assert(char_to_color('W') == WHITE);

    assert(is_black_white_char('B'));
    assert(is_black_white_char('W'));
    assert(!is_black_white_char('.'));
    assert(!is_black_white_char('X'));
}

} // namespace cgt_basics_test

void cgt_basics_test_all()
{
    cgt_basics_test::test_colors();
    cgt_basics_test::test_color_char();
    cgt_basics_test::test_in_range();
    cgt_basics_test::test_is_black_white();
    cgt_basics_test::test_is_empty_black_white();
    cgt_basics_test::test_opponent();
}
