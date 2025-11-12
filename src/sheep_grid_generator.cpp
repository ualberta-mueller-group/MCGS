#include "sheep_grid_generator.h"

#include <cassert>
#include <algorithm>

#include "cgt_basics.h"
#include "grid_generator.h"
#include "safe_arithmetic.h"
#include "utilities.h"
#include "throw_assert.h"

using namespace std;

////////////////////////////////////////////////// sheep_grid_generator methods

sheep_grid_generator::sheep_grid_generator(const int_pair& max_dims,
                                           int max_black_sheep,
                                           int max_white_sheep)
    : _max_dims(max_dims),
      _max_black_sheep(max_black_sheep),
      _max_white_sheep(max_white_sheep)
{
    THROW_ASSERT(_max_dims.first >= 0 &&  //
           _max_dims.second >= 0 && //
           _max_black_sheep >= 0 && //
           _max_white_sheep >= 0    //
    );

    _increment(true);
}

sheep_grid_generator::operator bool() const
{
    return i_grid_generator::dims_le_max_transpose(_dims, _max_dims);
}

void sheep_grid_generator::operator++()
{
    assert(*this);
    _increment(false);
}

const std::vector<int>& sheep_grid_generator::gen_board() const
{
    assert(*this &&                     //
           _total_sheep_correct(_board) //
    );

    return _board;
}

int_pair sheep_grid_generator::get_shape() const
{
    assert(*this);
    return _dims;
}

bool sheep_grid_generator::only_strips() const
{
    return false;
}

bool sheep_grid_generator::_increment(bool init)
{
    assert(init || *this);

    if (init)
        _dims = int_pair(0, 0);

    bool has_dims = !init;
    bool has_sheep_counts = !init;
    bool has_board = !init;

    while (true)
    {
        // Try to increment board
        if (has_sheep_counts && _increment_board(!has_board))
        {
            has_board = true;
            assert(*this);
            return true;
        }

        has_board = false;

        // Try to increment sheep_counts
        if (has_dims && _increment_sheep_counts(!has_sheep_counts))
        {
            has_sheep_counts = true;
            continue;
        }

        has_sheep_counts = false;

        // Try to increment dims
        if (_increment_dims(!has_dims))
        {
            has_dims = true;
            continue;
        }

        has_dims = false;

        assert(!*this);
        return false;
    }
}

bool sheep_grid_generator::_increment_dims(bool init)
{
    return i_grid_generator::increment_dims_transpose(_dims, _max_dims, init);
}

bool sheep_grid_generator::_increment_sheep_counts(bool init)
{
    if (_sheep_count_vec.empty())
    {
        for (int b = 0; b <= _max_black_sheep; b++)
            for (int w = 0; w <= _max_white_sheep; w++)
                _sheep_count_vec.emplace_back(b, w);

        if (!_sheep_count_vec.empty())
            sort(_sheep_count_vec.begin(), _sheep_count_vec.end(),
                 _sheep_count_order_fn);
    }

    // edge case...
    if (_dims.first == 0 && //
        _dims.second == 0   //
    )
    {
        _sheep_count = int_pair(0, 0);
        return init;
    }

    if (init)
        _sheep_count_idx = 0;
    else
        _sheep_count_idx++;

    if (_sheep_count_idx < _sheep_count_vec.size())
    {
        _sheep_count = _sheep_count_vec[_sheep_count_idx];
        return true;
    }

    return false;
}

