#pragma once

/*
    Implements "Elephants and Rhinos" from Lessons in Play by David Wolfe et al.
*/

// IWYU pragma: begin_exports
#include "game.h"
#include "strip.h"
// IWYU pragma: end_exports

#include <vector>
#include <ostream>

//////////////////////////////////////// elephants
class elephants : public strip
{
public:
    elephants(const std::string& game_as_string);
    elephants(const std::vector<int>& board);

    void play(const move& m, bw to_play) override;
    void undo_move() override;

protected:
    split_result _split_impl() const override;

    void _normalize_impl() override;
    void _undo_normalize_impl() override;

public:
    move_generator* create_move_generator(bw to_play) const override;

    void print(std::ostream& str) const override
    {
        str << "elephants:" << board_as_string();
    }

    game* inverse() const override; // caller takes ownership
};

//////////////////////////////////////// elephants_move_generator
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
