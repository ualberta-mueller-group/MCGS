#pragma once

#include "grid.h"
#include <vector>

struct bounding_box
{
    bounding_box(int row_shift, int col_shift, int_pair shape);

    int row_shift;
    int col_shift;
    int_pair shape;
};

inline bounding_box::bounding_box(int row_shift, int col_shift, int_pair shape)
    : row_shift(row_shift),
      col_shift(col_shift),
      shape(shape)
{
}


std::vector<int> trim_to_bounding_box(const std::vector<int>& src_board,
                                      const int_pair& src_shape,
                                      const bounding_box& dst_box);
