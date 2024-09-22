//---------------------------------------------------------------------------
// Implementation of clobber on a 1-dimensional 1xn board
//---------------------------------------------------------------------------

#ifndef clobber_1xn_H
#define clobber_1xn_H

#include "cgt_basics.h"
#include "strip.h"

class clobber_1xn : public strip
{
public:
    clobber_1xn(std::string game_as_string);
    void play(const move& m);
    void undo_move();
    move_generator* create_mg() const;
private:
};

std::ostream& operator<<(std::ostream& out, const clobber_1xn& g);

#endif // clobber_1xn_H
