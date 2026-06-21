/*
    Serialization API

    Defines template struct serializer<T> with 3 static functions:
        - save
        - load
        - load_ptr

    To make your (non-polymorphic) type serializable, implement the template
    specialization: serializer<Your_Type>, and implement the static functions
    save, load, and load_ptr. Several template specializations are defined in
    this file for STL types. Polymorphic types are handled differently,
    explained further below.

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
        Method: void T::save_impl(i_obuffer&) const
        Function: static dyn_serializable* T::load_impl(i_ibuffer&)
    3. Call register_dyn_serializable<T>() during mcgs_init_all()

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

    Currently implemented:
        - pointers
        - integral values
        - bool
        - enum values
        - std::optional<T>
        - std::string
        - std::vector<T>
        - std::shared_ptr<T>
        - std::unique_ptr<T>
        - std::pair<T1, T2>
        - std::unordered_map<T1, T2>
        - std::map<T1, T2>
        - std::set<T>
        - std::unordered_set<T>
        - std::monostate
        - std::variant<Ts...>

    Removed:
        - std::tuple<Ts...>
*/
#pragma once
#include <cstdlib>
#include <variant>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <optional>
#include <string>
#include <set>
#include <unordered_set>
#include <cstddef>
#include <cstdint>
#include <map>
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

struct serializer_ctx
{
public:
    inline serializer_ctx() : thermograph_cache_ptr(nullptr)
    {
    }

    void* thermograph_cache_ptr;
};

// clang-format off

////////////////////////////////////////////////// serializer<T>
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

// TODO implement STL types as serializer_impl instead so that specialization
// using serializer_ctx is easier
template <class T, class Enable = void>
struct serializer_impl
{
    static_assert(deferred_false_v<T>, "Not implemented!");
};

////////////////////////////////////////////////// save/load helpers
template <class T>
inline void serializer_load(i_ibuffer& is, T& val, serializer_ctx* ctx = nullptr)
{
    val = serializer<T>::load(is, ctx);
}

template <class T>
inline void serializer_save(i_obuffer& os, const T& val,
                            serializer_ctx* ctx = nullptr)
{
    serializer<T>::save(os, val, ctx);
}

//////////////////////////////////////// pointers
template <class T>
struct serializer<T*>
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_NoCV = std::remove_cv_t<T>;

    inline static void save(i_obuffer& os, const T* ptr, serializer_ctx* ctx)
    {
        serializer<T_NoCV>::save(os, *ptr, ctx);
    }

    inline static T* load(i_ibuffer& is, serializer_ctx* ctx)
    {
        return serializer<T_NoCV>::load_ptr(is, ctx);
    }
};

//////////////////////////////////////// integral types
template <class T>
struct serializer<
    T,
    std::enable_if_t<
        std::is_integral_v<T> && !std::is_same_v<T, bool>,
        void
    >
>
{
    inline static void save(i_obuffer& os, const T& val, serializer_ctx* ctx)
    {
        os.__write<T>(val);
    }

    inline static T load(i_ibuffer& is, serializer_ctx* ctx)
    {
        return is.__read<T>();
    }
};

//////////////////////////////////////// bool
template <>
struct serializer<bool>
{
    inline static void save(i_obuffer& os, const bool& val, serializer_ctx* ctx)
    {
        os.write_bool(val);
    }

    inline static bool load(i_ibuffer& is, serializer_ctx* ctx)
    {
        return is.read_bool();
    }
};

//////////////////////////////////////// enum types
template <class Enum_T>
struct serializer<
    Enum_T,
    std::enable_if_t<
        std::is_enum_v<Enum_T>,
        void
    >
>
{
    inline static void save(i_obuffer& os, const Enum_T& val, serializer_ctx* ctx)
    {
        os.write_enum<Enum_T>(val);
    }

    inline static Enum_T load(i_ibuffer& is, serializer_ctx* ctx)
    {
        return is.read_enum<Enum_T>();
    }
};

