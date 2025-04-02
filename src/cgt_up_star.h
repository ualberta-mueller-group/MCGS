//---------------------------------------------------------------------------
// Simple combinatorial games - multiples of up, down, with/without star
//---------------------------------------------------------------------------
#pragma once

#include "cgt_basics.h"
#include "game.h"
#include "safe_arithmetic.h"
#include "throw_assert.h"

//---------------------------------------------------------------------------

class up_star : public game
{
public:
    up_star(int value, bool star);
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    int num_ups() const { return _value; }

    bool has_star() const { return _star; }

    void print(std::ostream& str) const override;

protected:
    // m encodes the change in value and star for undo
    // m is a two part move where the second part is the star change
    // encoded as 0/1
    void _play_impl(const move& m, bw to_play) override;
    void _undo_move_impl() override;

    void _init_hash(local_hash& hash) override;

    void _normalize_impl() override;
    void _undo_normalize_impl() override;

    bool _order_less_impl(const game* rhs) const override;

private:
    int _value;
    bool _star;
};

inline up_star::up_star(int value, bool star) : _value(value), _star(star)
{
    init_game_type_info<up_star>(*this);
    THROW_ASSERT(negate_is_safe(_value));
}

//---------------------------------------------------------------------------
