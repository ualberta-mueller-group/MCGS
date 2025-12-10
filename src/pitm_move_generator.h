/*
    Wrapper for move_generator. Implements play-in-the-middle heuristic.

    Generates all moves from the move_generator during construction, 
    stores them as a list,
    then pre-computes the pitm-list from the middle of the move list 
    outwards towards the ends.
*/
#pragma once
#include <cstddef>
#include <vector>
#include "game.h"

class pitm_move_generator : public move_generator
{
public:
    // assumes ownership of gen.
    pitm_move_generator(move_generator* gen, bw to_play);

    virtual ~pitm_move_generator() {}

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    std::vector<move> _moves;
    size_t _next_move;
};
