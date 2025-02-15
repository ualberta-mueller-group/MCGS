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


    bool _validate_range(bound_t min, bound_t max,game_scale scale, sumgame& sum, game_bounds& bounds);
    vector<game_bounds*> find_bounds(sumgame& sum, const vector<bounds_options>& options);

    game* get_scale_game(bound_t scale_idx, game_scale scale) const;
    game* get_inverse_scale_game(bound_t scale_idx, game_scale scale) const;

private:
    void _refine_bounds(game_scale scale, game_bounds& bounds, sumgame& sum);

    game_bounds* _make_bounds(sumgame& sum, const bounds_options& opt);


    bool _is_less_or_equal_scale(sumgame& sum, game* inverse_scale_game);
    bool _is_greater_or_equal_scale(sumgame& sum, game* inverse_scale_game);




    comparison_result _get_step_comparison(sumgame& sum, game* inverse_scale_game, bool below_midpoint, int* solve_count);

    //void _step(vector<game*>& games, game_scale scale, search_region& region, game_bounds& bounds);
    void _step(search_region& region, game_scale scale, sumgame& sum, game_bounds& bounds);

    void _flip_tie_break_rule();

    void _reset();


    bool _assume_below_midpoint;
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

    if (lower_relation == COMP_EQUAL)
    {
        set_equal(lower);
    }

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

    if (upper_relation == COMP_EQUAL)
    {
        set_equal(upper);
    }

    _upper = upper;
    _upper_relation = upper_relation;
    _upper_valid = true;
}

void game_bounds::set_equal(bound_t lower_and_upper)
{
    set_lower(lower_and_upper, COMP_EQUAL);
    set_upper(lower_and_upper, COMP_EQUAL);
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

vector<game_bounds*> bounds_finder::find_bounds(sumgame& sum, const vector<bounds_options>& options)
{
    vector<game_bounds*> bounds_list;

    for (const bounds_options& opts : options)
    {
        assert(opts.min <= opts.max);

        _reset();

        game_bounds* gb = _make_bounds(sum, opts);
        bounds_list.push_back(gb);
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


bool bounds_finder::_validate_range(bound_t min, bound_t max,game_scale scale, sumgame& sum, game_bounds& bounds)
{
    if (!bounds.lower_valid())
    {
        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(min, scale));
        comparison_result relation = _get_step_comparison(sum, inverse_scale_game.get(), true, nullptr);

        if (relation != COMP_GREATER_OR_EQUAL && relation != COMP_GREATER && relation != COMP_EQUAL)
        {
            return false;
        } else
        {
            bounds.set_lower(min, relation);
        }
    }

    if (!bounds.upper_valid())
    {

        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(max, scale));
        comparison_result relation = _get_step_comparison(sum, inverse_scale_game.get(), false, nullptr);
        
        if (relation != COMP_LESS_OR_EQUAL && relation != COMP_LESS && relation != COMP_EQUAL)
        {
            return false;
        } else
        {
            bounds.set_upper(max, relation);
        }
    }

    return true;
}

void bounds_finder::_refine_bounds(game_scale scale, game_bounds& bounds, sumgame& sum)
{
    if (bounds.lower_valid() && bounds.get_lower_relation() == COMP_LESS_OR_EQUAL)
    {
        const bound_t lower = bounds.get_lower();

        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(lower, scale));

        bool is_ge = _is_greater_or_equal_scale(sum, inverse_scale_game.get());

        comparison_result relation = is_ge ? COMP_EQUAL : COMP_LESS;
        bounds.set_lower(lower, relation);
    }

    if (bounds.upper_valid() && bounds.get_upper_relation() == COMP_GREATER_OR_EQUAL)
    {
        const bound_t upper = bounds.get_upper();

        unique_ptr<game> inverse_scale_game(get_inverse_scale_game(upper, scale));

        bool is_le = _is_less_or_equal_scale(sum, inverse_scale_game.get());

        comparison_result relation = is_le ? COMP_EQUAL : COMP_GREATER;
        bounds.set_upper(upper, relation);
    }
}

game_bounds* bounds_finder::_make_bounds(sumgame& sum, const bounds_options& opt)
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
            _step(sr, opt.scale, sum, *bounds);
        }

        if (!bounds->both_valid() && !validated_range && _step_count >= 3)
        {
            validated_range = true;

            if (!_validate_range(opt.min, opt.max, opt.scale, sum, *bounds))
            {
                break;
            }
        }

        swap(_regions, _regions_next);
    }


    //_refine_bounds(games, opt.scale, *bounds);
    _refine_bounds(opt.scale, *bounds, sum);
    return bounds;
}

bool bounds_finder::_is_less_or_equal_scale(sumgame& sum, game* inverse_scale_game)
{
    _search_count++;
    sum.set_to_play(BLACK);
    return !sum.solve_with_games(inverse_scale_game);
}

