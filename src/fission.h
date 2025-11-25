#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "grid.h"
// IWYU pragma: end_exports

#include <string>
#include <ostream>
#include <vector>

#include "grid_hash.h"

constexpr unsigned int FISSION_GRID_HASH_MASK = GRID_HASH_ACTIVE_MASK_MIRRORS;

////////////////////////////////////////////////// class fission
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
    void print_move(std::ostream& str, const move& m) const override;

    game* inverse() const override;

protected:
#ifdef USE_GRID_HASH
    void _init_hash(local_hash& hash) const override;

    mutable grid_hash _gh;
#endif

    /*
        TODO

        Probably won't be fast. Need to mark all stones as "true wall", "maybe
        wall", or "not wall", based on whether or not it may have a move in the
        future. Subgames are then the 4-connected components (of EMPTY/BLACK
        "carved" into BORDER-filled grid).

        - 2x2 stone squares are "true walls"
        - Board edges contribute to "squares"

        "true wall" is either # or X that will never have a move
        "not wall" is X which either has a move, or can have a move later
        "maybe wall" if a wall has a maybe/true wall on each axis

        Figure out how to resolve maybe walls

        Replacing "true wall" stones with # is a good normalization step and
        is necessary for splits

        3 pass solution?

        1: Mark all stones with one of the following:
            - 'X' has immediate move
            - 'O' may have move
            - '#' has no moves

        2: Do a 2nd pass to resolve these?
            - How to resolve/propagate?

        3: Find connected components separated by walls

        Or can we try patterns and then brute force test some range of boards
        to see if we got them all?

        All stones become walls below:

        ....
        .XX.
        .XX.
        ....

        .#.
        #X.
        ...

        .#..
        #XX.
        ..#.

        .#..
        #X..
        .XX#
        ..#.

        If an adjacent stone has been proven to have a future move, then it is
        not a blocker

        Try to prove: "On a second pass, if all neighbors have been recursively
        checked and are still blockers, then they must always be blockers."

        Still insufficient, consider a ring of 2x2 squares each with a 1 wide
        gap bridged by a 1x1 stone

        "On a 2nd pass, if blocked on both axes after recursive checks of all
        neighbors, then we must be permanently blocked"

          XX
         XX
        XX
        #

        Probably the solution (???):
            1: Locally mark all stones as "O" "X" or "#"
            2: Outer loop find "X", propagate in inner loop
                Inner loop: pop from open set, do local check using all neighbors,
                expand neighbors if they're "X" or if we have become "X"
            3: Find components

            At end of step 2, stones not proven to have moves should be walls?
    */
};

////////////////////////////////////////////////// fission methods
inline void fission::print(std::ostream& str) const
{
    str << "fission:" << board_as_string();
}


