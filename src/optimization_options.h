#pragma once
#include <string>

class optimization_options
{
public:
    static std::string get_summary();

    inline static bool simplify_basic_cgt_games()
    {
        return _simplify_basic_cgt_games;
    }

    inline static void set_simplify_basic_cgt_games(bool val)
    {
        _simplify_basic_cgt_games = val;
    }

private:
    static bool _simplify_basic_cgt_games;
};

