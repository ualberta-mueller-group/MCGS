#include "hash_eval.h"
#include "cgt_switch.h"
#include "clobber_1xn.h"
#include <iostream>
#include <memory>
#include <unordered_set>
#include "all_game_headers.h"
#include "utilities.h"
#include <chrono>
#include <cstddef>
#include <cassert>
#include <vector>
#include <string>
#include <cstdint>
#include <ratio>
#include "sumgame.h"

using namespace std;

////////////////////////////////////////////////// hash_test
void hash_test::run_test(bool check_collisions)
{
    _reset();
    _check_collisions = check_collisions;

    chrono::time_point start = chrono::high_resolution_clock::now();
    _test_fn();
    chrono::time_point end = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> duration = end - start;
    _print_summary(duration);

    // clean up hash set to free up memory
    _reset();
}

void hash_test::_add_hash(const hash_t& hash)
{
    _add_hash_impl(hash);
}

void hash_test::_add_hash(game& g)
{
    if (!_add_hash_impl(g.get_local_hash()))
        cout << "COLLISION: " << g << endl;
}

void hash_test::_add_hash(sumgame& sg)
{
    _add_hash_impl(sg.get_global_hash());
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

bool hash_test::_add_hash_impl(const hash_t& hash)
{
    _n_games++;
    _n_zeroes += (hash == 0);

    _sum += hash;

    if (_check_collisions)
    {
        auto it = _hash_set.insert(hash);
        _n_collisions += (it.second == false);

        return it.second;
    }

    return true;
}

void hash_test::_print_summary(
    std::chrono::duration<double, std::milli>& duration) const
{
    cout << "Test name: \"" << _test_name() << "\"" << endl;
    cout << "Test description: \"" << _test_description() << "\"" << endl;
    cout << "Game count: " << _n_games << endl;

    if (_check_collisions)
    {
        cout << "Collisions: " << _n_collisions << endl;
        cout << "Collision %: "
             << (100.0 * (double) _n_collisions) / (double) _n_games << endl;
    }
    else
    {
        cout << "Not checking collisions..." << endl;
    }

    cout << "Zeroes: " << _n_zeroes << endl;

    if (!_check_collisions)
    {
        cout << "Duration (seconds): " << (duration.count() / 1000.0) << endl;
        cout << "Hashes/second: "
             << ((double) _n_games / (duration.count() / 1000.0)) << endl;
    }
    else
    {
        cout << "Duration invalid (collision check enabled)" << endl;
    }

    cout << "Sum of hashes (ignore this): " << _sum << endl;
    cout << endl;
}

////////////////////////////////////////////////// strip_iterator
strip_iterator::strip_iterator(size_t max_size)
    : _max_size(max_size), _first_board(true)
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
        if (_first_board)
        {
            _first_board = false;
            _has_next = true;
            return;
        }

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

const std::vector<int>& strip_iterator::operator*() const
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

////////////////////////////////////////////////// fraction_iterator
fraction_iterator::fraction_iterator(int top_size, int exponent_max,
                                     bool simplify)
    : _top_min(-top_size),
      _top_max(top_size),
      _exponent_max(exponent_max),
      _simplify(simplify),
      _frac(0)
{
    assert(top_size >= 0);
    assert(exponent_max >= 0);
    assert(exponent_max <= (int) size_in_bits<int>() - 1); // 31

    _increment(true);
}

void fraction_iterator::operator++()
{
    assert(_has_next);
    _increment(false);
}

fraction_iterator::operator bool() const
{
    return _has_next;
}

const fraction& fraction_iterator::operator*() const
{
    return _frac;
}

void fraction_iterator::_increment(bool init)
{
    assert(_has_next || init);

    _has_next |= init;
    while (_has_next)
    {
        _increment_step(init);
        init = false;

        if (!_simplify || _frac.bottom() == (1 << _exponent))
            return;
    }
}

inline void fraction_iterator::_increment_step(bool init)
{
    assert(_has_next || init);

    if (init)
    {
        _top = _top_min;
        _exponent = 0;

        _set_frac();
        _has_next = true;
        return;
    }

    if (_top < _top_max)
    {
        _top++;

        _set_frac();
        _has_next = true;
        return;
    }

    if (_exponent < _exponent_max)
    {
        _top = _top_min;
        _exponent++;

        _set_frac();
        _has_next = true;
        return;
    }

    _has_next = false;
    return;
}

void fraction_iterator::_set_frac()
{
    _frac.set(_top, (1 << _exponent));

    if (_simplify)
        _frac.simplify();
}

//////////////////////////////////////////////////////////// test
/// implementations

namespace {
namespace hash_tests {

class test_local_all_games : public hash_test
{
protected:
    std::string _test_name() const override { return "All Games (local)"; }

    std::string _test_description() const override
    {
        return "Local hashes for all game types, for large ranges.";
    }

    void _test_fn() override
    {
        const int LARGE_INT = 100000;
        const int SMALL_INT = 300;

        // integers
        for (int i = -LARGE_INT; i <= LARGE_INT; i++)
        {
            integer_game g(i);
            _add_hash(g);
        }

        // rational
        for (fraction_iterator it(LARGE_INT, 30); it; ++it)
        {
            const fraction& f = *it;

            dyadic_rational g(f);
            _add_hash(g);
        }

        // nimber
        for (int i = 0; i <= LARGE_INT; i++)
        {
            nimber g(i);
            _add_hash(g);
        }

        // up_star
        for (int i = -LARGE_INT; i <= LARGE_INT; i++)
        {
            up_star g1(i, true);
            up_star g2(i, false);

            _add_hash(g1);
            _add_hash(g2);
        }

        // switch_game
        for (fraction_iterator it1(SMALL_INT, 30); it1; ++it1)
        {
            for (fraction_iterator it2(SMALL_INT, 30); it2; ++it2)
            {
                switch_game g(*it1, *it2);
                _add_hash(g);
            }
        }

        // strip games
        for (strip_iterator it(15); it; ++it)
        {
            const vector<int>& board = *it;

            clobber_1xn c(board);
            _add_hash(c);

            nogo_1xn n(board);
            _add_hash(n);

            elephants e(board);
            _add_hash(e);
        }
    }
};

class test_local_all_strips : public hash_test
{
public:
    std::string _test_name() const override { return "All Strips (local)"; }

    std::string _test_description() const override
    {
        return "Local hashes for all strip games, 0-16 tiles.";
    }

    void _test_fn() override
    {
        for (strip_iterator it(16); it; ++it)
        {
            const vector<int>& board = *it;

            clobber_1xn c(board);
            _add_hash(c);

            elephants e(board);
            _add_hash(e);

            nogo_1xn n(board);
            _add_hash(n);
        }
    }
};

class test_local_mono_strips : public hash_test
{
public:
    std::string _test_name() const override { return "Mono Strips (local)"; }

    std::string _test_description() const override
    {
        return "Local hashes for long strips of a single color.";
    }

    void _test_fn() override
    {
        const size_t MAX_LEN = 50000;

        static const int COLORS[] = {EMPTY, BLACK, WHITE};
        const size_t N_COLORS = sizeof(COLORS) / sizeof(COLORS[0]);

        vector<int> board;
        board.reserve(MAX_LEN);

        for (size_t color_idx = 0; color_idx < N_COLORS; color_idx++)
        {
            const int color = COLORS[color_idx];
            board.clear();

            for (size_t size = 1; size <= MAX_LEN; size++)
            {
                board.push_back(color);
                assert(board.size() == size);

                clobber_1xn c(board);
                _add_hash(c);

                elephants e(board);
                _add_hash(e);

                nogo_1xn n(board);
                _add_hash(n);
            }
        }
    }
};

class test_local_alt_strips : public hash_test
{
    std::string _test_name() const override
    {
        return "Alternating Strips (local)";
    }

    std::string _test_description() const override
    {
        return "Hashes for long alternating strip games (XOXO...XO, local).";
    }

    void _test_fn() override
    {
        const size_t MAX_PAIRS = 50000;

        vector<int> board;
        board.reserve(2 * MAX_PAIRS);

        for (size_t i = 0; i < MAX_PAIRS; i++)
        {
            board.push_back(BLACK);
            board.push_back(WHITE);

            clobber_1xn c(board);
            _add_hash(c);

            elephants e(board);
            _add_hash(e);

            nogo_1xn n(board);
            _add_hash(n);
        }
    }
};

class test_local_uint : public hash_test
{
public:
    std::string _test_name() const override { return "C++ Integers (local)"; }

    std::string _test_description() const override
    {
        return "Hashes for all 26 bit ints, and all 64 bit ints "
               "consisting of a 26 bit int duplicated in 2 places with "
               "consistent "
               "byte boundary alignment";
    }

    void _test_fn() override
    {
        const uint32_t N_BITS = 26;

        for (uint32_t i = 0; i < (1 << N_BITS); i++)
        {
            local_hash lh1;
            lh1.toggle_value(0, i);
            _add_hash(lh1);

            if (i == 0)
                continue;

            uint64_t duplicated = i;
            duplicated |= (duplicated << 32);
            assert((duplicated >> 32) == i);

            local_hash lh2;
            lh2.toggle_value(0, duplicated);
            _add_hash(lh2);
        }
    }
};

class test_global_long_repeat : public hash_test
{
public:
    std::string _test_name() const override
    {
        return "Repeated clobber_1xn (global)";
    }

    std::string _test_description() const override
    {
        return "Sumgame hash for a single clobber_1xn board, repeated "
               "1-32768 times.";
    }

    void _test_fn() override
    {
        const size_t MAX_REPEAT = 32768;

        const std::string board = "XOXO";

        sumgame sum(BLACK);
        vector<std::unique_ptr<clobber_1xn>> games;

        for (size_t i = 0; i < MAX_REPEAT; i++)
        {
            games.emplace_back(new clobber_1xn(board));
            sum.add(games.back().get());
            _add_hash(sum);
        }
    }
};

} // namespace hash_tests
} // namespace

void hash_eval_all()
{
    {
        hash_tests::test_global_long_repeat test;
        test.run_test(true);
    }
}
