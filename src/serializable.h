#pragma once
#include <type_traits>
#include "iobuffer.h"
#include "throw_assert.h"
#include "type_table.h"

/*
    TODO: figure out when assert or throw is the correct choice

    These types deal with I/O, so assert should be used when a condition
    should be impossible. Use THROW_ASSERT if an I/O error could cause the error
*/

class serializable;
class i_dynamic_serializer;

////////////////////////////////////////////////// type traits
// TODO remove this
// NOLINTBEGIN(readability-identifier-naming)

//////////////////////////////////////// has_save_impl<T>
template <class T, class Enable = void>
struct has_save_impl
{
    static constexpr bool value = false;
};

template <class T>
struct has_save_impl<T,
    std::enable_if_t<
        std::is_same_v<
            decltype(&T::save_impl),
            void (T::*)(obuffer&) const
        >,
        void
    >
>
{
    static constexpr bool value = true;
};

template <class T>
static constexpr bool has_save_impl_v = has_save_impl<T>::value;

//////////////////////////////////////// has_load_impl<T>
template <class T, class Enable = void>
struct has_load_impl
{
    static constexpr bool value = false;
};

template <class T>
struct has_load_impl<T,
    std::enable_if_t<
        std::is_same_v<
            decltype(&T::load_impl),
            T* (*)(ibuffer&)
        >,
        void
    >
>
{
    static constexpr bool value = true;
};

template <class T>
static constexpr bool has_load_impl_v = has_load_impl<T>::value;

// NOLINTEND(readability-identifier-naming)
//////////////////////////////////////////////////



////////////////////////////////////////////////// serializable
class serializable: public i_type_table
{
public:
    virtual ~serializable()
    {
    }

    serializer_id_t get_dynamic_serializer_id() const;

private:
};

inline serializer_id_t serializable::get_dynamic_serializer_id() const
{
    type_table_t* table = type_table();
    const serializer_id_t& sid = table->serializer_id();
    THROW_ASSERT(sid != 0);
    return sid;
}

template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
serializer_id_t __get_dynamic_serializer_id_impl()
{
    static_assert(std::is_base_of_v<serializable, T>);

    type_table_t* table = type_table<T>();
    serializer_id_t sid = table->serializer_id();
    THROW_ASSERT(sid != 0);
    return sid;
}

template <class T>
inline serializer_id_t get_dynamic_serializer_id()
{
    static_assert(std::is_base_of_v<serializable, T>);

    static const serializer_id_t SID = __get_dynamic_serializer_id_impl<T>();
    return SID;
}

////////////////////////////////////////////////// i_dynamic_serializer
class i_dynamic_serializer
{
public:
    virtual ~i_dynamic_serializer()
    {
    }

    /*
        TODO: These don't both need to be called through the dynamic_serializer,
        but I leave it this way for now, for consistency/simplicity.
        Optimize/tweak this later
    */

    virtual void call_save(obuffer&, const serializable*) const = 0;
    virtual serializable* call_load(ibuffer&) const = 0;
};

template <class T>
class dynamic_serializer: public i_dynamic_serializer
{
    static_assert(std::is_base_of_v<serializable, T>);
    static_assert(has_save_impl_v<T>);
    static_assert(has_load_impl_v<T>);

    dynamic_serializer(serializer_id_t sid): _sid(sid)
    {
        THROW_ASSERT(get_dynamic_serializer_id<T>() == sid);
    }

    void call_save(obuffer& os, const serializable* obj) const override
    {
        assert(obj->get_dynamic_serializer_id() == _sid);

        const T* obj_casted = static_cast<const T*>(obj);
        assert(dynamic_cast<const T*>(obj) == obj_casted);

        obj_casted->save_impl(os);
    }

    serializable* call_load(ibuffer& is) const override
    {
        serializable* obj = T::load_impl(is);
        assert(obj->get_dynamic_serializer_id() == _sid);
        return obj;
    }

private:
    const serializer_id_t _sid;
};
