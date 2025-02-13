#include "bounds2.h"
#include "cgt_up_star.h"
#include "cgt_dyadic_rational.h"
#include "sumgame.h"

#include <iostream>
#include <limits>
#include <cassert>
#include <memory>

using namespace std;

//////////////////////////////////////// Type declarations

enum comparison_result 
{
    COMP_EQUAL,
    COMP_FUZZY,
    COMP_LESS_OR_EQUAL,
    COMP_GREATER_OR_EQUAL,
};

class search_region
{
public:
    search_region(bound_t low, bound_t high);
    search_region split(bound_t midpoint);

    bool valid();
    void invalidate();
    bound_t get_midpoint();

    bound_t low;
    bound_t high;
};

class bounds_finder
{
public:
    bounds_finder();


    bool _validate_range(vector<game*>& games, game_scale scale, game_bounds& bounds, bound_t min, bound_t max);
    vector<game_bounds*> find_bounds(vector<game*>& games, const vector<bounds_options>& options);

    game* get_scale_game(bound_t scale_idx, game_scale scale) const;
    game* get_inverse_scale_game(bound_t scale_idx, game_scale scale) const;

private:

    game_bounds* _make_bounds(vector<game*>& games, const bounds_options& opt);

    comparison_result _compare_to_zero(sumgame& sum, bool greater_first, int& sumgame_solve_count);
    void _step(vector<game*>& games, game_scale scale, search_region& region, game_bounds& bounds);
    void _flip_tie_break_rule();

    void _reset();


    bool _tie_break_greater_first;
    int _step_count;
    int _search_count;

    // TODO consider finding a better way to do this. Maybe this is fine though...
    vector<search_region> _regions;
    vector<search_region> _regions_next;
};

//////////////////////////////////////// game_bounds

game_bounds::game_bounds(): 
    low(numeric_limits<bound_t>::max()), low_valid(false), low_tight(false),
    high(numeric_limits<bound_t>::min()), high_valid(false), high_tight(false)
{ }

bound_t game_bounds::get_midpoint() const
{
    assert(low_valid && high_valid);

    // TODO Check for overflow?
    return (low + high) / 2;
}

void game_bounds::set_low(bound_t low)
{
    this->low = low;
    low_valid = true;
}

void game_bounds::set_high(bound_t high)
{
    this->high = high;
    high_valid = true;
}

bool game_bounds::both_valid() const
{
    return low_valid && high_valid;
}

ostream& operator<<(ostream& os, const game_bounds& gb)
{
    os << (gb.low_tight ? '(' : '[');

    if (gb.low_valid)
        os << gb.low;
    else
        os << '?';

    os << ' ';

    if (gb.high_valid)
        os << gb.high;
    else
        os << '?';

    os << (gb.high_tight ? ')' : ']');

    return os;
}

//////////////////////////////////////// bounds_options

//////////////////////////////////////// search_region

search_region::search_region(bound_t low, bound_t high): low(low), high(high)
{ }

search_region search_region::split(bound_t midpoint)
{
    // This object remains as the lower half, and returns a new search_region
    // representing the upper half

    assert(valid());
    assert(low <= midpoint);
    assert(midpoint <= high);

    bound_t old_high = high;
    high = midpoint - 1;

    return search_region(midpoint + 1, old_high);
}

bool search_region::valid()
{
    return low <= high;
}

void search_region::invalidate()
{
    low = numeric_limits<bound_t>::max();
    high = numeric_limits<bound_t>::min();
}

bound_t search_region::get_midpoint()
{
    assert(valid());

    // TODO check for overflow?
    return (low + high) / 2;
}

//////////////////////////////////////// bounds_finder

bounds_finder::bounds_finder()
{ }

vector<game_bounds*> bounds_finder::find_bounds(vector<game*>& games, const vector<bounds_options>& options)
{
    vector<game_bounds*> bounds_list;

    for (const bounds_options& opts : options)
    {
        _reset();

        assert(opts.min <= opts.max);

        game_bounds* gb = _make_bounds(games, opts);
        bounds_list.push_back(gb);

        cout << "Searches: " << _search_count << endl;
    }

    return bounds_list;
}

game* bounds_finder::get_scale_game(bound_t scale_idx, game_scale scale) const
{
    switch (scale)
    {
        case GAME_SCALE_UP_STAR:
        {
            return new up_star(scale_idx, true);
            break;
        }

        case GAME_SCALE_UP:
        {
            return new up_star(scale_idx, false);
            break;
        }

        case GAME_SCALE_DYADIC_RATIONAL:
        {
            return new dyadic_rational(scale_idx, 8);
            break;
        }

        default: 
        {
            assert(false);
            break;
        }
    }
}

game* bounds_finder::get_inverse_scale_game(bound_t scale_idx, game_scale scale) const
{
    return get_scale_game(-scale_idx, scale);
}

