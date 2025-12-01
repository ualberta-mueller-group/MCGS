#include "cgt_basics_test.h"
#include <cassert>

#include "cgt_basics.h"
#include "test_utilities.h"

namespace {
namespace cgt_basics_test {

void test_colors()
{
    assert(BLACK == 0);
    assert(WHITE == 1);
    assert(EMPTY == 2);
    assert(BORDER == 3);
    assert(ROW_SEP == 4);
    assert(COLOR_INVALID == 63);

    assert(LEFT == 0);
    assert(RIGHT == 1);
}

void test_is_black_white()
{
    assert(is_black_white(BLACK));
    assert(is_black_white(WHITE));
    assert(!is_black_white(EMPTY));
    assert(!is_black_white(BORDER));
    assert(!is_black_white(ROW_SEP));
    assert(!is_black_white(COLOR_INVALID));
    assert(!is_black_white(3000));
}

void test_is_empty_black_white()
{
    assert(is_empty_black_white(BLACK));
    assert(is_empty_black_white(WHITE));
    assert(is_empty_black_white(EMPTY));
    assert(!is_empty_black_white(BORDER));
    assert(!is_empty_black_white(ROW_SEP));
    assert(!is_empty_black_white(COLOR_INVALID));
    assert(!is_empty_black_white(3000));
}

void test_opponent()
{
    assert(opponent(BLACK) == WHITE);
    assert(opponent(WHITE) == BLACK);
    ASSERT_DID_THROW(opponent(EMPTY));
    ASSERT_DID_THROW(opponent(BORDER));
    ASSERT_DID_THROW(opponent(COLOR_INVALID));
    ASSERT_DID_THROW(opponent(3000));

    assert(ebw_opponent(BLACK) == WHITE);
    assert(ebw_opponent(WHITE) == BLACK);
    assert(ebw_opponent(EMPTY) == EMPTY);
    ASSERT_DID_THROW(ebw_opponent(BORDER));
    ASSERT_DID_THROW(ebw_opponent(COLOR_INVALID));
    ASSERT_DID_THROW(ebw_opponent(3000));
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

void test_in_interval()
{
    assert(in_interval(5, 1, 10));
    assert(in_interval(1, 1, 10));
    assert(in_interval(9, 1, 10));
    assert(in_interval(10, 1, 10));
    assert(!in_interval(0, 1, 10));
    assert(!in_interval(1000, 1, 10));
}

void test_is_player_char()
{
    assert(is_player_char('B'));
    assert(is_player_char('W'));

    assert(!is_player_char('X'));
    assert(!is_player_char('O'));
    assert(!is_player_char('.'));
    assert(!is_player_char('#'));
    assert(!is_player_char('|'));
    assert(!is_player_char('?'));
    assert(!is_player_char('A'));
    assert(!is_player_char('b'));
    assert(!is_player_char('w'));
}


void test_player_char_to_color()
{
    assert(player_char_to_color('B') == BLACK);
    assert(player_char_to_color('W') == WHITE);

    ASSERT_DID_THROW(player_char_to_color('X'));
    ASSERT_DID_THROW(player_char_to_color('O'));
    ASSERT_DID_THROW(player_char_to_color('.'));
    ASSERT_DID_THROW(player_char_to_color('#'));
    ASSERT_DID_THROW(player_char_to_color('|'));
    ASSERT_DID_THROW(player_char_to_color('?'));
    ASSERT_DID_THROW(player_char_to_color('A'));
    ASSERT_DID_THROW(player_char_to_color('b'));
    ASSERT_DID_THROW(player_char_to_color('w'));
}

void test_color_to_player_char()
{
    assert(color_to_player_char(BLACK) == 'B');
    assert(color_to_player_char(WHITE) == 'W');

    ASSERT_DID_THROW(color_to_player_char(EMPTY));
    ASSERT_DID_THROW(color_to_player_char(BORDER));
    ASSERT_DID_THROW(color_to_player_char(ROW_SEP));
    ASSERT_DID_THROW(color_to_player_char(COLOR_INVALID));
    ASSERT_DID_THROW(color_to_player_char(3000));
}

void test_is_valid_color()
{
    assert(is_valid_color(BLACK));
    assert(is_valid_color(WHITE));
    assert(is_valid_color(EMPTY));
    assert(is_valid_color(BORDER));
    assert(is_valid_color(ROW_SEP));
    assert(!is_valid_color(COLOR_INVALID));
    assert(!is_valid_color(8000));
}

void test_is_valid_char()
{
    assert(is_valid_char('X'));
    assert(is_valid_char('O'));
    assert(is_valid_char('.'));
    assert(is_valid_char('#'));
    assert(is_valid_char('|'));
    assert(!is_valid_char(CHAR_INVALID));
    assert(!is_valid_char('A'));
    assert(!is_valid_char('B'));
    assert(!is_valid_char('W'));
    assert(!is_valid_char('x'));
    assert(!is_valid_char('b'));
    assert(!is_valid_char('w'));
}

void test_char_to_color()
{
    assert(char_to_color('X') == BLACK);
    assert(char_to_color('O') == WHITE);
    assert(char_to_color('.') == EMPTY);
    assert(char_to_color('#') == BORDER);
    assert(char_to_color('|') == ROW_SEP);

    ASSERT_DID_THROW(char_to_color('?'));
    ASSERT_DID_THROW(char_to_color('B'));
    ASSERT_DID_THROW(char_to_color('W'));
    ASSERT_DID_THROW(char_to_color('A'));
    ASSERT_DID_THROW(char_to_color('x'));
    ASSERT_DID_THROW(char_to_color('b'));
    ASSERT_DID_THROW(char_to_color('w'));
}

void test_color_to_char()
{
    assert(color_to_char(BLACK) == 'X');
    assert(color_to_char(WHITE) == 'O');
    assert(color_to_char(EMPTY) == '.');
    assert(color_to_char(BORDER) == '#');
    assert(color_to_char(ROW_SEP) == '|');
    ASSERT_DID_THROW(color_to_char(COLOR_INVALID));
    ASSERT_DID_THROW(color_to_char(3000));
}

void test_inverse_color()
{
    assert(inverse_color(BLACK) == WHITE);
    assert(inverse_color(WHITE) == BLACK);

    assert(inverse_color(EMPTY) == EMPTY);
    assert(inverse_color(BORDER) == BORDER);
    assert(inverse_color(ROW_SEP) == ROW_SEP);

    ASSERT_DID_THROW(inverse_color(COLOR_INVALID));
    ASSERT_DID_THROW(inverse_color(3000));
}

void test_inverse_char()
{
    assert(inverse_char('X') == 'O');
    assert(inverse_char('O') == 'X');

    assert(inverse_char('.') == '.');
    assert(inverse_char('#') == '#');
    assert(inverse_char('|') == '|');

    ASSERT_DID_THROW(inverse_char('?'));
    ASSERT_DID_THROW(inverse_char('x'));
    ASSERT_DID_THROW(inverse_char('A'));
    ASSERT_DID_THROW(inverse_char('B'));
    ASSERT_DID_THROW(inverse_char('W'));
    ASSERT_DID_THROW(inverse_char('b'));
    ASSERT_DID_THROW(inverse_char('w'));
}

void test_is_stone_color()
{
    assert(is_stone_color(BLACK));
    assert(is_stone_color(WHITE));
    assert(!is_stone_color(EMPTY));
    assert(is_stone_color(BORDER));
    assert(!is_stone_color(ROW_SEP));

    assert(!is_stone_color(COLOR_INVALID));
    assert(!is_stone_color(3000));
}

void test_is_empty_or_stone_color()
{
    assert(is_empty_or_stone_color(BLACK));
    assert(is_empty_or_stone_color(WHITE));
    assert(is_empty_or_stone_color(EMPTY));
    assert(is_empty_or_stone_color(BORDER));
    assert(!is_empty_or_stone_color(ROW_SEP));

    assert(!is_empty_or_stone_color(COLOR_INVALID));
    assert(!is_empty_or_stone_color(3000));
}

void test_is_stone_char()
{
    assert(is_stone_char('X'));
    assert(is_stone_char('O'));
    assert(!is_stone_char('.'));
    assert(is_stone_char('#'));
    assert(!is_stone_char('|'));
    assert(!is_stone_char('?'));
    assert(!is_stone_char('x'));
    assert(!is_stone_char('B'));
    assert(!is_stone_char('W'));
    assert(!is_stone_char('A'));
    assert(!is_stone_char('b'));
    assert(!is_stone_char('w'));
}

void test_is_empty_or_stone_char()
{
    assert(is_empty_or_stone_char('X'));
    assert(is_empty_or_stone_char('O'));
    assert(is_empty_or_stone_char('.'));
    assert(is_empty_or_stone_char('#'));
    assert(!is_empty_or_stone_char('|'));
    assert(!is_empty_or_stone_char('?'));
    assert(!is_empty_or_stone_char('x'));
    assert(!is_empty_or_stone_char('B'));
    assert(!is_empty_or_stone_char('W'));
    assert(!is_empty_or_stone_char('A'));
}

} // namespace cgt_basics_test
} // namespace

void cgt_basics_test_all()
{
    cgt_basics_test::test_colors();
    cgt_basics_test::test_is_black_white();
    cgt_basics_test::test_is_empty_black_white();
    cgt_basics_test::test_opponent();
    cgt_basics_test::test_in_range();
    cgt_basics_test::test_in_interval();

    cgt_basics_test::test_is_player_char();
    cgt_basics_test::test_player_char_to_color();
    cgt_basics_test::test_color_to_player_char();
    cgt_basics_test::test_is_valid_color();
    cgt_basics_test::test_is_valid_char();
    cgt_basics_test::test_char_to_color();
    cgt_basics_test::test_color_to_char();
    cgt_basics_test::test_inverse_color();
    cgt_basics_test::test_inverse_char();
    cgt_basics_test::test_is_stone_color();
    cgt_basics_test::test_is_empty_or_stone_color();
    cgt_basics_test::test_is_stone_char();
    cgt_basics_test::test_is_empty_or_stone_char();
}
