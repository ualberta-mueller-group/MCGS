#pragma once

#include <vector>
#include <unordered_map>
#include "game.h"
#include "sumgame.h"

class sumgame_map_view
{
public:
    sumgame_map_view(sumgame& sum, sumgame_impl::change_record& record);

    std::vector<game*>* get_games_nullable(obj_id_t obj_id);
    std::vector<game*>& get_games(obj_id_t obj_id);

    void deactivate_game(game* g);
    void deactivate_games(std::vector<game*>& games);

    // T is derived from game class
    template <class T>
    void add_game(T* game_ptr)
    {
        static_assert(is_concrete_game_v<T>);
        assert(game_ptr != nullptr);
        assert(game_ptr->is_active());

        obj_id_t obj_id = get_obj_id<T>();
        std::vector<game*>& vec = _map[obj_id];

        vec.push_back(game_ptr);
        _record.added_games.push_back(game_ptr);
        _sum.add(game_ptr);
    }

private:
    sumgame& _sum;
    sumgame_impl::change_record& _record;

    std::unordered_map<obj_id_t, std::vector<game*>> _map;
};

