//---------------------------------------------------------------------------
// A wrapper to turn any partizan game into an impartial one.
// The impartial game moves are the union of the black and white moves.
//
//---------------------------------------------------------------------------

#pragma once

#include <vector>
#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "impartial_game.h"

//---------------------------------------------------------------------------
class impartial_game_wrapper : public impartial_game
{
public:
    impartial_game_wrapper(game* g);

    void play(const move& m) override;
    void undo_move() override;
    move_generator* create_move_generator() const override;
    void print(std::ostream& str) const override;

    // These functions needed by game class interface
    // They also make it possible to include any 
    // impartial game in any possibly (partizan) sum
    void play(const move& m, bw ignore_to_play) override;
    move_generator* create_move_generator(bw ignore_to_play) const override;
    
    game* wrapped_game() const {return _game;}
    game* inverse() const override; // caller takes ownership

protected:
    split_result _split_implementation() const override;

private:
    game* _game;
};

inline impartial_game_wrapper::impartial_game_wrapper(game* g) :
    impartial_game(), _game(g)
{ }

inline void impartial_game_wrapper::play(const move& m)
{
    const bw color = cgt_move::get_color(m);
    const move m_no_color = cgt_move::decode(m);
    play(m_no_color, color);
}

inline void impartial_game_wrapper::play(const move& m, bw to_play)
{
    _game->play(m, to_play);
    impartial_game::play(m, to_play);
}

inline void impartial_game_wrapper::undo_move()
{
    _game->undo_move();
    impartial_game::undo_move();
}

inline move_generator* 
impartial_game_wrapper::create_move_generator(bw ignore_to_play) const
{
    return create_move_generator();
}

