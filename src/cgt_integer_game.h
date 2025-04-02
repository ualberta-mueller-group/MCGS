//---------------------------------------------------------------------------
// Simple combinatorial games - integers
//---------------------------------------------------------------------------
#pragma once

#include "cgt_basics.h"
#include "game.h"
#include "safe_arithmetic.h"
#include "throw_assert.h"

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

    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override;

protected:
    void _play_impl(const move& m, bw to_play) override;
    void _undo_move_impl() override;

    void _init_hash(local_hash& hash) override;

    void _normalize_impl() override;
    void _undo_normalize_impl() override;

private:
    inline void _check_legal() const { THROW_ASSERT(negate_is_safe(_value)); }

    int _value;
};

inline integer_game::integer_game(int value) : _value(value)
{
    init_game_type_info<integer_game>(*this);
    _check_legal();
}

//---------------------------------------------------------------------------
