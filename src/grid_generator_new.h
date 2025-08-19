#pragma once

#include <iostream>
#include <vector>
#include <cstddef>
#include "utilities.h"
#include "grid.h"
#include "strip.h"

////////////////////////////////////////////////// grid_mask
namespace ggen_impl {
/*
    Mask for grid tiles, excluding SEP ('|').

    Used to define clobber and nogo generator orderings:
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

    Note that the row separator is missing, meaning the same index cannot be
    used for both the mask and the string representation of a grid game (if
    the game has at least 2 rows).
*/
class grid_mask
{
public:
    size_t size() const;
    void set_shape(const int_pair& shape);

    bool operator[](size_t idx) const;
    bool increment();

protected:
    std::vector<bool> _mask;
    std::vector<size_t> _indices;

    bool _increment_by_moving(size_t marker_idx, size_t max_pos);
    bool _increment_by_adding();

private:
    friend std::ostream& operator<<(std::ostream& os, const grid_mask& mask);
};

std::ostream& operator<<(std::ostream& os, const grid_mask& mask);

} // namespace ggen_impl


////////////////////////////////////////////////// ggen
class ggen
{
public:
    virtual ~ggen() {}

    virtual operator bool() const = 0;
    virtual void operator++() = 0;

    virtual const std::string& gen_board() const = 0;
    virtual const int_pair get_shape() const = 0;

protected:
    static bool _increment_shape_helper(int_pair& shape,
                                        const int_pair& max_shape);

    static void _init_board_helper(std::string& board, const int_pair& shape,
                                   char init_char);
};

////////////////////////////////////////////////// ggen_basic
class ggen_basic: public ggen
{
public:
    ggen_basic(const int_pair& max_shape);
    virtual ~ggen_basic() {}

    operator bool() const override;
    void operator++() override;

    const std::string& gen_board() const override;
    const int_pair get_shape() const override;

protected:
    const int_pair _max_shape;

    int_pair _shape;
    std::string _board;

    virtual bool _increment_shape();

    virtual void _init_board() = 0;
    virtual bool _increment_board() = 0;
};

////////////////////////////////////////////////// ggen_masked
class ggen_masked: public ggen_basic
{
public:
    ggen_masked(const int_pair& max_shape);
    virtual ~ggen_masked() {}

    void operator++() override;

protected:
    ggen_impl::grid_mask _mask;

    static void _init_board_helper_masked(std::string& board,
                                          const int_pair& shape,
                                          const ggen_impl::grid_mask& mask,
                                          char true_char, char false_char);

    virtual void _init_mask();
    virtual bool _increment_mask();
};

////////////////////////////////////////////////// ggen_default
class ggen_default: public ggen_basic
{
public:
    ggen_default(const int_pair& max_shape);
    virtual ~ggen_default() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};

////////////////////////////////////////////////// ggen_clobber
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
class ggen_nogo: public ggen_masked
{
public:
    ggen_nogo(const int_pair& max_shape);
    virtual ~ggen_nogo() {}

protected:
    void _init_board() override;
    bool _increment_board() override;
};

////////////////////////////////////////////////// grid_mask methods
namespace ggen_impl {

inline size_t grid_mask::size() const
{
    return _mask.size();
}

inline bool grid_mask::operator[](size_t idx) const
{
    return _mask[idx];
}


} // namespace ggen_impl


////////////////////////////////////////////////// ggen_basic methods
inline ggen_basic::ggen_basic(const int_pair& max_shape)
    : _max_shape(max_shape),
      _shape(0, 0)
{
}

inline ggen_basic::operator bool() const
{
    return (_shape.first <= _max_shape.first) && //
           (_shape.second <= _max_shape.second); //
}

inline const std::string& ggen_basic::gen_board() const
{
    assert(*this);
    return _board;
}

inline const int_pair ggen_basic::get_shape() const
{
    assert(*this);
    return _shape;
}

inline bool ggen_basic::_increment_shape()
{
    return _increment_shape_helper(_shape, _max_shape);
}

////////////////////////////////////////////////// ggen_masked methods
inline ggen_masked::ggen_masked(const int_pair& max_shape)
    : ggen_basic(max_shape)
{
}

inline void ggen_masked::_init_mask()
{
    _mask.set_shape(_shape);
}

inline bool ggen_masked::_increment_mask()
{
    return _mask.increment();
}

////////////////////////////////////////////////// ggen_default methods
inline ggen_default::ggen_default(const int_pair& max_shape)
    : ggen_basic(max_shape)
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

//////////////////////////////////////////////////
void test_grid_generator_new();
