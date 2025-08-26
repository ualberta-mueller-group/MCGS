/*
    Serialization API

    Defines template struct serializer<T> with two static functions: save and
    load.

    To make your (non-polymorphic) type serializable, implement the template
    specialization: serializer<Your_Type>, and implement the static functions
    save and load. Several template specializations are defined in this file
    for STL types. Polymorphic types are handled differently, explained further
    below.

    Save and load functions should recursively invoke the serializer template
    where necessary. See serializer<vector<T>> implementation in this file for
    an example.

    Example load usage:
    string some_string = serializer<string>::load(some_ibuffer);

    Example save usage:
    serializer<vector<string>>::save(some_obuffer, some_string_vec);

        In this example, serializer<vector<string>> invokes serializer<string>.
        This allows complicated nesting of types, i.e:

        serializer<vector<pair<int32_t, int16_t>>>::save(...)

    IMPORTANT: Currently, it is easy to accidentally write non-portable code
    by passing non fixed width integer types to the serializer template,
    i.e. serializer<int> instead of serializer<int32_t>

    NOTE: The default-valued "Enable" template parameter is there to allow
    conditional template specialization using SFINAE (substitution failure is
    not an error). This is used for integer types.

    Polymorphic types T should:
    1. Derive from interface dyn_serializable (dynamic_serializable.h)
    2. Define:
        Method: void T::save_impl(obuffer&) const
        Function: static dyn_serializable* T::load_impl(ibuffer&)
    3. Call register_dyn_serializable<T>() during mcgs_init()

    Polymorphic type save usage example:
        game* some_game_ptr = new clobber("XO");
        // All 3 valid:
        serializer<dyn_serializable*>::save(some_obuffer, some_game_ptr);
        serializer<game*>::save(some_obuffer, some_game_ptr);
        serializer<clobber*>::save(some_obuffer, some_game_ptr);

    After any of the above, all of the below are valid (remember to use delete):
        serializer<dyn_serializable*>::load(some_ibuffer);
        serializer<game*>::load(some_ibuffer);
        serializer<clobber*>::load(some_ibuffer);

    NOTE: Polymorphic types must use pointers as in these examples
*/
#pragma once
#include "iobuffer.h"
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <memory>

/*
    TODO break this into multiple files. Consider separating serializer
    templates, i.e. put templates for STL containers in another file

    TODO: how to handle floating point values?

    TODO: Explain SFINAE? ("Enable" template parameter)

    TODO: Maybe the top level function call shouldn't be inlined. Have inline
    _save(), and non-inline save() which calls _save()

    TODO: How to enforce portability with integer problem?

    TODO: Enforce interface of serializer template's static functions at compile
        time?
*/

////////////////////////////////////////////////// serialize<T>
template <class T, class Enable = void>
struct serializer
{
    static_assert(false, "Not implemented!");
};

//////////////////////////////////////// integral types
template <class T>
struct serializer<T,
    std::enable_if_t<
        std::is_integral_v<T>,
        void
    >
>
{
    inline static void save(obuffer& os, const T& val)
    {
        os.__write<T>(val);
    }

    inline static T load(ibuffer& is)
    {
        return is.__read<T>();
    }
};

//////////////////////////////////////// std::string
/*
    TODO: Is this actually correct? Check if control codes are correctly
    dealt with...
*/
template <>
struct serializer<std::string>
{
    inline static void save(obuffer& os, const std::string& str)
    {
        const size_t size = str.size();
        os.write_u64(size);

        for (size_t i = 0; i < size; i++)
            os.write_i8(str[i]);
    }

    inline static std::string load(ibuffer& is)
    {
        std::string str;

        const uint64_t size = is.read_u64();
        str.reserve(size);

        for (uint64_t i = 0; i < size; i++)
            str.push_back(is.read_i8());

        return str;
    }
};

//////////////////////////////////////// std::vector<T>
template <class T>
struct serializer<std::vector<T>>
{
    inline static void save(obuffer& os, const std::vector<T>& val)
    {
        const size_t size = val.size();
        os.write_u64(size);

        for (size_t i = 0; i < size; i++)
            serializer<T>::save(os, val[i]);
    }

    inline static std::vector<T> load(ibuffer& is)
    {
        std::vector<T> vec;

        const uint64_t size = is.read_u64();
        vec.reserve(size);

        for (uint64_t i = 0; i < size; i++)
            vec.emplace_back(serializer<T>::load(is));

        return vec;
    }
};

//////////////////////////////////////// std::shared_ptr<T>
template <class T>
struct serializer<std::shared_ptr<T>>
{
    static inline void save(obuffer& os, const std::shared_ptr<T>& smart_ptr)
    {
        const T* ptr = smart_ptr.get();
        serializer<T*>::save(os, ptr);
    }

    static inline std::shared_ptr<T> load(ibuffer& is)
    {
        T* ptr = serializer<T*>::load(is);
        return std::shared_ptr<T>(ptr);
    }
};

//////////////////////////////////////// std::unique_ptr<T>
template <class T>
struct serializer<std::unique_ptr<T>>
{
    static inline void save(obuffer& os, const std::unique_ptr<T>& smart_ptr)
    {
        const T* ptr = smart_ptr.get();
        serializer<T*>::save(os, ptr);
    }

    static inline std::unique_ptr<T> load(ibuffer& is)
    {
        T* ptr = serializer<T*>::load(is);
        return std::unique_ptr<T>(ptr);
    }
};


////////////////////////////////////////////////// std::pair<T1, T2>
template <class T1, class T2>
struct serializer<std::pair<T1, T2>>
{
    inline static void save(obuffer& os, const std::pair<T1, T2>& p)
    {
        serializer<T1>::save(os, p.first);
        serializer<T2>::save(os, p.second);
    }

    // TODO make this more efficient
    inline static std::pair<T1, T2> load(ibuffer& is)
    {
        // Put these on different lines so they aren't called in the wrong
        // order...
        T1 first = serializer<T1>::load(is);
        T2 second = serializer<T2>::load(is);
        return std::pair<T1, T2>(first, second);
    }
};

////////////////////////////////////////////////// std::unordered_map<T1, T2>
/*
    TODO: std::unordered_map has many more template parameters. Change this
    to handle them all...
*/
template <class T1, class T2>
struct serializer<std::unordered_map<T1, T2>>
{
    inline static void save(obuffer& os, const std::unordered_map<T1, T2>& m)
    {
        const size_t size = m.size();
        os.write_u64(size);

        for (auto it = m.begin(); it != m.end(); it++)
            serializer<std::pair<T1, T2>>::save(os, *it);
    }

    inline static std::unordered_map<T1, T2> load(ibuffer& is)
    {
        std::unordered_map<T1, T2> m;

        const uint64_t size = is.read_u64();
        m.reserve(size);

        for (size_t i = 0; i < size; i++)
            m.emplace(serializer<std::pair<T1, T2>>::load(is));

        return m;
    }
};
