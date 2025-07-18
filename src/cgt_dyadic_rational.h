//---------------------------------------------------------------------------
// Simple combinatorial games - dyadic rationals
//---------------------------------------------------------------------------

#pragma once

// IWYU pragma: begin_exports
#include "game.h"
// IWYU pragma: end_exports

#include "fraction.h"
#include <cassert>
#include <ostream>

class fraction;

//---------------------------------------------------------------------------

// The move encodes the old value for undo,
// since it is possible to move to the same rational from different
// starting points, e.g. from 13/8 to 12/8 = 3/2,
// and also from 7/4 to 6/4 = 3/2
class dyadic_rational : public game
{
public:
    dyadic_rational(int p, int q);
    dyadic_rational(const fraction& frac);

    fraction get_fraction() const;

    // The move argument is the OLD value that must be restored.
    void play(const move& m, bw to_play) override;
    void undo_move() override;

protected:
    split_result _split_impl() const override;

    void _init_hash(local_hash& hash) const override;

    relation _order_impl(const game* rhs) const override;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;
    void simplify();

    int p() const { return _p; }

    int q() const { return _q; }

    void print(std::ostream& str) const override;

private:
    void _check_legal() const;

    int _p, _q;
};

//---------------------------------------------------------------------------
