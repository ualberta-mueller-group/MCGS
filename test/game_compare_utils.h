#pragma once
/*
    Defines function templates for comparing collections of games by their
        game_type() and print() methods

    These functions operate on vectors of any game pointer types, and
        allow any combinations of constness, i.e.:
            vector<game*>
            vector<const game*>
            vector<shared_ptr<integer_game>>
            etc.
*/

#include "game.h"
#include <cstdint>
#include "sumgame.h"
#include "custom_traits.h"
#include <unordered_set>

namespace compare_games_by_print {

//////////////////////////////////////// internal helper classes
class __game_info // NOLINT
{
public:
    game_type_t type;
    std::string printed;

    __game_info(const game& g);
    bool operator==(const __game_info& rhs) const;
};

class __game_info_hash // NOLINT
{
public:
    uint64_t operator()(const __game_info& info) const;
};

typedef std::unordered_multiset<__game_info, __game_info_hash>
    __game_info_set; // NOLINT

//////////////////////////////////////// useful function templates
template <class Game_Ptr1, class Game_Ptr2>
bool same_games(const std::vector<Game_Ptr1>& games1,
                const std::vector<Game_Ptr2>& games2, bool omit_inactive = true)
{
    static_assert(custom_traits::is_some_game_ptr_v<Game_Ptr1>);
    static_assert(custom_traits::is_some_game_ptr_v<Game_Ptr2>);

    __game_info_set set1;
    __game_info_set set2;

    for (const Game_Ptr1& ptr : games1)
    {
        if (!omit_inactive || ptr->is_active())
            set1.emplace(__game_info(*ptr));
    }

    for (const Game_Ptr2& ptr : games2)
    {
        if (!omit_inactive || ptr->is_active())
            set2.emplace(__game_info(*ptr));
    }

    return set1 == set2;
}

template <class Game_Ptr>
bool sumgame_same_games(const sumgame& sum,
                        const std::vector<Game_Ptr>& compare_games,
                        bool omit_inactive_sumgame = true,
                        bool omit_inactive_compare = true)
{
    static_assert(custom_traits::is_some_game_ptr_v<Game_Ptr>);

    std::vector<const game*> sumgame_games;

    const int N = sum.num_total_games();
    for (int i = 0; i < N; i++)
    {
        const game* g = sum.subgame_const(i);
        if (!omit_inactive_sumgame || g->is_active())
            sumgame_games.push_back(g);
    }

    return same_games(sumgame_games, compare_games, omit_inactive_compare);
}

} // namespace compare_games_by_print
