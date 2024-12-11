#ifndef elephants_rhinos_H
#define elephants_rhinos_H

#include "strip.h"
#include "cgt_basics.h"

class elephants_rhinos : public strip
{
public:
    elephants_rhinos(const std::string& game_as_string);
    elephants_rhinos(const std::vector<int>& board);

    void play(const move& m, bw to_play) override; //TODO
    void undo_move() override; //TODO
    move_generator* create_move_generator(bw to_play) const override; //TODO
    game* inverse() const override; // caller takes ownership //TODO

};


#endif // elephants_rhinos_H

