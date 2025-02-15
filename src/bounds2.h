#pragma once

#include <vector>
#include "game.h"
#include <ostream>
#include <cstdint>

enum comparison_result 
{
    COMP_EQUAL = 0,
    COMP_FUZZY,
    COMP_LESS_OR_EQUAL,
    COMP_LESS,
    COMP_GREATER_OR_EQUAL,
    COMP_GREATER,
};

enum game_scale
{
    GAME_SCALE_UP_STAR,
    GAME_SCALE_UP,
    GAME_SCALE_DYADIC_RATIONAL,
};

typedef int32_t bound_t;

class game_bounds
{
public:
    game_bounds();

    void set_lower(bound_t lower, comparison_result lower_relation);
    void set_upper(bound_t upper, comparison_result upper_relation);

    bound_t get_midpoint() const;

    void invalidate_lower();
    void invalidate_upper();
    void invalidate_both();


    inline bool lower_valid() const
	{
        return _lower_valid;
	}

    inline bool upper_valid() const
	{
        return _upper_valid;
	}

    inline bool both_valid() const
	{
        return _lower_valid && _upper_valid;
	}

    inline bound_t get_lower() const
	{
        assert(_lower_valid);
        return _lower;
	}

    inline bound_t get_upper() const
	{
        assert(_upper_valid);
        return _upper;
    }

    inline comparison_result get_lower_relation() const
    {
        assert(_lower_valid);
        return _lower_relation;
    }

    inline comparison_result get_upper_relation() const
    {
        assert(_upper_valid);
        return _upper_relation;
    }

private:
    bound_t _lower;
    bool _lower_valid;
    comparison_result _lower_relation;

    bound_t _upper;
    bool _upper_valid;
    comparison_result _upper_relation;
};

std::ostream& operator<<(std::ostream& os, const game_bounds& gb);

struct bounds_options
{
    game_scale scale;
    bound_t min;
    bound_t max;
};

// TODO return vector<game_bounds> instead of vector<game_bounds*> ???
std::vector<game_bounds*> find_bounds(std::vector<game*>& games, const std::vector<bounds_options>& options);

void test_bounds2();
