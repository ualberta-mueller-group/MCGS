//---------------------------------------------------------------------------
// Implementation of NoGo on a 2-dimensional rectangular board
//---------------------------------------------------------------------------

#pragma once

#include "cgt_basics.h"
#include "grid.h"
#include <vector>

class nogo : public grid
{
public:
    nogo(std::string game_as_string);
    nogo(const std::vector<int>& board, int_pair shape);
    void play(const move& m, bw to_play) override;
    void undo_move() override;

protected:
    split_result _split_implementation() const override;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override
    {
        str << "nogo:" << board_as_string();
    }
};

std::ostream& operator<<(std::ostream& out, const nogo& g);
