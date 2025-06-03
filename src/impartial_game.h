//---------------------------------------------------------------------------
// Implementation of impartial games,
// and algorithms to solve them.
// See `development-notes.md` impartial games section for details
//---------------------------------------------------------------------------
#pragma once

#include <set>
#include <cassert>
#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "transposition.h"

//---------------------------------------------------------------------------
// Transposition table for impartial games
//---------------------------------------------------------------------------
struct impartial_ttable_entry
{
    int nim_value;

    impartial_ttable_entry() : nim_value(0) {}

    impartial_ttable_entry(int v) : nim_value(v) {}
};

typedef ttable<impartial_ttable_entry> impartial_tt;

//---------------------------------------------------------------------------
class impartial_game : public game
{
public:
    impartial_game();
    // solve single game; store result in _nim_value
    // tt_size is expressed as exponent of 2, e.g. 24 means 2^24
    int search_with_tt(int tt_size = 24) const;
    int search_impartial_game(impartial_tt& tt) const;

    /*
        TODO: this method should probably be hidden somehow; it's an
        implementation detail and is used by
        search_impartial_sumgame_with_timeout (impartial_sumgame.cpp)
    */
    int search_impartial_game_cancellable(impartial_tt& tt,
                                          const bool& over_time) const;

    // Impartial game interface
    virtual void play(const move& m);
    virtual move_generator* create_move_generator() const = 0;

    // These functions needed by game class interface
    // They also make it possible to include an
    // impartial game in any (possibly partizan) sum
    void play(const move& m, bw to_play) override;
    move_generator* create_move_generator(bw ignore_to_play) const override;

    bool is_impartial() const override final;

    bool is_solved() const;
    int nim_value() const; // available after it is solved
    virtual void set_solved(int nim_value);

    // Minimum excluded number
    static int mex(const std::set<int>& values);

private:
    using game::play; // avoid compiler warning
    bool _root_is_solved;
    int _nim_value;
};

inline impartial_game::impartial_game() : _root_is_solved(false), _nim_value(0)
{
}

inline bool impartial_game::is_impartial() const
{
    return true;
}

inline bool impartial_game::is_solved() const
{
    return (num_moves_played() == 0) && _root_is_solved;
}

inline int impartial_game::nim_value() const
{
    assert(num_moves_played() == 0);
    assert(_root_is_solved);
    return _nim_value;
}

inline void impartial_game::play(const move& m)
{
    /* NOTE: impartial_game_wrapper color hack

       Preserves the color bit of m, because ig_wrapper_move_generator
       may use it. No other games should do this.
    */
    const move m_no_color = cgt_move::decode(m);
    const bw color = cgt_move::get_color(m);
    game::play(m_no_color, color);
    assert(last_move() == m);
}

inline void impartial_game::play(const move& m, bw to_play)
{
    /* NOTE: impartial_game_wrapper color hack

    - to_play is the current minimax search player, and should be discarded.
    - The color bit of m may be used by ig_wrapper_move_generator, therefore
        it must be preserved.
    */
    impartial_game::play(m);
}

inline move_generator* impartial_game::create_move_generator(
    bw ignore_to_play) const
{
    return create_move_generator();
}
