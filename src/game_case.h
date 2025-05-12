#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "simple_text_hash.h"
// IWYU pragma: end_exports

#include "search_utils.h"

/*
    game_case:
        Games and other data returned from parsing input with file_parser.
        Games and data within are owned by the caller

    Moveable, not copyable. Caller must call cleanup_games() or release_games()
   before destructing. This is to prevent memory bugs, due to game ownership
        being unclear right now
*/
struct game_case
{
    game_case();
    ~game_case();

    // move constructor and move assignment operator
    game_case(game_case&& other) noexcept;
    game_case& operator=(game_case&& other) noexcept;

    void cleanup_games(); // delete all games
    void release_games(); // release ownership of games, and reset self to
                          // default values

    search_result run(unsigned long long timeout = 0) const;

    ebw to_play;
    search_value expected_value;
    bool impartial;

    std::vector<game*> games;
    std::string comments;
    simple_text_hash hash;

private:
    void _move_impl(game_case&& other) noexcept;
};

