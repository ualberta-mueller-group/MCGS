//---------------------------------------------------------------------------
// Simple combinatorial games - integers
//---------------------------------------------------------------------------
#pragma once

// IWYU pragma: begin_exports
#include "game.h"
// IWYU pragma: end_exports

#include "safe_arithmetic.h"
#include "throw_assert.h"
#include <ostream>

// This is needed for undo_move in the case when _value has reached 0
class integer_game : public game
{
public:
    integer_game(int value);

    int value() const { return _value; }

    inline void set_value(int value)
    {
        _value = value;
        _check_legal();
    }

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override;

protected:
    void _init_hash(local_hash& hash) const override;

    relation _order_impl(const game* rhs) const override;

private:
    inline void _check_legal() const { THROW_ASSERT(negate_is_safe(_value)); }

    int _value;
};

inline integer_game::integer_game(int value) : _value(value)
{
    _check_legal();
}

//---------------------------------------------------------------------------
