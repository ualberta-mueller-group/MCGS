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
#include "iobuffer.h"

class nogo_1xn : public strip
{
public:
    nogo_1xn(std::string game_as_string);
    nogo_1xn(const std::vector<int>& board);

    // checks that all stones have liberties
    bool is_legal() const;

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
        str << "nogo_1xn:" << board_as_string();
    }
    void print_move(std::ostream& str, const move& m) const override;
};

std::ostream& operator<<(std::ostream& out, const nogo_1xn& g);
