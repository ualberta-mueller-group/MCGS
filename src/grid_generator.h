#pragma once

#include <unordered_set>
#include <vector>
#include <iostream>
#include <memory>

#include "int_pair.h"
#include "grid_hash.h"

////////////////////////////////////////////////// class grid_mask
class grid_mask
{
public:
    grid_mask(unsigned int grid_hash_symmetry_mask);

    size_t size() const; // number of tiles (excluding SEP)
    int_pair get_shape() const;

    void set_shape(const int_pair& shape);

    const std::vector<bool>& get_mask() const;

    void reset();

    // TODO weird idx type -- should have one for ints?
    bool operator[](size_t idx) const;

    operator bool() const;
    void operator++();

protected:
    mutable grid_hash _gh;
    std::unordered_set<hash_t> _mask_hash_set;

    int_pair _current_shape;
    size_t _marker_count_end;

    std::vector<bool> _mask;
    std::vector<size_t> _indices; // locations of each marker in the mask

    bool _increment(bool init);
    bool _increment_no_hash_check();
    bool _increment_by_moving(size_t marker_idx, size_t max_pos);
    bool _increment_by_adding();

    bool _add_mask_hash_to_set();

private:
    friend std::ostream& operator<<(std::ostream& os, const grid_mask& mask);
};

std::ostream& operator<<(std::ostream& os, const grid_mask& mask);

////////////////////////////////////////////////// class i_grid_generator
class i_grid_generator
{
public:
    virtual ~i_grid_generator() {}

    virtual operator bool() const = 0;
    virtual void operator++() = 0;

    virtual const std::vector<int>& gen_board() const = 0;
    virtual int_pair get_shape() const = 0;

    virtual bool only_strips() const = 0;

    // TODO test these
    /*
       Functions increment_dims() and dims_le_max() with two flavors ("standard"
       and "transpose"). increment_dims() modifies "dims" argument, and returns
       the result of dims_le_max() after.

       Both sequences start with (0, 0) then (1, 1).

       "standard" increments column by 1. When column overflows, it becomes 1
       and the row is incremented.

       "transpose" swaps the dimensions and sometimes increments the larger one.
       On overflow, the new result is
       (smaller + 1, smaller + 1).

       NOTE: for "transpose", max_dims of (4, 3) is the same as max_dims of
       (3, 4).

       Example sequence for max_dims of (2, 3):
       "standard":
       (0, 0)
       (1, 1)
       (1, 2)
       (1, 3)
       (2, 1)
       (2, 2)
       (2, 3)

       "transpose" NOTE that max_dims of (2, 3) and (3, 2) will both yield this:
       (0, 0)
       (1, 1)
       (1, 2)
       (2, 1)
       (1, 3)
       (3, 1)
       (2, 2)
       (2, 3)
       (3, 2)
    */
    static bool increment_dims_standard(int_pair& dims,
                                            const int_pair& max_dims,
                                            bool init);

    static bool dims_le_max_standard(const int_pair& dims,
                                     const int_pair& max_dims);

    static bool increment_dims_transpose(int_pair& dims,
                                         const int_pair& max_dims, bool init);

    static bool dims_le_max_transpose(const int_pair& dims,
                                      const int_pair& max_dims);
};

////////////////////////////////////////////////// class grid_generator
class grid_generator: public i_grid_generator
{
public:
    virtual ~grid_generator() {}

    grid_generator(const int_pair& max_dims,
                   const std::vector<int>& tile_sequence, bool strips_only);

    grid_generator(const int_pair& max_dims,
                   const std::vector<int>& tile_sequence, bool mask_active_bit,
                   int mask_inactive_tile, bool strips_only,
                   unsigned int grid_hash_symmetry_mask);

    operator bool() const override;
    void operator++() override;

    const std::vector<int>& gen_board() const override;
    int_pair get_shape() const override;
    bool only_strips() const override;

    int get_current_size() const;

private:
    bool _increment(bool init);
    bool _increment_dimensions(bool init);
    bool _increment_mask(bool init);
    bool _increment_tiles(bool init);

    bool _real_board_matches_idx_board() const;

    void _init_board();
    bool _is_active_tile(int tile_idx) const;

    const bool _strips_only;
    const int_pair _max_dims;
    const std::vector<int> _tile_sequence;
    const size_t _idx_invalid;

    std::unique_ptr<grid_mask> _mask;
    const bool _mask_active_bit;
    const int _mask_inactive_tile;

    int_pair _current_dims;
    std::vector<size_t> _idx_board;
    std::vector<int> _real_board;
};

//////////////////////////////////////////////////
void test_grid_generator_stuff();
