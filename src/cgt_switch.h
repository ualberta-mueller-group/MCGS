//---------------------------------------------------------------------------
// Simple combinatorial games - switches
//---------------------------------------------------------------------------
#pragma once

#include "cgt_basics.h"
#include "cgt_integer_game.h"
#include "cgt_dyadic_rational.h"
#include "game.h"
#include "safe_arithmetic.h"
#include <memory>

//---------------------------------------------------------------------------

enum switch_kind
{
    SWITCH_KIND_PROPER = 0,
    SWITCH_KIND_PROPER_NORMALIZED,
    SWITCH_KIND_RATIONAL,
    SWITCH_KIND_CONVERTIBLE_NUMBER,
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
    split_result _split_implementation() const override;

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
    fraction _init_fraction(const fraction& f) const;
    relation _init_relation() const;
    switch_kind _init_kind() const;
    inline void _check_legal() const
    {
        THROW_ASSERT(_left.is_simplified() && _right.is_simplified());
        THROW_ASSERT(!negate_will_wrap(_left.top()));
        THROW_ASSERT(!negate_will_wrap(_right.top()));
    }

    const fraction _left, _right;
    const relation _rel;
    const switch_kind _kind; // Only valid if !is_rational()

    // nullptr if absent
    std::unique_ptr<dyadic_rational> _rational_game;
    size_t _move_depth;
};

inline switch_game::switch_game(int left, int right) : 
    _left(_init_fraction(left)), 
    _right(_init_fraction(right)),
    _rel(_init_relation()),
    _kind(_init_kind()),
    _rational_game(nullptr),
    _move_depth(0)
{
    assert(_left.is_simplified() && _right.is_simplified());
}

inline switch_game::switch_game(int left, const fraction& right) :
    _left(_init_fraction(left)), 
    _right(_init_fraction(right)),
    _rel(_init_relation()),
    _kind(_init_kind()),
    _rational_game(nullptr),
    _move_depth(0)
{
    assert(_left.is_simplified() && _right.is_simplified());
}

inline switch_game::switch_game(const fraction& left, int right) :
    _left(_init_fraction(left)), 
    _right(_init_fraction(right)),
    _rel(_init_relation()),
    _kind(_init_kind()),
    _rational_game(nullptr),
    _move_depth(0)
{
    assert(_left.is_simplified() && _right.is_simplified());
}

inline switch_game::switch_game(const fraction& left, const fraction& right) :
    _left(_init_fraction(left)), 
    _right(_init_fraction(right)),
    _rel(_init_relation()),
    _kind(_init_kind()),
    _rational_game(nullptr),
    _move_depth(0)
{
    assert(_left.is_simplified() && _right.is_simplified());
}

//---------------------------------------------------------------------------

