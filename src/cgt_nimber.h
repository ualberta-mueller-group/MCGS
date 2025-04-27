//---------------------------------------------------------------------------
// Simple combinatorial games - nimbers
//---------------------------------------------------------------------------
#pragma once

// IWYU pragma: begin_exports
#include "game.h"
// IWYU pragma: end_exports

#include <vector>
#include "cgt_basics.h"
#include "throw_assert.h"

//---------------------------------------------------------------------------

class nimber : public game
{
public:
    nimber(int value);
    move_generator* create_move_generator(bw to_play) const override;

    int value() const { return _value; }

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    game* inverse() const override;
    void print(std::ostream& str) const override;
    static int nim_sum(const std::vector<int>& values);     // uses Nim formula
    static int nim_sum(const std::vector<nimber*>& values); // uses Nim formula

protected:

    void _init_hash(local_hash& hash) const override;

    relation _order_impl(const game* rhs) const override;

private:
    int _value;
};

inline game* nimber::inverse() const
{
    return new nimber(_value);
}

inline nimber::nimber(int value) : _value(value)
{
    THROW_ASSERT(_value >= 0);
}

//---------------------------------------------------------------------------
