#include "grid_generator_new.h"
#include "grid_utils.h"

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {

bool increment_char_clobber_bw(char& c)
{
    if (c == 'X')
    {
        c = 'O';
        return true;
    }

    assert(c == 'O');
    c = 'X';
    return false;
}

bool increment_char_clobber_bwe(char& c)
{
    if (c == '.')
    {
        c = 'X';
        return true;
    }

    if (c == 'X')
    {
        c = 'O';
        return true;
    }

    assert(c == 'O');
    c = '.';
    return false;
}

} // namespace

////////////////////////////////////////////////// grid_mask methods
namespace ggen_impl {

void grid_mask::set_shape(const int_pair& shape)
{
    assert(shape.first >= 0 && shape.second >= 0);

    const int mask_size = shape.first * shape.second;

    _mask.resize(mask_size);
    for (size_t i = 0; i < mask_size; i++)
        _mask[i] = false;

    _indices.clear();
    _indices.reserve(mask_size);
}

bool grid_mask::increment()
{
    // At most as many markers as the board would allow
    assert(_indices.size() <= _mask.size());

    // If there is at least one marker, try to move right, starting with last
    // marker
    if (                                                            //
        (_indices.size() > 0) &&                                    //
        _increment_by_moving(_indices.size() - 1, _mask.size() - 1) //
        )                                                           //
        return true;

    // Otherwise add another marker and reset positions of all markers
    return _increment_by_adding();
}

bool grid_mask::_increment_by_moving(size_t marker_number, size_t max_pos)
{
    assert(max_pos >= marker_number);

    // Pick up marker i
    size_t& mi_pos = _indices[marker_number];
    assert(_mask[mi_pos]);
    _mask[mi_pos] = false;

    // Place marker i to the right if possible
    const size_t pos_right = mi_pos + 1;

    if (pos_right <= max_pos)
    {
        mi_pos = pos_right;
        assert(!_mask[pos_right]);
        _mask[pos_right] = true;
        return true;
    }

    // Otherwise try to move marker i-1 to the right, and place
    // marker i to its right
    if (marker_number > 0 && _increment_by_moving(marker_number - 1, max_pos - 1))
    {
        const size_t pos_prev = _indices[marker_number - 1];
        const size_t pos_new = pos_prev + 1;

        assert(pos_new < _mask.size() && !_mask[pos_new]);

        mi_pos = pos_new;
        _mask[pos_new] = true;
        return true;
    }

    return false;
}

bool grid_mask::_increment_by_adding()
{
    // No space for another marker
    if (_indices.size() >= _mask.size())
        return false;

    // Reset mask
    {
        const size_t mask_size = _mask.size();
        for (size_t i = 0; i < mask_size; i++)
            _mask[i] = false;
    }

    // Add another marker
    _indices.push_back(0);
    const size_t n_markers = _indices.size();

    for (size_t i = 0; i < n_markers; i++)
    {
        _indices[i] = i;
        _mask[i] = true;
    }

    return true;
}

// TODO why can't this just use the template from utilities.h? Header problem?
std::ostream& operator<<(std::ostream& os, const grid_mask& mask)
{
    const vector<bool>& vec = mask._mask;

    os << '[';

    const size_t N = vec.size();
    for (size_t i = 0; i < N; i++)
    {
        os << vec[i];

        if (i + 1 < N)
            os << ", ";
    }

    os << ']';
    return os;
}

} // namespace ggen_impl

////////////////////////////////////////////////// ggen methods
bool ggen::_increment_shape_helper(int_pair& shape, const int_pair& max_shape)
{
    assert((shape.first <= max_shape.first) && (shape.second <= max_shape.second));

    if (shape.first == 0 && shape.second == 0)
        shape = int_pair(1, 1);
    else
    {
        shape.second++;
        if (shape.second > max_shape.second)
        {
            shape.second = 1;
            shape.first++;
        }
    }

    return (shape.first <= max_shape.first) && (shape.second <= max_shape.second);
}

void ggen::_init_board_helper(std::string& board, const int_pair& shape,
                              char init_char)
{
    assert(shape.first >= 0 && shape.second >= 0);

    size_t board_size = shape.first * shape.second;
    if (board_size > 0)
        board_size += shape.first - 1;

    board.resize(board_size);

    size_t col_idx = 0;

    for (size_t i = 0; i < board_size; i++)
    {
        if (col_idx == shape.second)
        {
            // TODO use ROW_SEP instead of literal
            board[i] = '|';
            col_idx = 0;
            continue;
        }

        board[i] = init_char;
        col_idx++;
    }
}

////////////////////////////////////////////////// ggen_basic methods
void ggen_basic::operator++()
{
    assert(*this);

    if (_increment_board())
        return;

    if (_increment_shape())
    {
        _init_board();
        return;
    }
}

////////////////////////////////////////////////// ggen_masked methods
void ggen_masked::operator++()
{
    assert(*this);

    if (_increment_board())
        return;

    if (_increment_mask())
    {
        _init_board();
        return;
    }

    if (_increment_shape())
    {
        _init_mask();
        _init_board();
        return;
    }
}

void ggen_masked::_init_board_helper_masked(std::string& board,
                                            const int_pair& shape,
                                            const ggen_impl::grid_mask& mask,
                                            char true_char, char false_char)
{
    assert(shape.first >= 0 && shape.second >= 0);

    size_t board_size = shape.first * shape.second;
    if (board_size > 0)
        board_size += shape.first - 1;

    board.resize(board_size);

    size_t mask_idx = 0;
    size_t col_idx = 0;

    for (size_t i = 0; i < board_size; i++)
    {
        if (col_idx == shape.second)
        {
            // TODO use ROW_SEP instead of literal
            board[i] = '|';
            col_idx = 0;
            continue;
        }

        const char tile = mask[mask_idx] ? true_char : false_char;
        board[i] = tile;

        mask_idx++;
        col_idx++;
    }
}


////////////////////////////////////////////////// ggen_default methods
bool ggen_default::_increment_board()
{
    bool carry = true;

    for (auto it = _board.rbegin(); it != _board.rend(); it++)
    {
        char& c = *it;

        if (c == '|')
            continue;

        carry = !increment_char_clobber_bwe(c);

        if (!carry)
            break;
    }

    return !carry;
}

////////////////////////////////////////////////// ggen_clobber methods
bool ggen_clobber::_increment_board()
{
    bool carry = true;

    size_t mask_idx = 0;
    const size_t board_size = _board.size();

    for (size_t i = 0; i < board_size; i++)
    {
        if (_board[i] == '|')
            continue;

        if (!_mask[mask_idx++])
            continue;

        carry = !increment_char_clobber_bw(_board[i]);

        if (!carry)
            break;
    }

    return !carry;
}

////////////////////////////////////////////////// ggen_nogo methods
bool ggen_nogo::_increment_board()
{
    bool carry = true;

    size_t mask_idx = 0;
    const size_t board_size = _board.size();

    for (size_t i = 0; i < board_size; i++)
    {
        if (_board[i] == '|')
            continue;

        if (_mask[mask_idx++])
            continue;

        carry = !increment_char_clobber_bw(_board[i]);

        if (!carry)
            break;
    }

    return !carry;
}

//////////////////////////////////////////////////
void test_grid_generator_new()
{
    ggen_nogo gen(int_pair(2, 2));

    while (gen)
    {
        cout << gen.gen_board() << endl;
        ++gen;
    }
}

