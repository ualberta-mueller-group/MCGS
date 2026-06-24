#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "grid.h"
// IWYU pragma: end_exports

#include <vector>
#include <string>
#include <ostream>

#include "grid_hash.h"

class clobber : public grid
{
public:
    clobber(int n_rows, int n_cols);
    clobber(const std::vector<int>& board, int_pair shape);
    clobber(const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    // Serialization
    void save_impl(i_obuffer& os, serializer_ctx* ctx) const override;
    static dyn_serializable* load_impl(i_ibuffer& is, serializer_ctx* ctx);

    bool is_move(const int& from, const int& to, bw to_play) const;

protected:
    split_result _split_impl() const override;

    void _init_hash(local_hash& hash) const override;

    mutable grid_hash _gh;

public:
    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override;
    void print_move(std::ostream& str, const move& m, ebw to_play) const override;
    game* inverse() const override; // caller takes ownership
    game* clone() const override;

    move encode_grid_move_to_db(const move& m) const override;
    move decode_grid_move_from_db(const move& m) const override;

};
