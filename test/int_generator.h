/*
    int_generator<Int_T>

    Generates uniformly spaced integral values for a given integral type
    and interval. Always generates the min and max interval values.
*/
#pragma once

#include <type_traits>
#include <cassert>
#include "safe_arithmetic.h"

////////////////////////////////////////////////// class int_generator<Int_T>
template <class Int_T>
class int_generator
{
    static_assert(std::is_integral_v<Int_T>);

public:
    // NOLINTNEXTLINE(readability-identifier-naming)
    using UInt_T = std::make_unsigned_t<Int_T>;

    // NOTE: max_steps is only approximate
    int_generator(Int_T min_val, Int_T max_val, UInt_T max_steps);

    operator bool() const;
    Int_T gen_int() const;
    void operator++();

private:
    const Int_T _max_val;

    Int_T _step;

    Int_T _next_value;
    bool _has_value;
};

////////////////////////////////////////////////// int_generator<Int_T> methods
template <class Int_T>
int_generator<Int_T>::int_generator(Int_T min_val, Int_T max_val, UInt_T max_steps)
    : _max_val(max_val)
{
    assert(min_val <= max_val);
    assert(max_steps >= 1);

    _next_value = min_val;
    _has_value = true;

    _step = (static_cast<UInt_T>(max_val) - static_cast<UInt_T>(min_val)) /
            max_steps;


    if (_step < 0)
    {
        if (!negate_is_safe(_step))
        {
            assert(add_is_safe(_step, Int_T(1)));
            _step += 1;
        }

        assert(negate_is_safe(_step));
        _step = -_step;
    }

    if (_step == 0)
        _step = 1;

    assert(_step > 0);
}

template <class Int_T>
inline int_generator<Int_T>::operator bool() const
{
    return _has_value;
}

template <class Int_T>
inline Int_T int_generator<Int_T>::gen_int() const
{
    assert(*this);
    return _next_value;
}

template <class Int_T>
void int_generator<Int_T>::operator++()
{
    assert(*this);
    assert(_next_value <= _max_val);

    if (add_is_safe(_next_value, _step) && (_next_value + _step <= _max_val))
        _next_value += _step;
    else
    {
        if (_next_value < _max_val)
            _next_value = _max_val;
        else
            _has_value = false;
    }
}

