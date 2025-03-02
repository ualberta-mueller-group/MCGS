//---------------------------------------------------------------------------
// Simple combinatorial games - switches
//---------------------------------------------------------------------------
#pragma once

#include "cgt_basics.h"
#include "cgt_integer_game.h"
#include "cgt_dyadic_rational.h"
#include "game.h"
#include <memory>

const bool ALLOW_GENERAL_SWITCHES = true;
//---------------------------------------------------------------------------

enum switch_kind
{
    SWITCH_KIND_PROPER_SWITCH = 0,
    SWITCH_KIND_RATIONAL,
    SWITCH_KIND_NUMBER_AS_SWITCH,
};

class switch_game : public game
{
public:
    switch_game(int left, int right);
    switch_game(int left, const fraction& right);
    switch_game(const fraction& left, int right);
    switch_game(const fraction& left, const fraction& right);


    void play(const move& m, bw to_play) override;
    void undo_move() override;

protected:
    split_result split_implementation() const override;

public:
    move_generator* create_move_generator(bw to_play) const override;
    game* inverse() const override;
    
    const fraction& left() const { return _left;}
    const fraction& right() const { return _right;}
    inline bool is_rational() const { return _rational_game.get() != nullptr;}
    fraction value() const
    {
        assert(is_rational());
        return _rational_game->get_fraction();
    }
    void print(std::ostream& str) const override;
    switch_kind kind() const;

private:
    relation _init_relation();

    const fraction _left, _right;
    const relation _rel;

    // nullptr if absent
    std::unique_ptr<dyadic_rational> _rational_game;
};

inline switch_game::switch_game(int left, int right) : 
    _left(left), 
    _right(right),
    _rel(_init_relation()),
    _rational_game(nullptr)
{
    assert(ALLOW_GENERAL_SWITCHES || _rel == REL_GREATER);
}

inline switch_game::switch_game(int left, const fraction& right) :
    _left(left), 
    _right(right),
    _rel(_init_relation()),
    _rational_game(nullptr)
{
    assert(ALLOW_GENERAL_SWITCHES || _rel == REL_GREATER);
}

inline switch_game::switch_game(const fraction& left, int right) :
    _left(left), 
    _right(right),
    _rel(_init_relation()),
    _rational_game(nullptr)
{
    assert(ALLOW_GENERAL_SWITCHES || _rel == REL_GREATER);
}

inline switch_game::switch_game(const fraction& left, const fraction& right) :
    _left(left), 
    _right(right),
    _rel(_init_relation()),
    _rational_game(nullptr)
{
    assert(ALLOW_GENERAL_SWITCHES || _rel == REL_GREATER);
}

//---------------------------------------------------------------------------

