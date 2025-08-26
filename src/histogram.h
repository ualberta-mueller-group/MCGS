/*
    histogram type
*/
#pragma once
#include <iostream>
#include <cstdint>
#include <cassert>
#include <vector>

////////////////////////////////////////////////// class histogram
class histogram
{
public:
    histogram();
    histogram(const uint64_t& resize_up_to);

    void count(const uint64_t& val); // increment count
    void resize_at_least(const uint64_t& val);
    uint64_t get_count(const uint64_t& val) const;

private:
    friend std::ostream& operator<<(std::ostream&, const histogram&);

    std::vector<uint64_t> _counts;
};

std::ostream& operator<<(std::ostream& os, const histogram& hist);

////////////////////////////////////////////////// histogram methods
inline histogram::histogram()
{
}

inline histogram::histogram(const uint64_t& resize_up_to)
{
    resize_at_least(resize_up_to);
}

inline void histogram::count(const uint64_t& val)
{
    resize_at_least(val);
    _counts[val]++;
}

inline void histogram::resize_at_least(const uint64_t& val)
{
    if (val >= _counts.size())
        _counts.resize(val + 1, 0);
}

inline uint64_t histogram::get_count(const uint64_t& val) const
{
    assert(val < _counts.size());
    return _counts[val];
}