bool bounds_finder::_is_greater_or_equal_scale(sumgame& sum, game* inverse_scale_game)
{
    _search_count++;
    sum.set_to_play(WHITE);
    return !sum.solve_with_games(inverse_scale_game);
}

comparison_result get_comparison_result(bool le_known, bool is_le, bool ge_known, bool is_ge)
{
    assert(le_known || ge_known);

    if (le_known && ge_known)
    {
        if (!is_le && !is_ge) // 0 0
            return COMP_FUZZY;
        if (!is_le && is_ge) // 0 1
            return COMP_GREATER;
        if (is_le && !is_ge) // 1 0
            return COMP_LESS;
        if (is_le && is_ge) // 1 1
            return COMP_EQUAL;

        assert(false);
    }

    if (le_known && is_le)
    {
        assert(!ge_known);
        return COMP_LESS_OR_EQUAL;
    }

    if (ge_known && is_ge)
    {
        assert(!le_known);
        return COMP_GREATER_OR_EQUAL;
    }

    assert(false);
}

comparison_result flip_comparison_result(comparison_result relation)
{
    if (relation == COMP_LESS || relation == COMP_GREATER)
    {
        return relation == COMP_LESS ? COMP_GREATER : COMP_LESS;
    }

    if (relation == COMP_LESS_OR_EQUAL || relation == COMP_GREATER_OR_EQUAL)
    {
        return relation == COMP_LESS_OR_EQUAL ? COMP_GREATER_OR_EQUAL : COMP_LESS_OR_EQUAL;
    }

    return relation;
}

comparison_result bounds_finder::_get_step_comparison(sumgame& sum, game* inverse_scale_game, bool below_midpoint, int* solve_count)
{
    bool le_known = false;
    bool is_le = false;

    bool ge_known = false;
    bool is_ge = false;


    auto test_le = [&]() -> void
    {
        assert(!le_known);
        le_known = true;
        is_le = _is_less_or_equal_scale(sum, inverse_scale_game);
    };

    auto test_ge = [&]() -> void
    {
        assert(!ge_known);
        ge_known = true;
        is_ge = _is_greater_or_equal_scale(sum, inverse_scale_game);
    };

    auto is_conclusive = [&]() -> bool
    {
        return (le_known && is_le) || (ge_known && is_ge);
    };

    if (below_midpoint)
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
    assert(is_conclusive() || (le_known && ge_known));

    if (solve_count != nullptr)
    {
        *solve_count = (int) le_known + (int) ge_known;
    }

    return get_comparison_result(le_known, is_le, ge_known, is_ge);
}


void bounds_finder::_step(search_region& region, game_scale scale, sumgame& sum, game_bounds& bounds)
{
    _step_count++;
    assert(region.valid());

    int scale_idx = region.get_midpoint();
    unique_ptr<game> inverse_scale_game(get_inverse_scale_game(scale_idx, scale));
 
    // Pick a test (>= or <=) to try first, based on what side of the bounds space
    // "scale_idx" is on
    bool below_midpoint = _assume_below_midpoint;
    bool did_tie_break = true;

    int midpoint = 0;

    if (bounds.both_valid())
    {
        midpoint = bounds.get_midpoint();
    }

    if (scale_idx != midpoint)
    {
        did_tie_break = false;
        below_midpoint = scale_idx < midpoint;
    }


    // S - Gi compared to 0
    int sumgame_solve_count = 0;
    //comparison_result relation = _compare_to_zero(sum, greater_first, sumgame_solve_count);

    // S <relation> Gi
    comparison_result relation = _get_step_comparison(sum, inverse_scale_game.get(), below_midpoint, &sumgame_solve_count);

    // Gi <relation> S
    relation = flip_comparison_result(relation);

    if (did_tie_break && sumgame_solve_count > 1)
    {
        _flip_tie_break_rule();
    }

    switch (relation)
    {
        case COMP_GREATER_OR_EQUAL:
        case COMP_GREATER:
        {
            region.high = scale_idx - 1;
            bounds.set_upper(scale_idx, relation);
            break;
        }

        case COMP_LESS_OR_EQUAL:
        case COMP_LESS:
        {
            region.low = scale_idx + 1;
            bounds.set_lower(scale_idx, relation);
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
            bounds.set_equal(scale_idx);
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
    _assume_below_midpoint = !_assume_below_midpoint;
}

void bounds_finder::_reset()
{
    _assume_below_midpoint = true;
    _step_count = 0;
    _search_count = 0;

    _regions.clear();
    _regions_next.clear();
}




vector<game_bounds*> find_bounds(vector<game*>& games, const vector<bounds_options>& options)
{
    bounds_finder bf;

    sumgame sum(BLACK);
    sum.add_vec(games);

    return bf.find_bounds(sum, options);
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

        sumgame sum(BLACK);

        sum.add_vec(games);

        bounds_finder bf;
        vector<game_bounds*> bounds_list = bf.find_bounds(sum, {{GAME_SCALE_UP_STAR, -R, R}});

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
