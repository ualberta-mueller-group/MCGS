#include "sumgame_map_view.h"
#include "sumgame.h"
#include "type_table.h"
#include "sumgame_change_record.h"
#include "game.h"
#include <vector>
#include <utility>
#include <cassert>

using namespace std;

sumgame_map_view::sumgame_map_view(sumgame& sum,
                                   sumgame_impl::change_record& record)
    : _sum(sum), _record(record)
{
    // build game map
    const int N = sum.num_total_games();

    for (int i = 0; i < N; i++)
    {
        game* g = sum.subgame(i);

        if (g->is_active())
        {
            game_type_t gt = g->game_type();
            _map[gt].push_back(g);
        }
    }
}

vector<game*>* sumgame_map_view::get_games_nullable(game_type_t gt)
{
    auto it = _map.find(gt);

    if (it == _map.end())
    {
        return nullptr;
    }

    vector<game*>& map_games = it->second;
    _filter_inactive(map_games);

    if (map_games.empty())
    {
        return nullptr;
    }

    return &map_games;
}

vector<game*>& sumgame_map_view::get_games(game_type_t gt)
{
    vector<game*>& games = _map[gt];
    _filter_inactive(games);
    return games;
}

void sumgame_map_view::deactivate_game(game* g)
{
    assert(g->is_active());
    g->set_active(false);
    _record.deactivated_games.push_back(g);
}

void sumgame_map_view::deactivate_games(vector<game*>& games)
{
    for (game* g : games)
    {
        deactivate_game(g);
    }
}

void sumgame_map_view::_filter_inactive(std::vector<game*>& games)
{
    vector<game*> active_games;

    for (game* g : games)
    {
        if (g->is_active())
        {
            active_games.push_back(g);
        }
    }

    games = std::move(active_games);
}
