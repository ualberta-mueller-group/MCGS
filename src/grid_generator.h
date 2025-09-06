/*
   grid_generator interface for iterating over string representations of grids
   (and strips, if row dimension is 1)

    Abstract classes providing some functionality:
        grid_generator_base
        grid_generator_masked

    Non-abstract classes:
        grid_generator_default
        grid_generator_clobber
        grid_generator_nogo
*/
#pragma once

#include <iostream>
#include <vector>
#include <cassert>
#include <cstddef>
#include "grid.h"
#include "strip.h"

/*
   TODO some of these probably have edge cases when some functions are
   overridden. The virtual functions probably suggest more generality than
   is implemented, particularly around _init and _increment virtual functions

   Both _init and _increment functions should return bools and be allowed to
   fail

   TODO why aren't these game orderings faster when generating the database?
*/

////////////////////////////////////////////////// grid_mask
namespace grid_generator_impl {
/*
    Boolean mask generator for grids.

    The mask is flattened and doesn't represent row separators (SEP '|'). i.e.
    for a 3x3 grid, the mask is a flat array of 9 bools.

    Used to define clobber and nogo database game generator orderings:
        Clobber iterates in order of increasing number of stones (stones on
        the true spaces).

        NoGo iterates in order of decreasing number of stones (stones on the
        false spaces).

    Example sequence for 2x2:
        [0, 0, 0, 0]

        [1, 0, 0, 0]
        [0, 1, 0, 0]
        [0, 0, 1, 0]
        [0, 0, 0, 1]

        [1, 1, 0, 0]
        [1, 0, 1, 0]
        [1, 0, 0, 1]
        [0, 1, 1, 0]
        [0, 1, 0, 1]
        [0, 0, 1, 1]

        [1, 1, 1, 0]
        [1, 1, 0, 1]
        [1, 0, 1, 1]
        [0, 1, 1, 1]

        [1, 1, 1, 1]

    Note that because the row separator is not represented, indexing doesn't
    align between the string representation of a grid, and the mask (when the
    grid has at least 2 rows).
*/
class grid_mask
{
public:
    grid_mask();
    grid_mask(const int_pair& shape);

    size_t size() const; // number of tiles (excluding SEP)
    void set_shape(const int_pair& shape);

    bool operator[](size_t idx) const;

    operator bool() const;
    void operator++();

protected:
    size_t _marker_count_end;

    std::vector<bool> _mask;
    std::vector<size_t> _indices; // locations of each marker in the mask

    bool _increment_by_moving(size_t marker_idx, size_t max_pos);
    bool _increment_by_adding();

private:
    friend std::ostream& operator<<(std::ostream& os, const grid_mask& mask);
};

std::ostream& operator<<(std::ostream& os, const grid_mask& mask);

} // namespace grid_generator_impl

////////////////////////////////////////////////// grid_generator
/*
   Interface class for grid generators
*/
class grid_generator
{
public:
    virtual ~grid_generator() {}

    virtual operator bool() const = 0;
    virtual void operator++() = 0;

    virtual const std::string& gen_board() const = 0;
    virtual int_pair get_shape() const = 0;

    // Helper functions likely to be useful for most grid generators
    static bool increment_shape_helper(int_pair& shape,
                                       const int_pair& max_shape);

    static void init_board_helper(std::string& board, const int_pair& shape,
                                  char init_char);
};

////////////////////////////////////////////////// grid_generator_base
/*
    Abstract grid generator providing basic functionality to be used by
    most grid generator types

    Iterates over shapes i.e. for max_shape of 3x3:
        0x0 -> 1x1 -> 1x2 -> 1x3 -> 2x1 -> 2x2 -> ... -> 3x2 -> 3x3

    Derived type must implement:
        _init_board()
            Initialize board string for current shape, i.e.
            for 2x2: "XX|XX" or "..|.."
        _increment_board()
            Increment current string board (i.e "XX|XX" -> "OX|XX")
*/
class grid_generator_base : public grid_generator
{
public:
    grid_generator_base(const int_pair& max_shape);
    grid_generator_base(int max_rows, int max_cols);
    grid_generator_base(int max_cols);

    virtual ~grid_generator_base() {}

    operator bool() const override;
    void operator++() override;

    const std::string& gen_board() const override;
    int_pair get_shape() const override;

protected:
    const int_pair _max_shape;

    int_pair _shape;
    std::string _board;

    virtual bool _increment_shape();

    virtual void _init_board() = 0;
    virtual bool _increment_board() = 0;
};

////////////////////////////////////////////////// grid_generator_masked
/*
    Abstract grid generator adding mask functionality. Used for
    clobber and nogo generators (i.e. to iterate in order of increasing or
    decreasing number of stones)

    See grid_generator_base for mandatory functions to implement.
   Implementations should account for the current mask state

    _init_mask() and _increment_mask() are provided. See grid_mask class
    for ordering. Overrides operator++() to account for mask
*/
class grid_generator_masked : public grid_generator_base
{
public:
    grid_generator_masked(const int_pair& max_shape);
    grid_generator_masked(int max_rows, int max_cols);
    grid_generator_masked(int max_cols);

