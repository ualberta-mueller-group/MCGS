#ifndef elephants_H
#define elephants_H

/*
    Implements "Elephants and Rhinos" from Lessons in Play by David Wolfe et al.
*/

#include "strip.h"
#include "cgt_basics.h"

class elephants : public strip
{
public:
    elephants(const std::string& game_as_string);
    elephants(const std::vector<int>& board);

    void play(const move& m, bw to_play) override;
    void undo_move() override;
    move_generator* create_move_generator(bw to_play) const override;
    game* inverse() const override; // caller takes ownership
};


#endif // elephants_H

