//---------------------------------------------------------------------------
// Impartial game - kayles
//---------------------------------------------------------------------------
//  A simple impartial game.
//  See https://en.wikipedia.org/wiki/Kayles
//---------------------------------------------------------------------------
#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "impartial_game.h"
// IWYU pragma: end_exports

#include <cassert>
#include <ostream>

class kayles : public impartial_game
{
public:
    kayles(int value);
    void _init_hash(local_hash& hash) const override;
    void play(const move& m) override;
    void play(const move& m, bw ignore_to_play) override;
    void undo_move() override;
    move_generator* create_move_generator() const override;
    void print(std::ostream& str) const override;
    game* inverse() const override;

    int value() const { return _value; }

    bool is_split() const { return _smaller_part > 0; }

    static int static_result(int n);
    static void print_kayles_move(move m, std::ostream& str);
    static move encode(int take, int smaller, int larger);
    static void init_cache();
    void set_solved(int nim_value) override;

protected:
    relation _order_impl(const game* rhs) const override;
    split_result _split_impl() const override;

private:
    static void _decode(move m, int& take, int& smaller, int& larger);
    static void _store(int n, int nim_value);
    static int _get(int n); // -1 if not stored

    int _value;
    int _smaller_part; // used temporarily during play, after splitting game
};

inline kayles::kayles(int value)
    : impartial_game(), _value(value), _smaller_part(0)
{
    assert(_value >= 0);
}

inline void kayles::play(const move& m, bw ignore_to_play)
{
    play(m);
}
