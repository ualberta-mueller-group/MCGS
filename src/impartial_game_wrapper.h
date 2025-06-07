//---------------------------------------------------------------------------
// A wrapper to turn any partizan game into an impartial one.
// The impartial game moves are the union of the black and white moves.
//
//---------------------------------------------------------------------------

#pragma once

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "impartial_game.h"
#include <ostream>
#include <cassert>

//---------------------------------------------------------------------------
class impartial_game_wrapper : public impartial_game
{
public:
    impartial_game_wrapper(game* g); // game still owned by caller
    impartial_game_wrapper(game* g,
                           bool owns_game); // if true, game is owned by callee

    ~impartial_game_wrapper();

    void play(const move& m) override;
    void undo_move() override;
    move_generator* create_move_generator() const override;
    void print(std::ostream& str) const override;

    // These functions needed by game class interface
    // They also make it possible to include any
    // impartial game in any possibly (partizan) sum
    void play(const move& m, bw ignore_to_play) override;
    move_generator* create_move_generator(bw ignore_to_play) const override;

    game* wrapped_game() const { return _game; }

    game* inverse() const override; // caller takes ownership

protected:
    split_result _split_impl() const override; // See note in .cpp file
    void _init_hash(local_hash& hash) const override;

    void _normalize_impl() override;
    void _undo_normalize_impl() override;

    relation _order_impl(const game* rhs) const override;

private:
    game* _game;
    const bool _owns_game;
};

inline impartial_game_wrapper::impartial_game_wrapper(game* g)
    : impartial_game(), _game(g), _owns_game(false)
{
    /*
       Wrapping an impartial game is bad for performance, and is unsafe if
       the wrapped game is another impartial_game_wrapper, due to
       ig_wrapper_move_generator using the color bit of moves.

       This won't cause local hashes to cancel out, however
    */
    assert(!g->is_impartial());
}

inline impartial_game_wrapper::impartial_game_wrapper(game* g, bool owns_game)
    : impartial_game(), _game(g), _owns_game(owns_game)
{
    /*
       Wrapping an impartial game is bad for performance, and is unsafe if
       the wrapped game is another impartial_game_wrapper, due to
       ig_wrapper_move_generator using the color bit of moves.

       This won't cause local hashes to cancel out, however
    */
    assert(!g->is_impartial());
}

inline impartial_game_wrapper::~impartial_game_wrapper()
{
    if (_owns_game)
        delete _game;
}

inline void impartial_game_wrapper::play(const move& m)
{
    /* NOTE: impartial_game_wrapper color hack

       m's color bit is used to encode the color of the partizan game's
       move generator. We must preserve this information when calling other
       play() functions.
    */
    const bw color = cgt_move::get_color(m);
    const move m_no_color = cgt_move::decode(m);

    _game->play(m_no_color, color);
    impartial_game::play(m);
    assert(_game->last_move() == last_move());
}

inline void impartial_game_wrapper::play(const move& m, bw to_play)
{
    /* NOTE: impartial_game_wrapper color hack

    - to_play is the current minimax search player, and can be discarded.
    - m's color bit corresponds to the partizan game's move generator color,
        and must be preserved.
    */

    play(m);
}

inline void impartial_game_wrapper::undo_move()
{
    _game->undo_move();
    impartial_game::undo_move();
}

inline move_generator* impartial_game_wrapper::create_move_generator(
    bw ignore_to_play) const
{
    return create_move_generator();
}
