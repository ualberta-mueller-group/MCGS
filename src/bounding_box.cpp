#include "bounding_box.h"

#include "grid.h"
#include "grid_location.h"
#include <vector>
#include <cassert>

using namespace std;

////////////////////////////////////////////////// bounding_box methods
bounding_box::bounding_box(int row_shift, int col_shift, int_pair shape)
    : row_shift(row_shift),
      col_shift(col_shift),
      shape(shape)
{
}

//////////////////////////////////////////////////
vector<int> trim_to_bounding_box(const vector<int>& src_board,
                                 const int_pair& src_shape,
                                 const bounding_box& dst_box)
{
    vector<int> dst_board;

    const int& row_shift = dst_box.row_shift;
    const int& col_shift = dst_box.col_shift;
    const int_pair& dst_shape = dst_box.shape;

    assert(!grid_location::shape_is_empty(dst_shape));

    const int dst_size = dst_shape.first * dst_shape.second;
    dst_board.reserve(dst_size);

    int src_point = col_shift + (row_shift * src_shape.second);

    for (int r = 0; r < dst_shape.first; r++)
    {
        for (int c = 0; c < dst_shape.second; c++)
        {
            const int src_val = src_board[src_point + c];
            dst_board.push_back(src_val);
        }

        src_point += src_shape.second;
    }

    return dst_board;
}
