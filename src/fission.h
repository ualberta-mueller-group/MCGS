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

    /*
        TODO

        Probably won't be fast. Need to mark all stones as "true wall", "maybe
        wall", or "not wall", based on whether or not it may have a move in the
        future. Subgames are then the 4-connected components (of EMPTY/BLACK).

        - 2x2 stone squares are "true walls"
        - Board edges contribute to "squares"

        "true wall" is either # or X that will never have a move
        "not wall" is X which either has a move, or can have a move later
        "maybe wall" if a wall has a maybe/true wall on each axis

        Figure out how to resolve maybe walls

        Replacing "true wall" stones with # is a good normalization step
    */
#ifdef FISSION_SPLIT
    split_result _split_impl() const;
#endif
};

////////////////////////////////////////////////// fission methods
inline void fission::print(std::ostream& str) const
{
    str << "fission:" << board_as_string();
}
