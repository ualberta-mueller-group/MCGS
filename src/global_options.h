#pragma once
/*
    Manages global options

    To create a new global variable:
        1. see the "Options" section at the bottom of this file
        2. see the "Options" section at the bottom of global_options.cpp
*/

// IWYU pragma: begin_exports
#include <string>
// IWYU pragma: end_exports

#include <sstream>
#include <cstdint>
#include <cstddef>

// Whether or not a global_option is printed by "./MCGS --print-optimizations"
enum global_summary_enum
{
    GLOBAL_SUMMARY_INCLUDE = 0,
    GLOBAL_SUMMARY_EXCLUDE,
};

////////////////////////////////////////////////// abstract class
/// global_option_base
class global_option_base
{
public:
    /*
       Variable's name as it appears in the code, and whether or not this
       value is included in the config summary
    */
    global_option_base(const std::string& name,
                       global_summary_enum print_summary);

    virtual ~global_option_base();

    inline const std::string& name() const { return _name; }

    // i.e. some_cli_arg --> "--some-cli-arg"
    std::string flag() const;
    // i.e. some_cli_arg --> "--no-some-cli-arg"
    std::string no_flag() const;

    std::string virtual get_str() const = 0;         // value as string
    std::string virtual get_default_str() const = 0; // default value as string

    // i.e. "some_int: 21"
    std::string get_summary_single() const;

    // Combine summaries of all global_options (who are marked to be included)
    static std::string get_summary_all();

protected:
    // name(), but replace '_' with '-'
    std::string _name_with_dashes() const;

private:
    const std::string _name;
};

////////////////////////////////////////////////// class global_option
template <class T>
class global_option final : public global_option_base
{
public:
    /*
        "name" should be identifier name of the variable itself, i.e:

            // in global_options.h
            namespace global {
            extern global_option<int> some_int;
            }

            // in global_options.cpp (all 3 definitions are equivalent)
            namespace global {

            global_option<int> some_int("some_int", 21);
            global_option<int> some_int("some_int", 21, GLOBAL_SUMMARY_INCLUDE);

            // preferred method
            INIT_GLOBAL_WITH_SUMMARY(some_int, int, 21);

            }

    */
    global_option(const std::string& name, const T& default_value,
                  global_summary_enum print_summary = GLOBAL_SUMMARY_INCLUDE);

    inline const T& get() const;          // get current value
    inline const T& operator()() const;   // shorthand for get()
    std::string get_str() const override; // current value as string

    inline void set(const T& new_value);

    inline const T& get_default() const;          // default value
    std::string get_default_str() const override; // default value as string

private:
    const T _default_value;
    T _current_value;
};

////////////////////////////////////////////////// global_option implementation
template <class T>
global_option<T>::global_option(const std::string& name, const T& default_value,
                                global_summary_enum print_summary)
    : global_option_base(name, print_summary),
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
inline const T& global_option<T>::operator()() const
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

extern global_option<uint64_t> random_seed;
extern global_option<bool> simplify_basic_cgt;
extern global_option<size_t> tt_sumgame_idx_bits;
extern global_option<size_t> tt_imp_sumgame_idx_bits;
extern global_option<bool> use_db;
extern global_option<bool> clear_tt;

extern global_option<bool> silence_warnings;

} // namespace global
