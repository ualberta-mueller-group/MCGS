#pragma once

#include <chrono>
#include <optional>

// TODO unit test me
class stopwatch
{
public:
    void reset();
    void start();
    void stop();

    double get_duration_ms() const;

private:
    using chrono_clock_t = std::chrono::high_resolution_clock;
    using chrono_time_point_t = std::chrono::time_point<chrono_clock_t>;

    std::optional<chrono_time_point_t> _start_time;
    std::optional<chrono_time_point_t> _end_time;
    mutable std::optional<double> _time_ms;
};
