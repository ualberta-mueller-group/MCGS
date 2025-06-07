//---------------------------------------------------------------------------
// alternating_move_game - a game where both players take turns to play
// An alternating_move_game contains a game, and a player to play first
//---------------------------------------------------------------------------

#pragma once

// IWYU pragma: begin_exports
#include "game.h"
// IWYU pragma: end_exports

#include "cgt_basics.h"
#include "cgt_move.h"
#include <cassert>

class alternating_move_game
{
public:
    alternating_move_game(bw color);
    alternating_move_game(game& game, bw color);
    virtual ~alternating_move_game();

    bw to_play() const;
    bw opponent() const;
    void set_to_play(bw color);
    virtual bool solve() const;

    bool has_game_pos() const;

    game& game_pos()
    {
        has_game_pos();
        return *_game;
    }

    const game& game_pos() const
    {
        has_game_pos();
        return *_game;
    }

    virtual hash_t game_hash() const;

    // Default just returns false, a specific game may override
    virtual bool find_static_winner(bool& success) const;
    virtual void play(const move& m);
    virtual void undo_move();

private:
    bool _solve();

    game* _game;
    bw _to_play;
}; // class alternating_move_game

inline alternating_move_game::alternating_move_game(bw color)
    : _game(nullptr), _to_play(color)
{
    assert_black_white(color);
}

inline alternating_move_game::alternating_move_game(game& game, bw color)
    : _game(&game), _to_play(color)
{
    assert_black_white(color);
}

inline alternating_move_game::~alternating_move_game()
{
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

inline bool alternating_move_game::has_game_pos() const
{
    return _game != nullptr;
}

inline void alternating_move_game::play(const move& m)
{
    if (_game)
        _game->play(m, _to_play);
    _to_play = ::opponent(_to_play);
}

inline void alternating_move_game::undo_move()
{
    if (_game)
        _game->undo_move();
    _to_play = ::opponent(_to_play);
}

inline hash_t alternating_move_game::game_hash() const
{
    if (_game)
        return game_pos().get_local_hash();
    else
        return 0;
}

inline bool alternating_move_game::find_static_winner(bool& success) const
{
    return false;
}

//---------------------------------------------------------------------------
#ifdef ASSERT_RESTORE_DEBUG
class assert_restore_alternating_game
{
public:
    assert_restore_alternating_game(const alternating_move_game& game);
    virtual ~assert_restore_alternating_game();

private:
    const alternating_move_game& _game;

    const assert_restore_game* _arg;
    const bw _to_play;
    const hash_t _game_hash;
};

#else
class assert_restore_alternating_game
{
public:
    assert_restore_alternating_game(const alternating_move_game& game) {}

    virtual ~assert_restore_alternating_game() {}
};

#endif
