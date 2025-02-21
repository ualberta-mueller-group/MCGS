#include "game_bounds_test.h"
#include "bounds.h"

namespace {
/*
    constructor

    setters, getters

    midpoint
    invalidate

*/

void test_constructor()
{
    game_bounds gb;

    assert(!gb.lower_valid());
    assert(!gb.upper_valid());
    assert(!gb.both_valid());
}

void test_setters_and_getters()
{
    game_bounds gb;

    assert(!gb.lower_valid());
    assert(!gb.upper_valid());
    assert(!gb.both_valid());

    gb.set_lower(-5, REL_LESS_OR_EQUAL);

    assert(gb.lower_valid());
    assert(!gb.upper_valid());
    assert(!gb.both_valid());

    gb.set_upper(5, REL_GREATER_OR_EQUAL);

    assert(gb.lower_valid());
    assert(gb.upper_valid());
    assert(gb.both_valid());

    assert(gb.get_lower() == -5);
    assert(gb.get_lower_relation() == REL_LESS_OR_EQUAL);

    assert(gb.get_upper() == 5);
    assert(gb.get_upper_relation() == REL_GREATER_OR_EQUAL);
}

void test_midpoint()
{
    game_bounds gb;
    gb.set_lower(-3, REL_LESS);
    gb.set_upper(7, REL_GREATER_OR_EQUAL);

    assert(gb.get_midpoint() == 2);
}

void test_invalidate()
{
    game_bounds gb;
    gb.set_lower(-6, REL_LESS);
    gb.set_upper(-4, REL_GREATER_OR_EQUAL);

    assert(gb.both_valid());

    gb.invalidate_lower();

    assert(!gb.lower_valid());
    assert(gb.upper_valid());
    assert(!gb.both_valid());

    gb.set_lower(-6, REL_LESS);
    gb.invalidate_upper();

    assert(gb.lower_valid());
    assert(!gb.upper_valid());
    assert(!gb.both_valid());

    gb.set_upper(-4, REL_GREATER_OR_EQUAL);

    assert(gb.both_valid());
    gb.invalidate_both();
    assert(!gb.both_valid());
}

} // namespace

void game_bounds_test_all()
{
    test_constructor();
    test_setters_and_getters();
    test_midpoint();
    test_invalidate();
}
