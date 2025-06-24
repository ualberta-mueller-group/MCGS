//---------------------------------------------------------------------------
// Implementation of NoGo on a 1-dimensional 1xn board
//---------------------------------------------------------------------------

#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "strip.h"
// IWYU pragma: end_exports

#include <vector>
#include <ostream>

class nogo_1xn : public strip
{
public:
    nogo_1xn(std::string game_as_string);
    nogo_1xn(const std::vector<int>& board);

    bool is_legal() const;

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    // Serialization
    void save_impl(obuffer& os) const override;
    static dyn_serializable* load_impl(ibuffer& is);

protected:
    split_result _split_impl() const override;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override
    {
        str << "nogo_1xn:" << board_as_string();
    }
};

std::ostream& operator<<(std::ostream& out, const nogo_1xn& g);