bool bounds_finder::_validate_range(vector<game*>& games, game_scale scale, game_bounds& bounds, bound_t min, bound_t max)
{
    if (!bounds.low_valid)
    {
        sumgame sum(BLACK);
        sum.add_vec(games);

        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(min, scale));
        sum.add(inverse_scale_game.get());
        
        int sumgame_solve_count;
        comparison_result relation = _compare_to_zero(sum, false, sumgame_solve_count);

        if (relation != COMP_GREATER_OR_EQUAL)
        {
            return false;
        }
    }

    if (!bounds.high_valid)
    {
        sumgame sum(BLACK);
        sum.add_vec(games);

        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(max, scale));
        sum.add(inverse_scale_game.get());
        
        int sumgame_solve_count;
        comparison_result relation = _compare_to_zero(sum, true, sumgame_solve_count);

        if (relation != COMP_LESS_OR_EQUAL)
        {
            return false;
        }
    }

    return true;
}



game_bounds* bounds_finder::_make_bounds(vector<game*>& games, const bounds_options& opt)
{
    game_bounds* bounds = new game_bounds();

    _regions.push_back({opt.min, opt.max});
    
    bool validated_range = false;

    while (!_regions.empty())
    {
        _regions_next.clear();

        for (search_region& sr : _regions)
        {
            if (!sr.valid())
            {
                continue;
            }

            // Do one step of binary search within the region
            _step(games, opt.scale, sr, *bounds);
        }

        if (!bounds->both_valid() && !validated_range && _step_count >= 3)
        {
            validated_range = true;

            if (!_validate_range(games, opt.scale, *bounds, opt.min, opt.max))
            {
                return bounds;
            }
        }

        swap(_regions, _regions_next);
    }

    return bounds;
}

comparison_result bounds_finder::_compare_to_zero(sumgame& sum, bool greater_first, int& sumgame_solve_count)
{
    bool did_black = false;
    bool black_result = false;

    bool did_white = false;
    bool white_result = false;

    auto test_le = [&]() -> void
    {
        // 0 ?
        // G <= 0
        assert(!did_black);
        _search_count++;

        did_black = true;

        sum.set_to_play(BLACK);
        black_result = sum.solve();
    };

    auto test_ge = [&]() -> void
    {
        // ? 0
        // G >= 0
        assert(!did_white);
        _search_count++;

        did_white = true;

        sum.set_to_play(WHITE);
        white_result = sum.solve();
    };

    auto is_conclusive = [&]() -> bool
    {
        return (did_black && !black_result) || (did_white && !white_result);
    };

    if (greater_first)
    {
        test_ge();
        if (!is_conclusive())
            test_le();
    } else
    {
        test_le();
        if (!is_conclusive())
            test_ge();
    }

    // (<= or >=) or FUZZY; No EQUAL
    assert(is_conclusive() || (did_black && did_white));

    sumgame_solve_count = (int) did_black + (int) did_white;

    // <=
    if (did_black && !black_result)
    {
        return COMP_LESS_OR_EQUAL;
    }

    // >=
    if (did_white && !white_result)
    {
        return COMP_GREATER_OR_EQUAL;
    }

    // FUZZY
    assert((did_black && black_result) && (did_white && white_result));
    return COMP_FUZZY;
}


void bounds_finder::_step(vector<game*>& games, game_scale scale, search_region& region, game_bounds& bounds)
{
    _step_count++;
    assert(region.valid());

    int scale_idx = region.get_midpoint();

    // Get sum: S - Gi
    sumgame sum(BLACK);

    // add S
    for (game* g : games)
    {
        sum.add(g);
    }

    // add -Gi
    unique_ptr<game> inverse_scale_game(get_inverse_scale_game(scale_idx, scale));
    sum.add(inverse_scale_game.get());

 
    // Pick a test (>= or <=) to try first, based on what side of the bounds space
    // "scale_idx" is on
    bool greater_first = _tie_break_greater_first;
    bool did_tie_break = true;

    int midpoint = 0;

    if (bounds.both_valid())
    {
        midpoint = bounds.get_midpoint();
    }

    if (scale_idx != midpoint)
    {
        did_tie_break = false;
        greater_first = scale_idx < midpoint;
    }


    // S - Gi compared to 0
    int sumgame_solve_count = 0;
    comparison_result relation = _compare_to_zero(sum, greater_first, sumgame_solve_count);

    if (did_tie_break && sumgame_solve_count > 1)
    {
        _flip_tie_break_rule();
    }

    switch (relation)
    {
        case COMP_LESS_OR_EQUAL:
        {
            region.high = scale_idx - 1;
            bounds.set_high(scale_idx);
            break;
        }

        case COMP_GREATER_OR_EQUAL:
        {
            region.low = scale_idx + 1;
            bounds.set_low(scale_idx);
            break;
        }

        case COMP_FUZZY:
        {
            _regions_next.push_back(region.split(scale_idx));
            break;
        }

        default:
        {
            assert(false);
            break;
        }
    };

    _regions_next.push_back(region);
}


void bounds_finder::_flip_tie_break_rule()
{
    _tie_break_greater_first = !_tie_break_greater_first;
}

void bounds_finder::_reset()
{
    _tie_break_greater_first = true;
    _step_count = 0;
    _search_count = 0;

    _regions.clear();
    _regions_next.clear();
}




vector<game_bounds*> find_bounds(vector<game*>& games, const vector<bounds_options>& options)
{
    bounds_finder bf;
    return bf.find_bounds(games, options);
}



