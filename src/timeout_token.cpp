#include "timeout_token.h"

#include <atomic>
#include <cassert>
#include <thread>
#include <optional>
#include <future>
#include <chrono>

#ifndef __EMSCRIPTEN__
void timeout_source::start_timeout(unsigned long long timeout_ms)
{
    assert(!timeout_running());

    _should_stop->store(false, std::memory_order_relaxed);

    if (timeout_ms == 0)
    {
        _infinite_time = true;
        return;
    }

    _promise.emplace();
    assert(_promise.has_value());

    _future.emplace(_promise.value().get_future());
    assert(_promise.has_value() && _future.has_value());

    _timeout_thread.emplace(std::thread([this, timeout_ms]() -> void
    {
        //std::future_status status = std::future_status::ready;

        if (timeout_ms == 0)
            _future->wait();
        else
            _future->wait_for(std::chrono::milliseconds(timeout_ms));

        _should_stop->store(true, std::memory_order_relaxed);
    }));
}

void timeout_source::cancel_timeout()
{
    assert(timeout_running());

    _infinite_time = false;

    if (_timeout_thread.has_value())
    {
        assert(_promise.has_value());
        // The timeout thread is blocking on the corresponding std::future.
        // Here we signal it to wake up
        _promise->set_value();
        _timeout_thread->join();

        // Does this order matter?
        _timeout_thread.reset();
        _future.reset();
        _promise.reset();
    }
    
    _should_stop->store(true, std::memory_order_relaxed);
    assert(!timeout_running());
}
#else
void timeout_source::start_timeout(unsigned long long timeout_ms)
{
    assert(!timeout_running());
    _should_stop->store(false, std::memory_order_relaxed);
    _infinite_time = true;
    assert(timeout_running());
}

void timeout_source::cancel_timeout()
{
    assert(timeout_running());
    _infinite_time = false;
    _should_stop->store(true, std::memory_order_relaxed);
    assert(!timeout_running());
}
#endif
