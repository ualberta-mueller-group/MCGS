#include "bounds2.h"
#include "cgt_up_star.h"
#include "cgt_dyadic_rational.h"
#include "clobber_1xn.h"
#include "sumgame.h"

#include <iostream>
#include <limits>
#include <cassert>
#include <memory>

using namespace std;

//////////////////////////////////////// Type declarations


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
    void _refine_bounds(vector<game*>& games, game_scale scale, game_bounds& bounds);
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

    friend void test_bounds2();
};

//////////////////////////////////////// game_bounds

game_bounds::game_bounds(): 
    _lower(numeric_limits<bound_t>::max()), _lower_valid(false), _lower_relation(COMP_GREATER_OR_EQUAL),
    _upper(numeric_limits<bound_t>::min()), _upper_valid(false), _upper_relation(COMP_LESS_OR_EQUAL)
{ }

void game_bounds::set_lower(bound_t lower, comparison_result lower_relation)
{
    assert(lower_relation == COMP_LESS
            || lower_relation == COMP_LESS_OR_EQUAL 
            || lower_relation == COMP_EQUAL
    );

    _lower = lower;
    _lower_relation = lower_relation;
    _lower_valid = true;
}

void game_bounds::set_upper(bound_t upper, comparison_result upper_relation)
{
    assert(upper_relation == COMP_GREATER
            || upper_relation == COMP_GREATER_OR_EQUAL 
            || upper_relation == COMP_EQUAL
    );

    _upper = upper;
    _upper_relation = upper_relation;
    _upper_valid = true;
}

