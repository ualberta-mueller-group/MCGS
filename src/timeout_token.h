/*
    Utilities for managing timeouts.

    See development notes for more details.

    (std::chrono spends too much time in system calls, and our previous
    timeout mechanism was messy and may have had UB)
*/
#pragma once

#include <cassert>
#include <atomic>
#include <optional>
#include <memory>

#ifndef __EMSCRIPTEN__
#include <thread>
#include <future>
#endif

////////////////////////////////////////////////// class timeout_token
class timeout_token
{
public:
    explicit timeout_token(std::atomic<bool>* should_stop);

    bool stop_requested() const;

private:
    std::atomic<bool>* _should_stop;
};

////////////////////////////////////////////////// class timeout_source
class timeout_source
{
public:
    timeout_source();
    ~timeout_source();

    // No copy
    timeout_source(const timeout_source&) = delete;
    timeout_source& operator=(const timeout_source&) = delete;

    // No move
    timeout_source(timeout_source&&) = delete;
    timeout_source& operator=(timeout_source&&) = delete;

    // Has time run out?
    bool stop_requested() const;

    // true IFF timeout currently running
    bool timeout_running() const;

    // Starts timeout (0 means never time out)
    void start_timeout(unsigned long long timeout_ms);

    // Destroy timeout
    // NOTE: call this regardless of whether the task completed or timed out.
    void cancel_timeout();

    // Create a lightweight object which can poll the timeout state
    timeout_token get_timeout_token();

private:
    std::shared_ptr<std::atomic<bool>> _should_stop;

    bool _infinite_time; // true IFF timeout 0

#ifndef __EMSCRIPTEN__
    std::optional<std::thread> _timeout_thread;
    std::optional<std::promise<void>> _promise; // set to wake up timeout thread
    std::optional<std::future<void>> _future; // timeout thread blocks on this
#endif
};

////////////////////////////////////////////////// timeout_token methods
inline timeout_token::timeout_token(std::atomic<bool>* should_stop)
    : _should_stop(should_stop)
{
}

inline bool timeout_token::stop_requested() const
{
    return _should_stop->load(std::memory_order_relaxed);
}

////////////////////////////////////////////////// timeout_source methods
inline timeout_source::timeout_source()
    : _should_stop(new std::atomic<bool>(false)),
      _infinite_time(false)
{
    _should_stop->store(false, std::memory_order_relaxed);
    assert(!timeout_running());
}

inline timeout_source::~timeout_source()
{
    // Destroy timeout if necessary
    if (timeout_running())
        cancel_timeout();

    assert(!timeout_running());
}

inline bool timeout_source::stop_requested() const
{
    return _should_stop->load(std::memory_order_relaxed);
}

inline bool timeout_source::timeout_running() const
{
#ifndef __EMSCRIPTEN__
    return _infinite_time || _timeout_thread.has_value();
#else
    return _infinite_time;
#endif
}

inline timeout_token timeout_source::get_timeout_token()
{
    return timeout_token(_should_stop.get());
}
