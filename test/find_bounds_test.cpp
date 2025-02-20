#include "find_bounds_test.h"
#include "bounds.h"
#include "cgt_integer_game.h"
#include "clobber_1xn.h"
#include "nogo_1xn.h"
#include "elephants.h"
#include "cgt_up_star.h"
#include <string>
#include <memory>

using namespace std;

namespace {

void test_game(game* g, bound_scale scale, bound_t low, relation rel_low, bound_t high, relation rel_high, bound_t radius = 8)
{
    vector<bounds_options> opts_list;
    opts_list.push_back(bounds_options());
    bounds_options& opt = opts_list.back();

    opt.scale = scale;
    opt.min = -radius;
    opt.max = radius;

    vector<game_bounds*> bounds_list = find_bounds(g, opts_list);

    delete g;

    assert(bounds_list.size() == 1);
    game_bounds* bounds = bounds_list.back();

    assert(bounds->both_valid());

    assert(bounds->get_lower() == low);
    assert(bounds->get_lower_relation() == rel_low);

    assert(bounds->get_upper() == high);
    assert(bounds->get_upper_relation() == rel_high);

    delete bounds;
}

void test_game_invalid(game* g, bound_scale scale, bound_t radius = 8)
{
    vector<bounds_options> opts_list;
    opts_list.push_back(bounds_options());
    bounds_options& opt = opts_list.back();

    opt.scale = scale;
    opt.min = -radius;
    opt.max = radius;

    vector<game_bounds*> bounds_list = find_bounds(g, opts_list);

    delete g;

    assert(bounds_list.size() == 1);
    game_bounds* bounds = bounds_list.back();

    assert(!bounds->both_valid());

    delete bounds;
}

void test_clobber_1xn()
{
    test_game(new clobber_1xn("XXO.OXOX"), BOUND_SCALE_UP_STAR, 0, REL_LESS, 2, REL_GREATER);
    test_game(new clobber_1xn("XXO.OXOX"), BOUND_SCALE_UP, -1, REL_LESS, 3, REL_GREATER);
    test_game(new clobber_1xn("XXO.OXOX"), BOUND_SCALE_DYADIC_RATIONAL, -1, REL_LESS, 1, REL_GREATER);
}

void test_elephants()
{
    test_game_invalid(new elephants("X.O.O"), BOUND_SCALE_UP_STAR, 1000);
    test_game_invalid(new elephants("X.O.O"), BOUND_SCALE_UP, 1000);
    test_game(new elephants("X.O.O"), BOUND_SCALE_DYADIC_RATIONAL, -17, REL_LESS, -7, REL_GREATER, 32);
}

void test_nogo_1xn()
{
    test_game_invalid(new nogo_1xn("X.X.X.O"), BOUND_SCALE_UP_STAR, 1000);
    test_game_invalid(new nogo_1xn("X.X.X.O"), BOUND_SCALE_UP, 1000);
    test_game(new nogo_1xn("X.X.X.O"), BOUND_SCALE_DYADIC_RATIONAL, 16, REL_EQUAL, 16, REL_EQUAL, 16);
}

void test_simple_games()
{
    test_game(new up_star(5, true), BOUND_SCALE_UP_STAR, 5, REL_EQUAL, 5, REL_EQUAL); 

    test_game(new up_star(5, false), BOUND_SCALE_UP_STAR, 3, REL_LESS, 7, REL_GREATER);

    test_game_invalid(new integer_game(10), BOUND_SCALE_DYADIC_RATIONAL, 8);

}

} // namespace

void find_bounds_test_all()
{
    test_clobber_1xn();
    test_elephants();
    test_nogo_1xn();
    test_simple_games();
}
