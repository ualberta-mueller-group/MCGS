#pragma once

#include "game.h"
#include "grid.h"

#include <string>
#include <ostream>
#include <vector>

////////////////////////////////////////////////// class fission
//#define FISSION_SPLIT
class fission: public grid
{
public:
    fission(int n_rows, int n_cols);
    fission(const std::vector<int>& board, int_pair shape);
    fission(const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override;

    game* inverse() const override;

#ifdef FISSION_SPLIT
    split_result _split_impl() const;
#endif
};

////////////////////////////////////////////////// fission methods
inline void fission::print(std::ostream& str) const
{
    str << "fission:" << board_as_string();
}

