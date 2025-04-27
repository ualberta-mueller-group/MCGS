//---------------------------------------------------------------------------
// Simple combinatorial games - multiples of up, down, with/without star
//---------------------------------------------------------------------------
#pragma once

// IWYU pragma: begin_exports
#include "game.h"
// IWYU pragma: end_exports

#include "safe_arithmetic.h"
#include "throw_assert.h"

//---------------------------------------------------------------------------

class up_star : public game
{
public:
    up_star(int value, bool star);

    // m encodes the change in value and star for undo
    // m is a two part move where the second part is the star change
    // encoded as 0/1
    void play(const move& m, bw to_play) override;
    void undo_move() override;

    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    int num_ups() const { return _value; }

    bool has_star() const { return _star; }

    void print(std::ostream& str) const override;

protected:

    void _init_hash(local_hash& hash) const override;

    relation _order_impl(const game* rhs) const override;

private:
    int _value;
    bool _star;
};

inline up_star::up_star(int value, bool star) : _value(value), _star(star)
{
    THROW_ASSERT(negate_is_safe(_value));
}

//---------------------------------------------------------------------------
