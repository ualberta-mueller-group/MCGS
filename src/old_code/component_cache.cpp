/*
    TODO CHAR_BIT is incorrect (we use uint8_t)

    TODO why is this not so much of an improvement for 4x4 clobber sums?
    Is there somewhere else we can use this?
*/

#include "component_cache.h"
#include "grid_generator.h"
#include "grid_hash_orientation.h"
#include "grid_location.h"
#include "grid_mask.h"
#include "integral_conversion.h"
#include "safe_arithmetic.h"
#include "stopwatch.h"
#include "throw_assert.h"
#include "utilities.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>
#include <cstdint>

using namespace std;

//////////////////////////////////////////////////
namespace {
bool is_initialized = false;

component_cache cc4;
component_cache cc8;

} // namespace

////////////////////////////////////////////////// component_cache methods
component_cache::component_cache():
    _n_rows(-1), _n_cols(-1)
{
}

void component_cache::init_4_connected(int n_rows, int n_cols)
{
    _init_values(n_rows, n_cols, GRID_DIRS_CARDINAL.data(),
                 GRID_DIRS_CARDINAL.size());
}

void component_cache::init_8_connected(int n_rows, int n_cols)
{
    _init_values(n_rows, n_cols, GRID_DIRS_ALL.data(), GRID_DIRS_ALL.size());
}

bool component_cache::is_initialized() const
{
    return _n_rows != -1 && _n_cols != -1;
}

int component_cache::get_n_rows() const
{
    assert(is_initialized());
    return _n_rows;
}

int component_cache::get_n_cols() const
{
    assert(is_initialized());
    return _n_cols;
}

bool component_cache::is_one_component(uint64_t bit_pattern) const
{
    const uint8_t byte = _data[bit_pattern / 8];
    return (byte >> (bit_pattern % 8)) & 0x1;
}


void component_cache::_init_memory(int n_rows, int n_cols)
{
    _n_rows = n_rows;
    _n_cols = n_cols;

    const size_t n_rows_casted = integral_cast_checked<size_t>(n_rows);
    const size_t n_cols_casted = integral_cast_checked<size_t>(n_cols);

    const size_t entry_count_exponent = n_rows_casted * n_cols_casted;
    if (n_rows_casted > 0 && n_cols_casted > 0)
    {
        THROW_ASSERT(entry_count_exponent / n_rows_casted == n_cols_casted);
    }

    THROW_ASSERT(entry_count_exponent < size_in_bits<size_t>());
    const size_t n_entries = size_t(1) << entry_count_exponent;

    // 1 bit per entry
    size_t n_bytes = n_entries / CHAR_BIT;
    const size_t remainder = n_entries % CHAR_BIT;

    if (remainder != 0)
        n_bytes++;

    THROW_ASSERT(n_bytes * CHAR_BIT >= n_entries);
    _data.resize(n_bytes, 0);

    std::fill(_data.begin(), _data.end(), 0);

    cout << _n_rows << "x" << _n_cols << " component cache allocated with ";
    cout << _data.size() << " bytes." << endl;
}

void component_cache::_init_values(int n_rows, int n_cols, const grid_dir* dirs,
                                   size_t n_dirs)
{
    _init_memory(n_rows, n_cols);

    cout << n_rows << " " << n_cols << endl;
    cout << _data.size() << endl;

    const int_pair dims(n_rows, n_cols);

    grid_mask gm(GRID_HASH_ACTIVE_MASK_IDENTITY);
    gm.set_shape(int_pair(n_rows, n_cols));

    identity_functor func;


    assert(gm);
    for (; gm; ++gm)
    {
        const vector<bool>& board = gm.get_mask();

        const bool val =
            _is_one_component_by_search(n_rows, n_cols, board, dirs, n_dirs);

        const uint64_t bit_pattern = get_full_board_code(dims, board, func);
        _set(bit_pattern, val);

        assert(val == is_one_component(bit_pattern));
    }
}

void component_cache::_set(uint64_t bit_pattern, bool value)
{
    uint8_t& byte = _data[bit_pattern / 8];
    const uint8_t bit = uint8_t(1) << (bit_pattern % 8);

    if (value)
        byte |= bit;
    else
        byte &= ~bit;
}

