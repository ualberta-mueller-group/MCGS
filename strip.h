//---------------------------------------------------------------------------
// A game board which is a 1-dimensional strip
// Board representation, play and remove stones
//---------------------------------------------------------------------------

#ifndef strip_H
#define strip_H

#include "cgt_basics.h"
#include "game.h"

//---------------------------------------------------------------------------

class strip : public game
{
public:
    strip(const vector<int>& board);
    strip(const std::string& game_as_string);
    int size() const;
    int at(int p) const;
    
    // is p on board, and of given color?
    bool checked_is_color(int p, int color) const;
    void play_stone(int p, int color);
    void remove_stone(int p);
    // replaces whatever is there. 
    // Less checking than play_stone or remove_stone
    void replace(int p, int color);
    vector<int> inverse_board() const;
    std::string board_as_string() const;
    void print(std::ostream& str) const { str << board_as_string();}
private:
    vector<int> _board; // todo try char as well.
};

inline int strip::size() const
{
    return _board.size();
}

inline int strip::at(int p) const
{
    assert_range(p, 0, size());
    return _board[p];
}

inline bool strip::checked_is_color(int p, int color) const
{
    return in_range(p, 0, size())
        && _board[p] == color;
}

inline void strip::play_stone(int p, int color)
{
    assert_range(p, 0, size());
    assert_black_white(color);
    assert(_board[p] == EMPTY);
    _board[p] = color;
}

inline void strip::remove_stone(int p)
{
    assert_range(p, 0, size());
    assert(_board[p] != EMPTY);
    _board[p] = EMPTY;
}

inline void strip::replace(int p, int color)
{
    assert_range(p, 0, size());
    assert_empty_black_white(color);
    _board[p] = color;
}


//---------------------------------------------------------------------------

#endif // strip_H
