#pragma once

#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <memory>
#include <vector>
#include <cassert>
#include <unordered_map>

/*
       TODO think more about naming...

   type_id <-- too similar to std::typeid
   class_id
   obj_id
*/

typedef unsigned int obj_id_t;

/*
class __obj_id_impl // NOLINT(readability-identifier-naming)
{
private:
    static obj_id_t _next_id;

    template <class T>
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
*/


// TODO clean this up and optimize later, just get it working for now
extern std::unordered_map<std::type_index, obj_id_t> __obj_id_map; // NOLINT
extern obj_id_t __next_id; // NOLINT
obj_id_t __get_obj_id(const std::type_index& ti); // NOLINT

class i_obj_id
{
public:
    virtual obj_id_t get_obj_id() const final
    {
        const std::type_index& idx = std::type_index(typeid(*this));
        return __get_obj_id(idx);

    }
};

template <class T>
obj_id_t get_obj_id()
{
    static_assert(std::is_base_of_v<i_obj_id, T>);
    return __get_obj_id(std::type_index(typeid(T)));
}

////////////////////////////////////////

/*
class multi_type_stack
{
public:
    inline bool empty() const
    {
        return _stack.empty();
    }

    inline size_t size() const
    {
        return _stack.size();
    }

    inline void push(i_obj_id* obj)
    {
        _stack.push_back(obj);
    }

    inline void pop()
    {
        assert(!empty());
        _stack.pop_back();
    }

    template<class T>
    T* back() const
    {
        assert(!empty());

        i_obj_id* obj = _stack.back();
        assert(obj->get_obj_id() == get_obj_id<T>());

        return static_cast<T*>(obj);
    }

    inline i_obj_id* back_untyped() const
    {
        assert(!empty());
        return _stack.back();
    }

private:
    std::vector<i_obj_id*> _stack;
};

class assert_restore_multi_type_stack
{
public:
    assert_restore_multi_type_stack(const multi_type_stack& stack);
    ~assert_restore_multi_type_stack();

private:
    const multi_type_stack& _stack;
    const size_t _size;
    const i_obj_id* _back;
};
*/
