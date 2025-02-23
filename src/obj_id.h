#pragma once

#include <type_traits>

/*
       TODO think more about naming...

   type_id <-- too similar to std::typeid
   class_id
   obj_id
*/

typedef int obj_id_t;

class __obj_id_impl // NOLINT(readability-identifier-naming)
{
private:
    static obj_id_t _next_id;

    friend obj_id_t get_obj_id();
};

template <class T>
obj_id_t get_obj_id()
{
    static_assert(!std::is_abstract_v<T>);
    static const obj_id_t MY_ID = __obj_id_impl::_next_id++;
    return MY_ID;
}

class i_obj_id
{
public:
    virtual obj_id_t get_obj_id() const = 0;
};
