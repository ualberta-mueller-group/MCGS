#include "game_type_test.h"
#include "all_game_headers.h"
#include <unordered_set>
#include <vector>

using namespace std;

namespace {
//////////////////////////////////////// helper function templates

// get type for single game
template <class T>
void extract_type(vector<game_type_t>& types, T* game_ptr)
{
    static_assert(is_concrete_game_v<T>);

    game_type_t type_dynamic = game_ptr->game_type();
    game_type_t type_template = game_type<T>();

    assert(type_dynamic == type_template);
    types.push_back(type_dynamic);
    delete game_ptr;
}

// base case
template <class T>
void get_types(vector<game_type_t>& types, T* game_ptr)
{
    extract_type(types, game_ptr);
}

// recursive case
template <class T, class ...Ts>
void get_types(vector<game_type_t>& types, T* game_ptr, Ts... game_ptrs)
{
    extract_type(types, game_ptr);
    get_types(types, game_ptrs...);
}

template <class ...Ts>
void test_games(Ts... game_ptrs)
{
    vector<game_type_t> types;
    get_types(types, game_ptrs...);

    unordered_set<game_type_t> type_set;

    for (game_type_t& type : types)
    {
        auto it = type_set.insert(type);
        assert(it.second == true); // check not already inserted
    }
}
} // namespace

void game_type_test_all()
{
    // no need for cleanup, extract_type() will delete
    test_games(
        new dyadic_rational(4, 8),
        new integer_game(1),
        new nimber(5),
        new switch_game(5, 3),
        new up_star(4, true),
        new clobber_1xn("XO"),
        new elephants("X...O"),
        new nogo_1xn("X..O")
    );
}
