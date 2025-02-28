#pragma once
#include "sumgame.h"
#include <climits>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "obj_id.h"

//////////////////////////////////////// forward declarations
class sumgame;

//////////////////////////////////////// sumgame_impl::change_record
namespace sumgame_impl {

class change_record
{
public:
    ~change_record();

    void simplify_basic(sumgame& sum);
    void undo_simplify_basic(sumgame& sum);

    std::vector<game*> deactivated_games;
    std::vector<game*> added_games;

private:
    void _clear();
};

} // namespace sumgame_impl

//////////////////////////////////////// sumgame_map_view
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
        static_assert(std::is_base_of_v<game, T>);
        static_assert(!std::is_abstract_v<T>);
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

//////////////////////////////////////// cast_game

/*
    Convert game pointer types, after doing a few asserts.

    NOTE: This cast is generally unsafe. Only use in place of reinterpret_cast, not dynamic_cast
*/
template <class T_ptr>
inline T_ptr cast_game(game* g)
{
    static_assert(std::is_pointer_v<T_ptr>);
    using T = typename std::remove_pointer<T_ptr>::type; // NOLINT

    static_assert(std::is_base_of_v<game, T>);
    static_assert(!std::is_abstract_v<T>);

    assert(g != nullptr);
    assert(g->is_active());
    assert(g->get_obj_id() == get_obj_id<T>());

    return reinterpret_cast<T_ptr>(g);
}


template <class T>
void print_bits(std::ostream& os, const T& x)
{
    static_assert(std::is_integral_v<T>);

    const int n_bits = sizeof(T) * CHAR_BIT;

    // T could be signed, so we can't iteratively shift right from 
    // the most significant bit
    const T mask = T(1);

    for (int i = 0; i < n_bits; i++)
    {
        os << (((mask << (n_bits - 1 - i)) & x) != 0);
    }
}

