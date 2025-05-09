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

/*
    TODO: Is this correct? Can it be cleaned up? Performance implications?
*/
split_result impartial_game_wrapper::_split_impl() const
{
    split_result result = split_result(std::vector<game*>());

    // Check if wrapper is solved
    if (is_solved())
    {
        assert(result->empty());
        result->push_back(new nimber(nim_value()));
        return result;
    }

    // Check if wrapped game is solved
    if (_game->is_impartial())
    {
        impartial_game* game_imp = reinterpret_cast<impartial_game*>(_game);
        assert(dynamic_cast<impartial_game*>(_game) == game_imp);

        if (game_imp->is_solved())
        {
            assert(result->empty());
            result->push_back(new nimber(game_imp->nim_value()));
            return result;
        }
    }

    // Split wrapped game
    split_result wrapped_game_result = _game->split();

    // No split happened
    if (!wrapped_game_result.has_value())
        return wrapped_game_result;

    assert(result->empty());
    /*
        Check if games in the split are solved, and ensure they're all impartial...
        Don't leak memory!

        TODO: are these only solved if they're already nimbers?
    */
    for (game* g : *wrapped_game_result)
    {
        // Must have impartial subgames
        if (!g->is_impartial())
            result->push_back(new impartial_game_wrapper(g, true));
        else
        {
            impartial_game* g_imp = reinterpret_cast<impartial_game*>(g);
            assert(dynamic_cast<impartial_game*>(g) == g_imp);

            // subgame not a nimber, but already solved
            if (                                                     //
                !(g_imp->game_type() == ::game_type<nimber>()) &&    //
                (g_imp->is_solved())                                 //
                 )                                                   //
            {
                result->push_back(new nimber(g_imp->nim_value()));
                delete g_imp;
            } else
            {
                result->push_back(g);
            }
        }
    }

    return result;
}

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
