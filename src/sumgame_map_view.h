#pragma once

#include <type_traits>
#include <vector>
#include <unordered_map>
#include "game.h"
#include "sumgame.h"

class sumgame_map_view
{
public:
    sumgame_map_view(sumgame& sum, sumgame_impl::change_record& record);

    std::vector<game*>* get_games_nullable(game_type_t gt);
    std::vector<game*>& get_games(game_type_t gt);

    void deactivate_game(game* g);
    void deactivate_games(std::vector<game*>& games);

    // T is derived from game class
    template <class T>
    void add_game(T* game_ptr)
    {
        static_assert(std::is_base_of_v<game, T>);
        assert(game_ptr != nullptr);
        assert(game_ptr->is_active());

        game_type_t gt;
        if constexpr (is_concrete_game_v<T>) // constexpr needed because
                                             // game_type<T>() is invalid if T
                                             // is abstract
            gt = game_type<T>();             // preferred; no vtable lookup
        else
            gt = game_ptr->game_type();

        std::vector<game*>& vec = _map[gt];

        vec.push_back(game_ptr);
        _record.added_games.push_back(game_ptr);
        _sum.add(game_ptr);
    }

private:
    void _filter_inactive(std::vector<game*>& games);

    sumgame& _sum;
    sumgame_impl::change_record& _record;

    std::unordered_map<game_type_t, std::vector<game*>> _map;
};
