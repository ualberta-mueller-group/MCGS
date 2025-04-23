#pragma once
/*
    Manages global options

    To create a new global variable:
        1. see the "Options" section at the bottom of this file
        2. see the "Options" section at the bottom of global_options.cpp
*/

#include <string>
#include <sstream>

////////////////////////////////////////////////// class global_option_base

// Enables us to print a summary of all global_options
class global_option_base
{
public:
    // registers every global_option when it's constructed, so that we can get
    // a summary of all options
    global_option_base();

    virtual ~global_option_base()
    {
    }

    virtual std::string name() const = 0;
    virtual std::string get_summary_single() const = 0;

    // summary of all global_options
    static std::string get_summary_all();
private:
};

////////////////////////////////////////////////// class global_option
template <class T>
class global_option final: public global_option_base
{
public:
    /*
        "name" should be identifier name of the variable itself, i.e:

            // in global_options.h
            namespace global_options {
            extern global_option<int> some_int;
            }

            // in global_options.cpp
            namespace global_options {
            global_option<int> some_int("some_int", 21);

            // or preferably using a macro defined in global_options.cpp:
            INIT_GLOBAL(some_int, int, 21);
            }

    */
    global_option(const std::string& name, const T& default_value);

    const T& get() const;
    const T& get_default() const;

    void set(const T& new_value);

    std::string name() const override;

    // i.e. "some_value_name: 8"
    std::string get_summary_single() const override;

private:
    const std::string _name;

    const T _default_value;
    T _current_value;
};

////////////////////////////////////////////////// global_option implementation
template <class T>
global_option<T>::global_option(const std::string& name, const T& default_value)
    : _name(name),
    _default_value(default_value),
    _current_value(default_value)
{
}

template <class T>
const T& global_option<T>::get() const
{
    return _current_value;
}

template <class T>
const T& global_option<T>::get_default() const
{
    return _default_value;
}

template <class T>
void global_option<T>::set(const T& new_value)
{
    _current_value = new_value;
}

template <class T>
std::string global_option<T>::name() const
{
    return _name;
}

template <class T>
std::string global_option<T>::get_summary_single() const
{
    std::stringstream str;
    str << name() << ": " << get();
    return str.str();
}

//////////////////////////////////////////////////////////// Options
namespace global_options {

extern global_option<int> some_value1;
extern global_option<double> some_value3;
extern global_option<float> some_value2;

} // namespace global_options
