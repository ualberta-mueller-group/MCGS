//---------------------------------------------------------------------------
// Implementation of impartial_game_wrapper
//---------------------------------------------------------------------------
#include "impartial_game_wrapper.h"

#include <ostream>
#include <vector>
#include <cassert>
#include <utility>

#include "cgt_basics.h"
#include "cgt_move.h"
#include "cgt_nimber.h"
#include "game.h"
#include "global_options.h"
#include "pitm_move_generator.h"
#include "utilities.h"

const bool USE_PITM = false;

move impartial_game_wrapper::encode_grid_move_to_db(const move& m) const
{
    const bw color = cgt_move::get_color(m);

    const move m_nocolor = cgt_move::remove_color(m);
    const move m_nocolor_enc = wrapped_game()->encode_grid_move_to_db(m_nocolor);

    return cgt_move::set_color(m_nocolor_enc, color);
}

move impartial_game_wrapper::decode_grid_move_from_db(const move& m) const
{
    const bw color = cgt_move::get_color(m);

    const move m_nocolor = cgt_move::remove_color(m);
    const move m_nocolor_dec = wrapped_game()->decode_grid_move_from_db(m_nocolor);

    return cgt_move::set_color(m_nocolor_dec, color);
}

game* impartial_game_wrapper::inverse() const
{
    return new impartial_game_wrapper(wrapped_game()->inverse(), true);
}

game* impartial_game_wrapper::clone() const
{
    return new impartial_game_wrapper(wrapped_game()->clone(), true);
}

void impartial_game_wrapper::print(std::ostream& str) const
{
    str << "[impartial_game_wrapper of " << *_game << ']';
}

/*
    NOTE:
    This function assumes that the underlying game's _split_impl() doesn't
    change the available options. For example, (^+*) could split into ^ and *,
    and this would cause problems as (^+*) has different options.

    Currently none of our games do this.
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
        Check if games in the split are solved, and ensure they're all
        impartial... Don't leak memory!
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
            if (                                               //
                (g_imp->is_solved()) &&                        //
                !(g_imp->game_type() == ::game_type<nimber>()) //
                )                                              //
            {
                result->push_back(new nimber(g_imp->nim_value()));
                delete g_imp;
            }
            else
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

/*
    NOTE: _normalize_impl() and _undo_normalize_impl() both assume that the
    underlying game's implementations of these functions don't change the
    available options. This could happen if switch_games internally changed
    their values to be integers, where applicable. For example,
    {1/4 | 2} = 1, but this results in a different nim value.

    Currently none of our games do this.
*/
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
    const impartial_game_wrapper* other =
        reinterpret_cast<const impartial_game_wrapper*>(rhs);
    assert(dynamic_cast<const impartial_game_wrapper*>(rhs) == other);

    const game* g1 = _game;
    const game* g2 = other->_game;

    // This won't cause infinite recursion, as we're calling order() on the
    // wrapped games
    return g1->order(g2);
}

//---------------------------------------------------------------------------
namespace {
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
    move_generator* _mg_black;
    move_generator* _mg_white;
    move_generator* _generator;
};

ig_wrapper_move_generator::ig_wrapper_move_generator(
    const impartial_game_wrapper& wrapper)
    : move_generator(BLACK),
      _game(wrapper.wrapped_game()),
      _color(BLACK),
      _mg_black(_game->create_move_generator(BLACK)),
      _mg_white(_game->create_move_generator(WHITE)),
      _generator(nullptr)
{
    if (USE_PITM)
    {
        _mg_black = new pitm_move_generator(_mg_black, BLACK);
        _mg_white = new pitm_move_generator(_mg_white, WHITE);
    }
    _generator = _mg_black;
    _next_move(true);
}

ig_wrapper_move_generator::~ig_wrapper_move_generator()
{
    delete _mg_black;
    delete _mg_white;
}

void ig_wrapper_move_generator::_next_color()
{
    assert(_color == BLACK || _color == WHITE);
    _color = (_color == BLACK) ? WHITE : EMPTY;
    if (_color == WHITE)
        _generator = _mg_white;
}

