#pragma once
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include <string>
#include <cstddef>
#include <optional>
#include <cstdint>
#include <utility>
#include <memory>
#include "iobuffer.h"
#include "custom_traits.h"

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

////////////////////////////////////////////////// serializer<T>
// clang-format off
template <class T, class Enable = void>
struct serializer
{
    // See note about deferred_false_v in custom_traits.h
    static_assert(deferred_false_v<T>, "Not implemented!");

    /*
        TODO: This is not SFINAE-friendly. However, simply declaring the
        template with no default definition allows strange behavior.

        In particular, for class foo, if serializer<foo> is not defined,
        it is still legal to instantiate serializer<vector<foo>>, so long as
        the static functions are not called.

        It's unlikely we'll need type traits like has_serializer_v<T>,
        so it's probably better to have clearer compilation errors.
    */
};

////////////////////////////////////////////////// save/load helpers
template <class T>
inline void serializer_load(ibuffer& is, T& val)
{
    val = serializer<T>::load(is);
}

template <class T>
inline void serializer_save(obuffer& os, const T& val)
{
    serializer<T>::save(os, val);
}

//////////////////////////////////////// pointers
template <class T>
struct serializer<T*>
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_Nonconst = std::remove_cv_t<T>;

    inline static void save(obuffer& os, const T* ptr)
    {
        serializer<T_Nonconst>::save(os, *ptr);
    }

    inline static T* load(ibuffer& is)
    {
        return serializer<T_Nonconst>::load_ptr(is);
    }
};

//////////////////////////////////////// integral types
template <class T>
struct serializer<T,
    std::enable_if_t<
        std::is_integral_v<T> && !std::is_same_v<T, bool>,
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


//////////////////////////////////////// bool
template <>
struct serializer<bool>
{
    inline static void save(obuffer& os, const bool& val)
    {
        os.write_bool(val);
    }

    inline static bool load(ibuffer& is)
    {
        return is.read_bool();
    }
};

//////////////////////////////////////// enum types
template <class Enum_T>
struct serializer<Enum_T,
    std::enable_if_t<
        std::is_enum_v<Enum_T>,
        void
    >
>
{
    inline static void save(obuffer& os, const Enum_T& val)
    {
        os.write_enum<Enum_T>(val);
    }

    inline static Enum_T load(ibuffer& is)
    {
        return is.read_enum<Enum_T>();
    }
};

// clang-format on

//////////////////////////////////////// std::tuple (empty version)
template <>
struct serializer<std::tuple<>>
{
    inline static void save(obuffer& os, const std::tuple<>& tup)
    {
        os.write_u8(0);
    }

    inline static std::tuple<> load(ibuffer& os)
    {
        os.read_u8();
        return {};
    }
};

//////////////////////////////////////// std::tuple (non-empty version)

// TODO this is kind of messy and maybe slow?
template <class... Ts>
struct serializer<std::tuple<Ts...>>
{
    inline static void save(obuffer& os, const std::tuple<Ts...>& tup)
    {
        save_impl impl(os);
        std::apply(impl, tup);
    }

    inline static std::tuple<Ts...> load(ibuffer& is)
    {
        std::tuple<Ts...> tup;

        load_impl impl(is);
        std::apply(impl, tup);

        return tup;
    }

private:
    struct save_impl
    {
        inline save_impl(obuffer& os) : os(os) {}

        template <class T_First>
        inline void operator()(const T_First& first)
        {
            serializer<T_First>::save(os, first);
        }

        template <class T_First, class... T_Rest>
        inline void operator()(const T_First& first, const T_Rest&... rest)
        {
            serializer<T_First>::save(os, first);
            (*this)(rest...);
        }

        obuffer& os;
    };

    struct load_impl
    {
        inline load_impl(ibuffer& is): is(is) {}

        template <class T_First>
        inline void operator()(T_First& first)
        {
            first = serializer<T_First>::load(is);
        }

        template <class T_First, class... T_Rest>
        inline void operator()(T_First& first, T_Rest&... rest)
        {
            first = serializer<T_First>::load(is);
            (*this)(rest...);
        }

        ibuffer& is;
    };

};

//////////////////////////////////////// std::optional
template <class T>
struct serializer<std::optional<T>>
{
    inline static void save(obuffer& os, const std::optional<T>& opt)
    {
        const bool has_value = opt.has_value();

        os.write_bool(has_value);

        if (has_value)
            serializer<T>::save(os, *opt);
    }

    inline static std::optional<T> load(ibuffer& is)
    {
        const bool has_value = is.read_bool();

        if (has_value)
            return serializer<T>::load(is);

        return {};
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

        if (ptr == nullptr)
        {
            os.write_bool(false);
            return;
        }

        os.write_bool(true);
        serializer<T*>::save(os, ptr);
    }

    static inline std::shared_ptr<T> load(ibuffer& is)
    {
        const bool has_value = is.read_bool();

        T* ptr = nullptr;

        if (has_value)
            ptr = serializer<T*>::load(is);

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

        if (ptr == nullptr)
        {
            os.write_bool(false);
            return;
        }

        os.write_bool(true);
        serializer<T*>::save(os, ptr);
    }

    static inline std::unique_ptr<T> load(ibuffer& is)
    {
        const bool has_value = is.read_bool();

        T* ptr = nullptr;

        if (has_value)
            ptr = serializer<T*>::load(is);

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

////////////////////////////////////////////////// std::map<T1, T2>
/*
    TODO: similar problem as unordered_map?
*/
template <class T1, class T2>
struct serializer<std::map<T1, T2>>
{
    inline static void save(obuffer& os, const std::map<T1, T2>& m)
    {
        const size_t size = m.size();
        os.write_u64(size);

        for (auto it = m.begin(); it != m.end(); it++)
            serializer<std::pair<T1, T2>>::save(os, *it);
    }

    inline static std::map<T1, T2> load(ibuffer& is)
    {
        std::map<T1, T2> m;

        const uint64_t size = is.read_u64();
        //m.reserve(size);

        for (size_t i = 0; i < size; i++)
            m.emplace(serializer<std::pair<T1, T2>>::load(is));

        return m;
    }
};

////////////////////////////////////////////////// std::set<T>
template <class T>
struct serializer<std::set<T>>
{
    inline static void save(obuffer& os, const std::set<T> s)
    {
        const uint64_t size = s.size();
        os.write_u64(size);

        for (const T& val : s)
            serializer<T>::save(os, val);
    }

    inline static std::set<T> load(ibuffer& is)
    {
        std::set<T> s;

        const uint64_t size = is.read_u64();
        for (uint64_t i = 0; i < size; i++)
            s.emplace(serializer<T>::load(is));

        return s;
    }
};

////////////////////////////////////////////////// std::unordered_set<T>
template <class T>
struct serializer<std::unordered_set<T>>
{
    inline static void save(obuffer& os, const std::unordered_set<T>& s)
    {
        const uint64_t size = s.size();
        os.write_u64(size);

        for (const T& val : s)
            serializer<T>::save(os, val);
    }

    inline static std::unordered_set<T> load(ibuffer& is)
    {
        std::unordered_set<T> s;

        const uint64_t size = is.read_u64();
        for (uint64_t i = 0; i < size; i++)
            s.emplace(serializer<T>::load(is));

        return s;
    }
};

