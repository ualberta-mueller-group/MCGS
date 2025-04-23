#pragma once
// TODO

/*
Ideally, single macro to:
    - define class name for a global option
    - store class name as a member
    - define value type
    - define default value

    - initialize static values

This doesn't seem possible to do cleanly. String literals can't be passed
    as template arguments. I think there's a hack to do this but the code
    should be simple enough for students
*/


// This is awful
#define MAKE_GLOBAL_OPTION(Class_Name, Value_Type) \
class Class_Name \
{ \
public: \
        \
private: \
};


/*
    Solution:

    - make global_option<T>, with get(), set(), get_default(), name()
    - declare static global_option<T>s for every global option, in a
        namespace i.e. "namespace global_options"
    - make some nice init macro to use in the .cpp file, but keep it simple enough.
        Ideally it should take a class name as an argument, and also use that
        class name to set the "name" field
    - move globals out of cli_options and into global_options, and keep this file
        free of other "#includes". This header should ideally include nothing
    - constructor needs to "register" the variable somehow, so that a summary
        can be printed and used by the testing framework. All relevant
        configuration should be printed here

Don't make the fields static, just declare static global_option<T>s
*/
