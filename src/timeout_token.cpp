#include "timeout_token.h"

#include <atomic>
#include <chrono>
#include <optional>
#include <iostream>
#include <cassert>
#include <thread>
#include <future>

#include "mcgs_stop_token.h"

#define MAX_FIB 1024

namespace {

bool fib_flags[MAX_FIB];
uint64_t n_calls;

void init_flags()
{
    n_calls = 0;

    for (int i = 0; i < MAX_FIB; i++)
        fib_flags[i] = false;
}

int max_flag()
{
    int best = -1;

    for (int i = 0; i < MAX_FIB; i++)
    {
        if (fib_flags[i])
            best = i;
        else
            break;
    }

    return best;
}

std::optional<int> fib_timeout_token(const timeout_token& tt, int n)
{
    n_calls++;
    if (n < 2)
    {
        fib_flags[n] = true;
        return n;
    }

    if (tt.stop_requested())
        return {};
    const std::optional<int> f1 = fib_timeout_token(tt, n - 1);

    if (tt.stop_requested())
        return {};
    const std::optional<int> f2 = fib_timeout_token(tt, n - 2);

    if (tt.stop_requested())
        return {};

    assert(f1.has_value() && f2.has_value());
    fib_flags[n] = true;
    return *f1 + *f2;
}

inline std::optional<int> fib_mcgs_stop_token(const mcgs_stop_token& st, int n)
{
    n_calls++;
    if (n < 2)
    {
        fib_flags[n] = true;
        return n;
    }

    if (st.stop_requested())
        return {};
    const std::optional<int> f1 = fib_mcgs_stop_token(st, n - 1);

    if (st.stop_requested())
        return {};
    const std::optional<int> f2 = fib_mcgs_stop_token(st, n - 2);

    if (!f1.has_value() || !f2.has_value())
        return {};

    //if (st.stop_requested())
    //    return {};

    fib_flags[n] = true;
    return *f1 + *f2;
}


std::optional<int> fib_thread_ub(const std::atomic<bool>& should_stop, int n)
{
    n_calls++;
    if (n < 2)
    {
        fib_flags[n] = true;
        return n;
    }

    if (should_stop.load(std::memory_order_relaxed))
        return {};
    const std::optional<int> f1 = fib_thread_ub(should_stop, n - 1);

    if (should_stop.load(std::memory_order_relaxed))
        return {};
    const std::optional<int> f2 = fib_thread_ub(should_stop, n - 2);

    if (should_stop.load(std::memory_order_relaxed))
        return {};

    assert(f1.has_value() && f2.has_value());
    fib_flags[n] = true;
    return *f1 + *f2;
}

std::optional<int> call_thread_ub(unsigned long long timeout_ms, int n)
{
    std::atomic<bool> should_stop = false;

    // spawn a thread, then wait with a timeout for it to complete
    std::promise<std::optional<int>> promise;
    std::future<std::optional<int>> future = promise.get_future();

    std::thread thr([&]() -> void
    {
        std::optional<int> result = fib_thread_ub(should_stop, n);
        promise.set_value(result);
    });

    std::future_status status = std::future_status::ready;

    if (timeout_ms == 0)
        future.wait();
    else
        status = future.wait_for(std::chrono::milliseconds(timeout_ms));

    if (timeout_ms != 0 && status == std::future_status::timeout)
        // Stop the thread
        should_stop = true;

    future.wait();
    thr.join();

    assert(future.valid());

    return future.get();
}



} // namespace


//////////////////////////////////////////////////
/*
   - std::chrono is terribly slow (timeout_token implementation)
   - std::atomic<bool> is fast (std::memory_order_relaxed seems especially fast?)
*/
void test_timeout_token()
{
    const unsigned long long TIMEOUT_MS = 5000;

    int n;
    std::cout << "Input: " << std::flush;
    std::cin >> n;

    assert(n < MAX_FIB);
    init_flags();

    std::cout << "Starting" << std::endl;

    //timeout_token tt;
    //tt.start(TIMEOUT_MS);
    //fib_timeout_token(tt, n);

    //call_thread_ub(TIMEOUT_MS, n);

    mcgs_stop_source ss;
    mcgs_stop_token st = ss.get_stop_token();
    ss.start_timeout(TIMEOUT_MS);
    fib_mcgs_stop_token(st, n);

    const int best = max_flag();
    std::cout << "Best: " << best << std::endl;
    std::cout << "N calls: " << n_calls << std::endl;
    std::cout << "N was: " << n << std::endl;

}
