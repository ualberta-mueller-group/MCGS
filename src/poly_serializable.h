/*
    Unused code for serializing/deserializing polymorphic types. May be used in
    the future to store games in the database.

    TODO: figure out when assert or throw is the correct choice

    These types deal with I/O, so assert should be used when a condition
    should be impossible. Use THROW_ASSERT if an I/O error could cause the error

    Also fix NOLINTs
*/
#pragma once
#include <type_traits>
#include <cassert>

#include "iobuffer.h"
#include "serializer.h"
#include "throw_assert.h"
#include "type_table.h"

////////////////////////////////////////////////// declarations
class poly_serializable;

/*
    "Load" function pointer typedef

    i.e. function pointer to:
    static poly_serializable* clobber_1xn::load_impl(i_ibuffer&, serializer_ctx*)
*/
typedef poly_serializable* (*load_fn_ptr_t)(i_ibuffer&, serializer_ctx*);

////////////////////////////////////////////////// type traits
// NOLINTBEGIN(readability-identifier-naming)
// clang-format off

//////////////////////////////////////// has_save_impl<T>
template <class T, class Enable = void>
struct has_save_impl
{
    inline static constexpr bool value = false;
};

template <class T>
struct has_save_impl<T,
    std::enable_if_t<
        std::is_same_v<
            decltype(&T::save_impl),
            void (T::*)(i_obuffer&, serializer_ctx*) const
        >,
        void
    >
>
{
    inline static constexpr bool value = true;
};

template <class T>
inline static constexpr bool has_save_impl_v = has_save_impl<T>::value;

//////////////////////////////////////// has_load_impl<T>
template <class T, class Enable = void>
struct has_load_impl
{
    inline static constexpr bool value = false;
};

template <class T>
struct has_load_impl<T,
    std::enable_if_t<
        std::is_same_v<
            decltype(&T::load_impl),
            load_fn_ptr_t
        >,
        void
    >
>
{
    inline static constexpr bool value = true;
};

template <class T>
inline static constexpr bool has_load_impl_v = has_load_impl<T>::value;

// clang-format on
// NOLINTEND(readability-identifier-naming)

////////////////////////////////////////////////// class poly_serializable
class poly_serializable : public i_type_table
{
public:
    virtual ~poly_serializable() {}

    poly_serializable_id_t poly_serializable_id() const;

    virtual void save_impl(i_obuffer&, serializer_ctx*) const { THROW_ASSERT(false); }

private:
};

////////////////////////////////////////////////// poly_serializable methods
inline poly_serializable_id_t poly_serializable::poly_serializable_id() const
{
    const poly_serializable_id_t sid = type_table()->poly_serializable_id();
    THROW_ASSERT(sid != 0);
    return sid;
}

////////////////////////////////////////////////// implementation details
template <class T> // NOLINTNEXTLINE(readability-identifier-naming)
poly_serializable_id_t __get_poly_serializable_id_impl()
{
    static_assert(std::is_base_of_v<poly_serializable, T>);
    static_assert(!std::is_abstract_v<T>);
    static_assert(has_save_impl_v<T> && has_load_impl_v<T>);

    poly_serializable_id_t sid = type_table<T>()->poly_serializable_id();
    THROW_ASSERT(sid != 0);
    return sid;
}

// NOLINTNEXTLINE(readability-identifier-naming)
namespace __poly_serializable_impl {

void register_impl(type_table_t* type_table, load_fn_ptr_t load_function);
load_fn_ptr_t get_load_function(poly_serializable_id_t sid);

} // namespace __poly_serializable_impl

////////////////////////////////////////////////// poly_serializable_id<T>()
template <class T>
inline poly_serializable_id_t get_poly_serializable_id()
{
    static_assert(std::is_base_of_v<poly_serializable, T>);
    static_assert(!std::is_abstract_v<T>);
    static_assert(has_save_impl_v<T> && has_load_impl_v<T>);

    static const poly_serializable_id_t SID =
        __get_poly_serializable_id_impl<T>();
    return SID;
}

//////////////////////////////////////////////////
/// register_poly_serializable<T>()
template <class T>
void register_poly_serializable()
{
    static_assert(std::is_base_of_v<poly_serializable, T>);
    static_assert(!std::is_abstract_v<T>);
    static_assert(has_save_impl_v<T> && has_load_impl_v<T>);
    __poly_serializable_impl::register_impl(type_table<T>(), &T::load_impl);
}

////////////////////////////////////////////////// serializer<poly_serializable*>
/// and derived pointers
template <class T>
struct serializer<
    T*, std::enable_if_t<std::is_base_of_v<poly_serializable, T>, void>>
{
    static void save(i_obuffer& os, const poly_serializable* obj, serializer_ctx* ctx)
    {
        poly_serializable_id_t sid = obj->poly_serializable_id();
        THROW_ASSERT(sid > 0);
        os.write_u32(sid);

        obj->save_impl(os, ctx);
    }

    static T* load(i_ibuffer& is, serializer_ctx* ctx)
    {
        poly_serializable_id_t sid = is.read_u32();

        load_fn_ptr_t load_fn = __poly_serializable_impl::get_load_function(sid);
        assert(load_fn != nullptr);

        poly_serializable* obj = load_fn(is, ctx);

        if constexpr (!std::is_same_v<poly_serializable, T>)
        {
            T* obj_casted = dynamic_cast<T*>(obj);
            THROW_ASSERT(obj_casted != nullptr);
            return obj_casted;
        }
        else
            return obj;
    }
};
