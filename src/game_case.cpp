#include "game_case.h"


void run_command_t::reset()
{
    player = EMPTY;
    expected_outcome = TEST_RESULT_UNSPECIFIED;
    expected_nimber = -1;
}

game_case::game_case()
{
    release_games(); // sets variables to defaults
}

game_case::~game_case()
{
    // Caller should have cleaned up games...
    assert(games.size() == 0);
}

game_case::game_case(game_case&& other) noexcept
{
    _move_impl(std::forward<game_case>(other));
}

game_case& game_case::operator=(game_case&& other) noexcept
{
    _move_impl(std::forward<game_case>(other));

    return *this;
}

void game_case::cleanup_games()
{
    for (game* g : games)
    {
        delete g;
    }

    release_games();
}

// resets game_case, releasing ownership of its games without deleting them
void game_case::release_games()
{
    run_command.reset();

    games.clear();

    comments.clear();
    hash.clear();
}

void game_case::_move_impl(game_case&& other) noexcept
{
    assert(games.size() == 0);

    run_command = std::move(other.run_command);
    games = std::move(other.games);
    comments = std::move(other.comments);
    hash = std::move(other.hash);

    other.release_games();
}

