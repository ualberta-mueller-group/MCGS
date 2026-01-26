/*
   Utility for measuring the duration of some computation. Used by
   test cases (derived from i_test_case) to report search time in milliseconds.
*/
#pragma once

#include <chrono>
#include <optional>

// TODO unit test me
class stopwatch
{
public:
    void reset(); // reset to default state

    void start(); // precondition: start() has not been called
    void stop(); // precondition: start() was called, and stop() was not called

    // precondition: start() and stop() were called
    double get_duration_ms() const;

private:
    using stopwatch_clock_t = std::chrono::high_resolution_clock;
    using stopwatch_time_point_t = std::chrono::time_point<stopwatch_clock_t>;

    std::optional<stopwatch_time_point_t> _start_time;
    std::optional<stopwatch_time_point_t> _end_time;
    mutable std::optional<double> _time_ms;
};
