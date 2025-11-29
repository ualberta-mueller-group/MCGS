#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "grid.h"
// IWYU pragma: end_exports


#include <vector>
#include <iostream>
#include <cassert>
#include <string>
#include <utility>
#include "cgt_move.h"

#define SHEEP_ALT_MOVE

#define SHEEP_SPLIT

class sheep: public grid
{
public:
    sheep(const std::string& game_as_string);
    sheep(const std::vector<int>& board, int_pair shape);
    sheep(const std::pair<std::vector<int>, int_pair>& board_pair);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override;
    void print_move(std::ostream& str, const move& m) const override;
    game* inverse() const override; // caller takes ownership

    static constexpr int MAX_HERD = 50;

protected:
#ifdef SHEEP_SPLIT
    split_result _split_impl() const override;
#endif

private:
};

////////////////////////////////////////////////// helpers
inline bool herd_belongs_to_player(int herd, bw player)
{
    assert(is_black_white(player));

    if (player == BLACK)
        return herd > 0;
    else
        return herd < 0;
}

inline bool herd_movable_and_belongs_to_player(int herd, bw player)
{
    assert(is_black_white(player));

    if (player == BLACK)
        return herd > 1;
    else
        return herd < -1;
}

inline int apply_herd_sign(int herd_abs, bw player)
{
    assert(is_black_white(player) && herd_abs >= 0);
    return player == BLACK ? herd_abs : -herd_abs;
}

