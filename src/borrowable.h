/*
    Wrapper for global/class static values which may be written to from
    multiple places, i.e. `global_hash` helper objects. Prevents usage
    errors.

    Important: This is not thread-safe!
*/
#pragma once

#include <cassert>
#include <utility>

////////////////////////////////////////////////// class borrowable<T>
template <class T>
class borrowable
{
public:
    borrowable();
    borrowable(T&& initial_value);

    ~borrowable();

    // No copy/move
    borrowable(const borrowable& other) = delete;
    borrowable& operator=(const borrowable& other) = delete;
    borrowable(borrowable&& other) = delete;
    borrowable& operator=(borrowable&& other) = delete;

    [[nodiscard]] T* borrow();
    [[nodiscard]] const T* borrow() const;

    // Overwrites caller's pointer with nullptr
    void unborrow(T*& value); // Overwr
    void unborrow(const T*& value) const;

protected:
    T _value;
    mutable bool _is_borrowed;
};

////////////////////////////////////////////////// borrowable<T> methods
template <class T>
inline borrowable<T>::borrowable() : _is_borrowed(false)
{
}

template <class T>
inline borrowable<T>::borrowable(T&& initial_value)
    : _value(std::forward<T>(initial_value)), _is_borrowed(false)
{
}

template <class T>
inline borrowable<T>::~borrowable()
{
    assert(!_is_borrowed);
}

template <class T>
[[nodiscard]] inline T* borrowable<T>::borrow()
{
    assert(!_is_borrowed);
    _is_borrowed = true;
    return &_value;
}

template <class T>
[[nodiscard]] inline const T* borrowable<T>::borrow() const
{
    assert(!_is_borrowed);
    _is_borrowed = true;
    return &_value;
}

template <class T>
inline void borrowable<T>::unborrow(const T*& value) const
{
    assert(_is_borrowed && value == &_value);
    _is_borrowed = false;
    value = nullptr;
}

template <class T>
inline void borrowable<T>::unborrow(T*& value)
{
    assert(_is_borrowed && value == &_value);
    _is_borrowed = false;
    value = nullptr;
}

