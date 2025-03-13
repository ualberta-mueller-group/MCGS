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
    void play(const move& m, bw to_play);
    void undo_move();
    int value() const { return _value; }
    inline void set_value(int value)
    { 
        _value = value;
        _check_legal();
    }
    game* inverse() const;
    move_generator* create_move_generator(bw to_play) const;
    void print(std::ostream& str) const;
private:
    inline void _check_legal() const
    {
        THROW_ASSERT(!negate_will_wrap(_value));
    }

    int _value;
};

inline integer_game::integer_game(int value) : _value(value)
{
    _check_legal();
}
//---------------------------------------------------------------------------
