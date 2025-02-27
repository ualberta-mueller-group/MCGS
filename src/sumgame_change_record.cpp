#include "sumgame_change_record.h"
#include "obj_id.h"
#include <unordered_map>
#include <vector>

#include "cgt_nimber.h"
#include "cgt_up_star.h"

using namespace std;



namespace sumgame_impl {

change_record::~change_record()
{
    // owner should have called one of the undo functions
    assert(deactivated_games.empty());
    assert(added_games.empty());
}


void change_record::simplify_basic(sumgame& sum)
{
    // build game map
    unordered_map<obj_id_t, vector<game*>> game_map;
    
    {
        const int N = sum.num_total_games();

        for (int i = 0; i < N; i++)
        {
            game* g = sum.subgame(i);

            if (g->is_active())
            {
                obj_id_t obj_id = g->get_obj_id();
                game_map[obj_id].push_back(g);
            }
        }
    }

    // helper functions
    auto get_games = [&](obj_id_t obj_id) -> vector<game*>*
    {
        auto it = game_map.find(obj_id);

        if (it == game_map.end())
        {
            return nullptr;
        }

        vector<game*>& games = it->second;
        vector<game*> active_games;

        for (game* g : games)
        {
            if (g->is_active())
            {
                active_games.push_back(g);
            }
        }

        if (!active_games.empty())
        {
            swap(games, active_games);
            return &games;
        }

        game_map.erase(it);
        return nullptr;
    };

    auto deactivate_game = [&](game* g) -> void
    {
        assert(g->is_active());
        g->set_active(false);
        deactivated_games.push_back(g);
    };

    auto add_game = [&](game* g, obj_id_t obj_id, vector<game*>& vec) -> void
    {
        assert(g != nullptr);
        assert(g->is_active());
        assert(g->get_obj_id() == obj_id);
        assert(&game_map[obj_id] == &vec);

        added_games.push_back(g);
        sum.add(g);
        vec.push_back(g);
    };


    // Simplify nimber
    {
        vector<game*>* nimbers = get_games(get_obj_id<nimber>());
        
        if (nimbers != nullptr)
        {
            vector<int> heap_vec;

            for (game* g : *nimbers)
            {
                assert(g->is_active());
                assert(g->get_obj_id() == get_obj_id<nimber>());

                nimber* g_nimber = reinterpret_cast<nimber*>(g);

                int val = g_nimber->value();

                heap_vec.push_back(val);
            }

            if (heap_vec.size() >= 2 || (heap_vec.size() == 1 && heap_vec.back() < 2))
            {
                for (game* g : *nimbers)
                {
                    deactivate_game(g);
                }

                int sum = nimber::nim_sum(heap_vec);
                assert(sum >= 0);
            
                // 0: add nothing
                if (sum == 1) // 1: star
                {
                    add_game(new up_star(0, true), get_obj_id<up_star>(), game_map[get_obj_id<up_star>()]);
                } else if (sum >= 2)
                {
                    assert(sum >= 2); // >= 2: nimber
                    add_game(new nimber(sum), get_obj_id<nimber>(), *nimbers);
                }
            }


        }
    }

    // Simplify up_star
    {
        vector<game*>* up_stars = get_games(get_obj_id<up_star>());

        if (up_stars != nullptr && up_stars->size() >= 2)
        {
            int ups = 0;
            bool star = false;

            for (game* g : *up_stars)
            {
                assert(g->is_active());
                assert(g->get_obj_id() == get_obj_id<up_star>());

                up_star* g_up_star = reinterpret_cast<up_star*>(g);

                ups += g_up_star->num_ups();
                star ^= g_up_star->has_star();
                deactivate_game(g);
            }

            if (ups != 0 || star != false)
            {
                up_star* new_game = new up_star(ups, star);
                add_game(new_game, get_obj_id<up_star>(), *up_stars);
            }
        }
    }



}

void change_record::undo_simplify_basic(sumgame& sum)
{
    // IMPORTANT: reactivate before deleting to avoid use after free
    for (auto it = deactivated_games.rbegin(); it != deactivated_games.rend(); it++)
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


void change_record::_clear()
{
    deactivated_games.clear();
    added_games.clear();
}

} // namespace sumgame_impl
