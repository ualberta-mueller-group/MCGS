//---------------------------------------------------------------------------
// Implementation of NoGo on a 2-dimensional rectangular board
//---------------------------------------------------------------------------

#pragma once

#include "cgt_basics.h"
#include "grid.h"
#include <vector>

class nogo : public grid
{
public:
    nogo(std::string game_as_string);
    nogo(const std::vector<int>& board, int_pair shape);
    void play(const move& m, bw to_play) override;
    void undo_move() override;

protected:
    split_result _split_impl() const override;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override
    {
        str << "nogo:" << board_as_string();
    }
};

struct nogo_board
{
    std::vector<int> board;
    int_pair shape;
};

class nogo_rule
{
public:
    static bool is_legal(nogo_board nboard, int p, int toplay);
    static bool has_liberty(const nogo_board& nboard, int p);
    static std::vector<int> neighbors(const nogo_board& nboard, int p);
};

std::ostream& operator<<(std::ostream& out, const nogo& g);
std::ostream& operator<<(std::ostream& os, const nogo_board& nboard);