//////////////////////////////////////// std::optional
template <class T>
struct serializer<std::optional<T>>
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_NoCV = std::remove_cv_t<T>;

    inline static void save(i_obuffer& os, const std::optional<T>& opt,
                            serializer_ctx* ctx)
    {
        const bool has_value = opt.has_value();
        os.write_bool(has_value);

        if (has_value)
            serializer<T_NoCV>::save(os, *opt, ctx);
    }

    inline static std::optional<T> load(i_ibuffer& is, serializer_ctx* ctx)
    {
        const bool has_value = is.read_bool();

        if (has_value)
            return serializer<T_NoCV>::load(is, ctx);

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
    inline static void save(i_obuffer& os, const std::string& str,
                            serializer_ctx* ctx)
    {
        const size_t size = str.size();
        os.write_u64(size);

        for (size_t i = 0; i < size; i++)
            os.write_i8(str[i]);
    }

    inline static std::string load(i_ibuffer& is, serializer_ctx* ctx)
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
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_NoCV = std::remove_cv_t<T>;

    inline static void save(i_obuffer& os, const std::vector<T>& val,
                            serializer_ctx* ctx)
    {
        const size_t size = val.size();
        os.write_u64(size);

        for (size_t i = 0; i < size; i++)
            serializer<T_NoCV>::save(os, val[i], ctx);
    }

    inline static std::vector<T> load(i_ibuffer& is, serializer_ctx* ctx)
    {
        std::vector<T> vec;

        const uint64_t size = is.read_u64();
        vec.reserve(size);

        for (uint64_t i = 0; i < size; i++)
            vec.emplace_back(serializer<T_NoCV>::load(is, ctx));

        return vec;
    }
};

//////////////////////////////////////// std::shared_ptr<T>
template <class T>
struct serializer_impl<std::shared_ptr<T>>
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_NoCV = std::remove_cv_t<T>;

    static inline void save(i_obuffer& os, const std::shared_ptr<T>& smart_ptr,
                            serializer_ctx* ctx)
    {
        const T* ptr = smart_ptr.get();

        if (ptr == nullptr)
        {
            os.write_bool(false);
            return;
        }

        os.write_bool(true);
        serializer<T_NoCV*>::save(os, ptr, ctx);
    }

    static inline std::shared_ptr<T> load(i_ibuffer& is, serializer_ctx* ctx)
    {
        T* ptr = nullptr;

        const bool has_value = is.read_bool();

        if (has_value)
            ptr = serializer<T_NoCV*>::load(is, ctx);

        return std::shared_ptr<T>(ptr);
    }
};

template <class T>
struct serializer<std::shared_ptr<T>>
{
    static inline void save(i_obuffer& os, const std::shared_ptr<T>& smart_ptr,
                            serializer_ctx* ctx)
    {
        serializer_impl<std::shared_ptr<T>>::save(os, smart_ptr, ctx);
    }

    static inline std::shared_ptr<T> load(i_ibuffer& is, serializer_ctx* ctx)
    {
        return serializer_impl<std::shared_ptr<T>>::load(is, ctx);
    }
};

//////////////////////////////////////// std::unique_ptr<T>
template <class T>
struct serializer<std::unique_ptr<T>>
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_NoCV = std::remove_cv_t<T>;

    static inline void save(i_obuffer& os, const std::unique_ptr<T>& smart_ptr,
                            serializer_ctx* ctx)
    {
        const T* ptr = smart_ptr.get();

        if (ptr == nullptr)
        {
            os.write_bool(false);
            return;
        }

        os.write_bool(true);
        serializer<T_NoCV*>::save(os, ptr, ctx);
    }

    static inline std::unique_ptr<T> load(i_ibuffer& is, serializer_ctx* ctx)
    {
        T* ptr = nullptr;

        const bool has_value = is.read_bool();

        if (has_value)
            ptr = serializer<T_NoCV*>::load(is, ctx);

        return std::unique_ptr<T>(ptr);
    }
};

