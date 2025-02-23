#include "obj_id.h"

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
