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


////////////////////////////////////////////////// class domineering
class domineering: public grid
{
public:
    domineering(int n_rows, int n_cols);
    domineering(const std::vector<int>& board, int_pair shape);
    domineering(const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    // Serialization
    void save_impl(i_obuffer& os, serializer_ctx* ctx) const override;
    static dyn_serializable* load_impl(i_ibuffer& is, serializer_ctx* ctx);

    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override;
    void print_move(std::ostream& str, const move& m, ebw to_play) const override;
    game* inverse() const override; // caller takes ownership
    game* clone() const override;

    move encode_grid_move_to_db(const move& m) const override;
    move decode_grid_move_from_db(const move& m) const override;

protected:

    split_result _split_impl() const override;

    void _init_hash(local_hash& hash) const override;
    mutable grid_hash _gh;

};

////////////////////////////////////////////////// domineering methods
inline void domineering::print(std::ostream& str) const
{
    str << "domineering:" << board_as_string();
}
