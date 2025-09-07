#pragma once

#include "game.h"
#include "grid.h"

#include <ostream>
#include <vector>
#include <string>

#define ENABLEDOMSPLIT

////////////////////////////////////////////////// class domineering
class domineering: public grid
{
public:
    domineering(int n_rows, int n_cols);
    domineering(const std::vector<int>& board, int_pair shape);
    domineering(const std::string& game_as_string);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override;
    game* inverse() const override; // caller takes ownership

protected:

#ifdef ENABLEDOMSPLIT
    split_result _split_impl() const override;
#endif

};

////////////////////////////////////////////////// domineering methods
inline void domineering::print(std::ostream& str) const
{
    str << "domineering:" << board_as_string();
}
