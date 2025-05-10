#pragma once

// IWYU pragma: begin_exports
#include "game.h"
#include "simple_text_hash.h"
// IWYU pragma: end_exports

// expected outcome of a game_case
enum test_result
{
    TEST_RESULT_LOSS = 0,
    TEST_RESULT_WIN,
    TEST_RESULT_NIMBER,
    TEST_RESULT_UNSPECIFIED,
};

inline std::string test_result_to_string(const test_result& outcome)
{
    switch (outcome)
    {
        case TEST_RESULT_UNSPECIFIED:
        {
            return "Unspecified";
            break;
        }

        case TEST_RESULT_WIN:
        {
            return "Win";
            break;
        }

        case TEST_RESULT_LOSS:
        {
            return "Loss";
            break;
        }

        case TEST_RESULT_NIMBER:
        {
            return "Nimber";
            break;
        }

        default:
        {
            std::cerr << "test_result_to_string() invalid input: ";
            std::cerr << outcome << std::endl;
            exit(-1); // exit instead of assert (could be due to bad file input)
        }
    }

    std::cerr << "This string should not appear: see test_result_to_string()"
              << std::endl;
    exit(-1);
}

class run_command_t
{
public:
    run_command_t()
    {
        reset();
    }

    void reset();

    ebw player; // empty IFF nimber expected
    test_result expected_outcome;
    int expected_nimber;
};

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

    run_command_t run_command;

    std::vector<game*> games;
    std::string comments;
    simple_text_hash hash;

private:
    void _move_impl(game_case&& other) noexcept;
};

