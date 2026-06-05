/*
    Takes as input a game and a vector of DB-encoded moves. Generates these
    moves after decoding them.
*/
#pragma once

#include <vector>
#include <cstddef>
#include <cassert>

#include "game.h"


////////////////////////////////////////////////// class db_move_generator
class db_move_generator: public move_generator
{
public:
    db_move_generator(const game& g, bw to_play,
                      const std::vector<move>& db_encoded_moves);

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    const game& _g;

    const std::vector<move>& _db_encoded_moves;
    size_t _move_idx;
};

////////////////////////////////////////////////// db_move_generator methods

inline db_move_generator::db_move_generator(const game& g, bw to_play,
                                     const std::vector<move>& db_encoded_moves)
    : move_generator(to_play),
      _g(g),
      _db_encoded_moves(db_encoded_moves),
      _move_idx(0)
{
}

inline void db_move_generator::operator++()
{
    assert(*this);
    _move_idx++;
}

inline db_move_generator::operator bool() const
{
    return _move_idx < _db_encoded_moves.size();
}

inline move db_move_generator::gen_move() const
{
    assert(*this);
    const move move_enc = _db_encoded_moves[_move_idx];

    return _g.decode_grid_move_from_db(move_enc);
}