void ig_wrapper_move_generator::_next_move(bool init)
{
    assert(_color != EMPTY);

    // Try current move generator
    if (!init)
        ++(*_generator);
    bool found_move = *_generator;

    // Try next colors
    while (!found_move && _color != EMPTY)
    {
        _next_color();
        found_move = (_color != EMPTY) && *_generator;
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

    const bool has_move = *_generator;
    assert(has_move);
    return has_move;
}

move ig_wrapper_move_generator::gen_move() const
{
    /* NOTE: impartial_game_wrapper color hack

       Here we encode the partisan game's move generator color into the color
       bit of the move. This is safe only if no other move generators do this,
       and we don't generate a move from another ig_wrapper_move_generator.
    */
    assert(operator bool());

    const move m = _generator->gen_move();
    assert(cgt_move::get_color(m) == 0);

    const bw color = _color;

    return cgt_move::set_color(m, color);
}
} // namespace


//---------------------------------------------------------------------------
namespace {
class ig_wrapper_alternating_move_generator: public move_generator
{
public:
    ig_wrapper_alternating_move_generator(const impartial_game_wrapper& wrapper);
    ~ig_wrapper_alternating_move_generator();

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    void _next_move(bool init);

    const game& _game; // the wrapped game, not the wrapper itself
    move_generator* _mg_current;
    move_generator* _mg_next;
};

ig_wrapper_alternating_move_generator::ig_wrapper_alternating_move_generator(
    const impartial_game_wrapper& wrapper)
    : move_generator(BLACK),
      _game(*wrapper.wrapped_game()),
      _mg_current(_game.create_move_generator(BLACK)),
      _mg_next(_game.create_move_generator(WHITE))
{
    if (USE_PITM)
    {
        _mg_current = new pitm_move_generator(_mg_current, BLACK);
        _mg_next = new pitm_move_generator(_mg_next, WHITE);
    }

    _next_move(true);
}

ig_wrapper_alternating_move_generator::~ig_wrapper_alternating_move_generator()
{
    if (_mg_current != nullptr)
        delete _mg_current;
    if (_mg_next != nullptr)
        delete _mg_next;
}

void ig_wrapper_alternating_move_generator::operator++()
{
    assert(*this);
    _next_move(false);
}

ig_wrapper_alternating_move_generator::operator bool() const
{
    assert(LOGICAL_IMPLIES(_mg_current == nullptr, _mg_next == nullptr));
    return _mg_current != nullptr;
}

move ig_wrapper_alternating_move_generator::gen_move() const
{
    assert(*this && *_mg_current);

    const move m = _mg_current->gen_move();
    assert(cgt_move::get_color(m) == 0);

    const bw color = _mg_current->to_play();
    return cgt_move::set_color(m, color);
}

void ig_wrapper_alternating_move_generator::_next_move(bool init)
{
    assert(init || *this);
    assert(_mg_current != nullptr);

    if (!init)
    {
        assert(*_mg_current);
        ++(*_mg_current);

        if (_mg_next != nullptr)
            std::swap(_mg_current, _mg_next);
    }

    while (1)
    {
        if (_mg_current == nullptr)
        {
            assert(_mg_next == nullptr);
            break;
        }

        if (*_mg_current)
            break;

        delete _mg_current;
        _mg_current = nullptr;

        std::swap(_mg_current, _mg_next);
    }
}

} // namespace

//---------------------------------------------------------------------------
move_generator* impartial_game_wrapper::create_move_generator() const
{
    return create_specific_move_generator(global::imp_wrapper_alternate_color());
}

move_generator* impartial_game_wrapper::create_specific_move_generator(
    bool use_alternating_version) const
{
    if (use_alternating_version)
        return new ig_wrapper_alternating_move_generator(*this);
    else
        return new ig_wrapper_move_generator(*this);
}

void impartial_game_wrapper::print_move(std::ostream& str, const move& m,
                                        ebw to_play_ignore) const
{
    assert(is_empty_black_white(to_play_ignore));
    print_move(str, m);
}

void impartial_game_wrapper::print_move(std::ostream& str, const move& m) const
{
    const bw color = cgt_move::get_color(m);
    const ::move m_no_color = cgt_move::remove_color(m);

    _game->print_move(str, m_no_color, color);
}
