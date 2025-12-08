#include "stopwatch.h"
#include <cassert>

void stopwatch::reset()
{
    _start_time.reset();
    _end_time.reset();
    _time_ms.reset();
}

void stopwatch::start()
{
    assert(!_start_time.has_value() && !_end_time.has_value() &&
           !_time_ms.has_value());

    _start_time = stopwatch::chrono_clock_t::now();
}

void stopwatch::stop()
{
    assert(_start_time.has_value() && !_end_time.has_value() &&
           !_time_ms.has_value());

    _end_time = stopwatch::chrono_clock_t::now();
}

double stopwatch::get_duration_ms() const
{
    assert(_start_time.has_value() && _end_time.has_value());

    if (_time_ms.has_value())
        return _time_ms.value();

    std::chrono::duration<double, std::milli> duration =
        _end_time.value() - _start_time.value();

    _time_ms = duration.count();

    return _time_ms.value();
}
