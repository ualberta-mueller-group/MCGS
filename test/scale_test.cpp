#include "scale_test.h"
#include "bounds.h"
#include "cgt_dyadic_rational.h"
#include "cgt_up_star.h"

#include <sstream>
#include <string>
#include <cassert>

using namespace std;

namespace {

void assert_scale_game(bound_t scale_idx, bound_scale scale, game* g,
                       game* inv_g)
{
    game* scale_game = get_scale_game(scale_idx, scale);
    game* inv_scale_game = get_inverse_scale_game(scale_idx, scale);

    auto get_name = [](game& ga) -> string
    {
        stringstream str;
        str << ga;
        return str.str();
    };

    assert(get_name(*g) == get_name(*scale_game));
    assert(get_name(*inv_g) == get_name(*inv_scale_game));

    delete scale_game;
    delete inv_scale_game;
    delete g;
    delete inv_g;
}

void test_scale_up_star()
{
    assert_scale_game(-3, BOUND_SCALE_UP_STAR, new up_star(-3, true),
                      new up_star(3, true));

    assert_scale_game(-2, BOUND_SCALE_UP_STAR, new up_star(-2, true),
                      new up_star(2, true));

    assert_scale_game(-1, BOUND_SCALE_UP_STAR, new up_star(-1, true),
                      new up_star(1, true));

    assert_scale_game(0, BOUND_SCALE_UP_STAR, new up_star(0, true),
                      new up_star(0, true));

    assert_scale_game(1, BOUND_SCALE_UP_STAR, new up_star(1, true),
                      new up_star(-1, true));

    assert_scale_game(2, BOUND_SCALE_UP_STAR, new up_star(2, true),
                      new up_star(-2, true));

    assert_scale_game(3, BOUND_SCALE_UP_STAR, new up_star(3, true),
                      new up_star(-3, true));
}

void test_scale_up()
{
    assert_scale_game(-3, BOUND_SCALE_UP, new up_star(-3, false),
                      new up_star(3, false));

    assert_scale_game(-2, BOUND_SCALE_UP, new up_star(-2, false),
                      new up_star(2, false));

    assert_scale_game(-1, BOUND_SCALE_UP, new up_star(-1, false),
                      new up_star(1, false));

    assert_scale_game(0, BOUND_SCALE_UP, new up_star(0, false),
                      new up_star(0, false));

    assert_scale_game(1, BOUND_SCALE_UP, new up_star(1, false),
                      new up_star(-1, false));

    assert_scale_game(2, BOUND_SCALE_UP, new up_star(2, false),
                      new up_star(-2, false));

    assert_scale_game(3, BOUND_SCALE_UP, new up_star(3, false),
                      new up_star(-3, false));
}

void test_scale_dyadic_rational()
{
    assert_scale_game(-3, BOUND_SCALE_DYADIC_RATIONAL,
                      new dyadic_rational(-3, 8), new dyadic_rational(3, 8));

    assert_scale_game(-2, BOUND_SCALE_DYADIC_RATIONAL,
                      new dyadic_rational(-2, 8), new dyadic_rational(2, 8));

    assert_scale_game(-1, BOUND_SCALE_DYADIC_RATIONAL,
                      new dyadic_rational(-1, 8), new dyadic_rational(1, 8));

    assert_scale_game(0, BOUND_SCALE_DYADIC_RATIONAL, new dyadic_rational(0, 8),
                      new dyadic_rational(0, 8));

    assert_scale_game(1, BOUND_SCALE_DYADIC_RATIONAL, new dyadic_rational(1, 8),
                      new dyadic_rational(-1, 8));

    assert_scale_game(2, BOUND_SCALE_DYADIC_RATIONAL, new dyadic_rational(2, 8),
                      new dyadic_rational(-2, 8));

    assert_scale_game(3, BOUND_SCALE_DYADIC_RATIONAL, new dyadic_rational(3, 8),
                      new dyadic_rational(-3, 8));
}

} // namespace

void scale_test_all()
{
    test_scale_up_star();
    test_scale_up();
    test_scale_dyadic_rational();
}
