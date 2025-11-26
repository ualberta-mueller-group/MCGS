#include "grid_generator_test.h"

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#include "cgt_basics.h"
#include "grid_generator.h"
#include "grid_hash.h"
#include "strip.h"
#include "grid.h"
#include "test_utilities.h"

using namespace std;

namespace {

[[maybe_unused]] void print_sequence(const vector<string>& sequence)
{
    cout << "========================================" << endl;
    for (const string& str : sequence)
        cout << "\"" << str << "\"" << endl;
    cout << "========================================" << endl;
}

vector<string> get_board_string_sequence(grid_generator& gen)
{
    vector<string> sequence;

    while (gen)
    {
        sequence.emplace_back(
            grid::board_to_string(gen.gen_board(), gen.get_shape()));

        ++gen;
    }

    return sequence;
}


// 0 size grids
void test_basic1()
{
    const vector<string> exp_sequence =
    {
        "",
    };

    grid_generator gen1(int_pair(0, 0), {BORDER, WHITE}, false);
    vector<string> sequence1 = get_board_string_sequence(gen1);

    grid_generator gen2(int_pair(0, 0), {BORDER, WHITE}, true);
    vector<string> sequence2 = get_board_string_sequence(gen2);

    grid_generator gen3(int_pair(0, 1), {BORDER, WHITE}, false);
    vector<string> sequence3 = get_board_string_sequence(gen3);

    grid_generator gen4(int_pair(0, 1), {BORDER, WHITE}, true);
    vector<string> sequence4 = get_board_string_sequence(gen4);

    grid_generator gen5(int_pair(1, 0), {BORDER, WHITE}, false);
    vector<string> sequence5 = get_board_string_sequence(gen5);

    grid_generator gen6(int_pair(1, 0), {BORDER, WHITE}, true);
    vector<string> sequence6 = get_board_string_sequence(gen6);

    assert(sequence1 == exp_sequence);
    assert(sequence2 == exp_sequence);
    assert(sequence3 == exp_sequence);
    assert(sequence4 == exp_sequence);
    assert(sequence5 == exp_sequence);
    assert(sequence6 == exp_sequence);
}


// Strips test
void test_basic2()
{
    const vector<string> exp_sequence =
    {
        "",
        "#",
        "O",
        "##",
        "#O",
        "O#",
        "OO",
        "###",
        "##O",
        "#O#",
        "#OO",
        "O##",
        "O#O",
        "OO#",
        "OOO",
    };

    grid_generator gen1(int_pair(1, 3), {BORDER, WHITE}, true);
    vector<string> sequence1 = get_board_string_sequence(gen1);

    ASSERT_DID_THROW(grid_generator gen2(int_pair(3, 1), {BORDER, WHITE}, true););

    assert(sequence1 == exp_sequence);
}

// 1x2 and 2x1 (should be the same)
void test_basic3()
{
    const vector<string> exp_sequence =
    {
        "",
        "#",
        "O",
        "##",
        "#O",
        "O#",
        "OO",
        "#|#",
        "#|O",
        "O|#",
        "O|O",
    };

    grid_generator gen1(int_pair(1, 2), {BORDER, WHITE}, false);
    vector<string> sequence1 = get_board_string_sequence(gen1);

    grid_generator gen2(int_pair(2, 1), {BORDER, WHITE}, false);
    vector<string> sequence2 = get_board_string_sequence(gen2);

    assert(sequence1 == exp_sequence);
    assert(sequence2 == exp_sequence);
}

// another 1x2 and 2x1
void test_basic4()
{
    const vector<string> exp_sequence =
    {
        "",

        ".",
        "X",
        "O",

        "..",
        ".X",
        ".O",
        "X.",
        "XX",
        "XO",
        "O.",
        "OX",
        "OO",

        ".|.",
        ".|X",
        ".|O",
        "X|.",
        "X|X",
        "X|O",
        "O|.",
        "O|X",
        "O|O",
    };

    grid_generator gen1(int_pair(1, 2), {EMPTY, BLACK, WHITE}, false);
    vector<string> sequence1 = get_board_string_sequence(gen1);

    grid_generator gen2(int_pair(2, 1), {EMPTY, BLACK, WHITE}, false);
    vector<string> sequence2 = get_board_string_sequence(gen2);


    assert(sequence1 == exp_sequence);
    assert(sequence2 == exp_sequence);
}

// clobber without symmetry pruning
void test_masked1()
{
    const vector<string> exp_sequence =
    {
        "",
        ".",
        "X",
        "O",
        "..",
        "X.",
        "O.",
        ".X",
        ".O",
        "XX",
        "XO",
        "OX",
        "OO",
        ".|.",
        "X|.",
        "O|.",
        ".|X",
        ".|O",
        "X|X",
        "X|O",
        "O|X",
        "O|O",
    };

    grid_generator gen1(int_pair(1, 2), {BLACK, WHITE}, true, EMPTY, false,
                        GRID_HASH_ACTIVE_MASK_IDENTITY);
    vector<string> sequence1 = get_board_string_sequence(gen1);

    grid_generator gen2(int_pair(2, 1), {BLACK, WHITE}, true, EMPTY, false,
                        GRID_HASH_ACTIVE_MASK_IDENTITY);
    vector<string> sequence2 = get_board_string_sequence(gen2);

    assert(sequence1 == exp_sequence);
    assert(sequence2 == exp_sequence);
}

// nogo without symmetry pruning
void test_masked2()
{
    const vector<string> exp_sequence =
    {
        "",
        "X",
        "O",
        ".",
        "XX",
        "XO",
        "OX",
        "OO",
        ".X",
        ".O",
        "X.",
        "O.",
        "..",
        "X|X",
        "X|O",
        "O|X",
        "O|O",
        ".|X",
        ".|O",
        "X|.",
        "O|.",
        ".|.",
    };

    grid_generator gen1(int_pair(1, 2), {BLACK, WHITE}, false, EMPTY, false,
                        GRID_HASH_ACTIVE_MASK_IDENTITY);
    vector<string> sequence1 = get_board_string_sequence(gen1);

    grid_generator gen2(int_pair(2, 1), {BLACK, WHITE}, false, EMPTY, false,
                        GRID_HASH_ACTIVE_MASK_IDENTITY);
    vector<string> sequence2 = get_board_string_sequence(gen2);

    assert(sequence1 == exp_sequence);
    assert(sequence2 == exp_sequence);
}

// ALL symmetry pruning
void test_masked3()
{
    const vector<string> exp_sequence =
    {
        "",
        ".",
        "X",
        "O",
        "..",
        "X.",
        "O.",
        //".X",
        //".O",
        "XX",
        "XO",
        "OX",
        "OO",
        //".|.",
        //"X|.",
        //"O|.",
        //".|X",
        //".|O",
        //"X|X",
        //"X|O",
        //"O|X",
        //"O|O",
    };

    grid_generator gen1(int_pair(1, 2), {BLACK, WHITE}, true, EMPTY, false,
                        GRID_HASH_ACTIVE_MASK_ALL);
    vector<string> sequence1 = get_board_string_sequence(gen1);

    grid_generator gen2(int_pair(2, 1), {BLACK, WHITE}, true, EMPTY, false,
                        GRID_HASH_ACTIVE_MASK_ALL);
    vector<string> sequence2 = get_board_string_sequence(gen2);

    assert(sequence1 == exp_sequence);
    assert(sequence2 == exp_sequence);
}

// MIRRORS symmetry pruning
void test_masked4()
{
    const vector<string> exp_sequence =
    {
        "",
        ".",
        "#",
        "..",
        "#.",
        //".#",
        "##",
        ".|.",
        "#|.",
        //".|#",
        "#|#",
        "..|..",
        "#.|..",
        //".#|..",
        //"..|#.",
        //"..|.#",
        "##|..",
        "#.|#.",
        "#.|.#",
        //".#|#.",
        //".#|.#",
        //"..|##",
        "##|#.",
        //"##|.#",
        //"#.|##",
        //".#|##",
        "##|##",
    };

    grid_generator gen1(int_pair(2, 2), {BORDER}, true, EMPTY, false,
                        GRID_HASH_ACTIVE_MASK_MIRRORS);
    vector<string> sequence1 = get_board_string_sequence(gen1);

    assert(sequence1 == exp_sequence);
}

// strip with ALL symmetry pruning
void test_masked5()
{
    const vector<string> exp_sequence =
    {
        "",
        ".",
        "#",
        "..",
        "#.",
        //".#",
        "##",
        "...",
        "#..",
        ".#.",
        //"..#",
        "##.",
        "#.#",
        //".##",
        "###",
        "....",
        "#...",
        ".#..",
        //"..#.",
        //"...#",
        "##..",
        "#.#.",
        "#..#",
        ".##.",
        //".#.#",
        //"..##",
        "###.",
        "##.#",
        //"#.##",
        //".###",
        "####",
    };

    grid_generator gen1(int_pair(1, 4), {BORDER}, true, EMPTY, true,
                        GRID_HASH_ACTIVE_MASK_ALL);
    vector<string> sequence1 = get_board_string_sequence(gen1);

    assert(sequence1 == exp_sequence);
}


} // namespace

//////////////////////////////////////////////////
void grid_generator_test_all()
{
    test_basic1();
    test_basic2();
    test_basic3();
    test_basic4();

    test_masked1();
    test_masked2();
    test_masked3();
    test_masked4();
    test_masked5();

    cout << __FILE__ << endl;
}

