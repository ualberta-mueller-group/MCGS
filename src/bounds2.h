#pragma once

#include <vector>
#include "game.h"
#include <ostream>
#include <cstdint>


// TODO figure out a better way to handle this. Maybe the outermost find_bounds() function can take a vector of radii?
#define BOUND_RADIUS 100
#define BOUND_MIN -BOUND_RADIUS
#define BOUND_MAX BOUND_RADIUS

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

    bound_t get_midpoint() const;
    void set_low(bound_t low);
    void set_high(bound_t high);
    bool both_valid() const;


    bound_t low;
    bool low_valid;
    comparison_result low_relation;

    bound_t high;
    bool high_valid;
    comparison_result high_relation;
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
