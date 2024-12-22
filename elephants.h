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

protected:
    split_result split_implementation() const override;

public:
    move_generator* create_move_generator(bw to_play) const override;
    game* inverse() const override; // caller takes ownership
};

class elephants_move_generator : public move_generator
{
public:
    elephants_move_generator(const elephants& game, bw to_play);

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

    // should be private, but used in unit tests
    bool is_move(int from, int to, bw to_play) const;

private:

    const elephants& _game;
    int _idx;
    int _dir;
};



#endif // elephants_H

