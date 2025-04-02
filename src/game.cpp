#include "game.h"

#include <cassert>
#include <limits>
#include <memory>

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

//---------------------------------------------------------------------------
assert_restore_game::assert_restore_game(const game& g)
    : _game(g), _game_hash(g.game_hash())
{
}

assert_restore_game::~assert_restore_game()
{
    assert_equal(_game_hash, _game.game_hash());
}

void game::play(const move& m, int to_play)
{
    assert(cgt_move::get_color(m) == 0);
    const move mc = cgt_move::encode(m, to_play);
    _move_stack.push_back(mc);
    //     std::cout << "move "<< cgt_move::print(m) << "\n";
    //     std::cout << "move + color "<< cgt_move::print(mc) << std::endl;

    if (_hash_valid())
        _hash_state = HASH_STATE_OK;

    _play_impl(m, to_play);
    _push_undo_code(GAME_UNDO_PLAY);

    if (_hash_state == HASH_STATE_OK) // not updated
        _hash_state = HASH_STATE_INVALID;
}

void game::undo_move()
{
    if (_hash_valid())
        _hash_state = HASH_STATE_OK;

    _undo_move_impl();
    _pop_undo_code(GAME_UNDO_PLAY);

    if (_hash_state == HASH_STATE_OK) // not updated
        _hash_state = HASH_STATE_INVALID;

    _move_stack.pop_back();
}

const local_hash& game::compute_hash()
{
    if (_hash_valid())
        return _hash;
    else
    {
        _hash.reset();
        _hash.toggle_type(game_type());
        _hash_state = HASH_STATE_OK;

        _init_hash(_hash);
        _hash_state = HASH_STATE_OK;
    }

    return _hash;
}

void game::normalize()
{
    _push_undo_code(GAME_UNDO_NORMALIZE);
    _normalize_impl();
}

void game::undo_normalize()
{
    _pop_undo_code(GAME_UNDO_NORMALIZE);
    _undo_normalize_impl();
}


bool game::order_less(const game* rhs) const
{
    const game_type_t type1 = game_type();
    const game_type_t type2 = rhs->game_type();

    if (type1 != type2)
        return type1 < type2;

    assert(type1 == type2);
    return this->_order_less_impl(rhs);
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
