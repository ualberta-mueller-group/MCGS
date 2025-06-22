#pragma once
#include "iobuffer.h"
#include <type_traits>
#include <vector>
/*
    TODO break this into multiple files. Consider separating serialize
    templates, i.e. put templates for STL containers in another file

    TODO: how to handle floating point values?
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

//////////////////////////////////////// TODO: serializable* (and derived *)

//////////////////////////////////////// vector<T>
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
