//---------------------------------------------------------------------------
// A game board which is a 1-dimensional strip
// Board representation, play and remove stones
//---------------------------------------------------------------------------
#pragma once

// IWYU pragma: begin_exports
#include <string>
#include "game.h"
// IWYU pragma: end_exports

#include "cgt_basics.h"
#include <vector>
#include <cassert>

//---------------------------------------------------------------------------

int clobber_char_to_color(char c);
char color_to_clobber_char(int color);

class strip : public game
{
public:
    strip(const std::vector<int>& board);
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
    std::vector<int> inverse_board() const;
    std::vector<int> inverse_mirror_board() const;
    std::string board_as_string() const;
    const std::vector<int>& board_const() const;

    // void print(std::ostream& str) const { str << board_as_string();}

protected:
    void _init_hash(local_hash& hash) const override;

    void _normalize_impl() override;
    void _undo_normalize_impl() override;

    relation _order_impl(const game* rhs) const override;

    static relation _compare_boards(const std::vector<int>& board1,
                                    const std::vector<int>& board2,
                                    bool mirror1 = false, bool mirror2 = false);

    void _mirror_self();

    void _set_board(const std::vector<int>& new_board);
    void _set_board(const std::vector<int>&& new_board);

    static void _save_board(obuffer& os, const std::vector<int>& board);
    static std::vector<int> _load_board(ibuffer& is);

private:
    void _check_legal() const;

    std::vector<bool> _default_normalize_did_mirror;
    std::vector<int> _board; // todo try char as well.
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
    return in_range(p, 0, size()) //
           && _board[p] == color; //
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

inline const std::vector<int>& strip::board_const() const
{
    return _board;
}

inline void strip::_set_board(const std::vector<int>& new_board)
{
    _board = new_board;
}

inline void strip::_set_board(const std::vector<int>&& new_board)
{
    // TODO: Do I move the rvalue reference again, or do I just assign it?
    _board = std::move(new_board);
}


//---------------------------------------------------------------------------
