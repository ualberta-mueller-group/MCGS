/*
    TODO there's probably a better way to do this but it works...

    Get rid of this idea entirely?


    Better solutions (?):

    1. Instead of creating a thread that calls the function, make the
       stop_source create a thread that requests the stop, and call the function
       on the main thread, passing a stop_token to it

    OR

    2. Check if previous slowness of std::chrono is from some inefficient
       conversion between time formats, or something else i.e. waiting for
       syscalls
*/
#pragma once
#include <optional>
#include <future>
#include <cassert>
#include <thread>
#include "mcgs_stop_token.h"

// Call a function pointer (can't be a non-static class method)
template <class Return_T, class... Arg_Ts>
std::optional<Return_T> call_with_timeout(
    unsigned long long timeout,
    std::optional<Return_T> (*func)(const mcgs_stop_token&, Arg_Ts...),
    Arg_Ts... args)
{
    std::promise<std::optional<Return_T>> promise;
    std::future<std::optional<Return_T>> future = promise.get_future();

    mcgs_stop_source stopsource;

    std::thread thr([&]() -> void
    {
        promise.set_value(func(stopsource.get_token(), args...));
    });

    std::future_status status = std::future_status::ready;

    if (timeout == 0)
        future.wait();
    else
        status = future.wait_for(std::chrono::milliseconds(timeout));

    if (timeout != 0 && status == std::future_status::timeout)
        stopsource.request_stop();

    future.wait();
    thr.join();

    assert(future.valid());
    return future.get();
}

// Call a pointer to a non-static class method
template <class Return_T, class T, class... Arg_Ts>
std::optional<Return_T> call_with_timeout(
    unsigned long long timeout, T* obj,
    std::optional<Return_T> (T::*func)(const mcgs_stop_token&, Arg_Ts...),
    Arg_Ts... args)
{
    std::promise<std::optional<Return_T>> promise;
    std::future<std::optional<Return_T>> future = promise.get_future();

    mcgs_stop_source stopsource;

    std::thread thr([&]() -> void
    {
        promise.set_value((obj->*func)(stopsource.get_token(), args...));
    });

    std::future_status status = std::future_status::ready;

    if (timeout == 0)
        future.wait();
    else
        status = future.wait_for(std::chrono::milliseconds(timeout));

    if (timeout != 0 && status == std::future_status::timeout)
        stopsource.request_stop();

    future.wait();
    thr.join();

    assert(future.valid());
    return future.get();
}

//////////////////////////////////////////////////
void test_call_with_timeout();
