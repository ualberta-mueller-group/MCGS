/*
    This file is unused. Its intended purpose is to increase node rates for
    grid games by allowing split functions to terminate early, by quickly
    proving that a move could not have split the board into more components.

    Class component_cache generates all 4 or 8 connected boolean grids, and
    determines whether each grid is exactly 1 connected component.

    It can be queried using an arbitrary window of a grid game (so long as
    the window fits within the generated size).

    Some unit tests are at the bottom of the .cpp file.
*/
#pragma once

#include <vector>
#include <cstdint>

#include "grid_location.h"

#define COMPONENT_CACHE_ROWS 4
#define COMPONENT_CACHE_COLS 4

////////////////////////////////////////////////// class component_cache
class component_cache
{
public:
    component_cache();

    void init_4_connected(int n_rows, int n_cols);
    void init_8_connected(int n_rows, int n_cols);

    bool is_initialized() const;

    int get_n_rows() const;
    int get_n_cols() const;

    bool is_one_component(uint64_t bit_pattern) const;

    template <class Tile_T, class Functor_T>
    static uint64_t get_full_board_code(int_pair board_dims,
                                        const std::vector<Tile_T>& board,
                                        Functor_T& func);

    template <class Tile_T, class Functor_T>
    static uint64_t get_window_code(int_pair board_dims,
                                    const std::vector<Tile_T>& board,
                                    int_pair window_start, int_pair window_dims,
                                    Functor_T& func,
                                    int_pair component_cache_dims);

    struct identity_functor
    {
        bool operator()(bool b) const
        {
            return b;
        }
    };

private:
    void _init_memory(int n_rows, int n_cols);

    void _init_values(int n_rows, int n_cols, const grid_dir* dirs,
                      size_t n_dirs);

    void _set(uint64_t bit_pattern, bool value);

    static bool _is_one_component_by_search(int n_rows, int n_cols,
                                            const std::vector<bool>& board,
                                            const grid_dir* dirs,
                                            size_t n_dirs);

    // friend serializer<component_cache>;

    int _n_rows;
    int _n_cols;
    std::vector<uint8_t> _data;
};



//////////////////////////////////////////////////
void init_global_component_caches();
const component_cache& get_4_connected_component_cache();
const component_cache& get_8_connected_component_cache();

//////////////////////////////////////////////////
void test_component_cache_stuff();

//////////////////////////////////////////////////
template <class Tile_T, class Functor_T>
uint64_t component_cache::get_full_board_code(int_pair board_dims,
                                              const std::vector<Tile_T>& board,
                                              Functor_T& func)
{
    assert(board.size() ==
           static_cast<size_t>(board_dims.first * board_dims.second));

    uint64_t pattern = 0;

    //std::cout << "FULL START" << std::endl;

    size_t shift = 0;
    for (const Tile_T& t : board)
    {
        const bool bit = func(t);
        const uint64_t mask = uint64_t(bit) << shift;
        shift++;
        pattern |= mask;

        //std::cout << bit << " ";

        //if (shift % board_dims.second == 0)
        //    std::cout << std::endl;
    }

    //std::cout << "FULL END" << std::endl;

    return pattern;
}

template <class Tile_T, class Functor_T>
uint64_t component_cache::get_window_code(int_pair board_dims,
                                          const std::vector<Tile_T>& board,
                                          int_pair window_start,
                                          int_pair window_dims, Functor_T& func,
                                          int_pair component_cache_dims)
{
    uint64_t pattern = 0;

    const int n_rows = board_dims.first;
    const int n_cols = board_dims.second;
    assert(board.size() == static_cast<size_t>(n_rows * n_cols));

    const int start_row = window_start.first;
    const int start_col = window_start.second;

    const int end_row = start_row + window_dims.first;
    const int end_col = start_col + window_dims.second;

    int cache_point_base = 0;
    int local_point_base = (start_col + start_row * n_cols);

    //std::cout << "WIN START" << std::endl;

    for (int r = start_row; r < end_row; r++)
    {
        for (int c = 0; c < window_dims.second; c++)
        {
            const int local_point = local_point_base + c;
            const int cache_point = cache_point_base + c;

            const bool bit_val = func(board[local_point]);
            const uint64_t bit_mask = uint64_t(bit_val) << cache_point;

            pattern |= bit_mask;

            //std::cout << bit_val << " ";
        }

        //std::cout << std::endl;

        cache_point_base += component_cache_dims.second;
        local_point_base += n_cols;
    }

    //std::cout << "WIN END" << std::endl;

    return pattern;
}