////////////////////////////////////////////////// std::pair<T1, T2>
template <class T1, class T2>
struct serializer<std::pair<T1, T2>>
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T1_NoCV = std::remove_cv_t<T1>;
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T2_NoCV = std::remove_cv_t<T2>;

    inline static void save(i_obuffer& os, const std::pair<T1, T2>& p,
                            serializer_ctx* ctx)
    {
        serializer<T1_NoCV>::save(os, p.first, ctx);
        serializer<T2_NoCV>::save(os, p.second, ctx);
    }

    inline static std::pair<T1, T2> load(i_ibuffer& is, serializer_ctx* ctx)
    {
        T1_NoCV val1 = serializer<T1_NoCV>::load(is, ctx);
        T2_NoCV val2 = serializer<T2_NoCV>::load(is, ctx);

        return {std::move(val1), std::move(val2)};
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
    inline static void save(i_obuffer& os, const std::unordered_map<T1, T2>& m,
                            serializer_ctx* ctx)
    {
        const size_t size = m.size();
        os.write_u64(size);

        for (const std::pair<const T1, T2>& map_pair : m)
            serializer<std::pair<const T1, T2>>::save(os, map_pair, ctx);
    }

    inline static std::unordered_map<T1, T2> load(i_ibuffer& is,
                                                  serializer_ctx* ctx)
    {
        std::unordered_map<T1, T2> m;

        const uint64_t size = is.read_u64();
        m.reserve(size);

        for (uint64_t i = 0; i < size; i++)
            m.emplace(serializer<std::pair<const T1, T2>>::load(is, ctx));

        return m;
    }
};

////////////////////////////////////////////////// std::map<T1, T2>
template <class T1, class T2>
struct serializer<std::map<T1, T2>>
{
    inline static void save(i_obuffer& os, const std::map<T1, T2>& m,
                            serializer_ctx* ctx)
    {
        const size_t size = m.size();
        os.write_u64(size);

        for (const std::pair<const T1, T2>& map_pair : m)
            serializer<std::pair<const T1, T2>>::save(os, map_pair, ctx);
    }

    inline static std::map<T1, T2> load(i_ibuffer& is, serializer_ctx* ctx)
    {
        std::map<T1, T2> m;

        const uint64_t size = is.read_u64();
        // m.reserve(size);

        for (uint64_t i = 0; i < size; i++)
            m.emplace(serializer<std::pair<const T1, T2>>::load(is, ctx));

        return m;
    }
};

////////////////////////////////////////////////// std::set<T>
template <class T>
struct serializer<std::set<T>>
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_NoCV = std::remove_cv_t<T>;

    inline static void save(i_obuffer& os, const std::set<T>& s,
                            serializer_ctx* ctx)
    {
        const uint64_t size = s.size();
        os.write_u64(size);

        for (const T& val : s)
            serializer<T_NoCV>::save(os, val, ctx);
    }

    inline static std::set<T> load(i_ibuffer& is, serializer_ctx* ctx)
    {
        std::set<T> s;

        const uint64_t size = is.read_u64();
        for (uint64_t i = 0; i < size; i++)
            s.emplace(serializer<T_NoCV>::load(is, ctx));

        return s;
    }
};

////////////////////////////////////////////////// std::unordered_set<T>
template <class T>
struct serializer<std::unordered_set<T>>
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_NoCV = std::remove_cv_t<T>;

    inline static void save(i_obuffer& os, const std::unordered_set<T>& s,
                            serializer_ctx* ctx)
    {
        const uint64_t size = s.size();
        os.write_u64(size);

        for (const T& val : s)
            serializer<T_NoCV>::save(os, val, ctx);
    }

    inline static std::unordered_set<T> load(i_ibuffer& is, serializer_ctx* ctx)
    {
        std::unordered_set<T> s;

        const uint64_t size = is.read_u64();
        for (uint64_t i = 0; i < size; i++)
            s.emplace(serializer<T_NoCV>::load(is, ctx));

        return s;
    }
};

////////////////////////////////////////////////// std::monostate
// TODO should this write anything to disk at all?
template <>
struct serializer<std::monostate>
{
    inline static void save(i_obuffer& os, const std::monostate& val, serializer_ctx* ctx)
    {
        //os.write_bool(false);
    }

    inline static std::monostate load(i_ibuffer& is, serializer_ctx* ctx)
    {
        std::monostate val;

        //const bool b = is.read_bool();
        //if (b != false)
        //    std::abort();

        return val;
    }

};

////////////////////////////////////////////////// std::variant<Ts...>
template <class... Ts>
struct serializer<std::variant<Ts...>>
{
    typedef std::variant<Ts...> variant_t;

