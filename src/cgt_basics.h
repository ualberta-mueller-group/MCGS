//---------------------------------------------------------------------------
// Basic cgt and game constants and utilities
// Players, colors, ranges, outcome_class, 
// conversion color to char and back
//---------------------------------------------------------------------------

#pragma once

#include <cassert>

const int BLACK = 0;
const int WHITE = 1;
const int EMPTY = 2;
const int BORDER = 3; // outside of board

const int LEFT = BLACK;
const int RIGHT = WHITE;

typedef int bw; // black or white
typedef int ebw; // empty, black or white


/**
 The four outcome classes of a combinatorial game, plus unknown:
 P = previous player win = 2nd player win = 0
 N = next player win = first player win
 L = left wins = black wins = X wins, no matter who goes first
 R = right wins = white wins = O wins, no matter who goes first
 U = unknown - used when outcome is not computed (yet)
 */
enum outcome_class { P, N, L, R, U };

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
    return value >= low
        && value < high;
}

// range includes low but excludes high
inline void assert_range(int value, int low, int high)
{
    assert(value >= low);
    assert(value < high);
}

//---------------------------------------------------------------------------
// Mapping from colors to char, X for BORDER
const char color_code[] = {'B', 'W', '.', 'X'};

inline int char_to_color(char c)
{
    return c == 'B' ? BLACK : WHITE;
}

inline char color_char(int color)
{
    assert_range(color, 0, 4);
    return color_code[color];
}

inline bool is_black_white_char(char c)
{
    return c == 'B' || c == 'W';
}
//---------------------------------------------------------------------------
// Utilities for assertions

inline void assert_equal(int a, int b)
{ 
    assert(a == b);
}
//---------------------------------------------------------------------------
