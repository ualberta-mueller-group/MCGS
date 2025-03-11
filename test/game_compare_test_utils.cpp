#include "game_compare_test_utils.h"
#include "sumgame.h"
#include <sstream>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>


//////////////////////////////////////// declarations
bool same_games_by_type_and_print(const std::vector<const game*>& games1, const std::vector<const game*>& games2);

bool sumgame_is_games(const sumgame& sum, const std::vector<const game*>& games);
bool sumgame_is_games(const sumgame& sum, const std::vector<std::shared_ptr<const game>>& games);

//////////////////////////////////////// helpers
using namespace std;

class game_info
{
public:
    game_type_t type;
    std::string printed;

    game_info(const game* g);

    bool operator==(const game_info& rhs) const;
};

class game_info_hash
{
public:
    uint64_t operator()(const game_info& info) const;
};

////////////////////

game_info::game_info(const game* g)
{
    assert(g->is_active());

    type = g->game_type();

    stringstream stream;
    g->print(stream);
    printed = stream.str();
}

bool game_info::operator==(const game_info& rhs) const
{
    return (type == rhs.type) && (printed == rhs.printed);
}

uint64_t game_info_hash::operator()(const game_info& info) const
{
    static hash<game_type_t> hasher1;
    static hash<string> hasher2;

    return hasher1(info.type) ^ hasher2(info.printed);
}

//////////////////////////////////////// functions
bool same_games_by_type_and_print(const vector<const game*>& games1, const vector<const game*>& games2)
{
    unordered_multiset<game_info, game_info_hash> games1_set;
    unordered_multiset<game_info, game_info_hash> games2_set;

    for (const game* g : games1)
        games1_set.insert(g);

    for (const game* g : games2)
        games2_set.insert(g);

    return games1_set == games2_set;
}

bool sumgame_is_games(const sumgame& sum, const vector<const game*>& games)
{
    vector<const game*> sum_games;

    {
        const int N = sum.num_total_games();
        for (int i = 0; i < N; i++)
        {
            const game* g = sum.subgame_const(i);
            if (g->is_active())
                sum_games.push_back(g);
        }
    }

    return same_games_by_type_and_print(sum_games, games);
}

bool sumgame_is_games(const sumgame& sum, const vector<shared_ptr<const game>>& games)
{
    vector<const game*> games_const;

    for (const shared_ptr<const game>& g : games)
    {
        assert(g->is_active());
        games_const.push_back(g.get());
    }

    return sumgame_is_games(sum, games_const);
}

////////////////////////////////////////

void game_compare_test_main()
{
    cout << "TODO FINISH THESE UTILITY FUNCTIONS AND NAME THEM BETTER" << endl;
}
