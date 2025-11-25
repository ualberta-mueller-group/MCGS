#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "grid.h"
// IWYU pragma: end_exports

#include "grid_hash.h"

#include <ostream>
#include <vector>
#include <string>

#define AMAZONS_SPLIT

constexpr unsigned int AMAZONS_GRID_HASH_MASK = GRID_HASH_ACTIVE_MASK_ALL;

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
    void print_move(std::ostream& str, const move& m) const override;
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
