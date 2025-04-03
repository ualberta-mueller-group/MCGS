#pragma once

#include <cstdint>
#include <unordered_set>
#include <string>
#include "hashing.h"
#include "game.h"
#include "sumgame.h"

////////////////////////////////////////////////// hash_test
class hash_test
{
public:
    hash_test()
        : _hash_set(),
        _n_games(0),
        _n_collisions(0),
        _n_zeroes(0),
        _sum(0)
    {
    }

    virtual ~hash_test()
    {
    }

    void run_test(bool check_collisions = true);

protected:
    virtual void _test_fn() = 0;
    virtual std::string _test_name() const = 0;
    virtual std::string _test_description() const = 0;

    void _add_hash(const hash_t& hash);
    void _add_hash(game& g);
    void _add_hash(sumgame& sg);
    void _add_hash(local_hash& lh);
    void _add_hash(global_hash& gh);

private:
    void _reset();
    void _add_hash_impl(const hash_t& hash);
    void _print_summary() const;

    std::unordered_set<hash_t> _hash_set;
    uint64_t _n_games;
    uint64_t _n_collisions;
    uint64_t _n_zeroes;
    hash_t _sum;

    bool _check_collisions;
};

////////////////////////////////////////////////// strip_iterator
class strip_iterator
{
public:
    strip_iterator(size_t max_size);

    void operator++();
    operator bool() const;
    const std::vector<int>& get() const;

private:
    static int _convert(int val);

    const size_t _max_size;
    std::vector<int> _board;
    mutable std::vector<int> _board_compatible;
    bool _has_next;
};

