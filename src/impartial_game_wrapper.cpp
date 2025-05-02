//---------------------------------------------------------------------------
// Implementation of impartial_game_wrapper
//---------------------------------------------------------------------------
#include "impartial_game_wrapper.h"

#include <ostream>
#include <string>
#include <sstream>
#include "cgt_basics.h"
#include "cgt_nimber.h"

using impartial_wrapper_move::encode_wrapped_move;

void impartial_game_wrapper::_init_hash(local_hash& hash)
{
    // The local_hash passed to a game's _init_hash method 
    // already has the game's game_type_t accounted for. 
    // All impartial_game_wrapper instances have the same 
    // game_type_t, but this should be OK in this case, 
    // as the wrapped game's hash already includes its type.
    hash.toggle_value(0, _game->get_local_hash());
}

game* impartial_game_wrapper::inverse() const
{
    return new impartial_game_wrapper(_game->inverse());
}

void impartial_game_wrapper::print(std::ostream& str) const
{
    str << "[impartial_game_wrapper of " << *_game << ']';
}

//---------------------------------------------------------------------------
class ig_wrapper_move_generator : public move_generator
{
public:
    ig_wrapper_move_generator(const impartial_game_wrapper& wrapper);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    // Not const inside, but const to outside, 
    // and called by const operator bool
    void switch_to_white() const;

    const game* _game; // the wrapped game, not the wrapper itself
    ebw _color;
    move_generator* _color_mg;
};

ig_wrapper_move_generator::ig_wrapper_move_generator(
    const impartial_game_wrapper& wrapper)
    : move_generator(BLACK), _game(wrapper.wrapped_game()),
      _color(BLACK),
      _color_mg(_game->create_move_generator(BLACK))
{ }

void ig_wrapper_move_generator::switch_to_white() const
{
    assert(_color == BLACK);
    auto mg = const_cast<ig_wrapper_move_generator*>(this);
    mg->_color = WHITE;
    mg->_color_mg = _game->create_move_generator(WHITE);
}

void ig_wrapper_move_generator::operator++()
{
    ++(*_color_mg);
    if (! (*_color_mg)) // end of moves for sub-generator
    {
        delete(_color_mg);
        if (_color == BLACK)
            switch_to_white();
        else
            _color = EMPTY;
    }
}

// Is there still a move?
// If there is no more black move, then switch to the white move generator
// and try that one.
ig_wrapper_move_generator::operator bool() const
{
    if (_color == EMPTY)
        return false;
    if ((_color == BLACK) && !(*_color_mg))
        switch_to_white();
    return (*_color_mg);
}

move ig_wrapper_move_generator::gen_move() const
{
    assert(operator bool());
    const move m = _color_mg->gen_move();
    return encode_wrapped_move(m, _color);
}

//---------------------------------------------------------------------------
move_generator* impartial_game_wrapper::create_move_generator() const
{
    return new ig_wrapper_move_generator(*this);
}
