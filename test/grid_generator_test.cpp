#include "grid_generator_test.h"
#include "grid_generator.h"
#include "strip.h"
#include "grid.h"

#include <string>
#include <cstddef>
#include <vector>
#include <tuple>
#include <cassert>
#include <unordered_set>

using namespace std;

namespace {

unordered_set<string> get_all_boards(grid_generator& gen)
{
    unordered_set<string> strings;

    while (gen)
    {
        strings.insert(gen.gen_board());
        ++gen;
    }

    return strings;
}

inline unordered_set<string> get_all_boards(grid_generator&& gen)
{
    return get_all_boards(gen);
}

int get_stone_count(const string& board)
{
    int count = 0;

    const char BLACK_STONE = color_to_clobber_char(BLACK);
    const char WHITE_STONE = color_to_clobber_char(WHITE);

    const size_t N = board.size();
    for (size_t i = 0; i < N; i++)
    {
        const char c = board[i];

        if (c == BLACK_STONE || c == WHITE_STONE)
            count++;
    }

    return count;
}

void assert_stone_ordering(grid_generator& gen, bool increasing)
{
    int_pair prev_shape(0, 0);
    int prev_stones = 0;

    for (; gen; ++gen)
    {
        const int_pair next_shape = gen.get_shape();
        const string& board = gen.gen_board();

        if (next_shape != prev_shape)
        {
            prev_shape = next_shape;
            prev_stones = get_stone_count(board);
            continue;
        }

        int next_stones = get_stone_count(board);

        if (increasing)
            assert(next_stones >= prev_stones);
        else
            assert(next_stones <= prev_stones);

        prev_stones = next_stones;
    }
}

void gen_for_dims(vector<string>& boards, int rows, int cols)
{
    assert(rows > 0 && cols > 0);

    string board;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
            board.push_back('.');

        if (r + 1 < rows)
            board.push_back('|');
    }

    boards.push_back(board);

    auto increment = [&]() -> bool
    {
        bool carry = true;

        for (auto it = board.rbegin(); it != board.rend(); it++)
        {
            if (!carry)
                break;

            const char c = *it;

            if (c == '|')
                continue;

            carry = false;

            if (c == '.')
                *it = 'X';
            else if (c == 'X')
                *it = 'O';
            else if (c == 'O')
            {
                *it = '.';
                carry = true;
            }
        }

        return !carry;
    };

    while (increment())
        boards.push_back(board);
}

void test_1x2()
{
    vector<string> expected = {
        "",   //
        ".",  //
        "X",  //
        "O",  //
        "..", //
        ".X", //
        ".O", //
        "X.", //
        "XX", //
        "XO", //
        "O.", //
        "OX", //
        "OO", //
    };

    grid_generator_default gen1(2);
    grid_generator_default gen2(1, 2);

    for (const string& exp : expected)
    {
        assert(gen1);
        assert(gen2);

        assert(gen1.gen_board() == exp);
        assert(gen2.gen_board() == exp);

        ++gen1;
        ++gen2;
    }
    assert(!gen1);
    assert(!gen2);
}

void test_2x1()
{
    vector<string> expected = {
        "",    //
        ".",   //
        "X",   //
        "O",   //
        ".|.", //
        ".|X", //
        ".|O", //
        "X|.", //
        "X|X", //
        "X|O", //
        "O|.", //
        "O|X", //
        "O|O", //
    };

    grid_generator_default gen(2, 1);
    for (const string& exp : expected)
    {
        assert(gen);
        assert(gen.gen_board() == exp);
        ++gen;
    }
    assert(!gen);
}

void test_2x2()
{
    vector<string> expected;

    expected.push_back("");
    gen_for_dims(expected, 1, 1);
    gen_for_dims(expected, 1, 2);
    gen_for_dims(expected, 2, 1);
    gen_for_dims(expected, 2, 2);

    grid_generator_default gen(2, 2);

    for (const string& exp : expected)
    {
        assert(gen);
        assert(gen.gen_board() == exp);
        ++gen;
    }
    assert(!gen);
}

void test_empties()
{
    // rows, columns, expected
    typedef tuple<int, int, string> test_case_t;

    vector<test_case_t> test_cases = {
        {0, 0, ""},            //
        {0, 1, ""},            //
        {1, 0, ""},            //
                               //
        {1, 1, "."},           //
        {1, 2, ".."},          //
        {1, 3, "..."},         //
                               //
        {2, 1, ".|."},         //
        {2, 2, "..|.."},       //
        {2, 3, "...|..."},     //
                               //
                               //
        {3, 1, ".|.|."},       //
        {3, 2, "..|..|.."},    //
        {3, 3, "...|...|..."}, //
    };

    for (const test_case_t& test_case : test_cases)
    {
        const int r = get<0>(test_case);
        const int c = get<1>(test_case);
        const string& exp = get<2>(test_case);

        string got;
        grid_generator::init_board_helper(got, int_pair(r, c), '.');
        assert(got == exp);
    }
}

void test_clobber()
{
    grid_generator_clobber gen_strip(5);
    grid_generator_clobber gen_grid(3, 3);

    assert_stone_ordering(gen_strip, true);
    assert_stone_ordering(gen_grid, true);
}

void test_nogo()
{
    grid_generator_nogo gen_strip(5);
    grid_generator_nogo gen_grid(3, 3);

    assert_stone_ordering(gen_strip, false);
    assert_stone_ordering(gen_grid, false);
}

void test_equality()
{
    {

        vector<unordered_set<string>> strings;
        strings.reserve(3);

        strings.emplace_back(get_all_boards(grid_generator_default(5)));
        strings.emplace_back(get_all_boards(grid_generator_clobber(5)));
        strings.emplace_back(get_all_boards(grid_generator_nogo(5)));

        assert(strings.size() == 3);
        assert(strings[0] == strings[1]);
        assert(strings[1] == strings[2]);
    }

    {
        vector<unordered_set<string>> strings;
        strings.reserve(3);

        strings.emplace_back(get_all_boards(grid_generator_default(3, 3)));
        strings.emplace_back(get_all_boards(grid_generator_clobber(3, 3)));
        strings.emplace_back(get_all_boards(grid_generator_nogo(3, 3)));

        assert(strings.size() == 3);
        assert(strings[0] == strings[1]);
        assert(strings[1] == strings[2]);
    }
}

} // namespace

//////////////////////////////////////////////////

void grid_generator_test_all()
{
    test_1x2();
    test_2x1();
    test_2x2();
    test_empties();
    test_clobber();
    test_nogo();
    test_equality();
}
