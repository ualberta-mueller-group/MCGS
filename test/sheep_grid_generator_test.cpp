#include "sheep_grid_generator_test.h"

#include <cstdint>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "grid_generator.h"
#include "sheep_grid_generator.h"
#include "utilities.h"

using namespace std;

typedef std::pair<int_pair, std::vector<int>> board_pair_t;

//////////////////////////////////////////////////
// hash functions for unordered_set

/*
    NOTE: The multiplications here make the tests 30x faster than simple
    XORing
*/
template <class T>
struct std::hash<std::vector<T>>
{
    uint64_t operator()(const std::vector<T>& vec) const noexcept
    {
        uint64_t result = 0;

        result ^= vec.size();

        std::hash<T> hash_fn;

        const size_t SIZE = vec.size();

        for (size_t i = 0; i < SIZE; i++)
            result ^= ((i + 1) * hash_fn(vec[i]));

        return result;
    }
};

template <class T1, class T2>
struct std::hash<std::pair<T1, T2>>
{
    uint64_t operator()(const std::pair<T1, T2>& p) const noexcept
    {
        uint64_t result = 0;

        std::hash<T1> hash_fn1;
        std::hash<T2> hash_fn2;

        result ^= hash_fn1(p.first);
        result ^= (2 * hash_fn2(p.second));

        return result;
    }
};

