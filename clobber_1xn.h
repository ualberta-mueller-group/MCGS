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
    void play(const move& m, bw to_play);
    void undo_move();
    game* inverse() const;
    move_generator* create_move_generator(bw to_play) const;
private:
};

std::ostream& operator<<(std::ostream& out, const clobber_1xn& g);

#endif // clobber_1xn_H
