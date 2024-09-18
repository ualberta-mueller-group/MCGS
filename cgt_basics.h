#ifndef cgt_basics_H
#define cgt_basics_H

const int BLACK = 0;
const int WHITE = 1;
const int EMPTY = 2;
const int BORDER = 3; // outside of board

const int LEFT = BLACK;
const int RIGHT = WHITE;

/**
 The four outcome classes of a combinatorial game, plus unknown:
 P = previous player win = 2nd player win = 0
 N = next player win = first player win
 L = left wins = black wins = X wins, no matter who goes first
 R = right wins = white wins = o wins, no matter who goes first
 U = unknown - used when outcome is not computed (yet)
 */
enum outcome_class { P, N, L, R, U };

//---------------------------------------------------------------------------
// Utility for colors

inline bool is_black_white(int c)
{
    return c == BLACK || c == WHITE;
}

inline void assert_black_white(int color)
{
    assert(is_black_white(color));
}

inline int opponent(int c)
{
    assert_black_white(c);
    return BLACK + WHITE - c;
}

//---------------------------------------------------------------------------
// Mapping from colors to char, X for BORDER
const char color_code[] = {'B', 'W', '.', 'X'};

inline int char_to_color(char c)
{
    return c == 'B' ? BLACK : WHITE;
}

inline bool is_black_white_char(char c)
{
    return c == 'B' || c == 'W';
}

#endif // cgt_basics_H