namespace {

// Recursive search solution producing the expected boards.
void search(unordered_set<board_pair_t>& board_set,
            vector<int>& temp_board, const int_pair& dims, unsigned int idx,
            int black_remaining, int white_remaining)
{
    assert(0 <= idx &&                 //
           idx <= temp_board.size() && //
           black_remaining >= 0 &&     //
           white_remaining >= 0        //
    );

    if (black_remaining == 0 && white_remaining == 0)
    {
        auto inserted = board_set.emplace(dims, temp_board);
        assert(inserted.second); // no duplicates

        return;
    }

    if (idx >= temp_board.size())
        return;

    // 1
    if (black_remaining > 0)
    {
        assert(temp_board[idx] == 0);

        temp_board[idx] = 1;
        search(board_set, temp_board, dims, idx + 1, black_remaining - 1,
               white_remaining);
        temp_board[idx] = 0;
    }

    // -1
    if (white_remaining > 0)
    {
        assert(temp_board[idx] == 0);

        temp_board[idx] = -1;
        search(board_set, temp_board, dims, idx + 1, black_remaining,
               white_remaining - 1);
        temp_board[idx] = 0;
    }

    // 0
    assert(temp_board[idx] == 0);
    search(board_set, temp_board, dims, idx + 1, black_remaining,
           white_remaining);

    // 2, 3, ..., black_remaining
    for (int b = 2; b <= black_remaining; b++)
    {
        assert(temp_board[idx] == 0);
        temp_board[idx] = b;

        search(board_set, temp_board, dims, idx + 1, black_remaining - b,
               white_remaining);

        temp_board[idx] = 0;
    }

    // -2, -3, ..., -white_remaining
    for (int w = 2; w <= white_remaining; w++)
    {
        assert(temp_board[idx] == 0);
        temp_board[idx] = -w;

        search(board_set, temp_board, dims, idx + 1, black_remaining,
               white_remaining - w);

        temp_board[idx] = 0;
    }
}

unordered_set<board_pair_t> compute_found_set(const int_pair& max_dims,
                                                   int max_black, int max_white)
{
    unordered_set<board_pair_t> found_set;

    sheep_grid_generator* gen =
        new sheep_grid_generator(max_dims, max_black, max_white);

    while (*gen)
    {
        auto inserted = found_set.emplace(gen->get_shape(), gen->gen_board());
        assert(inserted.second); // no duplicates

        ++(*gen);
    }

    delete gen;

    return found_set;
}

unordered_set<board_pair_t> compute_expected_set(const int_pair& max_dims,
                                                   int max_black, int max_white)
{
    unordered_set<board_pair_t> expected_set;

    int_pair dims;
    bool init_dims = true;
    while (i_grid_generator::increment_dims_transpose(dims, max_dims, init_dims))
    {
        init_dims = false;

        const int area = dims.first * dims.second;
        vector<int> temp_board(area, 0);

        for (int b = 0; b <= max_black; b++)
            for (int w = 0; w <= max_white; w++)
                search(expected_set, temp_board, dims, 0, b, w);
    }

    return expected_set;
}

//////////////////////////////////////////////////
void manual_test()
{
    /*
       max dimensions to generate
       max black sheep
       max white sheep
       set of expected board pairs (dimensions + board vector)
    */
    typedef tuple<int_pair, int, int, unordered_set<board_pair_t>>
        test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            int_pair(2, 2),
            2, 2,
            {
                {{0, 0}, {}},
                {{1, 1}, {0}},
                {{1, 1}, {-1}},
                {{1, 1}, {1}},
                {{1, 1}, {-2}},
                {{1, 1}, {2}},
                {{1, 2}, {0, 0}},
                {{1, 2}, {-1, 0}},
                {{1, 2}, {0, -1}},
                {{1, 2}, {1, 0}},
                {{1, 2}, {0, 1}},
                {{1, 2}, {1, -1}},
                {{1, 2}, {-1, 1}},
                {{1, 2}, {-1, -1}},
                {{1, 2}, {0, -2}},
                {{1, 2}, {-2, 0}},
                {{1, 2}, {1, 1}},
                {{1, 2}, {0, 2}},
                {{1, 2}, {2, 0}},
                {{1, 2}, {1, -2}},
                {{1, 2}, {-2, 1}},
                {{1, 2}, {-1, 2}},
                {{1, 2}, {2, -1}},
                {{1, 2}, {2, -2}},
                {{1, 2}, {-2, 2}},
                {{2, 1}, {0, 0}},
                {{2, 1}, {-1, 0}},
                {{2, 1}, {0, -1}},
                {{2, 1}, {1, 0}},
                {{2, 1}, {0, 1}},
                {{2, 1}, {1, -1}},
                {{2, 1}, {-1, 1}},
                {{2, 1}, {-1, -1}},
                {{2, 1}, {0, -2}},
                {{2, 1}, {-2, 0}},
                {{2, 1}, {1, 1}},
                {{2, 1}, {0, 2}},
                {{2, 1}, {2, 0}},
                {{2, 1}, {1, -2}},
                {{2, 1}, {-2, 1}},
                {{2, 1}, {-1, 2}},
                {{2, 1}, {2, -1}},
                {{2, 1}, {2, -2}},
                {{2, 1}, {-2, 2}},
                {{2, 2}, {0, 0, 0, 0}},
                {{2, 2}, {-1, 0, 0, 0}},
                {{2, 2}, {0, -1, 0, 0}},
                {{2, 2}, {0, 0, -1, 0}},
                {{2, 2}, {0, 0, 0, -1}},
                {{2, 2}, {1, 0, 0, 0}},
                {{2, 2}, {0, 1, 0, 0}},
                {{2, 2}, {0, 0, 1, 0}},
                {{2, 2}, {0, 0, 0, 1}},
                {{2, 2}, {1, -1, 0, 0}},
                {{2, 2}, {1, 0, -1, 0}},
                {{2, 2}, {1, 0, 0, -1}},
                {{2, 2}, {-1, 1, 0, 0}},
                {{2, 2}, {-1, 0, 1, 0}},
                {{2, 2}, {-1, 0, 0, 1}},
                {{2, 2}, {0, 1, -1, 0}},
                {{2, 2}, {0, 1, 0, -1}},
                {{2, 2}, {0, -1, 1, 0}},
                {{2, 2}, {0, -1, 0, 1}},
                {{2, 2}, {0, 0, 1, -1}},
                {{2, 2}, {0, 0, -1, 1}},
                {{2, 2}, {-1, -1, 0, 0}},
                {{2, 2}, {-1, 0, -1, 0}},
                {{2, 2}, {-1, 0, 0, -1}},
                {{2, 2}, {0, -1, -1, 0}},
                {{2, 2}, {0, -1, 0, -1}},
                {{2, 2}, {0, 0, -1, -1}},
                {{2, 2}, {0, 0, 0, -2}},
                {{2, 2}, {0, 0, -2, 0}},
                {{2, 2}, {0, -2, 0, 0}},
                {{2, 2}, {-2, 0, 0, 0}},
                {{2, 2}, {1, 1, 0, 0}},
                {{2, 2}, {1, 0, 1, 0}},
                {{2, 2}, {1, 0, 0, 1}},
                {{2, 2}, {0, 1, 1, 0}},
                {{2, 2}, {0, 1, 0, 1}},
                {{2, 2}, {0, 0, 1, 1}},
                {{2, 2}, {0, 0, 0, 2}},
                {{2, 2}, {0, 0, 2, 0}},
                {{2, 2}, {0, 2, 0, 0}},
                {{2, 2}, {2, 0, 0, 0}},
                {{2, 2}, {1, -1, -1, 0}},
                {{2, 2}, {1, -1, 0, -1}},
                {{2, 2}, {1, 0, -1, -1}},
                {{2, 2}, {1, 0, 0, -2}},
                {{2, 2}, {1, 0, -2, 0}},
                {{2, 2}, {1, -2, 0, 0}},
                {{2, 2}, {-1, 1, -1, 0}},
                {{2, 2}, {-1, 1, 0, -1}},
                {{2, 2}, {-1, -1, 1, 0}},
                {{2, 2}, {-1, -1, 0, 1}},
                {{2, 2}, {-1, 0, 1, -1}},
                {{2, 2}, {-1, 0, -1, 1}},
                {{2, 2}, {0, 1, -1, -1}},
                {{2, 2}, {0, 1, 0, -2}},
                {{2, 2}, {0, 1, -2, 0}},
                {{2, 2}, {0, -1, 1, -1}},
                {{2, 2}, {0, -1, -1, 1}},
                {{2, 2}, {0, 0, 1, -2}},
                {{2, 2}, {0, 0, -2, 1}},
                {{2, 2}, {0, -2, 1, 0}},
                {{2, 2}, {0, -2, 0, 1}},
                {{2, 2}, {-2, 1, 0, 0}},
                {{2, 2}, {-2, 0, 1, 0}},
                {{2, 2}, {-2, 0, 0, 1}},
                {{2, 2}, {1, 1, -1, 0}},
                {{2, 2}, {1, 1, 0, -1}},
                {{2, 2}, {1, -1, 1, 0}},
                {{2, 2}, {1, -1, 0, 1}},
                {{2, 2}, {1, 0, 1, -1}},
                {{2, 2}, {1, 0, -1, 1}},
                {{2, 2}, {-1, 1, 1, 0}},
                {{2, 2}, {-1, 1, 0, 1}},
                {{2, 2}, {-1, 0, 1, 1}},
                {{2, 2}, {-1, 0, 0, 2}},
                {{2, 2}, {-1, 0, 2, 0}},
                {{2, 2}, {-1, 2, 0, 0}},
                {{2, 2}, {0, 1, 1, -1}},
                {{2, 2}, {0, 1, -1, 1}},
                {{2, 2}, {0, -1, 1, 1}},
                {{2, 2}, {0, -1, 0, 2}},
                {{2, 2}, {0, -1, 2, 0}},
                {{2, 2}, {0, 0, -1, 2}},
                {{2, 2}, {0, 0, 2, -1}},
                {{2, 2}, {0, 2, -1, 0}},
                {{2, 2}, {0, 2, 0, -1}},
                {{2, 2}, {2, -1, 0, 0}},
                {{2, 2}, {2, 0, -1, 0}},
                {{2, 2}, {2, 0, 0, -1}},
                {{2, 2}, {1, 1, -1, -1}},
                {{2, 2}, {1, 1, 0, -2}},
                {{2, 2}, {1, 1, -2, 0}},
                {{2, 2}, {1, -1, 1, -1}},
                {{2, 2}, {1, -1, -1, 1}},
                {{2, 2}, {1, 0, 1, -2}},
                {{2, 2}, {1, 0, -2, 1}},
                {{2, 2}, {1, -2, 1, 0}},
                {{2, 2}, {1, -2, 0, 1}},
                {{2, 2}, {-1, 1, 1, -1}},
                {{2, 2}, {-1, 1, -1, 1}},
                {{2, 2}, {-1, -1, 1, 1}},
                {{2, 2}, {-1, -1, 0, 2}},
                {{2, 2}, {-1, -1, 2, 0}},
                {{2, 2}, {-1, 0, -1, 2}},
                {{2, 2}, {-1, 0, 2, -1}},
                {{2, 2}, {-1, 2, -1, 0}},
                {{2, 2}, {-1, 2, 0, -1}},
                {{2, 2}, {0, 1, 1, -2}},
                {{2, 2}, {0, 1, -2, 1}},
                {{2, 2}, {0, -1, -1, 2}},
                {{2, 2}, {0, -1, 2, -1}},
                {{2, 2}, {0, 0, 2, -2}},
                {{2, 2}, {0, 0, -2, 2}},
                {{2, 2}, {0, 2, -1, -1}},
                {{2, 2}, {0, 2, 0, -2}},
                {{2, 2}, {0, 2, -2, 0}},
                {{2, 2}, {0, -2, 1, 1}},
                {{2, 2}, {0, -2, 0, 2}},
                {{2, 2}, {0, -2, 2, 0}},
                {{2, 2}, {2, -1, -1, 0}},
                {{2, 2}, {2, -1, 0, -1}},
                {{2, 2}, {2, 0, -1, -1}},
                {{2, 2}, {2, 0, 0, -2}},
                {{2, 2}, {2, 0, -2, 0}},
                {{2, 2}, {2, -2, 0, 0}},
                {{2, 2}, {-2, 1, 1, 0}},
                {{2, 2}, {-2, 1, 0, 1}},
                {{2, 2}, {-2, 0, 1, 1}},
                {{2, 2}, {-2, 0, 0, 2}},
                {{2, 2}, {-2, 0, 2, 0}},
                {{2, 2}, {-2, 2, 0, 0}},
            },
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const int_pair& max_dims = get<0>(test_case);
        const int max_black = get<1>(test_case);
        const int max_white = get<2>(test_case);
        const unordered_set<board_pair_t>& expected_set = get<3>(test_case);

        const unordered_set<board_pair_t> found_set =
            compute_found_set(max_dims, max_black, max_white);

        assert(expected_set == found_set);
    }
}

void test_main()
{
    /*
       max dimensions
       max black sheep
       max white sheep
    */
    typedef tuple<int_pair, int, int> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {
            int_pair(3, 3), 3, 3,
        },

        {
            int_pair(2, 4), 2, 1,
        },
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const int_pair& max_dims = get<0>(test_case);
        const int max_black = get<1>(test_case);
        const int max_white = get<2>(test_case);

        const unordered_set<board_pair_t> expected_set =
            compute_expected_set(max_dims, max_black, max_white);

        const unordered_set<board_pair_t> found_set =
            compute_found_set(max_dims, max_black, max_white);

        assert(expected_set == found_set);
    }
}

} // namespace

//////////////////////////////////////////////////
void sheep_grid_generator_test_all()
{
    cout << __FILE__ << endl;

    test_main();
    manual_test();
}