    virtual ~grid_generator_masked() {}

    void operator++() override;

    /*
       i.e. true/false characters for clobber: 'X' and '.'
       for nogo: '.' and 'X'
    */
    static void init_board_helper_masked(
        std::string& board, const int_pair& shape,
        const grid_generator_impl::grid_mask& mask, char true_char,
        char false_char);

protected:
    grid_generator_impl::grid_mask _mask;

    virtual void _init_mask();
    virtual bool _increment_mask();
};

////////////////////////////////////////////////// grid_generator_default
/*
   Non-abstract basic grid generator.

   i.e for max_shape 2x2:
       "" -> "." -> "X" -> "O" -> ".." -> ... ->
       "..|.." -> "..|.X" -> "..|.O" -> "..|X." -> "..|XX" -> ... -> "OO|OO"
*/
class grid_generator_default : public grid_generator_base
{
public:
    grid_generator_default(const int_pair& max_shape);
    grid_generator_default(int max_rows, int max_cols);
    grid_generator_default(int max_cols);

    virtual ~grid_generator_default() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};

////////////////////////////////////////////////// grid_generator_clobber
/*
   Non-abstract grid generator for Clobber ordering

   i.e. for max_shape 2x2 (empties on the false part of the grid_mask):

    "" -> "." -> "X" -> "O" -> ".." -> ... ->
    "..|.." -> "X.|.." -> "O.|.." -> ".X|.." ->
    ".O|.." -> "..|X." -> "..|O." -> "..|.X" -> "..|.O" -> "XX|.." -> "OX|.." ->
    "XO|.." -> "OO|.." -> "X.|X." -> "O.|X." -> "X.|O." -> "O.|O." -> "X.|.X" ->
    "O.|.X" -> "X.|.O" -> "O.|.O" -> ".X|X." -> ... -> "OO|OO"
*/
class grid_generator_clobber : public grid_generator_masked
{
public:
    grid_generator_clobber(const int_pair& max_shape);
    grid_generator_clobber(int max_rows, int max_cols);
    grid_generator_clobber(int max_cols);

    virtual ~grid_generator_clobber() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};

////////////////////////////////////////////////// grid_generator_nogo
/*
   Non-abstract grid generator for NoGo ordering

    i.e. for max_shape 2x2 (empties on the true part of the grid_mask):

   "" -> "X" -> "O" -> "." -> "XX" -> "OX" -> "XO" -> "OO" -> ".X" -> ".O" ->
   "X." -> "O." -> ".." -> ... -> "X.|O." -> "O.|O." -> "XX|.." -> "OX|.." ->
   "XO|.." -> "OO|.." -> "..|.X" -> "..|.O" -> "..|X." -> "..|O." -> ".X|.." ->
   ".O|.." -> "X.|.." -> "O.|.." -> "..|.."
*/
class grid_generator_nogo : public grid_generator_masked
{
public:
    grid_generator_nogo(const int_pair& max_shape);
    grid_generator_nogo(int max_rows, int max_cols);
    grid_generator_nogo(int max_cols);

    virtual ~grid_generator_nogo() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};


////////////////////////////////////////////////// grid_generator_domineering
/*
   Non-abstract grid generator for domineering. Uses NoGo-like ordering, but
   contents are either '.' or '#'

    i.e. for max_shape 2x2 (empties on the true part of the grid_mask):

    "" -> "#" -> "." -> "##" -> ".#" -> "#." -> ".." -> "#|#" -> ".|#" -> "#|."
    -> ".|." -> "##|##" -> ".#|##" -> "#.|##" -> "##|.#" -> "##|#." -> "..|##"
    -> ".#|.#" -> ".#|#." -> "#.|.#" -> "#.|#." -> "##|.." -> "..|.#" -> "..|#."
    -> ".#|.." -> "#.|.." -> "..|.."
*/
class grid_generator_domineering: public grid_generator_masked
{
public:
    grid_generator_domineering(const int_pair& max_shape);
    grid_generator_domineering(int max_rows, int max_cols);
    grid_generator_domineering(int max_cols);

    virtual ~grid_generator_domineering() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};

////////////////////////////////////////////////// grid_mask methods
namespace grid_generator_impl {

inline grid_mask::grid_mask() : _marker_count_end(0)
{
}

inline grid_mask::grid_mask(const int_pair& shape)
{
    set_shape(shape);
}

inline size_t grid_mask::size() const
{
    assert(*this);
    return _mask.size();
}

inline bool grid_mask::operator[](size_t idx) const
{
    assert(*this);
    return _mask[idx];
}

inline grid_mask::operator bool() const
{
    return _indices.size() < _marker_count_end;
}

} // namespace grid_generator_impl

