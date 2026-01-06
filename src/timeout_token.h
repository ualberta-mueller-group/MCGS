/*
    Utilities for handling timeouts
*/
#pragma once

#include <chrono>
#include <optional>
#include <cassert>

////////////////////////////////////////////////// class timeout_token
class timeout_token
{
public:
    timeout_token();

    // true IFF start() has been called, and reset() has not since been called
    bool is_initialized() const;

    // 0 means infinite time
    void start(unsigned long long timeout_ms); // must not be initialized yet
    void reset();

    bool stop_requested() const; // must be initialized first

private:
    using timeout_clock_t = std::chrono::steady_clock;
    using timeout_time_point_t = std::chrono::time_point<timeout_clock_t>;

    bool _infinite_time;
    std::optional<timeout_time_point_t> _end_time;
};

////////////////////////////////////////////////// timeout_token methods
inline timeout_token::timeout_token()
    : _infinite_time(false)
{
}

inline void timeout_token::start(unsigned long long timeout_ms)
{
    assert(!is_initialized());

    if (timeout_ms == 0)
    {
        _infinite_time = true;
        _end_time.reset();
    }
    else
    {
        _infinite_time = false;
        _end_time =
            timeout_clock_t::now() + std::chrono::milliseconds(timeout_ms);
    }

    assert(is_initialized());
}

inline void timeout_token::reset()
{
    _infinite_time = false;
    _end_time.reset();
    assert(!is_initialized());
}

inline bool timeout_token::is_initialized() const
{
    return _infinite_time || _end_time.has_value();
}

inline bool timeout_token::stop_requested() const
{
    assert(is_initialized());
    
    if (_infinite_time)
        return false;

    return timeout_clock_t::now() > _end_time.value();
}

//////////////////////////////////////////////////
void test_timeout_token();


