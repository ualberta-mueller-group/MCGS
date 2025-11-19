#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "grid.h"
// IWYU pragma: end_exports

#include <ostream>
#include <vector>
#include <cassert>
#include <string>

#include "grid_hash.h"
#include "utilities.h"

constexpr unsigned int DOMINEERING_GRID_HASH_MASK =
    GRID_HASH_ACTIVE_MASK_MIRRORS;

#define DOMINEERING_SPLIT

////////////////////////////////////////////////// class domineering
class domineering: public grid
{
public:
    domineering(int n_rows, int n_cols);
    domineering(const std::vector<int>& board, int_pair shape);
    domineering(const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override;
    void print_move(std::ostream& str, const move& m) const override;
    game* inverse() const override; // caller takes ownership

protected:

#ifdef DOMINEERING_SPLIT
    split_result _split_impl() const override;
#endif

#ifdef USE_GRID_HASH
    void _init_hash(local_hash& hash) const override;
    mutable grid_hash _gh;
#endif

};

////////////////////////////////////////////////// domineering methods
inline void domineering::print(std::ostream& str) const
{
    str << "domineering:" << board_as_string();
}
