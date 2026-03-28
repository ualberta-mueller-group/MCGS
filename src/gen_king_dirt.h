#pragma once

#include "grid.h"
#include "grid_hash.h"
#include <vector>

////////////////////////////////////////////////// class gen_king_dirt

/*
    params is a vector of 3 elements:
    - black_unplaced >= 0 (remaining pieces for black to place)
    - white_unplaced >= 0 (remaining pieces for white to place)
    - must_place in [0, 1]
*/
class gen_king_dirt: public grid
{
public:
    gen_king_dirt(const std::vector<int>& params, const std::vector<int>& board,
                  int_pair shape);

    gen_king_dirt(const std::vector<int>& params,
                  const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override;
    void print_move(std::ostream& str, const move& m, ebw to_play) const override;

    game* inverse() const override;

    int get_black_unplaced() const;
    int get_white_unplaced() const;
    bool get_must_place() const;

    bool has_unplaced_stones(bw for_player) const;

protected:
    void _play_place_stone(const int_pair& place_coords, bw to_play);
    void _play_slide_stone(const int_pair& from_coords,
                          const int_pair& to_coords, bw to_play);

    void _undo_move_place_stone(const int_pair& place_coords, bw to_play);
    void _undo_move_slide_stone(const int_pair& from_coords,
                               const int_pair& to_coords, bw to_play);

    //split_result _split_impl() const override;

    void _init_hash(local_hash& hash) const override;

    void _init_params(const std::vector<int>& params);
    void _preprocess_board();

    static void _normalize_params(int& empty_count, std::vector<int>& params);

    std::vector<int> _params;
    std::vector<std::vector<int>> _params_stack;

    int _empty_space_count;

    mutable grid_hash _gh;

public:
    static constexpr size_t PARAM_COUNT = 3;
protected:
    static constexpr size_t PIDX_BLACK_UNPLACED = 0;
    static constexpr size_t PIDX_WHITE_UNPLACED = 1;
    static constexpr size_t PIDX_MUST_PLACE = 2;
};

////////////////////////////////////////////////// gen_king_dirt methods
inline int gen_king_dirt::get_black_unplaced() const
{
    assert(_params.size() == PARAM_COUNT);
    return _params[PIDX_BLACK_UNPLACED];
}

inline int gen_king_dirt::get_white_unplaced() const
{
    assert(_params.size() == PARAM_COUNT);
    return _params[PIDX_WHITE_UNPLACED];
}

inline bool gen_king_dirt::get_must_place() const
{
    assert(_params.size() == PARAM_COUNT);
    return static_cast<bool>(_params[PIDX_MUST_PLACE]);
}
