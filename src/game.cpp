#include "game.h"
#include "cgt_basics.h"
#include "warn_default.h"

#include <cassert>
#include <limits>
#include <memory>
#include <iostream>
#include <cstddef>

using std::unique_ptr;

std::ostream& operator<<(std::ostream& os, const split_result& split)
{

    if (!split)
    {
        os << "<NO SPLIT>";
    }
    else
    {
        const size_t N = split->size();
        assert(N < std::numeric_limits<size_t>::max());

        for (size_t i = 0; i < N; i++)
        {
            game* g = (*split)[i];

            os << *g;

            if (i + 1 < N)
            {
                os << " ";
            }
        }
    }

    return os;
}

bool game::has_moves() const
{
    unique_ptr<move_generator> gen_b =
        unique_ptr<move_generator>(create_move_generator(BLACK));

    if (*gen_b)
    {
        return true;
    }

    unique_ptr<move_generator> gen_w =
        unique_ptr<move_generator>(create_move_generator(WHITE));

    if (*gen_w)
    {
        return true;
    }

    return false;
}

void game::play(const move& m, int to_play)
{
    assert(cgt_move::get_color(m) == 0);
    const move mc = cgt_move::encode(m, to_play);
    _move_stack.push_back(mc);

    _pre_hash_update();

#ifdef GAME_UNDO_DEBUG
    __push_undo_code(GAME_UNDO_PLAY);
#endif
}

void game::undo_move()
{
    _pre_hash_update();
#ifdef GAME_UNDO_DEBUG
    __pop_undo_code(GAME_UNDO_PLAY);
#endif
    _move_stack.pop_back();
}

hash_t game::get_local_hash() const
{
    if (_hash_state == HASH_STATE_UP_TO_DATE)
        return _hash.get_value();
    else
    {
        _hash.reset();
        _hash.toggle_type(game_type());
        _hash_state = HASH_STATE_NEED_UPDATE;

        _init_hash(_hash);
        _hash_state = HASH_STATE_UP_TO_DATE;
    }

    return _hash.get_value();
}

void game::normalize()
{
    _pre_hash_update();

#ifdef GAME_UNDO_DEBUG
    __push_undo_code(GAME_UNDO_NORMALIZE);
#endif
    _normalize_impl();
}

void game::undo_normalize()
{
    _pre_hash_update();

    _undo_normalize_impl();
#ifdef GAME_UNDO_DEBUG
    __pop_undo_code(GAME_UNDO_NORMALIZE);
#endif
}

relation game::order(const game* rhs) const
{
    const game_type_t type1 = game_type();
    const game_type_t type2 = rhs->game_type();

    if (type1 != type2)
        return type1 < type2 ? REL_LESS : REL_GREATER;

    assert(type1 == type2);
    relation rel = this->_order_impl(rhs);

    assert(                   //
        rel == REL_UNKNOWN || //
        rel == REL_LESS ||    //
        rel == REL_EQUAL ||   //
        rel == REL_GREATER    //
    );                        //

    // If ordering hook isn't implemented, and stable sorting is used, don't
    // change the order of games
    if (rel == REL_UNKNOWN)
        rel = REL_EQUAL;

    assert((this != rhs) || (rel == REL_EQUAL));

    return rel;
}

void game::invalidate_hash() const
{
    _hash_state = HASH_STATE_INVALID;
    _hash.reset();
}

split_result game::_split_impl() const
{
    WARN_DEFAULT_IMPL();

    return split_result(); // no value
}

void game::_normalize_impl()
{
    WARN_DEFAULT_IMPL();

    // Trivial default implementation
    if (_hash_updatable())
        _mark_hash_updated();
}

void game::_undo_normalize_impl()
{
    WARN_DEFAULT_IMPL();

    // Trivial default implementation
    if (_hash_updatable())
        _mark_hash_updated();
}

relation game::_order_impl(const game* rhs) const
{
    WARN_DEFAULT_IMPL();

    // Trivial default implementation
    return REL_UNKNOWN;
}

#ifdef GAME_UNDO_DEBUG
inline void game::__push_undo_code(game_undo_code code)
{
    _undo_code_stack.push_back(code);
}

inline void game::__pop_undo_code(game_undo_code code)
{
    assert(!_undo_code_stack.empty());
    assert(_undo_code_stack.back() == code);
    _undo_code_stack.pop_back();
}
#endif

void game::_pre_hash_update()
{
    if (_hash_state == HASH_STATE_UP_TO_DATE)
        _hash_state = HASH_STATE_NEED_UPDATE;
    else
        _hash_state = HASH_STATE_INVALID;
}

//---------------------------------------------------------------------------

#ifdef ASSERT_RESTORE_DEBUG
assert_restore_game::assert_restore_game(const game& g)
    : _game(g),
      _local_hash(g.get_local_hash()),
      _move_stack_size(g.num_moves_played()),
      _undo_stack_size(g.undo_stack_size())
{
}

assert_restore_game::~assert_restore_game()
{
    assert(_local_hash == _game.get_local_hash());
    assert(_move_stack_size == _game.num_moves_played());
    assert(_undo_stack_size == _game.undo_stack_size());
}
#endif