bool component_cache::_is_one_component_by_search(int n_rows, int n_cols,
                                                  const vector<bool>& board,
                                                  const grid_dir* dirs,
                                                  size_t n_dirs)
{
    const int_pair grid_shape(n_rows, n_cols);
    const int grid_size = n_rows * n_cols;

    vector<bool> closed_set(grid_size, false);
    vector<grid_location> open_stack;

    bool found_prev_component = false;

    for (grid_location start(grid_shape); start.valid();
         start.increment_position())
    {
        const int start_point = start.get_point();
        if (closed_set[start_point])
            continue;

        const bool start_bit = board[start_point];
        if (start_bit == false)
            continue;

        if (found_prev_component)
            return false;

        found_prev_component = true;

        closed_set[start_point] = true;
        open_stack.push_back(start);

        while (!open_stack.empty())
        {
            grid_location loc1 = open_stack.back();
            open_stack.pop_back();

            for (size_t dir_idx = 0; dir_idx < n_dirs; dir_idx++)
            {
                const grid_dir dir = dirs[dir_idx];

                grid_location loc2 = loc1;
                if (!loc2.move(dir))
                    continue;

                const int point2 = loc2.get_point();
                if (closed_set[point2])
                    continue;

                if (board[point2] == false)
                    continue;

                closed_set[point2] = true;
                open_stack.push_back(loc2);
            }
        }
    }

    return found_prev_component;
}

void init_global_component_caches()
{
    assert(!is_initialized);

    stopwatch sw;

    cout << "Initializing component caches..." << endl;

    sw.start();
    cc4.init_4_connected(COMPONENT_CACHE_ROWS, COMPONENT_CACHE_COLS);
    cc8.init_8_connected(COMPONENT_CACHE_ROWS, COMPONENT_CACHE_COLS);
    sw.stop();

    cout << "Done in " << (sw.get_duration_ms() / 1000.0) << " seconds" << endl;

    is_initialized = true;
}

const component_cache& get_4_connected_component_cache()
{
    assert(is_initialized);
    return cc4;
}

const component_cache& get_8_connected_component_cache()
{
    assert(is_initialized);
    return cc8;
}

