#include "bounds.h"

#include <iostream>
#include <limits>
#include <cassert>
#include <vector>
#include <string>

#include "cgt_basics.h"
#include "cgt_up_star.h"
#include "cgt_dyadic_rational.h"
#include "integral_conversion.h"
#include "sumgame.h"
#include "throw_assert.h"
#include "safe_arithmetic.h"
#include "bounds_finder.h"

using namespace std;

std::string bound_scale_to_string(bound_scale scale)
{
    switch (scale)
    {
        case BOUND_SCALE_UP_STAR:
            return "up_star";
        case BOUND_SCALE_UP:
            return "up";
        case BOUND_SCALE_DYADIC_RATIONAL:
            return "dyadic_rational";
    }

    assert(false);
}

bool scale_is_infinitesimal(bound_scale scale)
{
    switch (scale)
    {
        case BOUND_SCALE_UP_STAR:
        case BOUND_SCALE_UP:
            return true;
        case BOUND_SCALE_DYADIC_RATIONAL:
            return false;
    }

    assert(false);
}

//////////////////////////////////////// bound_scale functions
game* get_scale_game(bound_t scale_idx, bound_scale scale)
{
    const int scale_idx_int = integral_cast_checked<int>(scale_idx);

    switch (scale)
    {
        case BOUND_SCALE_UP_STAR:
        {
            return new up_star(scale_idx_int, true);
            break;
        }

        case BOUND_SCALE_UP:
        {
            return new up_star(scale_idx_int, false);
            break;
        }

        case BOUND_SCALE_DYADIC_RATIONAL:
        {
            return new dyadic_rational(scale_idx_int, 8);
            break;
        }

        default:
        {
            assert(false);
            return nullptr;
            break;
        }
    }
}

game* get_inverse_scale_game(bound_t scale_idx, bound_scale scale)
{
    THROW_ASSERT(negate_is_safe(scale_idx));
    return get_scale_game(-scale_idx, scale);
}

//////////////////////////////////////// game_bounds
game_bounds::game_bounds(bound_scale scale)
    : _scale(scale),
      _lower(numeric_limits<bound_t>::max()),
      _lower_valid(false),
      _lower_relation(REL_UNKNOWN),
      _upper(numeric_limits<bound_t>::min()),
      _upper_valid(false),
      _upper_relation(REL_UNKNOWN)
{
}

void game_bounds::set_lower(bound_t lower, relation lower_relation)
{
    assert(lower_relation == REL_LESS || lower_relation == REL_LESS_OR_EQUAL ||
           lower_relation == REL_EQUAL);

    if (lower_valid())
        assert(lower >= _lower);

    _set_lower(lower, lower_relation);

    if (lower_relation == REL_EQUAL)
        _set_upper(lower, REL_EQUAL);

    if (upper_valid())
        assert(_lower <= _upper);
}

void game_bounds::set_upper(bound_t upper, relation upper_relation)
{
    assert(upper_relation == REL_GREATER ||
           upper_relation == REL_GREATER_OR_EQUAL ||
           upper_relation == REL_EQUAL);

    if (upper_valid())
        assert(upper <= _upper);

    _set_upper(upper, upper_relation);

    if (upper_relation == REL_EQUAL)
        _set_lower(upper, REL_EQUAL);

    if (lower_valid())
        assert(_lower <= _upper);
}

void game_bounds::set_equal(bound_t lower_and_upper)
{
    _set_lower(lower_and_upper, REL_EQUAL);
    _set_upper(lower_and_upper, REL_EQUAL);

    assert(_lower_valid && _upper_valid);
    assert(_lower_relation == REL_EQUAL && _upper_relation == REL_EQUAL);
    assert(_lower == _upper);
}

bound_t game_bounds::get_midpoint() const
{
    assert(both_valid());

    THROW_ASSERT(add_is_safe(_lower, _upper));
    return (_lower + _upper) / 2;
}

void game_bounds::invalidate_lower()
{
    _lower_valid = false;
}

void game_bounds::invalidate_upper()
{
    _upper_valid = false;
}

void game_bounds::invalidate_both()
{
    _lower_valid = false;
    _upper_valid = false;
}

void game_bounds::_set_lower(bound_t lower, relation lower_relation)
{
    _lower = lower;
    _lower_relation = lower_relation;
    _lower_valid = true;
}

void game_bounds::_set_upper(bound_t upper, relation upper_relation)
{
    _upper = upper;
    _upper_relation = upper_relation;
    _upper_valid = true;
}

ostream& operator<<(ostream& os, const game_bounds& gb)
{
    os << bound_scale_to_string(gb.get_scale()) << ": ";

    // opening brace, lower bound
    if (gb.lower_valid())
    {
        switch (gb.get_lower_relation())
        {
            case REL_LESS_OR_EQUAL:
            {
                os << '[';
                break;
            }

            case REL_LESS:
            {
                os << '(';
                break;
            }

            case REL_EQUAL:
            {
                os << "==";
                break;
            }

            default:
            {
                assert(false);
                os << "[?!";
                break;
            }
        }
        os << gb.get_lower() << " ";
    }
    else
    {
        os << "[? ";
    }

    // upper bound, closing brace
    if (gb.upper_valid())
    {
        os << gb.get_upper();

        switch (gb.get_upper_relation())
        {
            case REL_GREATER_OR_EQUAL:
            {
                os << ']';
                break;
            }

            case REL_GREATER:
            {
                os << ')';
                break;
            }

            case REL_EQUAL:
            {
                os << "==";
                break;
            }

            default:
            {
                assert(false);
                os << "?!]";
                break;
            }
        }
    }
    else
    {
        os << "?]";
    }

    return os;
}

////////////////////////////////////////
vector<game_bounds_ptr> find_bounds(sumgame& sum,
                                    const vector<bounds_options>& options)
{
    assert_restore_sumgame ars(sum);
    bw old_to_play = sum.to_play();

    bounds_finder bf;
    vector<game_bounds_ptr> bounds_list = bf.find_bounds(sum, options);

    sum.set_to_play(old_to_play);
    return bounds_list;
}

vector<game_bounds_ptr> find_bounds(vector<game*>& games,
                                    const vector<bounds_options>& options)
{
    sumgame sum(BLACK);
    sum.add(games);
    assert_restore_sumgame ars(sum);
    return find_bounds(sum, options);
}

vector<game_bounds_ptr> find_bounds(game* game,
                                    const vector<bounds_options>& options)
{
    assert(game != nullptr);
    sumgame sum(BLACK);
    sum.add(game);
    assert_restore_game arg(*game);
    return find_bounds(sum, options);
}
