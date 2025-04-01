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

    if (_hash_state == HASH_STATE_OK) // not updated
        _hash_state = HASH_STATE_INVALID;
}

void game::undo_move()
{
    if (_hash_valid())
        _hash_state = HASH_STATE_OK;

    _undo_move_impl();

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

