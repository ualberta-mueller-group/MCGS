#include "mcgs_stop_token.h"

#include <atomic>
#include <cassert>
#include <thread>
#include <optional>
#include <future>
#include <chrono>

void mcgs_stop_source::start_timeout(unsigned long long timeout_ms)
{
    assert(!timeout_running());

    _should_stop->store(false, std::memory_order_relaxed);

    _promise.emplace();
    assert(_promise.has_value());

    _future.emplace(_promise.value().get_future());
    assert(_promise.has_value() && _future.has_value());

    _timeout_thread.emplace(std::thread([this, timeout_ms]() -> void
    {
        std::future_status status = std::future_status::ready;

        if (timeout_ms == 0)
            _future->wait();
        else
            status = _future->wait_for(std::chrono::milliseconds(timeout_ms));

        if (status == std::future_status::timeout)
            _should_stop->store(true, std::memory_order_relaxed);
    }));
}

void mcgs_stop_source::cancel_timeout()
{
    assert(timeout_running());

    // The timeout thread is blocking on the corresponding std::future.
    // Here we signal it to wake up
    _promise->set_value();
    _timeout_thread->join();

    _timeout_thread.reset();
    _future.reset();
    _promise.reset();

    assert(!timeout_running());
}

