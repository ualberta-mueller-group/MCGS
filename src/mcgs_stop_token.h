/*
    Utilities for managing timeouts (because std::chrono spends too much
    time in system calls, and our previous timeout mechanism was messy and may
    have had UB)

    class mcgs_stop_token is used to poll whether time has run out, and
    is used by functions which should respect timeouts
    (i.e. sumgame::solve_with_timeout). 

    class mcgs_stop_source manages the timeout, and creates an mcgs_stop_token
    which has access to the shared timeout state. The mcgs_stop_token is only
    valid for the lifetime of the mcgs_stop_source which created it.

    Example usage:
        mcgs_stop_source ss;
        mcgs_stop_token st = ss.get_stop_token();

        ss.start_timeout(timeout_ms); // Time starts running here
        std::optional<int> fib = fibbonaci(ss, ...); // May or may not time out
        ss.cancel_timeout(); // Call regardless of completion/timeout status

    start_timeout() creates a thread which blocks until the timeout
    expires or cancel_timeout() is called. cancel_timeout() destroys the thread.

    The consuming function may pass around the mcgs_stop_token by const
    reference -- it doesn't need to copy the mcgs_stop_token (though this is
    also allowed)


    TODO: have we avoided all UB?

    TODO IMPORTANT: measure overhead of this implementation! gcc is ~5x slower
    than clang for the fibbonaci example in timeout_token.cpp, even when
    mcgs_stop_token and all atomics are omitted from the function... How to
    measure overhead then?
*/
#pragma once

#include <cassert>
#include <atomic>
#include <thread>
#include <optional>
#include <future>

////////////////////////////////////////////////// class mcgs_stop_token
class mcgs_stop_token
{
public:
    explicit mcgs_stop_token(std::atomic<bool>* should_stop);

    bool stop_requested() const;

private:
    std::atomic<bool>* _should_stop;
};

////////////////////////////////////////////////// class mcgs_stop_source
class mcgs_stop_source
{
public:
    mcgs_stop_source();
    ~mcgs_stop_source();

    // No copy
    mcgs_stop_source(const mcgs_stop_source&) = delete;
    mcgs_stop_source& operator=(const mcgs_stop_source&) = delete;

    // No move
    mcgs_stop_source(mcgs_stop_source&&) = delete;
    mcgs_stop_source& operator=(mcgs_stop_source&&) = delete;

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
    mcgs_stop_token get_stop_token();

private:
    std::shared_ptr<std::atomic<bool>> _should_stop;

    std::optional<std::thread> _timeout_thread;
    std::optional<std::promise<void>> _promise; // set to wake up timeout thread
    std::optional<std::future<void>> _future; // timeout thread blocks on this
};

////////////////////////////////////////////////// mcgs_stop_token methods
inline mcgs_stop_token::mcgs_stop_token(std::atomic<bool>* should_stop)
    : _should_stop(should_stop)
{
}

inline bool mcgs_stop_token::stop_requested() const
{
    return _should_stop->load(std::memory_order_relaxed);
}

////////////////////////////////////////////////// mcgs_stop_source methods
inline mcgs_stop_source::mcgs_stop_source()
    : _should_stop(new std::atomic<bool>())
{
    _should_stop->store(false, std::memory_order_relaxed);
}

inline mcgs_stop_source::~mcgs_stop_source()
{
    // Destroy thread if necessary
    if (timeout_running())
        cancel_timeout();

    assert(!timeout_running());
}

inline bool mcgs_stop_source::stop_requested() const
{
    return _should_stop->load(std::memory_order_relaxed);
}

inline bool mcgs_stop_source::timeout_running() const
{
    return _timeout_thread.has_value();
}

inline mcgs_stop_token mcgs_stop_source::get_stop_token()
{
    return mcgs_stop_token(_should_stop.get());
}
