#pragma once
#include <utility>
#include <functional>

// (row, col)
typedef std::pair<int, int> int_pair;

// hash for unordered_set, needed for unordered_set<pair<int, int>>
template<>
class std::hash<std::pair<int, int>>
{
public:
    inline size_t operator()(const pair<int, int>& p) const noexcept
    {
        return p.first ^ p.second;
    }
};
