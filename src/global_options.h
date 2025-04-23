#pragma once
/*
    Manages global options

    To create a new global variable:
        1. see the "Options" section at the bottom of this file
        2. see the "Options" section at the bottom of global_options.cpp
*/

#include <string>
#include <sstream>
#include <cstdint>
#include <stddef.h>

////////////////////////////////////////////////// class global_option_base
class global_option_base
{
public:
    global_option_base(const std::string& name);
    virtual ~global_option_base();

    inline const std::string& name() const
    {
        return _name;
    }

    // i.e. some_cli_arg --> "--some-cli-arg"
    std::string flag() const;
    // i.e. some_cli_arg --> "--no-some-cli-arg"
    std::string no_flag() const;

    std::string virtual get_str() const = 0; // value as string
    std::string virtual get_default_str() const = 0; // default value as string

    // i.e. some_int: 21
    std::string get_summary_single() const;

    // Combine summaries of all global_options
    static std::string get_summary_all();

protected:
    std::string _name_with_dashes() const;

private:
    const std::string _name;
};

////////////////////////////////////////////////// class global_option
template <class T>
class global_option final: public global_option_base
{
public:
    /*
        "name" should be identifier name of the variable itself, i.e:

            // in global_options.h
            namespace global {
            extern global_option<int> some_int;
            }

            // in global_options.cpp
            namespace global {
            global_option<int> some_int("some_int", 21);

            // or preferably using a macro defined in global_options.cpp:
            INIT_GLOBAL(some_int, int, 21);
            }

    */
    global_option(const std::string& name, const T& default_value);

    inline const T& get() const;
    inline const T& operator*() const; // shorthand for get()
    std::string get_str() const override;

    inline void set(const T& new_value);

    inline const T& get_default() const;
    std::string get_default_str() const override;

private:
    const T _default_value;
    T _current_value;
};

////////////////////////////////////////////////// global_option implementation
template <class T>
global_option<T>::global_option(const std::string& name, const T& default_value)
    : global_option_base(name),
    _default_value(default_value),
    _current_value(default_value)
{
}

template <class T>
inline const T& global_option<T>::get() const
{
    return _current_value;
}

template <class T>
inline const T& global_option<T>::operator*() const
{
    return _current_value;
}

template <class T>
std::string global_option<T>::get_str() const
{
    std::stringstream str;
    str << get();
    return str.str();
}

template <class T>
inline void global_option<T>::set(const T& new_value)
{
    _current_value = new_value;
}

template <class T>
inline const T& global_option<T>::get_default() const
{
    return _default_value;
}

template <class T>
std::string global_option<T>::get_default_str() const
{
    std::stringstream str;
    str << get();
    return str.str();
}

//////////////////////////////////////////////////////////// Options
namespace global {

extern global_option<uint64_t> random_table_seed;
extern global_option<bool> subgame_split;
extern global_option<bool> simplify_basic_cgt;
extern global_option<size_t> tt_sumgame_idx_bits;

} // namespace global_options
