#include "sheep_generator_sketch.h"

#include <vector>
#include <iostream>
#include <cassert>

#include "int_pair.h"
#include "utilities.h"

using namespace std;


namespace {

////////////////////////////////////////////////// class
class sheep_generator
{
public:
    void search_start(int_pair dims, int b_sheep, int w_sheep, int k);
    void search(int idx, int b_sheep, int w_sheep, int remaining_spaces);

    static bool is_possible(int b_sheep, int w_sheep, int remaining_spaces, int k);

    int_pair dims;
    int area;
    vector<int> board;

    int k;

};

////////////////////////////////////////////////// methods


void sheep_generator::search_start(int_pair dims, int b_sheep, int w_sheep, int k)
{
    assert(k >= 1);

    this->dims = dims;
    area = dims.first * dims.second;

    if (!is_possible(b_sheep, w_sheep, area, k))
        return;

    this->board.resize(area, 0);

    this->k = k;

    search(0, b_sheep, w_sheep, area);
}

void sheep_generator::search(int idx, int b_sheep, int w_sheep, int remaining_spaces)
{
    if (b_sheep == 0 && w_sheep == 0)
    {
        cout << board << endl;
        return;
    }

    if (idx == area)
        return;

    if (!is_possible(b_sheep, w_sheep, remaining_spaces, k))
        return;

    assert(board[idx] == 0);

    // 1, -1, 0, 2, 3, ..., -2, -3, ...
    if (b_sheep > 0)
    {
        board[idx] = 1;
        search(idx + 1, b_sheep - 1, w_sheep, remaining_spaces - 1);
        board[idx] = 0;
    }

    if (w_sheep > 0)
    {
        board[idx] = -1;
        search(idx + 1, b_sheep, w_sheep -1, remaining_spaces - 1);
        board[idx] = 0;
    }

    search(idx + 1, b_sheep, w_sheep, remaining_spaces);

    for (int b = 2; b <= min(b_sheep, k); b++)
    {
        board[idx] = b;
        search(idx + 1, b_sheep - b, w_sheep, remaining_spaces - 1);
        board[idx] = 0;
    }

    for (int w = 2; w <= min(w_sheep, k); w++)
    {
        board[idx] = -w;
        search(idx + 1, b_sheep, w_sheep - w, remaining_spaces - 1);
        board[idx] = 0;
    }
}


bool sheep_generator::is_possible(int b_sheep, int w_sheep, int remaining_spaces, int k)
{
    const int b_spaces = div_ceil(b_sheep, k);
    const int w_spaces = div_ceil(w_sheep, k);
    return (b_spaces + w_spaces) < remaining_spaces;
}

} // namespace

void test_sheep_generator_sketch()
{
    sheep_generator gen;
    gen.search_start(int_pair(2, 3), 3, 3, 2);
}
