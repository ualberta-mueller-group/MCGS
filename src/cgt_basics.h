//---------------------------------------------------------------------------
// Basic cgt and game constants and utilities
// Players, colors, ranges, outcome_class,
// conversion color to char and back
//---------------------------------------------------------------------------

#pragma once

#include <cassert>
#include <climits>
#include <limits>
#include "throw_assert.h"


/*
   TODO move this elsewhere?

    Colors have 2 representations:
    - char
    - int (also called "color" by relevant utility functions)

    Colors have exactly one of several types:
    - Stones (BLACK 'X', WHITE 'O', BORDER '#')
    - Empty spaces (EMPTY '.')
    - Row separators (ROW_SEP '|')
    - Invalid (COLOR_INVALID, CHAR_INVALID = '?')
        - This is an error and should never occur. If NDEBUG is not defined,
          exceptions should catch usage errors in the color API functions, but
          with NDEBUG defined, COLOR_INVALID/CHAR_INVALID are returned instead

        - You should use is_valid_color() or is_valid_char() before calling
          conversion functions, if the input is a result of non-sanitized user
          input.

    ROW_SEP has special semantics for grids, and is not allowed in strips.
    It shouldn't be used as a stone color.
*/


// Colors must be in the range [0, NUM_MAX_COLORS = 256)
// (assuming char is 8 bits)
constexpr int NUM_MAX_COLORS = 1 << (CHAR_BIT);

const int BLACK = 0;
const int WHITE = 1;
const int EMPTY = 2;
const int BORDER = 3;
const int ROW_SEP = 4;

const char CHAR_INVALID = '?';
const int COLOR_INVALID = CHAR_INVALID; // 63

const int LEFT = BLACK;
const int RIGHT = WHITE;

typedef int bw;  // black or white
typedef int ebw; // empty, black or white

/*
 The four outcome classes of a combinatorial game, plus unknown:
 P = previous player win = 2nd player win = 0
 N = next player win = first player win
 L = left wins = black wins = X wins, no matter who goes first
 R = right wins = white wins = O wins, no matter who goes first
 U = unknown - used when outcome is not computed (yet)
 */
enum outcome_class
{
    P = 0,
    N,
    L,
    R,
    U
};

enum relation
{
    REL_EQUAL = 0,
    REL_FUZZY,
    REL_LESS_OR_EQUAL,
    REL_LESS,
    REL_GREATER_OR_EQUAL,
    REL_GREATER,
    REL_UNKNOWN,
};

// A move encoding any move in some simple CGT games.
enum simple_cgt_move_code
{
    INTEGER_MOVE_CODE = 0,
    SWITCH_MOVE_CODE,
};

//---------------------------------------------------------------------------
// Utility for colors

inline bool is_black_white(int c)
{
    return c == BLACK || c == WHITE;
}

inline void assert_black_white(bw color)
{
    assert(is_black_white(color));
}

inline int opponent(bw c)
{
    assert_black_white(c);
    return BLACK + WHITE - c;
}

inline bool is_empty_black_white(int c)
{
    return c == BLACK || c == WHITE || c == EMPTY;
}

inline void assert_empty_black_white(ebw color)
{
    assert(is_empty_black_white(color));
}

inline int ebw_opponent(int c)
{
    assert_empty_black_white(c);
    if (c == EMPTY)
        return EMPTY;
    return opponent(c);
}

//---------------------------------------------------------------------------
// Utilities for ranges

// range includes low but excludes high
inline bool in_range(int value, int low, int high)
{
    return value >= low     //
           && value < high; //
}

// range includes low but excludes high
inline void assert_range(int value, int low, int high)
{
    assert(value >= low);
    assert(value < high);
}

// TODO test
// interval includes both low and high
template <class T>
inline bool in_interval(const T& val, const T& low, const T& high)
{
    return low <= val && val <= high;
}

//---------------------------------------------------------------------------
/// Mapping from colors to char, X for BORDER
///const char COLOR_CODE[] = {'B', 'W', '.', 'X'};
///
///inline int char_to_color(char c) // OK
///{
///    return c == 'B' ? BLACK : WHITE;
///}
///
///inline char color_char(int color)
///{
///    assert_range(color, 0, 4);
///    return COLOR_CODE[color];
///}
///
///inline bool is_black_white_char(char c)
///{
///    return c == 'B' || c == 'W';
///}

namespace __color_impl { // NOLINT(readability-identifier-naming)
const char* get_color_to_char_table();
const int* get_char_to_color_table();
} // namespace __color_impl

namespace mcgs_init {
void init_color_tables();
} // namespace mcgs_init

inline bool is_player_char(char c)
{
    return c == 'B' || c == 'W';
}

inline int player_char_to_color(char c)
{
    THROW_ASSERT_DEBUG(is_player_char(c));
    return c == 'B' ? BLACK : WHITE;
}

inline char color_to_player_char(int color)
{
    THROW_ASSERT_DEBUG(is_black_white(color));
    return color == BLACK ? 'B' : 'W';
}

inline bool is_valid_color(int color)
{
    static const char* const COLOR_TO_CHAR_TABLE =
        __color_impl::get_color_to_char_table();

    if (!(0 <= color && color < NUM_MAX_COLORS))
        return false;
    return COLOR_TO_CHAR_TABLE[(unsigned char) color] != CHAR_INVALID;
}

inline bool is_valid_char(char c)
{
    static_assert(std::numeric_limits<unsigned char>::min() == 0 &&
                  std::numeric_limits<unsigned char>::max() ==
                      NUM_MAX_COLORS - 1);

    static const int* const CHAR_TO_COLOR_TABLE =
        __color_impl::get_char_to_color_table();

    return CHAR_TO_COLOR_TABLE[(unsigned char) c] != COLOR_INVALID;
}

inline int char_to_color(char c)
{
    static_assert(std::numeric_limits<unsigned char>::min() == 0 &&
                  std::numeric_limits<unsigned char>::max() ==
                      NUM_MAX_COLORS - 1);

    static const int* const CHAR_TO_COLOR_TABLE =
        __color_impl::get_char_to_color_table();

    const int color = CHAR_TO_COLOR_TABLE[(unsigned char) c];

    THROW_ASSERT_DEBUG(color != COLOR_INVALID);

    return color;
}

inline char color_to_char(int color)
{
    static const char* const COLOR_TO_CHAR_TABLE =
        __color_impl::get_color_to_char_table();

    const char c = COLOR_TO_CHAR_TABLE[(unsigned char) color];

    THROW_ASSERT_DEBUG(c != CHAR_INVALID);

    return c;
}

inline int inverse_color(int color)
{
    THROW_ASSERT_DEBUG(is_valid_color(color));

    if (is_black_white(color))
        return opponent(color);

    return color;
}

inline char inverse_char(char c)
{
    THROW_ASSERT_DEBUG(is_valid_char(c));

    if (c == 'X' || c == 'O')
        return c == 'X' ? 'O' : 'X';

    return c;
}

inline bool is_stone_color(int color)
{
    return color != EMPTY && color != ROW_SEP && is_valid_color(color);
}

inline bool is_empty_or_stone_color(int color)
{
    return color != ROW_SEP && is_valid_color(color);
}

inline bool is_stone_char(char c)
{
    return c != '.' && c != '|' && is_valid_char(c);
}

inline bool is_empty_or_stone_char(char c)
{
    return c != '|' && is_valid_char(c);
}


//---------------------------------------------------------------------------
// Utilities for assertions

inline void assert_equal(int a, int b)
{
    assert(a == b);
}

//---------------------------------------------------------------------------
