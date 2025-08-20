#pragma once

#include <iostream>
#include <type_traits>
#include <vector>
#include <cstddef>
#include "grid.h"
#include "strip.h"
#include "db_game_generator.h"

/*
   TODO some of these probably have edge cases when some functions are
   overridden. The virtual functions probably suggest more generality than
   is implemented, particularly around _init and _increment virtual functions

   Both _init and _increment functions should return bools and be allowed to
   fail

   TODO why aren't these game orderings faster?
*/

////////////////////////////////////////////////// grid_mask
namespace ggen_impl {
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

} // namespace ggen_impl


////////////////////////////////////////////////// ggen
/*
   Interface class for grid generators
*/
class ggen
{
public:
    virtual ~ggen() {}

    virtual operator bool() const = 0;
    virtual void operator++() = 0;

    virtual const std::string& gen_board() const = 0;
    virtual int_pair get_shape() const = 0;

protected:
    // Helper functions likely to be useful for most grid generators
    static bool _increment_shape_helper(int_pair& shape,
                                        const int_pair& max_shape);

    static void _init_board_helper(std::string& board, const int_pair& shape,
                                   char init_char);
};

////////////////////////////////////////////////// ggen_base
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
class ggen_base: public ggen
{
public:
    ggen_base(const int_pair& max_shape);
    virtual ~ggen_base() {}

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

////////////////////////////////////////////////// ggen_masked
/*
    Abstract grid generator adding mask functionality. Used for
    clobber and nogo generators (i.e. to iterate in order of increasing or
    decreasing number of stones)

    See ggen_base for mandatory functions to implement. Implementations should
    account for the current mask state

    _init_mask() and _increment_mask() are provided. See grid_mask class
    for ordering. Overrides operator++() to account for mask
*/
class ggen_masked: public ggen_base
{
public:
    ggen_masked(const int_pair& max_shape);
    virtual ~ggen_masked() {}

    void operator++() override;

protected:
    ggen_impl::grid_mask _mask;

    /*
       i.e. true/false characters for clobber: 'X' and '.'
       for nogo: '.' and 'X'
    */
    static void _init_board_helper_masked(std::string& board,
                                          const int_pair& shape,
                                          const ggen_impl::grid_mask& mask,
                                          char true_char, char false_char);

    virtual void _init_mask();
    virtual bool _increment_mask();
};

////////////////////////////////////////////////// ggen_default
/*
   Non-abstract basic grid generator.

   i.e for max_shape 2x2:
       "" -> "." -> "X" -> "O" -> ".." -> ... -> 
       "..|.." -> "..|.X" -> "..|.O" -> "..|X." -> "..|XX" -> ... -> "OO|OO"
*/
class ggen_default: public ggen_base
{
public:
    ggen_default(const int_pair& max_shape);
    virtual ~ggen_default() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};

////////////////////////////////////////////////// ggen_clobber
/*
   Non-abstract grid generator for Clobber ordering

   i.e. for max_shape 2x2 (empties on the false part of the grid_mask):

    "" -> "." -> "X" -> "O" -> ".." -> ... ->
    "..|.." -> "X.|.." -> "O.|.." -> ".X|.." ->
    ".O|.." -> "..|X." -> "..|O." -> "..|.X" -> "..|.O" -> "XX|.." -> "OX|.." ->
    "XO|.." -> "OO|.." -> "X.|X." -> "O.|X." -> "X.|O." -> "O.|O." -> "X.|.X" ->
    "O.|.X" -> "X.|.O" -> "O.|.O" -> ".X|X." -> ... -> "OO|OO"
*/
class ggen_clobber: public ggen_masked
{
public:
    ggen_clobber(const int_pair& max_shape);
    virtual ~ggen_clobber() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};

    
////////////////////////////////////////////////// ggen_nogo
/*
   Non-abstract grid generator for NoGo ordering

    i.e. for max_shape 2x2 (empties on the true part of the grid_mask):

   "" -> "X" -> "O" -> "." -> "XX" -> "OX" -> "XO" -> "OO" -> ".X" -> ".O" ->
   "X." -> "O." -> ".." -> ... -> "X.|O." -> "O.|O." -> "XX|.." -> "OX|.." ->
   "XO|.." -> "OO|.." -> "..|.X" -> "..|.O" -> "..|X." -> "..|O." -> ".X|.." ->
   ".O|.." -> "X.|.." -> "O.|.." -> "..|.."
*/
class ggen_nogo: public ggen_masked
{
public:
    ggen_nogo(const int_pair& max_shape);
    virtual ~ggen_nogo() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};

////////////////////////////////////////////////// gridlike_game_generator
template <class Game_T, class Generator_T>
class gridlike_game_generator: public db_game_generator
{
    static_assert(                                 //
        std::is_base_of_v<grid, Game_T> ||         //
        std::is_base_of_v<strip, Game_T>,          //
        "Game must be derived from grid or strip." //
    );                                             //

    static_assert(!std::is_abstract_v<Game_T>, "Game must not be abstract.");

    static_assert(                              //
        std::is_base_of_v<ggen, Generator_T> && //
        !std::is_abstract_v<Generator_T>        //
    );                                          //

public:
    virtual ~gridlike_game_generator() {}

    gridlike_game_generator(int max_cols);
    gridlike_game_generator(int max_rows, int max_cols);