    // NOLINTNEXTLINE(readability-identifier-naming)
    inline static constexpr uint8_t variant_npos_u8 =
        static_cast<uint8_t>(std::variant_npos);

    // Can have at most 254 alternatives due to disk encoding using a uint8_t
    static_assert(std::variant_size_v<variant_t> < variant_npos_u8);

    inline static void save(i_obuffer& os, const variant_t& val,
                            serializer_ctx* ctx)
    {
        const size_t variant_idx = val.index();
        if (variant_idx == std::variant_npos)
            std::abort();

        const uint8_t variant_idx_u8 = static_cast<uint8_t>(variant_idx);
        os.write_u8(variant_idx_u8);

        save_impl<0, Ts...>(os, val, ctx);
    }

    inline static variant_t load(i_ibuffer& is, serializer_ctx* ctx)
    {
        variant_t val;

        const uint8_t variant_idx_u8 = is.read_u8();
        if (variant_idx_u8 == variant_npos_u8)
            std::abort();

        load_impl<0, Ts...>(is, val, ctx, static_cast<size_t>(variant_idx_u8));

        return val;
    }

    inline static variant_t* load_ptr(i_ibuffer& is, serializer_ctx* ctx)
    {
        variant_t* val = new variant_t();

        const uint8_t variant_idx_u8 = is.read_u8();
        if (variant_idx_u8 == variant_npos_u8)
            std::abort();

        load_impl<0, Ts...>(is, *val, ctx, static_cast<size_t>(variant_idx_u8));

        return val;
    }

    template <size_t variant_idx, class T_First>
    inline static void save_impl(i_obuffer& os, const variant_t& val,
                                 serializer_ctx* ctx)
    {
        static_assert(
            std::is_same_v<T_First, std::variant_alternative_t<
                                        variant_idx, variant_t>>);

        // NOLINTNEXTLINE(readability-identifier-naming)
        using T_First_NoCV = std::remove_cv_t<T_First>;

        if (val.index() == variant_idx)
            serializer<T_First_NoCV>::save(os, std::get<variant_idx>(val), ctx);
        else
            std::abort();
    }

    template <size_t variant_idx, class T_First, class T_Second,
              class... T_Rest>
    inline static void save_impl(i_obuffer& os, const variant_t& val,
                                 serializer_ctx* ctx)
    {
        static_assert(
            std::is_same_v<T_First, std::variant_alternative_t<
                                        variant_idx, variant_t>>);

        // NOLINTNEXTLINE(readability-identifier-naming)
        using T_First_NoCV = std::remove_cv_t<T_First>;

        if (val.index() == variant_idx)
            serializer<T_First_NoCV>::save(os, std::get<variant_idx>(val), ctx);
        else
            save_impl<variant_idx + 1, T_Second, T_Rest...>(os, val, ctx);
    }

    template <size_t variant_idx, class T_First>
    inline static void load_impl(i_ibuffer& is, variant_t& val,
                                 serializer_ctx* ctx, size_t disk_index)
    {
        static_assert(
            std::is_same_v<T_First, std::variant_alternative_t<
                                        variant_idx, variant_t>>);

        // NOLINTNEXTLINE(readability-identifier-naming)
        using T_First_NoCV = std::remove_cv_t<T_First>;

        if (disk_index == variant_idx)
            val.template emplace<variant_idx>(
                serializer<T_First_NoCV>::load(is, ctx));
        else
            std::abort();
    }

    template <size_t variant_idx, class T_First, class T_Second,
              class... T_Rest>
    inline static void load_impl(i_ibuffer& is, variant_t& val,
                                 serializer_ctx* ctx, size_t disk_index)
    {
        static_assert(
            std::is_same_v<T_First, std::variant_alternative_t<
                                        variant_idx, variant_t>>);

        // NOLINTNEXTLINE(readability-identifier-naming)
        using T_First_NoCV = std::remove_cv_t<T_First>;

        if (disk_index == variant_idx)
            val.template emplace<variant_idx>(
                serializer<T_First_NoCV>::load(is, ctx));
        else
            load_impl<variant_idx + 1, T_Second, T_Rest...>(is, val, ctx,
                                                            disk_index);
    }
};
