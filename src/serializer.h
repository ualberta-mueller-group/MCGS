#pragma once
#include "iobuffer.h"
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <memory>
/*
    TODO break this into multiple files. Consider separating serialize
    templates, i.e. put templates for STL containers in another file

    TODO: how to handle floating point values?

    TODO: Explain SFINAE ("Enable" template parameter)
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

    inline static std::pair<T1, T2> load(ibuffer& is)
    {
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
