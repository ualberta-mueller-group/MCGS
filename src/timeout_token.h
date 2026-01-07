/*
    Utilities for managing timeouts

    (std::chrono spends too much time in system calls, and our previous
    timeout mechanism was messy and may have had UB)

    class timeout_token is used by functions which should respect timeouts,
    to poll whether time has run out (i.e. sumgame::solve_with_timeout). 

    class timeout_source manages the timeout, and creates a timeout_token
    which has access to the shared timeout state. The timeout_token is only
    valid for the lifetime of the timeout_source which created it.

    Example usage:
        timeout_source src;
        timeout_token tok = src.get_timeout_token();

        src.start_timeout(timeout_ms); // Time starts running here
        std::optional<int> fib = fibonacci(tok, ...); // May or may not time out
        src.cancel_timeout(); // Call regardless of completion/timeout status

    start_timeout() creates a thread which blocks until the timeout
    expires or cancel_timeout() is called. cancel_timeout() destroys the thread.
    The thread will be destroyed by the timeout_source destructor if present,
    but it's probably best to destroy the thread once it's no longer needed.
    When a timeout of 0 is used, no thread is created, and the timeout never
    ends.

    The consuming function may pass around the timeout_token either by value
    or by reference.

    TODO: Have we avoided all UB? Is std::memory_order_relaxed good enough (
    so long as a stale value can't be read forever)???
*/
#pragma once

#include <cassert>
#include <atomic>
#include <optional>

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

    // true IFF timeout thread exists
    bool timeout_running() const;

    // creates timeout thread. 0 means never time out
    void start_timeout(unsigned long long timeout_ms);

    // destroy timeout thread (after safely stopping it)
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
    // Destroy thread if necessary
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
