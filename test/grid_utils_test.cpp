#include "grid_utils_test.h"

#include "grid_utils.h"
#include "grid.h"
#include "test_utilities.h"
#include <vector>
#include <tuple>
#include <cassert>
#include <cstddef>

using namespace std;

namespace {

void test_get_set()
{
    // 2 rows, 3 columns
    const int_pair shape(2, 3);

    grid_location gl1(shape);
    assert(gl1.valid());
    assert(!gl1.is_empty());
    assert(gl1.get_shape() == int_pair(2, 3));
    assert(gl1.get_coord() == int_pair(0, 0));
    assert(gl1.get_point() == 0);

    grid_location gl2(shape, {1, 2});
    assert(gl2.valid());
    assert(!gl2.is_empty());
    assert(gl2.get_coord() == int_pair(1, 2));
    assert(gl2.get_point() == 5);

    grid_location gl3(shape, 4);
    assert(gl3.valid());
    assert(!gl3.is_empty());
    assert(gl3.get_coord() == int_pair(1, 1));
    assert(gl3.get_point() == 4);

    // Change shape
    gl3.set_shape(int_pair(4, 5));
    assert(gl3.valid());

    assert(gl3.get_shape() == int_pair(4, 5)); // shape changed
    assert(gl3.get_coord() == int_pair(1, 1)); // coord shouldn't change
    assert(gl3.get_point() == 6); // point should change

    // Change point
    gl3.set_point(9);
    assert(gl3.valid());
    assert(gl3.get_coord() == int_pair(1, 4));
    assert(gl3.get_point() == 9);

    // Change coord
    gl3.set_coord(int_pair(3, 3));
    assert(gl3.valid());
    assert(gl3.get_coord() == int_pair(3, 3));
    assert(gl3.get_point() == 18);

    // reset_position()
    gl3.reset_position();
    assert(gl3.valid());
    assert(!gl3.is_empty());
    assert(gl3.get_coord() == int_pair(0, 0));
    assert(gl3.get_point() == 0);

    // Construct empty shape
    grid_location gl4(int_pair(0, 0));
    assert(!gl4.valid());
    assert(gl4.is_empty());
    assert(gl4.get_shape() == int_pair(0, 0));

    // Now set shape
    gl4.set_shape(int_pair(1, 1));
    assert(gl4.valid());
    assert(!gl4.is_empty());
    assert(gl4.get_shape() == int_pair(1, 1));
    assert(gl4.get_coord() == int_pair(0, 0));
    assert(gl4.get_point() == 0);

    gl4.set_point(0);
    assert(gl4.valid());
    gl4.set_coord(int_pair(0, 0));
    assert(gl4.valid());
}

void test_mutators()
{
    // Test iteration
    grid_location loc1(int_pair(3, 7));
    for (int i = 0; i < 3 * 7; i++)
    {
        assert(loc1.valid());
        assert(loc1.get_point() == i);
        assert(loc1.get_coord() == int_pair(i / 7, i % 7));
        loc1.increment_position();
    }
    assert(!loc1.valid());

    // Test resetting
    auto iterate_to_end = [](grid_location& gl) -> void
    {
        while (gl.valid())
            gl.increment_position();
    };

    iterate_to_end(loc1);
    loc1.reset_position();
    assert(loc1.get_coord() == int_pair(0, 0) && loc1.get_point() == 0);

    iterate_to_end(loc1);
    loc1.set_point(0);
    assert(loc1.get_coord() == int_pair(0, 0) && loc1.get_point() == 0);

    iterate_to_end(loc1);
    loc1.set_coord(int_pair(0, 0));
    assert(loc1.get_coord() == int_pair(0, 0) && loc1.get_point() == 0);

    // Test moves
    auto test_move = [](const grid_dir* dir_arr, size_t dir_arr_size,
        const vector<int_pair>& expected_coords) -> void
    {
        grid_location gl(int_pair(5, 5));

        for (size_t i = 0; i < dir_arr_size; i++)
        {
            gl.set_coord({2, 2});

            grid_dir dir = dir_arr[i];
            const int_pair& expected = expected_coords[i];

            // get_neighbor_coord method and static function
            int_pair coord;
            assert(gl.get_neighbor_coord(coord, dir));
            assert(coord == expected);
            int_pair coord_static;
            assert(grid_location::get_neighbor_coord(coord_static,
                                                     gl.get_coord(),
                                                     dir, gl.get_shape()));
            assert(coord_static == expected);

            // grid::move() method
            assert(gl.move(dir));
            assert(gl.valid());
            assert(gl.get_coord() == expected);

            // get_neighbor_point method and static function
            int expected_point = gl.get_point();
            gl.set_coord({2, 2});
            int point;
            assert(gl.get_neighbor_point(point, dir));
            assert(point == expected_point);
            int point_static;
            assert(grid_location::get_neighbor_point(point_static,
                                                     gl.get_coord(),
                                                     dir, gl.get_shape()));
            assert(point_static == expected_point);
        }
    };

    vector<int_pair> expected_cardinal =
    {
        {1, 2}, {2, 3}, {3, 2}, {2, 1},
    };

    vector<int_pair> expected_diagonal =
    {
        {1, 3}, {3, 3}, {3, 1}, {1, 1},
    };

    vector<int_pair> expected_all =
    {
        {1, 2}, {1, 3}, {2, 3}, {3, 3}, {3, 2}, {3, 1}, {2, 1}, {1, 1},
    };

    test_move(GRID_DIRS_CARDINAL.data(), GRID_DIRS_CARDINAL.size(),
        expected_cardinal);

    test_move(GRID_DIRS_DIAGONAL.data(), GRID_DIRS_DIAGONAL.size(),
        expected_diagonal);

    test_move(GRID_DIRS_ALL.data(), GRID_DIRS_ALL.size(),
        expected_all);

    // Invalid moves
    for (grid_dir dir : GRID_DIRS_ALL)
    {
        grid_location gl({3, 3}, {1, 1});
        assert(gl.move(dir));
        assert(!gl.move(dir));
        assert(gl.valid());

        int_pair coord;
        int point;
        assert(!gl.get_neighbor_coord(coord, dir));
        assert(!gl.get_neighbor_point(point, dir));

        assert(!grid_location::get_neighbor_coord(coord, gl.get_coord(), dir,
               gl.get_shape()));

        assert(!grid_location::get_neighbor_point(point, gl.get_coord(), dir,
               gl.get_shape()));
    }
}

void test_exceptions()
{
    auto test_illegal = [](grid_location& gl) -> void
    {
        ASSERT_DID_THROW(gl.get_coord());
        ASSERT_DID_THROW(gl.get_point());

        if (gl.is_empty())
        {
            ASSERT_DID_THROW(gl.set_coord({0, 0}));
            ASSERT_DID_THROW(gl.set_coord({1, 0}));
            ASSERT_DID_THROW(gl.set_coord({0, 1}));
            ASSERT_DID_THROW(gl.set_point(0));
            ASSERT_DID_THROW(gl.set_point(1));
            ASSERT_DID_THROW(gl.reset_position());
        }

        ASSERT_DID_THROW(gl.set_coord({-1, -1}));
        ASSERT_DID_THROW(gl.set_point(-1));

        ASSERT_DID_THROW(
            int_pair coord;
            gl.get_neighbor_coord(coord, GRID_DIR_LEFT);
        );

        ASSERT_DID_THROW(gl.move(GRID_DIR_UP));
        ASSERT_DID_THROW(gl.increment_position());
    };

    // Empty shape
    grid_location gl1({0, 0});
    assert(!gl1.valid());
    test_illegal(gl1);

    // After iterating to end
    grid_location gl2({4, 4});
    while (gl2.valid())
        gl2.increment_position();

    test_illegal(gl2);

    // Invalid shapes
    ASSERT_DID_THROW(grid_location({-1, 1}));
    ASSERT_DID_THROW(grid_location({1, -1}));
    ASSERT_DID_THROW(grid_location({-1, -1}));

    // Invalid coords
    ASSERT_DID_THROW(grid_location({0, 0}, {0, 0}));
    ASSERT_DID_THROW(grid_location({0, 0}, {-1, 0}));
    ASSERT_DID_THROW(grid_location({0, 0}, {0, -1}));
    ASSERT_DID_THROW(grid_location({0, 0}, {-1, -1}));
    ASSERT_DID_THROW(grid_location({1, 1}, {0, 1}));
    ASSERT_DID_THROW(grid_location({1, 1}, {1, 0}));
    ASSERT_DID_THROW(grid_location({1, 1}, {-1, 0}));
    ASSERT_DID_THROW(grid_location({1, 1}, {0, -1}));
    ASSERT_DID_THROW(grid_location({1, 1}, {-1, -1}));

    // Invalid points
    ASSERT_DID_THROW(grid_location({0, 0}, 0));
    ASSERT_DID_THROW(grid_location({0, 0}, -1));
    ASSERT_DID_THROW(grid_location({0, 0}, 1));
    ASSERT_DID_THROW(grid_location({1, 1}, -1));
    ASSERT_DID_THROW(grid_location({1, 1}, 1));

    // Moving location out of shape's bounds
    grid_location gl3({3, 3}, {2, 2});
    assert(gl3.valid());
    ASSERT_DID_THROW(gl3.set_coord({-1, -1}));
    ASSERT_DID_THROW(gl3.set_coord({1, -1}));
    ASSERT_DID_THROW(gl3.set_coord({-1, 1}));
    ASSERT_DID_THROW(gl3.set_coord({2, 3}));
    ASSERT_DID_THROW(gl3.set_coord({3, 2}));
    ASSERT_DID_THROW(gl3.set_coord({3, 3}));
    ASSERT_DID_THROW(gl3.set_point(-1));
    ASSERT_DID_THROW(gl3.set_point(9));

    // Static functions
    for (grid_dir dir : GRID_DIRS_ALL)
    {
        const int_pair empty_shape(0, 0);
        const int_pair zero_coord(0, 0);

        const int_pair valid_shape(3, 3);
        const int_pair invalid_coord(2, 3);

        // get neighbor coord
        ASSERT_DID_THROW(
            int_pair coord;
            grid_location::get_neighbor_coord(coord, zero_coord, dir,
                                              empty_shape);
        );

        ASSERT_DID_THROW(
            int_pair coord;
            grid_location::get_neighbor_coord(coord, invalid_coord, dir,
                                              valid_shape);
        );

        // get neighbor point
        ASSERT_DID_THROW(
            int point;
            grid_location::get_neighbor_point(point, zero_coord, dir,
                                              empty_shape);
        );

        ASSERT_DID_THROW(
            int point;
            grid_location::get_neighbor_point(point, invalid_coord, dir,
                                              valid_shape);
        );
    }
}

void test_conversion()
{
    // shape, coord, point
    typedef tuple<int_pair, int_pair, int> test_case_t;

    int_pair shape1(3, 4);
    int_pair shape2(2, 3);

    vector<test_case_t> test_cases =
    {
        {shape1, {0, 0}, 0},
        {shape1, {0, 1}, 1},
        {shape1, {0, 2}, 2},
        {shape1, {0, 3}, 3},
        {shape1, {1, 0}, 4},
        {shape1, {1, 1}, 5},
        {shape1, {1, 2}, 6},
        {shape1, {1, 3}, 7},
        {shape1, {2, 0}, 8},
        {shape1, {2, 1}, 9},
        {shape1, {2, 2}, 10},
        {shape1, {2, 3}, 11},
        {shape2, {0, 0}, 0},
        {shape2, {0, 1}, 1},
        {shape2, {0, 2}, 2},
        {shape2, {1, 0}, 3},
        {shape2, {1, 1}, 4},
        {shape2, {1, 2}, 5},
    };

    for (const test_case_t& test_case : test_cases)
    {
        const int_pair& shape = get<0>(test_case);
        const int_pair& expected_coord = get<1>(test_case);
        const int& expected_point = get<2>(test_case);

        assert(grid_location::coord_to_point(expected_coord, shape)
                == expected_point);

        assert(grid_location::point_to_coord(expected_point, shape)
                == expected_coord);
    }

    ASSERT_DID_THROW(grid_location::coord_to_point({0, 0}, {-1, -1}));
    ASSERT_DID_THROW(grid_location::coord_to_point({0, 0}, {-1, 0}));
    ASSERT_DID_THROW(grid_location::coord_to_point({0, 0}, {0, -1}));
    ASSERT_DID_THROW(grid_location::coord_to_point({0, 0}, {0, 0}));
    ASSERT_DID_THROW(grid_location::coord_to_point({-1, 0}, {0, 0}));
    ASSERT_DID_THROW(grid_location::coord_to_point({-1, 0}, {1, 1}));
    ASSERT_DID_THROW(grid_location::coord_to_point({1, 0}, {1, 1}));
    ASSERT_DID_THROW(grid_location::coord_to_point({0, 1}, {1, 1}));

    ASSERT_DID_THROW(grid_location::point_to_coord(0, {-1, -1}));
    ASSERT_DID_THROW(grid_location::point_to_coord(0, {-1, 0}));
    ASSERT_DID_THROW(grid_location::point_to_coord(0, {0, -1}));
    ASSERT_DID_THROW(grid_location::point_to_coord(0, {0, 0}));
    ASSERT_DID_THROW(grid_location::point_to_coord(-1, {0, 0}));
    ASSERT_DID_THROW(grid_location::point_to_coord(1, {0, 0}));
    ASSERT_DID_THROW(grid_location::point_to_coord(-1, {1, 1}));
    ASSERT_DID_THROW(grid_location::point_to_coord(1, {1, 1}));
}

} // namespace

void grid_utils_test_all()
{
    test_get_set();
    test_mutators();
    test_exceptions();
    test_conversion();
}
