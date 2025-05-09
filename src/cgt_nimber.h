//---------------------------------------------------------------------------
// Simple combinatorial games - nimbers
//---------------------------------------------------------------------------
#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "impartial_game.h"
// IWYU pragma: end_exports

#include <vector>
#include "cgt_basics.h"
#include "throw_assert.h"
#include <ostream>

//---------------------------------------------------------------------------

class nimber : public impartial_game
{
public:
    nimber(int value);

    // Impartial game interface
    void play(const move& m) override;
    move_generator* create_move_generator() const override;

    // value() is updated as *this changes during search
    // nim_value() is the root's value
    int value() const { return _value; }
    
    void play(const move& m, bw to_play) override;
    void undo_move() override;
    
    game* inverse() const override;
    void print(std::ostream& str) const override;
    static int nim_sum(const std::vector<int>& values);     // uses Nim formula
    static int nim_sum(const std::vector<nimber*>& values); // uses Nim formula
    static void add_nimber(int&sum, int nimber);

protected:

    void _init_hash(local_hash& hash) const override;

    relation _order_impl(const game* rhs) const override;

private:
    // _value is updated as *this changes during search
    // In contrast, impartial_game::_nim_value is for root only
    int _value;
};

inline nimber::nimber(int value) : impartial_game(), _value(value)
{
    THROW_ASSERT(value >= 0);
    set_solved(value);
}

inline game* nimber::inverse() const
{
    return new nimber(_value);
}

inline void nimber::play(const move& m)
{
    nimber::play(m, BLACK);
}

inline void nimber::add_nimber(int&sum, int nimber)
{
    sum ^= nimber;
}