/*
    Confusing reformulation of a recursive search problem, into an iterative
    one.

    Imagine we have a recursive function:

    search(vector<int>& current_board, int current_idx, int black_remaining,
           int white_remaining)

    _board is initialized to the current grid dimensions (with 0s), then the
    first call is:
    search(_board, 0, _sheep_count.first, _sheep_count.second)

    This call must print all boards (with the given dimensions)
    with exactly _sheep_count.first total
    black sheep, and _sheep_count.second total white sheep

    This is done by making a choice of sheep to allocate at current_idx,
    then recursively calling search() at current_idx + 1, with updated
    black_remaining and white_remaining.

    The actual function below is an iterative solution
    that returns every time it finds such a board, and must continue where it
    left off upon the next call
*/
bool sheep_grid_generator::_increment_board(bool init)
{
    if (init)
    {
        _board.clear();
        const int area = _dims.first * _dims.second;
        _board.resize(area, 0);

        _current_idx = 0;
    }

    const size_t BOARD_SIZE = _board.size();

    // edge case
    if (BOARD_SIZE == 0)
        return init;

    /*
       How many sheep are left to allocate. The board is accepted when we've
       allocated ALL sheep.

       Either all have been allocated previously and we are resuming search,
       or this is a new board and we have yet to allocate any.
    */
    int black_remaining = init ? _sheep_count.first : 0;
    int white_remaining = init ? _sheep_count.second : 0;

    /*
       True IFF we're visiting this index by moving right.

       Or anologously, true IFF we would be at the start of a recursive call.
    */
    bool new_idx = init;

    while (true)
    {
        assert(_current_idx < BOARD_SIZE &&                        //
               in_interval(black_remaining, 0, _sheep_count.first) && //
               in_interval(white_remaining, 0, _sheep_count.second)   //
        );

        int prev_choice = _board[_current_idx];

        // Not a true "choice" but the board should be 0 here
        assert(LOGICAL_IMPLIES(new_idx, prev_choice == 0));

        // Undo previous choice (if we've made one)
        if (!new_idx)
        {
            _board[_current_idx] = 0;

            if (prev_choice >= 0) // refund sheep
                black_remaining += prev_choice;
            else
                white_remaining -= prev_choice;
        }

        // Get new choice
        optional<int> choice_opt = _get_next_choice(
            black_remaining, white_remaining, prev_choice, new_idx);
        new_idx = false;

        // Apply if possible
        if (choice_opt.has_value())
        {
            const int choice = choice_opt.value();
            _board[_current_idx] = choice;

            if (choice >= 0) // spend sheep
                black_remaining -= choice;
            else
                white_remaining += choice;

            // Accept if this board is a solution
            if (black_remaining == 0 && white_remaining == 0)
                return true;

            // If not, try to move right.
            // If we can't move right, we stay here.
            if (_current_idx + 1 < BOARD_SIZE)
            {
                _current_idx += 1;
                new_idx = true;
            }
            continue;
        }

        // No more choices, must move left

        // If not possible, there are no more solutions
        if (_current_idx == 0)
            return false;

        // Move left
        _current_idx--;
        new_idx = false;
    }
}


bool sheep_grid_generator::_total_sheep_correct(const vector<int>& board) const
{
    int total_black = 0;
    int total_white = 0;

    for (const int& val : board)
    {
        if (val > 0)
            total_black += val;
        if (val < 0)
            total_white -= val;
    }

    return total_black == _sheep_count.first && //
           total_white == _sheep_count.second;  //
}

// 1, -1, 0, 2, 3, ..., black_remaining, -2, -3, ..., -white_remaining
optional<int> sheep_grid_generator::_get_next_choice(int black_remaining,
                                           int white_remaining, int prev,
                                           bool init)
{
    assert(black_remaining >= 0 && //
           white_remaining >= 0    //
    );


    // 1
    if (init)
    {
        if (black_remaining > 0)
            return 1;

        prev = 1;
    }

    // -1
    if (prev == 1)
    {
        if (white_remaining > 0)
            return -1;

        prev = -1;
    }

    // 0
    if (prev == -1)
        return 0;

    if (prev == 0)
        prev = 1;

    // 2, ..., black_remaining
    if (prev >= 0 && prev < black_remaining)
        return prev + 1;

    // -2, ..., -white_remaining
    if (prev >= 0)
        prev = -1;

    THROW_ASSERT(negate_is_safe(white_remaining));
    if (prev > -white_remaining)
        return prev - 1;

    return {};
}

bool sheep_grid_generator::_sheep_count_order_fn(const std::pair<int, int>& p1,
                                                 const std::pair<int, int>& p2)
{
    const int max1 = max(p1.first, p1.second);
    const int max2 = max(p2.first, p2.second);

    if (max1 != max2)
        return max1 < max2;

    const int min1 = min(p1.first, p1.second);
    const int min2 = min(p2.first, p2.second);

    if (min1 != min2)
        return min1 < min2;

    return false;
}

//////////////////////////////////////////////////
void test_sheep_grid_generator()
{

    sheep_grid_generator gen(int_pair(2, 3), 2, 2);

    while (gen)
    {
        cout << gen.get_shape() << " " << gen.gen_board() << endl;
        ++gen;
    }

    //const int_pair max_dims(3, 2);
    //int_pair dims;
    //bool init = true;

    //while (i_grid_generator::increment_dims_transpose(dims, max_dims, init))
    //{
    //    cout << dims << endl;
    //    init = false;
    //}

}

