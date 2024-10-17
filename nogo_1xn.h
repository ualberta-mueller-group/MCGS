//---------------------------------------------------------------------------
// Implementation of clobber on a 1-dimensional 1xn board
//---------------------------------------------------------------------------

#ifndef nogo_1xn_H
#define nogo_1xn_H

#include "cgt_basics.h"
#include "strip.h"


class nogo_1xn : public strip
{
public:
    nogo_1xn(std::string game_as_string);
    void play(const move& m, bw to_play);
    void undo_move();
    move_generator* create_move_generator(bw to_play) const;

    std::vector<int> get_board() const { return _board; };
};


std::ostream& operator<<(std::ostream& out, const nogo_1xn& g);


#endif
