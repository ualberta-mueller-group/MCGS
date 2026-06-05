/*
    Takes as input a game G and a set of DB-encoded dominated moves. Generates
    moves from G which are not in this set.
*/
#pragma once

#include <memory>
#include <cassert>
#include <set>

#include "game.h"

//////////////////////////////////////////////////
// class filtering_move_generator

class filtering_move_generator : public move_generator
{
public:
    filtering_move_generator(const game& g, bw to_play,
                             const std::set<move>& db_encoded_dom_moves);

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    void _increment(bool init);

    const game& _g;

    const std::set<move>& _db_encoded_dom_moves;

    std::unique_ptr<move_generator> _gen;
    bool _has_move;
    move _m;
};

//////////////////////////////////////////////////
// filtering_move_generator methods

inline filtering_move_generator::filtering_move_generator(
    const game& g, bw to_play, const std::set<move>& db_encoded_dom_moves)
    : move_generator(to_play),
      _g(g),
      _db_encoded_dom_moves(db_encoded_dom_moves),
      _gen(g.create_move_generator(to_play)),
      _has_move(false)
{
    assert(_gen.get() != nullptr);
    _increment(true);
}

inline void filtering_move_generator::operator++()
{
    assert(*this);
    _increment(false);
}

inline filtering_move_generator::operator bool() const
{
    return _has_move;
}

inline move filtering_move_generator::gen_move() const
{
    assert(*this);
    return _m;
}