//////////////////////////////////////////////////
namespace {

struct vec_bool_functor
{
    bool operator()(bool b) const
    {
        return b;
    }
};


void test_indexing()
{
    typedef tuple<

        int_pair,     // Full board dims
        vector<bool>, // Full board
        int_pair,     // Board dims
        int_pair,     // Window start
        int_pair,     // Window dims
        vector<bool>  // Board
        >
        test_case_t; //

    // clang-format off
    vector<test_case_t> test_cases
    {
        // Start test 1
        {
            {4, 4},
            {
                0, 1, 0, 0,
                0, 1, 1, 0,
                0, 0, 0, 0,
                0, 0, 0, 0,
            },
            {3, 3}, {0, 0}, {3, 3},
            {
                0, 1, 0,
                0, 1, 1,
                0, 0, 0,
            },
        },
        // End test 1

        // Start test 2
        {
            {4, 3},
            {
                1, 1, 0,
                0, 1, 1,
                0, 0, 0,
                0, 0, 1,
            },
            {5, 4}, {1, 1}, {4, 3},
            {
                0, 0, 0, 0,
                0, 1, 1, 0,
                0, 0, 1, 1,
                0, 0, 0, 0,
                0, 0, 0, 1,
            },
        },
        // End test 2

        // Start test 3
        {
            {6, 7},
            {
                0, 0, 0, 0, 0, 0, 0,
                0, 1, 0, 1, 0, 0, 0,
                0, 0, 1, 0, 0, 0, 0,
                0, 0, 1, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0,
            },
            {10, 10}, {2, 1}, {4, 4},
            {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 
                0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
            },
        },
        // End test 3

        // Start test 4
        {
            {4, 4},
            {
                1, 1, 0, 0,
                0, 1, 0, 0,
                0, 1, 1, 1,
                0, 0, 0, 0,
            },
            {3, 4}, {0, 0}, {3, 4},
            {
                1, 1, 0, 0,
                0, 1, 0, 0,
                0, 1, 1, 1,
            },
        },
        // End test 4

    };
    // clang-format on

    //const component_cache& cc = get_4_connected_component_cache();

    vec_bool_functor func;
    uint64_t test_number = 0;

    for (const test_case_t& test_case : test_cases)
    {
        //cout << "TEST NUMBER " << test_number++ << endl;

        const int_pair& full_board_dims = get<0>(test_case);
        const vector<bool>& full_board = get<1>(test_case);
        const int_pair& board_dims = get<2>(test_case);
        const int_pair& window_start = get<3>(test_case);
        const int_pair& window_dims = get<4>(test_case);
        const vector<bool>& board = get<5>(test_case);

        //cout << "TEST: ";
        //cout << full_board_dims << endl;
        //cout << "\t" << full_board << endl;
        //cout << board_dims << " ";
        //cout << window_start << " ";
        //cout << window_dims << endl;
        //cout << "\t" << board << endl;
        //cout << endl;

        const uint64_t full_board_code = component_cache::get_full_board_code(
            full_board_dims, full_board, func);

        const uint64_t window_code = component_cache::get_window_code(
            board_dims, board, window_start, window_dims, func,
            full_board_dims);

        assert(full_board_code == window_code);
    }

}

void test_patterns()
{
    typedef tuple<int_pair, bool, bool, vector<bool>> test_case_t;

    vector<test_case_t> test_cases
    {
        {
            {3, 3}, 0, 0,
            {
                0, 0, 0,
                0, 0, 0,
                0, 0, 0,
            },
        },

        {
            {3, 3}, 1, 1,
            {
                0, 0, 0,
                0, 1, 0,
                0, 0, 0,
            },
        },

        {
            {3, 3}, 0, 1,
            {
                1, 0, 0,
                0, 1, 0,
                0, 0, 0,
            },
        },

        {
            {3, 3}, 1, 1,
            {
                1, 1, 0,
                0, 1, 0,
                0, 0, 0,
            },
        },

        {
            {4, 4}, 1, 1,
            {
                1, 1, 0, 0,
                0, 1, 0, 0,
                0, 1, 1, 1,
                0, 0, 0, 1,
            },
        },

        {
            {3, 4}, 1, 1,
            {
                1, 1, 0, 0,
                0, 1, 0, 0,
                0, 1, 1, 1,
            },
        },

        {
            {3, 4}, 0, 0,
            {
                1, 1, 0, 1,
                0, 1, 0, 0,
                0, 1, 1, 1,
            },
        },

    };

    vec_bool_functor func;

    const component_cache& global_cc_4 = get_4_connected_component_cache();
    const int_pair cc4_dims(global_cc_4.get_n_rows(), global_cc_4.get_n_cols());

    const component_cache& global_cc_8 = get_8_connected_component_cache();
    const int_pair cc8_dims(global_cc_8.get_n_rows(), global_cc_8.get_n_cols());

    uint64_t test_number = 0;
    for (const test_case_t& test_case : test_cases)
    {
        const int_pair& dims = get<0>(test_case);
        const bool& exp_val_4 = get<1>(test_case);
        const bool& exp_val_8 = get<2>(test_case);
        const vector<bool>& board = get<3>(test_case);

        cout << "TEST NUMBER " << test_number++ << endl;
        cout << dims << " ";
        cout << exp_val_4 << " ";
        cout << exp_val_8 << endl;
        cout << "\t" << board << endl;
        cout << endl;

        const uint64_t code4 = component_cache::get_window_code(
            dims, board, {0, 0}, dims, func, cc4_dims);

        const uint64_t code8 = component_cache::get_window_code(
            dims, board, {0, 0}, dims, func, cc8_dims);

        const bool cache4_val = global_cc_4.is_one_component(code4);
        const bool cache8_val = global_cc_8.is_one_component(code8);

        assert(cache4_val == exp_val_4);
        assert(cache8_val == exp_val_8);
    }
}

} // namespace

void test_component_cache_stuff()
{
    test_indexing();
    test_patterns();
}

