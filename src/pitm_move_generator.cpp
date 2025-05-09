#include "pitm_move_generator.h"
#include "game.h"
#include <cassert>
#include <vector>
#include <cstddef>

pitm_move_generator::pitm_move_generator(move_generator* gen, bw to_play)
    : move_generator(to_play),
    _moves(),
    _next_move(0)
{
    assert(gen != nullptr);

    std::vector<move> sequential_moves;
    while (*gen)
    {
        const move m = gen->gen_move();
        ++(*gen);

        sequential_moves.push_back(m);
    }

    const size_t move_count = sequential_moves.size();
    _moves.reserve(move_count);

    if (move_count > 0)
    {
        const size_t midpoint = (0 + (move_count - 1)) / 2;
        _moves.push_back(sequential_moves[midpoint]);

        size_t dist = 0;

        bool in_range = true;
        while (in_range)
        {
            in_range = false;
            dist++;

            const size_t idx_pos = midpoint + dist;
            const size_t idx_neg = midpoint - dist;

            // positive direction
            if (idx_pos < move_count)
            {
                in_range = true;
                _moves.push_back(sequential_moves[idx_pos]);
            }

            // Negative direction
            if (midpoint >= dist)
            {
                in_range = true;
                _moves.push_back(sequential_moves[idx_neg]);
            }
        }
    }

    delete gen;
}

void pitm_move_generator::operator++()
{
    assert(*this);
    _next_move++;
}

pitm_move_generator::operator bool() const
{
    const bool have_moves = _next_move < _moves.size();
    return have_moves;
}

move pitm_move_generator::gen_move() const
{
    assert(*this);
    return _moves[_next_move];
}