////////////////////////////////////////////////// grid_generator_base methods
inline grid_generator_base::grid_generator_base(const int_pair& max_shape)
    : _max_shape(max_shape), _shape(0, 0)
{
}

inline grid_generator_base::grid_generator_base(int max_rows, int max_cols)
    : _max_shape(max_rows, max_cols), _shape(0, 0)
{
}

inline grid_generator_base::grid_generator_base(int max_cols)
    : _max_shape(1, max_cols), _shape(0, 0)
{
}

inline grid_generator_base::operator bool() const
{
    return (_shape.first <= _max_shape.first) && //
           (_shape.second <= _max_shape.second); //
}

inline const std::string& grid_generator_base::gen_board() const
{
    assert(*this);
    return _board;
}

inline int_pair grid_generator_base::get_shape() const
{
    assert(*this);
    return _shape;
}

inline bool grid_generator_base::_increment_shape()
{
    return increment_shape_helper(_shape, _max_shape);
}

////////////////////////////////////////////////// grid_generator_masked methods
inline grid_generator_masked::grid_generator_masked(const int_pair& max_shape)
    : grid_generator_base(max_shape), _mask(int_pair(0, 0))
{
}

inline grid_generator_masked::grid_generator_masked(int max_rows, int max_cols)
    : grid_generator_base(max_rows, max_cols), _mask(int_pair(0, 0))
{
}

inline grid_generator_masked::grid_generator_masked(int max_cols)
    : grid_generator_base(1, max_cols), _mask(int_pair(0, 0))
{
}

inline void grid_generator_masked::_init_mask()
{
    _mask.set_shape(_shape);
}

inline bool grid_generator_masked::_increment_mask()
{
    ++_mask;
    return _mask;
}

////////////////////////////////////////////////// grid_generator_default
/// methods
inline grid_generator_default::grid_generator_default(const int_pair& max_shape)
    : grid_generator_base(max_shape)
{
}

inline grid_generator_default::grid_generator_default(int max_rows,
                                                      int max_cols)
    : grid_generator_base(max_rows, max_cols)
{
}

inline grid_generator_default::grid_generator_default(int max_cols)
    : grid_generator_base(1, max_cols)
{
}

inline void grid_generator_default::_init_board()
{
    const char empty_char = color_to_clobber_char(EMPTY);
    init_board_helper(_board, _shape, empty_char);
}

////////////////////////////////////////////////// grid_generator_clobber
/// methods
inline grid_generator_clobber::grid_generator_clobber(const int_pair& max_shape)
    : grid_generator_masked(max_shape)
{
}

inline grid_generator_clobber::grid_generator_clobber(int max_rows,
                                                      int max_cols)
    : grid_generator_masked(max_rows, max_cols)
{
}

inline grid_generator_clobber::grid_generator_clobber(int max_cols)
    : grid_generator_masked(1, max_cols)
{
}

inline void grid_generator_clobber::_init_board()
{
    const char black_char = color_to_clobber_char(BLACK);
    const char empty_char = color_to_clobber_char(EMPTY);
    init_board_helper_masked(_board, _shape, _mask, black_char, empty_char);
}

////////////////////////////////////////////////// grid_generator_nogo methods
inline grid_generator_nogo::grid_generator_nogo(const int_pair& max_shape)
    : grid_generator_masked(max_shape)
{
}

inline grid_generator_nogo::grid_generator_nogo(int max_rows, int max_cols)
    : grid_generator_masked(max_rows, max_cols)
{
}

inline grid_generator_nogo::grid_generator_nogo(int max_cols)
    : grid_generator_masked(1, max_cols)
{
}

inline void grid_generator_nogo::_init_board()
{
    const char black_char = color_to_clobber_char(BLACK);
    const char empty_char = color_to_clobber_char(EMPTY);
    init_board_helper_masked(_board, _shape, _mask, empty_char, black_char);
}

//////////////////////////////////////////////////
// grid_generator_domineering methods
inline grid_generator_domineering::grid_generator_domineering(
    const int_pair& max_shape)
    : grid_generator_masked(max_shape)
{
}

inline grid_generator_domineering::grid_generator_domineering(int max_rows,
                                                              int max_cols)
    : grid_generator_masked(max_rows, max_cols)
{
}

inline grid_generator_domineering::grid_generator_domineering(int max_cols)
    : grid_generator_masked(1, max_cols)
{
}

inline void grid_generator_domineering::_init_board()
{
    const char empty_char = color_to_clobber_char(EMPTY);
    const char border_char = '#'; // TODO clean up grid stuff...

    init_board_helper_masked(_board, _shape, _mask, empty_char, border_char);
}

inline bool grid_generator_domineering::_increment_board()
{
    return false;
}
