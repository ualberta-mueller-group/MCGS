#include "sumgame_change_record.h"
#include "obj_id.h"
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "cgt_nimber.h"
#include "cgt_up_star.h"

using namespace std;

////////////////////////////////////////

class sumgame_map_view
{
public:
    sumgame_map_view(sumgame& sum, sumgame_impl::change_record& record);

    vector<game*>* get_games_nullable(obj_id_t obj_id);
    vector<game*>& get_games(obj_id_t obj_id);

    void deactivate_game(game* g);
    void deactivate_games(vector<game*>& games);
    void add_game(game* g, obj_id_t obj_id);

private:
    sumgame& _sum;
    sumgame_impl::change_record& _record;

    unordered_map<obj_id_t, vector<game*>> _map;
};

void simplify_basic_nimber(sumgame_map_view& map_view);
void simplify_basic_up_star(sumgame_map_view& map_view);

////////////////////////////////////////
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

    _map.erase(it);
    return nullptr;
}

vector<game*>& sumgame_map_view::get_games(obj_id_t obj_id)
{
    vector<game*>* ptr = get_games_nullable(obj_id);

    if (ptr != nullptr)
    {
        return *ptr;
    }

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

}



void sumgame_map_view::add_game(game* g, obj_id_t obj_id)
{
    assert(g != nullptr);
    assert(g->is_active());
    assert(g->get_obj_id() == obj_id);

    vector<game*>& vec = _map[obj_id];

    _record.added_games.push_back(g);
    _sum.add(g);
    vec.push_back(g);
}


////////////////////////////////////////

namespace sumgame_impl {

change_record::~change_record()
{
    // owner should have called one of the undo functions
    assert(deactivated_games.empty());
    assert(added_games.empty());
}

void change_record::simplify_basic(sumgame& sum)
{
    sumgame_map_view map_view(sum, *this);
    simplify_basic_nimber(map_view);
    simplify_basic_up_star(map_view);
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

////////////////////////////////////////

/*
    Convert game pointer types, after doing a few asserts.

    NOTE: This cast is generally unsafe. Only use in place of reinterpret_cast, not dynamic_cast
*/
template <class T_ptr>
inline T_ptr cast_game(game* g)
{
    static_assert(is_pointer_v<T_ptr>);
    using T = typename remove_pointer<T_ptr>::type; // NOLINT

    static_assert(is_base_of_v<game, T>);
    static_assert(!is_abstract_v<T>);

    assert(g != nullptr);
    assert(g->is_active());
    assert(g->get_obj_id() == get_obj_id<T>());

    return reinterpret_cast<T_ptr>(g);
}

void simplify_basic_nimber(sumgame_map_view& map_view)
{
    vector<game*>* nimbers = map_view.get_games_nullable(get_obj_id<nimber>());

    if (nimbers == nullptr)
    {
        return;
    }
        
    vector<int> heap_vec;

    for (game* g : *nimbers)
    {
        nimber* g_nimber = cast_game<nimber*>(g);

        int val = g_nimber->value();
        heap_vec.push_back(val);
    }

    if (heap_vec.size() >= 2 || (heap_vec.size() == 1 && heap_vec.back() < 2))
    {
        map_view.deactivate_games(*nimbers);

        int sum = nimber::nim_sum(heap_vec);
        assert(sum >= 0);
    
        // 0: add nothing
        if (sum == 1) // 1: star
        {
            up_star* new_game = new up_star(0, true);
            obj_id_t obj_id = get_obj_id<up_star>();

            map_view.add_game(new_game, obj_id);
        } else if (sum >= 2)
        {
            assert(sum >= 2); // >= 2: nimber

            nimber* new_game = new nimber(sum);
            obj_id_t obj_id = get_obj_id<nimber>();

            map_view.add_game(new_game, obj_id);
        }
    }

}

void simplify_basic_up_star(sumgame_map_view& map_view)
{
    vector<game*>* up_stars = map_view.get_games_nullable(get_obj_id<up_star>());

    if (up_stars == nullptr || up_stars->size() < 2)
    {
        return;
    }

    int ups = 0;
    bool star = false;

    for (game* g : *up_stars)
    {
        up_star* g_up_star = cast_game<up_star*>(g);

        ups += g_up_star->num_ups();
        star ^= g_up_star->has_star();

        map_view.deactivate_game(g);
    }

    if (ups != 0 || star != false)
    {
        up_star* new_game = new up_star(ups, star);
        obj_id_t obj_id = get_obj_id<up_star>();

        map_view.add_game(new_game, obj_id);
    }
}

