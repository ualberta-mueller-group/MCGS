#include "sumgame_change_record.h"
#include "game.h"
#include <climits>
#include <vector>
#include <cassert>
#include <utility>
#include "sumgame_map_view.h"
#include "cgt_game_simplification.h"

using namespace std;

//////////////////////////////////////// change_record
namespace sumgame_impl {

change_record::~change_record()
{
    // owner should have called one of the undo functions
    assert(deactivated_games.empty());
    assert(added_games.empty());
}

change_record::change_record(change_record&& other) noexcept
{
    _move_impl(std::forward<change_record>(other));
}

change_record& change_record::operator=(change_record&& other) noexcept
{
    _move_impl(std::forward<change_record>(other));
    return *this;
}

void change_record::simplify_basic(sumgame& sum)
{
    sumgame_map_view map_view(sum, *this);
    simplify_basic_all(map_view);
    return;
}

void change_record::undo_simplify_basic(sumgame& sum)
{
    // IMPORTANT: reactivate before deleting to avoid use after free
    for (auto it = deactivated_games.rbegin(); it != deactivated_games.rend();
         it++)
    {
        game* g = *it;

        assert(!g->is_active());
        g->set_active(true);
    }

    for (auto it = added_games.rbegin(); it != added_games.rend(); it++)
    {
        game* g = *it;

        assert(g->is_active());
        sum.pop(g);
        delete g;
    }

    _clear();
}

bool change_record::no_change() const
{
    return deactivated_games.empty() && added_games.empty();
}

void change_record::_clear()
{
    deactivated_games.clear();
    added_games.clear();
}

void change_record::_move_impl(change_record&& other) noexcept
{
    deactivated_games = std::move(other.deactivated_games);
    added_games = std::move(other.added_games);

    other._clear();
}

} // namespace sumgame_impl
