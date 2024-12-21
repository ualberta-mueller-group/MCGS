//---------------------------------------------------------------------------
// Implementation of Clobber on a 1-dimensional strip
//---------------------------------------------------------------------------

#ifndef clobber_1xn_H
#define clobber_1xn_H

#include "cgt_basics.h"
#include "strip.h"

class clobber_1xn : public strip
{
public:
    clobber_1xn(const vector<int>& board);
    clobber_1xn(std::string game_as_string);
    void play(const move& m, bw to_play) override;
    void undo_move() override;

protected:
    split_result split_implementation() const override;

public:
    game* inverse() const override;
    move_generator* create_move_generator(bw to_play) const override;
    void print(std::ostream& str) const override { str << "clobber_1xn:" << board_as_string();}
private:
};

std::ostream& operator<<(std::ostream& out, const clobber_1xn& g);

#endif // clobber_1xn_H
