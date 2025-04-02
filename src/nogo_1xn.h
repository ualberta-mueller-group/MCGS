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

protected:
    split_result _split_impl() const override;
    void _play_impl(const move& m, bw to_play) override;
    void _undo_move_impl() override;

    void _init_hash(local_hash& hash) override;

    void _normalize_impl() override;
    void _undo_normalize_impl() override;

    bool _order_less_impl(const game* rhs) const override;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override
    {
        str << "nogo_1xn:" << board_as_string();
    }
};

std::ostream& operator<<(std::ostream& out, const nogo_1xn& g);
