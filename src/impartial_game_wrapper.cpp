//---------------------------------------------------------------------------
// Implementation of impartial_game_wrapper
//---------------------------------------------------------------------------
#include "impartial_game_wrapper.h"

#include <ostream>
#include <cassert>
#include "cgt_basics.h"
#include "cgt_nimber.h"
#include "impartial_wrapper_move.h"

using impartial_wrapper_move::encode_wrapped_move;

void impartial_game_wrapper::_init_hash(local_hash& hash) const
{
    // The local_hash passed to a game's _init_hash method 
    // already has the game's game_type_t accounted for. 
    // All impartial_game_wrapper instances have the same 
    // game_type_t, but this should be OK in this case, 
    // as the wrapped game's hash already includes its type.
    hash.toggle_value(0, _game->get_local_hash());
}

void impartial_game_wrapper::_normalize_impl()
{
    _game->normalize();
}

void impartial_game_wrapper::_undo_normalize_impl()
{
    _game->undo_normalize();
}

relation impartial_game_wrapper::_order_impl(const game* rhs) const
{
    const impartial_game_wrapper* other = reinterpret_cast<const impartial_game_wrapper*>(rhs);
    assert(dynamic_cast<const impartial_game_wrapper*>(rhs) == other);

    const game* g1 = _game;
    const game* g2 = other->_game;

    // This won't cause infinite recursion, as we're calling order() on the
    // wrapped games
    return g1->order(g2);
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
    ~ig_wrapper_move_generator();

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    void _next_color();
    void _next_move(bool init);

    const game* _game; // the wrapped game, not the wrapper itself
    ebw _color;
    move_generator* _color_mg;
};

ig_wrapper_move_generator::ig_wrapper_move_generator(
    const impartial_game_wrapper& wrapper)
    : move_generator(BLACK), _game(wrapper.wrapped_game()),
      _color(BLACK),
      _color_mg(_game->create_move_generator(BLACK))
{
    _next_move(true);
}

ig_wrapper_move_generator::~ig_wrapper_move_generator()
{
    if (_color_mg != nullptr)
        delete _color_mg;
}

void ig_wrapper_move_generator::_next_color()
{
    assert(_color == BLACK || _color == WHITE);
    assert(_color_mg != nullptr);

    delete _color_mg;
    _color_mg = nullptr;

    _color = (_color == BLACK) ? WHITE : EMPTY;

    if (_color != EMPTY)
        _color_mg = _game->create_move_generator(_color);
}

void ig_wrapper_move_generator::_next_move(bool init)
{
    assert(_color != EMPTY);
    assert(_color_mg != nullptr);
    assert(init || *_color_mg);

    // Try current move generator
    if (!init)
        ++(*_color_mg);
    bool found_move = *_color_mg;

    // Try next colors
    while (!found_move && _color != EMPTY)
    {
        _next_color();
        found_move = (_color != EMPTY) && *_color_mg;
    }
}

void ig_wrapper_move_generator::operator++()
{
    _next_move(false);
}

// Is there still a move?
ig_wrapper_move_generator::operator bool() const
{
    if (_color == EMPTY)
        return false;

    assert(_color_mg != nullptr);
    const bool has_move = *_color_mg;

    assert(has_move);
    return has_move;
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