    operator bool() const override;
    void operator++() override;
    game* gen_game() const override;

protected:
    Generator_T _gen;

    void _init();
    bool _game_legal() const;

};

////////////////////////////////////////////////// grid_mask methods
namespace ggen_impl {

inline grid_mask::grid_mask()
    : _marker_count_end(0)
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

} // namespace ggen_impl

////////////////////////////////////////////////// ggen_base methods
inline ggen_base::ggen_base(const int_pair& max_shape)
    : _max_shape(max_shape),
      _shape(0, 0)
{
}

inline ggen_base::operator bool() const
{
    return (_shape.first <= _max_shape.first) && //
           (_shape.second <= _max_shape.second); //
}

inline const std::string& ggen_base::gen_board() const
{
    assert(*this);
    return _board;
}

inline int_pair ggen_base::get_shape() const
{
    assert(*this);
    return _shape;
}

inline bool ggen_base::_increment_shape()
{
    return _increment_shape_helper(_shape, _max_shape);
}

////////////////////////////////////////////////// ggen_masked methods
inline ggen_masked::ggen_masked(const int_pair& max_shape)
    : ggen_base(max_shape),
      _mask(int_pair(0, 0))
{
}

inline void ggen_masked::_init_mask()
{
    _mask.set_shape(_shape);
}

inline bool ggen_masked::_increment_mask()
{
    ++_mask;
    return _mask;
}

////////////////////////////////////////////////// ggen_default methods
inline ggen_default::ggen_default(const int_pair& max_shape)
    : ggen_base(max_shape)
{
}

inline void ggen_default::_init_board()
{
    const char empty_char = color_to_clobber_char(EMPTY);
    _init_board_helper(_board, _shape, empty_char);
}

////////////////////////////////////////////////// ggen_clobber methods
inline ggen_clobber::ggen_clobber(const int_pair& max_shape)
    : ggen_masked(max_shape)
{
}

inline void ggen_clobber::_init_board()
{
    const char black_char = color_to_clobber_char(BLACK);
    const char empty_char = color_to_clobber_char(EMPTY);
    _init_board_helper_masked(_board, _shape, _mask, black_char, empty_char);
}

////////////////////////////////////////////////// ggen_nogo methods
inline ggen_nogo::ggen_nogo(const int_pair& max_shape)
    : ggen_masked(max_shape)
{
}

inline void ggen_nogo::_init_board()
{
    const char black_char = color_to_clobber_char(BLACK);
    const char empty_char = color_to_clobber_char(EMPTY);
    _init_board_helper_masked(_board, _shape, _mask, empty_char, black_char);
}


////////////////////////////////////////////////// has_is_legal_v<T>
// True IFF T has method: bool is_legal() const

// NOLINTBEGIN(readability-identifier-naming)
template <class T, class Enable = void>
struct has_is_legal
{
    static constexpr bool value = false;
};

template <class T>
struct has_is_legal<
    T,
    std::enable_if_t<
        std::is_same_v<
            bool (T::*)() const,
            decltype(&T::is_legal)
        >,
        void
    >
>
{
    static constexpr bool value = true;
};

template <class T>
static constexpr bool has_is_legal_v = has_is_legal<T>::value;
// NOLINTEND(readability-identifier-naming)

//////////////////////////////////////////////////
// gridlike_game_generator methods

template <class Game_T, class Generator_T>
gridlike_game_generator<Game_T, Generator_T>::gridlike_game_generator(
    int max_cols)
    : _gen(int_pair(1, max_cols))
{
    assert(max_cols >= 0);
    _init();
}

template <class Game_T, class Generator_T>
gridlike_game_generator<Game_T, Generator_T>::gridlike_game_generator(
    int max_rows, int max_cols)
    : _gen(int_pair(max_rows, max_cols))
{
    static_assert(std::is_base_of_v<grid, Game_T>,
                  "This constructor is for grids");

    assert(max_rows >= 0 && max_cols >= 0);
    _init();
}

template <class Game_T, class Generator_T>
inline gridlike_game_generator<Game_T, Generator_T>::operator bool() const
{
    return _gen;
}

template <class Game_T, class Generator_T>
void gridlike_game_generator<Game_T, Generator_T>::operator++()
{
    assert(*this);

    do
    {
        assert(_gen);
        ++_gen;
    }
    while (_gen && !_game_legal());
}


template <class Game_T, class Generator_T>
game* gridlike_game_generator<Game_T, Generator_T>::gen_game() const
{
    assert(*this);
    return new Game_T(_gen.gen_board());
}

template <class Game_T, class Generator_T>
void gridlike_game_generator<Game_T, Generator_T>::_init()
{
    if (!_gen)
    {
        assert(!*this);
        return;
    }

    if (_game_legal())
    {
        assert(*this);
        return;
    }

    ++(*this);
}

template <class Game_T, class Generator_T>
bool gridlike_game_generator<Game_T, Generator_T>::_game_legal() const
{
    assert(_gen);

    try
    {
        Game_T g(_gen.gen_board());

        if constexpr (has_is_legal_v<Game_T>)
            return g.is_legal();

        return true;
    }
    catch (std::exception& e)
    {
        return false;
    }

    assert(false);
}

//////////////////////////////////////////////////
void test_grid_generator_new();
