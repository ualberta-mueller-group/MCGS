#pragma once

#include <vector>
#include "game.h"
#include <ostream>


// TODO figure out a better way to handle this. Maybe the outermost find_bounds() function can take a vector of radii?
#define BOUND_RADIUS 16
#define BOUND_MIN -BOUND_RADIUS
#define BOUND_MAX BOUND_RADIUS

enum game_scale
{
    GAME_SCALE_UP_STAR,
    GAME_SCALE_UP,
    GAME_SCALE_DYADIC_RATIONAL,
};

class game_bounds
{
public:
    game_bounds();
    int get_midpoint();
    void set_low(int low);
    void set_high(int high);
    bool both_valid();

    int low;
    bool low_valid;
    bool low_tight;

    int high;
    bool high_valid;
    bool high_tight;
};

std::ostream& operator<<(std::ostream& os, const game_bounds& gb);


std::vector<game_bounds*> find_bounds(std::vector<game*>& games, const std::vector<game_scale>& scales);
