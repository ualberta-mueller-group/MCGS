#include "obj_id.h"

std::unordered_map<std::type_index, obj_id_t> __obj_id_map; // NOLINT
obj_id_t __next_id = 0; // NOLINT

obj_id_t __get_obj_id(const std::type_index& ti) // NOLINT
{
    auto it = __obj_id_map.find(ti);
    obj_id_t obj_id = 0;

    if (it == __obj_id_map.end())
    {
        obj_id = __next_id++;
        __obj_id_map.insert({ti, obj_id});
    } else
    {
        obj_id = it->second;
    }

    return obj_id;

}

/*
obj_id_t __obj_id_impl::_next_id = 0;



assert_restore_multi_type_stack::assert_restore_multi_type_stack(const multi_type_stack& stack)
    : _stack(stack), _size(stack.size()), _back(stack.empty() ? nullptr : stack.back_untyped())
{
}

assert_restore_multi_type_stack::~assert_restore_multi_type_stack()
{
    assert(_stack.size() == _size);
    if (!_stack.empty())
    {
        assert(_stack.back_untyped() == _back);
    }
}



*/
