#pragma once
#include <string>

class optimization_options
{
public:
    static std::string get_summary();

    // subgame_split
    inline static bool subgame_split()
    {
        return _subgame_split;
    }

    inline static void set_subgame_split(bool val)
    {
        _subgame_split = val;
    }

    // simplify_basic_cgt_games
    inline static bool simplify_basic_cgt_games()
    {
        return _simplify_basic_cgt_games;
    }

    inline static void set_simplify_basic_cgt_games(bool val)
    {
        _simplify_basic_cgt_games = val;
    }

private:
    static bool _subgame_split;
    static bool _simplify_basic_cgt_games;
};
