#include "grid_generator_new.h"
#include "cgt_basics.h"
#include "strip.h"

////////////////////////////////////////////////// classes
class ggen_mask
{
public:
    ggen_mask() {}

    size_t size() const;
    bool operator[](const size_t idx) const;
    void set_shape(const int_pair& shape);
    bool increment();

protected:
    std::vector<bool> _mask;
    std::vector<size_t> _indices;

    bool _increment_by_moving(size_t marker_idx, size_t max_pos);
    bool _increment_by_adding();

private:
    friend std::ostream& operator<<(std::ostream& os, const ggen_mask& mask);
};

class ggen
{
public:
    ggen(const int_pair& max_shape);
    virtual ~ggen() {}

    // Methods
    operator bool() const;
    const std::string& gen_board() const;
    void operator++();
    const int_pair get_shape() const;

protected:
    // Data
    const int_pair _max_shape;

    int_pair _shape;
    ggen_mask _mask;
    std::string _board;

    // Helpers
    static bool _increment_shape_helper(int_pair& shape, const int_pair& max_shape);

    static void _init_board_helper(std::string& board, const int_pair& shape,
                                   const ggen_mask& mask, const char true_char,
                                   const char false_char);

    // Methods
    bool _increment_shape();
    bool _increment_mask();

    // Abstract methods
    virtual void _init_board() = 0;
    virtual bool _increment_board() = 0;
};

class ggen_clobber: public ggen
{
public:
    ggen_clobber(const int_pair& max_shape);

protected:
    void _init_board() override;
    bool _increment_board() override;
};

class ggen_nogo: public ggen
{
public:
    ggen_nogo(const int_pair& max_shape);

protected:
    void _init_board() override;
    bool _increment_board() override;
};

////////////////////////////////////////////////// methods
//////////////////////////////////////// ggen_mask
size_t ggen_mask::size() const
{
    return _mask.size();
}

bool ggen_mask::operator[](const size_t idx) const
{
    return _mask[idx];
}

void ggen_mask::set_shape(const int_pair& shape)
{
    assert(shape.first >= 0 && shape.second >= 0);

    const int mask_size = shape.first * shape.second;

    _mask.resize(mask_size);
    for (size_t i = 0; i < mask_size; i++)
        _mask[i] = false;

    _indices.clear();
    _indices.reserve(mask_size);
}

bool ggen_mask::increment()
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

bool ggen_mask::_increment_by_moving(size_t marker_number, size_t max_pos)
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

bool ggen_mask::_increment_by_adding()
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

std::ostream& operator<<(std::ostream& os, const ggen_mask& mask)
{
    os << mask._mask;
    return os;
}

//////////////////////////////////////// ggen
ggen::ggen(const int_pair& max_shape): _max_shape(max_shape)
{
}

ggen::operator bool() const
{
    return _shape.first <= _max_shape.first;
}

const std::string& ggen::gen_board() const
{
    assert(*this);
    return _board;
}

void ggen::operator++()
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
        _mask.set_shape(_shape);
        _init_board();
        return;
    }
}

const int_pair ggen::get_shape() const
{
    assert(*this);
    return _shape;
}

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
                              const ggen_mask& mask, const char true_char,
                              const char false_char)
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

bool ggen::_increment_shape()
{
    return _increment_shape_helper(_shape, _max_shape);
}

bool ggen::_increment_mask()
{
    return _mask.increment();
}



//////////////////////////////////////// ggen_clobber
namespace {
bool increment_char_clobber(char& c)
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

} // namespace

ggen_clobber::ggen_clobber(const int_pair& max_shape): ggen(max_shape)
{
}

void ggen_clobber::_init_board()
{
    const char char_black = color_to_clobber_char(BLACK);
    const char char_empty = color_to_clobber_char(EMPTY);
    _init_board_helper(_board, _shape, _mask, char_black, char_empty);
}

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

        carry = !increment_char_clobber(_board[i]);

        if (!carry)
            break;
    }

    return !carry;
}

//////////////////////////////////////// ggen_nogo
ggen_nogo::ggen_nogo(const int_pair& max_shape): ggen(max_shape)
{
}

void ggen_nogo::_init_board()
{
    const char char_black = color_to_clobber_char(BLACK);
    const char char_empty = color_to_clobber_char(EMPTY);
    _init_board_helper(_board, _shape, _mask, char_empty, char_black);
}

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

        carry = !increment_char_clobber(_board[i]);

        if (!carry)
            break;
    }

    return !carry;
}


//////////////////////////////////////////////////

using namespace std;

void test_grid_generator_new()
{
    ggen_nogo gen(int_pair(2, 2));

    while (gen)
    {
        cout << gen.gen_board() << endl;
        ++gen;
    }


}

