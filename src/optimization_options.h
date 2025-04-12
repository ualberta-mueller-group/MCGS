#pragma once
#include <string>

class optimization_options
{
public:
    static std::string get_summary();

    // subgame_split
    inline static bool subgame_split() { return _subgame_split; }

    inline static void set_subgame_split(bool val) { _subgame_split = val; }

    // simplify_basic_cgt
    inline static bool simplify_basic_cgt() { return _simplify_basic_cgt; }

    inline static void set_simplify_basic_cgt(bool val)
    {
        _simplify_basic_cgt = val;
    }

    // sumgame_ttable
    inline static bool sumgame_ttable()
    {
        return _sumgame_ttable;
    }

    inline static void set_sumgame_ttable(bool val)
    {
        _sumgame_ttable = val;
    }

private:
    static bool _subgame_split;
    static bool _simplify_basic_cgt;
    static bool _sumgame_ttable;
};
