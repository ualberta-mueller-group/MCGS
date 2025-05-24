#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "grid.h"
// IWYU pragma: end_exports

#include <vector>
#include <string>
#include <ostream>

class clobber : public grid
{
public:
    clobber(int n_rows, int n_cols);
    clobber(const std::vector<int>& board, int_pair shape);
    clobber(const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    bool is_move(const int& from, const int& to, bw to_play) const;

protected:
    // Disabled for now, too slow...
#ifdef CLOBBER_SPLIT
    split_result _split_impl() const override;
#endif

public:
    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override;
    game* inverse() const override; // caller takes ownership
};
