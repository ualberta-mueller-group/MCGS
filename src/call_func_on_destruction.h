#pragma once

template <class Func>
class call_func_on_destruction
{
public:
    call_func_on_destruction(Func fn) : _fn(fn) {}

    ~call_func_on_destruction() { _fn(); }

private:
    Func _fn;
};
