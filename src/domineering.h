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

////////////////////////////////////////////////// move encoding
// TODO put this elsewhere or use cgt_move functions?
inline int encode_domineering_coord(const int_pair& coord)
{
    const int& r = coord.first;
    const int& c = coord.second;

    assert(0 == (r & ~get_bit_mask_lower<int>(7)));
    assert(0 == (c & ~get_bit_mask_lower<int>(7)));

    return r | (c << 7);
}

inline int_pair decode_domineering_coord(int encoded)
{
    assert(0 == (encoded & ~get_bit_mask_lower<int>(14)));

    const int DOMINEERING_MASK = get_bit_mask_lower<int>(7);

    const int r = encoded & DOMINEERING_MASK;
    const int c = (encoded >> 7) & DOMINEERING_MASK;

    return {r, c};
}

