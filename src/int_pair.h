#pragma once
#include <utility>
#include <functional>
#include <cstddef>

// (row, col)
typedef std::pair<int, int> int_pair;

// hash for unordered_set, needed for unordered_set<pair<int, int>>
template<>
class std::hash<std::pair<int, int>>
{
public:
    // TODO uint64_t instead of size_t?
    inline size_t operator()(const pair<int, int>& p) const noexcept
    {
        return p.first ^ p.second;
    }
};
