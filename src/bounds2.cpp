#include "bounds2.h"
#include "clobber_1xn.h"
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
    search_region(int low, int high);
    search_region split(int midpoint);

    bool valid();
    void invalidate();
    int get_midpoint();

    int low;
    int high;
};


class bounds_finder
{
public:
    bounds_finder();

    vector<game_bounds*> find_bounds(vector<game*>& games, const vector<game_scale>& scales);

    game* get_scale_game(int scale_idx, game_scale scale);
    game* get_inverse_scale_game(int scale_idx, game_scale scale);

private:
    void _reset();

    game_bounds* _make_bounds(vector<game*>& games, game_scale scale);

    comparison_result _compare_to_zero(sumgame& sum, bool assume_greater, int* sumgame_solve_count = nullptr);
    void _step(search_region& region, game_bounds* bounds, vector<game*>& games, game_scale scale);
    void _invert_tie_break_direction();

    // TODO search index type; can I hide it in the .cpp file?


    int _tie_break_direction;
    int _step_count;
    int _search_count;

    // TODO consider finding a better way to do this. Maybe this is fine though...
    vector<search_region> _regions;
    vector<search_region> _regions_next;
};






//////////////////////////////////////// game_bounds

game_bounds::game_bounds(): 
    low(numeric_limits<int>::max()), low_valid(false), low_tight(false),
    high(numeric_limits<int>::min()), high_valid(false), high_tight(false)
{ }


int game_bounds::get_midpoint()
{
    assert(low_valid && high_valid);
    return (low + high) / 2;
}

void game_bounds::set_low(int low)
{
    this->low = low;
    low_valid = true;
}

void game_bounds::set_high(int high)
{
    this->high = high;
    high_valid = true;
}


bool game_bounds::both_valid()
{
    return low_valid && high_valid;
}


//////////////////////////////////////// search_region


search_region::search_region(int low, int high): low(low), high(high)
{ }


search_region search_region::split(int midpoint)
{
    // This object remains as the lower half, and returns a new search_region
    // representing the upper half

    assert(valid());
    assert(low <= midpoint);
    assert(midpoint <= high);

    int old_high = high;
    high = midpoint - 1;

    return search_region(midpoint + 1, old_high);
}

bool search_region::valid()
{
    return low <= high;
}

void search_region::invalidate()
{
    low = numeric_limits<int>::max();
    high = numeric_limits<int>::min();
}

int search_region::get_midpoint()
{
    assert(valid());
    return (low + high) / 2;
}

//////////////////////////////////////// bounds_finder

bounds_finder::bounds_finder()
{ }

vector<game_bounds*> bounds_finder::find_bounds(vector<game*>& games, const vector<game_scale>& scales)
{
    vector<game_bounds*> bounds_list;

    for (const game_scale& scale : scales)
    {
        _reset();

        game_bounds* gb = _make_bounds(games, scale);
        bounds_list.push_back(gb);
    }

    return bounds_list;
}

game* bounds_finder::get_scale_game(int scale_idx, game_scale scale)
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

game* bounds_finder::get_inverse_scale_game(int scale_idx, game_scale scale)
{
    return get_scale_game(-scale_idx, scale);
}

void bounds_finder::_reset()
{
    _tie_break_direction = -1;
    _step_count = 0;
    _search_count = 0;

    _regions.clear();
    _regions_next.clear();
}

game_bounds* bounds_finder::_make_bounds(vector<game*>& games, game_scale scale)
{
    game_bounds* bounds = new game_bounds();


    _regions.push_back({BOUND_MIN, BOUND_MAX});

    while (!_regions.empty())
    {
        _regions_next.clear();

        for (search_region &sr : _regions)
        {
            if (!sr.valid())
            {
                continue;
            }

            // Do one step of binary search within the region
            _step(sr, bounds, games, scale);
        }

        swap(_regions, _regions_next);
    }




    return bounds;
}


comparison_result bounds_finder::_compare_to_zero(sumgame& sum, bool assume_greater, int* sumgame_solve_count)
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

    if (assume_greater)
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

    if (sumgame_solve_count != nullptr)
    {
        *sumgame_solve_count = (int) did_black + (int) did_white;
    }

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

    assert((did_black && black_result) && (did_white && white_result));
    return COMP_FUZZY;
}


void bounds_finder::_step(search_region& region, game_bounds* bounds, vector<game*>& games, game_scale scale)
{
    _step_count++;
    assert(region.valid());


    int idx = region.get_midpoint();

    // Get sum: S - Gi
    unique_ptr<game> inverse_scale_game(get_inverse_scale_game(idx, scale));

    sumgame sum(BLACK);

    for (game* g : games)
    {
        sum.add(g);
    }

    sum.add(inverse_scale_game.get());

 
    // What side of the bounds are we on?
    int bounds_side = _tie_break_direction;
    bool did_tie_break = true;

    int midpoint = 0;

    if (bounds->both_valid())
    {
        midpoint = bounds->get_midpoint();
    }

    if (idx != midpoint)
    {
        did_tie_break = false;
        bounds_side = idx < midpoint ? -1 : 1;
    }

    assert(bounds_side == -1 || bounds_side == 1);

    bool assume_greater = bounds_side < 0;

    // S - Gi compared to 0
    int sumgame_solve_count = 0;
    comparison_result relation = _compare_to_zero(sum, assume_greater, &sumgame_solve_count);

    if (did_tie_break && sumgame_solve_count > 1)
    {
        _invert_tie_break_direction();
    }

    switch (relation)
    {
        case COMP_LESS_OR_EQUAL:
        {
            region.high = idx - 1;
            bounds->set_high(idx);
            break;
        }

        case COMP_GREATER_OR_EQUAL:
        {
            region.low = idx + 1;
            bounds->set_low(idx);
            break;
        }

        case COMP_FUZZY:
        {
            _regions_next.push_back(region.split(idx));
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


void bounds_finder::_invert_tie_break_direction()
{
    _tie_break_direction *= -1;
}

ostream& operator<<(ostream& os, const game_bounds& gb)
{
    os << (gb.low_tight ? '(' : '[');
    os << (gb.low_valid ? gb.low : '?');

    os << ' ';

    os << (gb.high_valid ? gb.high : '?');
    os << (gb.high_tight ? ')' : ']');

    return os;
}


vector<game_bounds*> find_bounds(vector<game*>& games, const vector<game_scale>& scales)
{
    bounds_finder bf;
    return bf.find_bounds(games, scales);
}

