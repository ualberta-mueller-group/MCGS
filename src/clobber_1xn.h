//---------------------------------------------------------------------------
// Implementation of Clobber on a 1-dimensional strip
//---------------------------------------------------------------------------

#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "strip.h"
// IWYU pragma: end_exports

#include <vector>
#include <ostream>

#include "iobuffer.h"

class clobber_1xn : public strip
{
public:
    clobber_1xn(const std::vector<int>& board);
    clobber_1xn(std::string game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    // Serialization
    void save_impl(obuffer& os) const override;
    static dyn_serializable* load_impl(ibuffer& is);

protected:
    split_result _split_impl() const override;

    void _normalize_impl() override;
    void _undo_normalize_impl() override;

private:
    std::vector<bool> _normalize_did_change;
    std::vector<std::vector<int>> _normalize_boards;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override
    {
        str << "clobber_1xn:" << board_as_string();
    }

    // Standard board with n consecutive "XO" pairs
    static std::string xoxo(int n);
};

std::ostream& operator<<(std::ostream& out, const clobber_1xn& g);