bound_t game_bounds::get_midpoint() const
{
    assert(both_valid());

    // TODO Check for overflow?
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

ostream& operator<<(ostream& os, const game_bounds& gb)
{
    // opening brace, lower bound
    if (gb.lower_valid())
    {
        switch (gb.get_lower_relation())
        {
            case COMP_LESS_OR_EQUAL:
			{
                os << '[';
				break;
			}

            case COMP_LESS:
			{
                os << '(';
				break;
			}

            case COMP_EQUAL:
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
    } else
    {
        os << "[? ";
    }

    // upper bound, closing brace
    if (gb.upper_valid())
    {
        os << gb.get_upper();

        switch (gb.get_upper_relation())
        {
            case COMP_GREATER_OR_EQUAL:
			{
                os << ']';
				break;
			}

            case COMP_GREATER:
			{
                os << ')';
				break;
			}

            case COMP_EQUAL:
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
    } else
    {
        os << "?]";
    }

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

        //cout << "Searches: " << _search_count << endl;
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

        // TODO "relation" class?
        if (relation != COMP_GREATER_OR_EQUAL && relation != COMP_GREATER && relation != COMP_EQUAL)
        {
            return false;
        } else
        {
            // TODO game_bounds should have setter functions to handle this...
            bounds.set_low(min);
            bounds.low_relation = relation;
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

        if (relation != COMP_LESS_OR_EQUAL && relation != COMP_LESS && relation != COMP_EQUAL)
        {
            return false;
        } else
        {
            bounds.set_high(max);
            bounds.high_relation = relation;
        }
    }

    return true;
}


void bounds_finder::_refine_bounds(vector<game*>& games, game_scale scale, game_bounds& bounds)
{
    if (bounds.low_valid && bounds.low_relation == COMP_GREATER_OR_EQUAL)
    {
        // ? 0
        sumgame sum(BLACK);

        sum.add_vec(games);

        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(bounds.low, scale));
        sum.add(inverse_scale_game.get());

        bool black_first = sum.solve();
        _search_count++;

        bounds.low_relation = black_first ? COMP_GREATER : COMP_EQUAL;
    }

    if (bounds.high_valid && bounds.high_relation == COMP_LESS_OR_EQUAL)
    {
        // 0 ?
        sumgame sum(WHITE);

        sum.add_vec(games);

        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(bounds.high, scale));
        sum.add(inverse_scale_game.get());

        bool white_first = sum.solve();
        _search_count++;

        bounds.high_relation = white_first ? COMP_LESS : COMP_EQUAL;
    }

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
                break;
            }
        }

        swap(_regions, _regions_next);
    }


    _refine_bounds(games, opt.scale, *bounds);
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

    // ==
    if ((did_black && did_white) && (!black_result && !white_result))
    {
        cout << "EQ" << endl;
        return COMP_EQUAL;
    }

    // >
    if ((did_black && did_white) && (black_result && !white_result))
    {
        return COMP_GREATER;
    }

    // <
    if ((did_black && did_white) && (!black_result && white_result))
    {
        return COMP_LESS;
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
        case COMP_LESS:
        {
            region.high = scale_idx - 1;
            bounds.set_high(scale_idx);
            bounds.high_relation = relation;
            break;
        }

        case COMP_GREATER_OR_EQUAL:
        case COMP_GREATER:
        {
            region.low = scale_idx + 1;
            bounds.set_low(scale_idx);
            bounds.low_relation = relation;
            break;
        }

        case COMP_FUZZY:
        {
            _regions_next.push_back(region.split(scale_idx));
            break;
        }

        case COMP_EQUAL:
        {
            region.invalidate();
            bounds.set_low(scale_idx);
            bounds.set_high(scale_idx);
            bounds.low_relation = COMP_EQUAL;
            bounds.high_relation = COMP_EQUAL;
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




////////////////////////////////////////

void test_bounds2()
{
    vector<string> tests
    {

        ".OOXXO.X.XOXX.XXXOOXXOO.O",
"OOOXX.X.X.OOXXOOX.XX.XXOO",
"XXOXXOOXXX...O.X.OXOXXOOO",
"OO.XOXXOXXXXO.O.OXX..OXOX",
"X.XXOXOOXX.OOXXXXO..OO.OX",
"XX.OXOOXXOOXXX..XOOX.OXO.",
"XO..XXOXOOX.OXXXX.OOXXO.O",
"OXX...O.XXOXXO.OXXXXXOOOO",
"OXOXXOXOO.OOXXX.X.XX.OOX.",
"XXOX..OXXXOOOXOXX.OO.XOX.",
"O.XOOOXO..XX.XXOOXX.OXXOX",
"OXXO.XOOX..XXOOXX.OOX.XXO",
"OXOOOX.XOOX.X.OXXXXO..XXO",
"XOOXOXXOXXO.XXO..XXOXO..O",
"OOO..XOXOO.XXXXXXXOX.O.XO",
"XXX..OXOXXOXOOXXOOOX...XO",
"OOXXXXXXOO.XXXXX.OO..OO.O",
"XXOXXOOOX...OOXX.OOXX.XXO",
".OXXOOXOOXXXOXO.XXO...OXX",
"XX.O.XXX.OXOOXXXOXX.OOO.O",
"OXXOOXOOX.X.O..XOX.OXXXXO",
"X.XOO.OOOXXOX.XXXOXXO.O.X",
".OOOXXOXO.XOX.XXXXOXO.OX.",
"XOO.XO.XX.OXOO.OXX.XXOOXX",
"X.OOXOOXXX...XXXXOXO.OOOX",
"XOOXOO.X.OX.OOXXXOXX..OXX",
"XXOX.XXOOOXOXX.XO..X.OOXO",
"OOXXO.XXXXX.OXXO..OO.OXXO",
".X..XOOXOX.XOXO.XOXXXOXOO",
"X.OOOXOXOOXXX.OXXX..X.OXO",
"XOXX.OXXXOO.OXOOXOXOX.X..",
"OXXX..XXXXOOXOOXX..OOOX.O",
".XOXXX.O..OOOOOXX.XXOOXXX",
"XOXOXO..OO..XOXOXXOXXX.XO",
"O..OX.OXOXXOXXXX..OXXOXOO",
"XXOX.X..OOXOOXOOX.OX.XXXO",
"O..OO.XXOOX.XOXXXX.OXOXXO",
"O.XOO.XXOOO..OXX.XXXOOXXX",
"X.XO.OXOXOXOO.X.XOXOXX.XO",
"OXX.OOXOXOXX.X.OOXXOX.O.X",
"O.OXX.X.XOX.OOXXOOXOOX.XX",
"O.XO.XX..XXOXOO.OXXXXOOXO",
".XXXXXOOO.OX.OOO.OX.XXXOX",
"XXXXXOOOX.O.O.X.XO.XOOOXX",
"OXOOOOXX...XXXXOXO.XXO.XO",
"X.OXX.O.XOOXXXOOXOXOXXO..",
"OXX.XXO.XOO..XOOOOX.XXOXX",
".O...XOOXXXXOX.OXOOXXOOXX",
"OOX...XOXOXOXXXOX.OOX.XOX",
"XXXXO.OXOOOXOXX.OO.XXX..O",
"XXOX.XOOXXO.OOXX.OX.X.OXO",
"OX.XXOOOOOOOXX.XOXX.XXX..",
".X.XXXOXXOOXX.OOX.OX.OOXO",
"OXXOOXXO.XOX...XXOX.OXOOX",
"X.OXXOXX.OOXO.OXOXXX.X.OO",
"O.XX.O.XXOXOX.OO.XXOOXXXO",
"OO..XXXX..XXOXX.XOOOOOOXX",
"XXOOXO..XXOXO.XXXOX.OO.XO",
"XO.X.XOXXOXOX.XOXXOOXO..O",
"XOXO..XXXX.OOOOXXXOX..OOX",
"OXXXO.OXXO.OOXX...XXOOXXO",
"XO.OXOXX.XOXXXXOXO.OOOX..",
"OO..XOXXXXX.XXXO.OXOXOO.O",
"XXXO.XXOOXOXO.XO.XXO.XOO.",
"X.O.X.XX.X.OOXOOOXXOOXXOX",
".XX.OXXOOOX.XXOX.OO.XOOXX",
"XOXXXOXXOXOOX...XOO.XO.XO",
"XOXXXOOOOX.XOX.OO..XXXO.X",
"..OOXOOX..XXX.OOXOXOXXOXX",
"OXXXOXO.XOXOOX..XXXOO.O.X",
"XXOO.OXXOX.X.X.XOOXXOXO.O",
"XXX.OOXO..OOXXXXX.OXO.OOX",
"OXXOXO..OXXO.X.O.XOXOXXXO",
"O.O.OXOX..X.OXXXXOXOXXOOX",
"XO.XXOOXOOXXXXXXO.O...XOO",
"OXX.OOOO.X.X.XXXO.OXOXXXO",
"XX.O.XOXOXXOXXOO.OXXOXO..",
"XOOX.OXOXOXO..XXOO.OXXX.X",
".OO.OXXOXO.OXXXXO.XOX.XOX",
"XOX.XOXX.OXOX.OOXOXOX.X.O",



    };


    const bound_t R = 8;


    int total_searches = 0;

    /*
       Clobber tests, UP_STAR

       R = 16
       predict: 760
       assume greater: 908
       assume less: 907

       2022 Clobber: 1126
    */
    for (const string& str : tests)
    {
        vector<game*> games;

        games.push_back(new clobber_1xn(str));


        bounds_finder bf;
        vector<game_bounds*> bounds_list = bf.find_bounds(games, {{GAME_SCALE_UP_STAR, -R, R}});

        assert(bounds_list.size() == 1);
        assert(bounds_list[0]->both_valid());

        total_searches += bf._search_count;
        cout << bf._search_count << " " << *bounds_list[0] << endl;

        for (game* g : games)
        {
            delete g;
        }

        for (game_bounds* gb : bounds_list)
        {
            delete gb;
        }
    }

    cout << "Total: " << total_searches << endl;

}
