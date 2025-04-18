//---------------------------------------------------------------------------
// Implementation of Clobber on a 1-dimensional strip
//---------------------------------------------------------------------------

#pragma once

#include "cgt_basics.h"
#include "strip.h"
#include <vector>

class clobber_1xn : public strip
{
public:
    clobber_1xn(const std::vector<int>& board);
    clobber_1xn(std::string game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

protected:
    split_result _split_impl() const override;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override
    {
        str << "clobber_1xn:" << board_as_string();
    }
};

std::ostream& operator<<(std::ostream& out, const clobber_1xn& g);
