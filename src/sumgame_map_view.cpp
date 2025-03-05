#include "sumgame_map_view.h"

using namespace std;

sumgame_map_view::sumgame_map_view(sumgame& sum, sumgame_impl::change_record& record): _sum(sum), _record(record)
{
    // build game map
    const int N = sum.num_total_games();

    for (int i = 0; i < N; i++)
    {
        game* g = sum.subgame(i);

        if (g->is_active())
        {
            obj_id_t obj_id = g->get_obj_id();
            _map[obj_id].push_back(g);
        }
    }
}

vector<game*>* sumgame_map_view::get_games_nullable(obj_id_t obj_id)
{
    auto it = _map.find(obj_id);

    if (it == _map.end())
    {
        return nullptr;
    }

    vector<game*>& map_games = it->second;
    vector<game*> active_games;

    for (game* g : map_games)
    {
        if (g->is_active())
        {
            active_games.push_back(g);
        }
    }

    map_games = std::move(active_games);

    if (map_games.empty())
    {
        return nullptr;
    }

    return &map_games;
}

vector<game*>& sumgame_map_view::get_games(obj_id_t obj_id)
{
    return _map[obj_id];
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

