//---------------------------------------------------------------------------
// alternating_move_game - a game where both players take turns to play
// An alternating_move_game contains a game, and a player to play first
//---------------------------------------------------------------------------

#ifndef alternating_move_game_H
#define alternating_move_game_H

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"

class alternating_move_game
{
public:
    alternating_move_game(game& game, bw color);
    bw to_play() const;
    bw opponent() const;
    void set_to_play(bw color);
    virtual bool solve();
    game& game_pos() {return _game;}

    // Default just returns false, a specific game may override
    virtual bool find_static_winner(bool& success) const;
    virtual void play(const move& m);
    virtual void undo_move();
private:
    game& _game;
    bw _to_play;
}; // alternating_move_game

inline alternating_move_game::alternating_move_game(game& game, bw to_play) :
    _game(game),
    _to_play(to_play)
{
    assert_black_white(to_play);
}

inline bw alternating_move_game::to_play() const
{
    return _to_play;
}

inline bw alternating_move_game::opponent() const
{
    return ::opponent(_to_play);
}

inline void alternating_move_game::set_to_play(bw to_play)
{
    assert_black_white(to_play);
    _to_play = to_play;
}

inline void alternating_move_game::play(const move& m)
{
    _game.play(m, _to_play);
    _to_play = ::opponent(_to_play);
}

inline void alternating_move_game::undo_move()
{
    _game.undo_move();
    _to_play = ::opponent(_to_play);
}

inline bool alternating_move_game::find_static_winner(bool& success) const
{
    return false;
}

#endif // alternating_move_game_H
