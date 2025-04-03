#include "hash_eval.h"
#include "hashing_benchmark.h"
#include <iostream>

using namespace std;

////////////////////////////////////////////////// hash_test
void hash_test::run_test(bool check_collisions)
{
    _reset();

    _check_collisions = check_collisions;
    _test_fn();
    _print_summary();

    // clean up hash set to free up memory
    _reset();
}

void hash_test::_add_hash(const hash_t& hash)
{
    _add_hash_impl(hash);
}

void hash_test::_add_hash(game& g)
{
    _add_hash_impl(g.compute_hash().get_value());
}

void hash_test::_add_hash(sumgame& sg)
{
    _add_hash_impl(sg.get_global_hash_value());
}

void hash_test::_add_hash(local_hash& lh)
{
    _add_hash_impl(lh.get_value());
}

void hash_test::_add_hash(global_hash& gh)
{
    _add_hash_impl(gh.get_value());
}

void hash_test::_reset()
{
    _hash_set = unordered_set<hash_t>();
    _n_games = 0;
    _n_collisions = 0;
    _n_zeroes = 0;
    _check_collisions = true;
    _sum = 0;
}

void hash_test::_add_hash_impl(const hash_t& hash)
{
    _n_games++;
    _n_zeroes += (hash == 0);

    if (_check_collisions)
    {
        auto it = _hash_set.insert(hash);
        _n_collisions += (it.second == false);
    }

    _sum += hash;
}

void hash_test::_print_summary() const
{
    cout << "Test name: \"" << _test_name() << "\"" << endl;
    cout << "Test description: \"" << _test_description() << "\"" << endl;
    cout << "Game count: " << _n_games << endl;

    if (_check_collisions)
    {
        cout << "Collisions: " << _n_collisions << endl;
        cout << "Collision %: " << (100.0 * (double) _n_collisions) / (double) _n_games << endl;
    } else
    {
        cout << "Not checking collisions..." << endl;
    }

    cout << "Zeroes: " << _n_zeroes << endl;
    cout << "Sum of hashes (ignore this): " << _sum << endl;
    cout << endl;
}

////////////////////////////////////////////////// strip_iterator
strip_iterator::strip_iterator(size_t max_size): _max_size(max_size)
{
    assert(max_size >= 1);

    _has_next = true;
    ++(*this);
}

void strip_iterator::operator++()
{
    assert(_has_next);

    if (_board.size() == 0)
    {
        _board.resize(1);

        _has_next = true;
        return;
    }

    _board.back() += 1;
    bool carry = false;

    for (auto it = _board.rbegin(); it != _board.rend(); it++)
    {
        int& val = *it;

        if (carry)
        {
            val += 1;
            carry = false;
        }

        if (val >= 3)
        {
            assert(val == 3);
            val %= 3;
            carry = true;
        }
    }

    if (!carry)
    {
        _has_next = true;
        return;
    }

    const size_t size = _board.size();

    if (size == _max_size)
    {
        _has_next = false;
        return;
    }

    _board.clear();
    _board.resize(size + 1);

    _has_next = true;
}

strip_iterator::operator bool() const
{
    return _has_next;
}

const vector<int>& strip_iterator::get() const
{
    const size_t N = _board.size();
    _board_compatible.resize(N);

    for (size_t i = 0; i < N; i++)
    {
        _board_compatible[i] = strip_iterator::_convert(_board[i]);
    }

    return _board_compatible;
}

int strip_iterator::_convert(int val)
{
    static const int TABLE[] = {EMPTY, BLACK, WHITE};

    assert(val >= 0 && val <= 2);
    return TABLE[val];
}

