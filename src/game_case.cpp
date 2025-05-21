#include "game_case.h"
#include "cgt_basics.h"
#include "search_utils.h"
#include <cassert>
#include <utility>

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
    to_play = EMPTY;
    expected_value = search_value();
    impartial = false;

    games.clear();
    comments.clear();
    hash.clear();
}

search_result game_case::run(unsigned long long timeout) const
{
    if (impartial)
    {
        assert(to_play == EMPTY);
        return search_impartial(games, &expected_value, timeout);
    }
    else
    {
        assert(is_black_white(to_play));
        return search_partizan(games, to_play, &expected_value, timeout);
    }
}

void game_case::_move_impl(game_case&& other) noexcept
{
    assert(games.size() == 0);

    to_play = std::move(other.to_play);
    expected_value = std::move(other.expected_value);
    impartial = std::move(other.impartial);

    games = std::move(other.games);
    comments = std::move(other.comments);
    hash = std::move(other.hash);

    other.release_games();
}
