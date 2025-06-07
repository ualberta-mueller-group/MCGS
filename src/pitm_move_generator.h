#pragma once
#include "game.h"
#include <vector>
#include <cstddef>

class pitm_move_generator : public move_generator
{
public:
    pitm_move_generator(move_generator* gen, bw to_play);

    virtual ~pitm_move_generator() {}

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    std::vector<move> _moves;
    size_t _next_move;
};
