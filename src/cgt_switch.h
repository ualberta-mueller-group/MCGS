//---------------------------------------------------------------------------
// Simple combinatorial games - switches
//---------------------------------------------------------------------------
#pragma once

#include "cgt_basics.h"
#include "cgt_integer_game.h"
#include "game.h"
#include <memory>

//---------------------------------------------------------------------------

class switch_game : public game
{
public:
    switch_game(int left, int right);
    void play(const move& m, bw to_play) override;
    void undo_move() override;

protected:
    split_result split_implementation() const override;

public:
    move_generator* create_move_generator(bw to_play) const override;
    game* inverse() const override;
    
    int left() const { return _left;}
    int right() const { return _right;}
    inline bool is_integer() const { return _int_game.get() != nullptr;}
    int value() const
    {
        assert(is_integer());
        return _int_game->value();
    }
    void print(std::ostream& str) const override;

private:
    const int _left, _right;

    // nullptr if absent
    std::unique_ptr<integer_game> _int_game;
};

inline switch_game::switch_game(int left, int right) : 
    _left(left), 
    _right(right),
    _int_game(nullptr)
{
    assert(left > right);
}

//---------------------------------------------------------------------------
