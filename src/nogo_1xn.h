//---------------------------------------------------------------------------
// Implementation of NoGo on a 1-dimensional 1xn board
//---------------------------------------------------------------------------

#pragma once

#include "cgt_basics.h"
#include "strip.h"
#include <vector>

class nogo_1xn : public strip
{
public:
    nogo_1xn(std::string game_as_string);
    nogo_1xn(const std::vector<int>& board);
    void play(const move& m, bw to_play) override;
    void undo_move() override;

protected:
    split_result _split_implementation() const override;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override
    {
        str << "nogo_1xn:" << board_as_string();
    }
};

std::ostream& operator<<(std::ostream& out, const nogo_1xn& g);
