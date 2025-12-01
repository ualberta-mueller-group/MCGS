#pragma once

#include <optional>
#include <vector>
#include <utility>
#include <cstddef>

#include "grid_generator.h"
#include "int_pair.h"

////////////////////////////////////////////////// class sheep_grid_generator

class sheep_grid_generator: public i_grid_generator
{
public:
    sheep_grid_generator(const int_pair& max_dims, int max_black_sheep,
                         int max_white_sheep);

    virtual ~sheep_grid_generator() {}

    operator bool() const override;
    void operator++() override;

    const std::vector<int>& gen_board() const override;
    int_pair get_shape() const override;

    bool only_strips() const override;

private:
    /*
       - grid dimensions
       - sheep counts
       - board
    */
    bool _increment(bool init);
    bool _increment_dims(bool init);
    bool _increment_sheep_counts(bool init);
    bool _increment_board(bool init);

    void _populate_board_vec(std::vector<int>& temp_board, unsigned int idx,
                             int black_remaining, int white_remaining);

    bool _total_sheep_correct(const std::vector<int>& board) const;

    static std::optional<int> _get_next_choice(int black_remaining,
                                               int white_remaining, int prev,
                                               bool init);

    static bool _sheep_count_order_fn(const std::pair<int, int>& p1,
                                      const std::pair<int, int>& p2);

    const int_pair _max_dims;
    const int _max_black_sheep;
    const int _max_white_sheep;

    // part 1
    int_pair _dims;

    // part 2
    std::vector<std::pair<int, int>> _sheep_count_vec;
    size_t _sheep_count_idx;
    std::pair<int, int> _sheep_count;

    // part 3
    std::vector<int> _board;
    size_t _current_idx;
};


