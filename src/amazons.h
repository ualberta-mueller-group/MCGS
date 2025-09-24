#pragma once

#include "game.h"
#include "grid.h"

#include "grid_hash.h"

#include <ostream>
#include <vector>
#include <string>

#define AMAZONS_SPLIT

////////////////////////////////////////////////// class amazons
class amazons: public grid
{
public:
    amazons(int n_rows, int n_cols);
    amazons(const std::vector<int>& board, int_pair shape);
    amazons(const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override;
    game* inverse() const override; // caller takes ownership

protected:

#ifdef AMAZONS_SPLIT
    split_result _split_impl() const override;
#endif

#ifdef USE_GRID_HASH
    void _init_hash(local_hash& hash) const override;

    mutable grid_hash _gh;
#endif
};

////////////////////////////////////////////////// amazons methods
inline void amazons::print(std::ostream& str) const
{
    str << "amazons:" << board_as_string();
}
