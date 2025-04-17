#include "game.h"
#include "cgt_basics.h"

#include <cassert>
#include <limits>
#include <memory>
#include <typeindex>
#include <unordered_set>

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
    //     std::cout << "move "<< cgt_move::print(m) << "\n";
    //     std::cout << "move + color "<< cgt_move::print(mc) << std::endl;

    _pre_hash_update();

    _push_undo_code(GAME_UNDO_PLAY);
    _play_impl(m, to_play);

    _post_hash_update();

}

void game::undo_move()
{
    _pre_hash_update();

    _undo_move_impl();
    _pop_undo_code(GAME_UNDO_PLAY);

    _post_hash_update();

    _move_stack.pop_back();
}

hash_t game::get_local_hash()
{
    if (_hash_valid())
        return _hash.get_value();
    else
    {
        _hash.reset();
        _hash.toggle_type(game_type());
        _hash_state = HASH_STATE_OK;

        _init_hash(_hash);
        _hash_state = HASH_STATE_OK;
    }

    return _hash.get_value();
}

void game::normalize()
{
    _pre_hash_update();

    _push_undo_code(GAME_UNDO_NORMALIZE);
    _normalize_impl();

    _post_hash_update();
}

void game::undo_normalize()
{
    _pre_hash_update();

    _undo_normalize_impl();
    _pop_undo_code(GAME_UNDO_NORMALIZE);

    _post_hash_update();
}

relation game::order(const game* rhs) const
{
    assert(this != rhs); // not technically an error, but this shouldn't happen

    const game_type_t type1 = game_type();
    const game_type_t type2 = rhs->game_type();

    if (type1 != type2)
        return type1 < type2 ? REL_LESS : REL_GREATER;

    assert(type1 == type2);
    relation rel = this->_order_impl(rhs);

    assert(                       //
            rel == REL_UNKNOWN || //
            rel == REL_LESS ||    //
            rel == REL_EQUAL ||   //
            rel == REL_GREATER    //
            );                    //

    if (rel != REL_UNKNOWN)
        return rel;

    // If ordering hook isn't implemented, and stable sorting is used, don't
    // change the order of games
    return REL_EQUAL;
}

void game::_normalize_impl()
{
    // Trivial default implementation
    if(_hash_valid())
        _mark_hash_updated();
}

void game::_undo_normalize_impl()
{
    // Trivial default implementation
    if(_hash_valid())
        _mark_hash_updated();
}

relation game::_order_impl(const game* rhs) const
{
#ifndef NO_WARN_DEFAULT_IMPL
    static std::unordered_set<std::type_index> unimplemented_set;

    std::type_index tidx(typeid(*this));

    if (unimplemented_set.find(tidx) == unimplemented_set.end())
    {
        unimplemented_set.insert(tidx);

        std::cerr << "WARNING: Game type \"" << tidx.name() << "\" doesn't "
            "implement ordering hook!" << std::endl;
    }
#endif

    // Trivial default implementation
    return REL_UNKNOWN;
}

void game::_push_undo_code(game_undo_code code)
{
    _undo_code_stack.push_back(code);
}

void game::_pop_undo_code(game_undo_code code)
{
    assert(!_undo_code_stack.empty());
    assert(_undo_code_stack.back() == code);
    _undo_code_stack.pop_back();
}

void game::_pre_hash_update()
{
    if (_hash_valid())
        _hash_state = HASH_STATE_OK;
}

void game::_post_hash_update()
{
    if (_hash_state != HASH_STATE_UPDATED)
        _hash_state = HASH_STATE_INVALID;
}
