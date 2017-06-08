/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/sugar/buffer_t.hpp"

#include <iosfwd>

#include <limits>
#include <utility>
#include <functional> // plus, bit_and, etc
#include <tuple> // ignore_t
#include <cstdint>

// (standalone version): https://github.com/edouarda/brigand
#define BRIGAND_NO_BOOST_SUPPORT
#include <brigand/brigand.hpp>

// https://github.com/jonathanpoelen/brigand/blob/ext_call/brigand/functions/lambda/call.hpp
namespace brigand
{
namespace detail
{
    template<template<class...> class F>
    struct call_
    {};

    template <template<class...> class F, class... Args>
    struct apply<call_<F>, brigand::list<Args...>>
    {
        using type = F<Args...>;
    };

    template<template<class> class F>
    bind<F, _1> get_call_impl(call_<F>, int);

    template<template<class, class> class F>
    bind<F, _1, _2> get_call_impl(call_<F>, int);

    template<template<class, class, class> class F>
    bind<F, _1, _2, _3> get_call_impl(call_<F>, int);

    template<class F>
    F get_call_impl(F, char);
}

    template<template<class...> class F>
    using call = decltype(detail::get_call_impl(detail::call_<F>{}, 1));

    template<class L, class P>
    using copy_if = remove_if<L, bind<not_, P>>;
}

// brigand::split_if
namespace brigand
{
namespace detail
{
    template<class L>
    struct splitted_list
    { using type = list<L>; };

    template<>
    struct splitted_list<list<>>
    { using type = list<>; };

    template<class Seq, class Pred, std::size_t d = 0, class I = index_if<Seq, Pred>>
    struct split_if_impl
    {
        using splitted = split_at<Seq, size_t<(I::value + d)>>;
        using right = front<pop_front<splitted>>;
        using type = brigand::append<
            typename splitted_list<front<splitted>>::type,
            typename split_if_impl<
                right,
                Pred, 1,
                index_if<pop_front<right>, Pred>
            >::type
        >;
    };

    template<class Seq, class Pred, std::size_t d>
    struct split_if_impl<Seq, Pred, d, no_such_type_>
    { using type = list<Seq>; };

    template<template<class...> class L, class Pred, std::size_t d>
    struct split_if_impl<L<>, Pred, d, no_such_type_>
    { using type = list<>; };
}

    namespace lazy {
        template<class L, class Pred>
        using split_if = detail::split_if_impl<L, Pred>;
    }

    template<class L, class Pred>
    using split_if = typename detail::split_if_impl<L, Pred>::type;
}

// brigand::unique
namespace brigand
{
namespace detail
{
    template<class L, class T>
    using push_back_if_unique = std::conditional_t<
        brigand::none<L, brigand::bind<std::is_same, brigand::_1, brigand::pin<T>>>::value,
        brigand::push_back<L, T>,
        L
    >;
}
    template<class L>
    using unique = brigand::fold<
        L,
        brigand::list<>,
        brigand::call<detail::push_back_if_unique>
    >;
}

namespace cexpr
{
    template<class T, class F>
    void cifv(std::true_type, T && v, F && f)
    { f(std::forward<T>(v)); }

    template<class T, class F>
    void cifv(std::false_type, T &&, F &&)
    {}

    template<class T, class True, class False>
    constexpr decltype(auto)
    cifv(std::true_type, T && v, True && f, False &&)
    { return f(std::forward<T>(v)); }

    template<class T, class True, class False>
    constexpr decltype(auto)
    cifv(std::false_type, T && v, True &&, False && f)
    { return f(std::forward<T>(v)); }

    template<class True, class False>
    constexpr True select(std::true_type, True && r, False &&)
    { return std::forward<True>(r); }

    template<class True, class False>
    constexpr False select(std::false_type, True &&, False && r)
    { return std::forward<False>(r); }
}

namespace meta
{
    namespace detail
    {
        template<template<class...> class Tpl, class T>
        struct is_layout_impl
        : std::false_type
        {};

        template<template<class...> class Tpl, class... Ts>
        struct is_layout_impl<Tpl, Tpl<Ts...>>
        : std::true_type
        {};


        template<class I, template<I> class Tpl, class T>
        struct is_integral_layout_impl
        : std::false_type
        {};

        template<class I, template<I> class Tpl, I i>
        struct is_integral_layout_impl<I, Tpl, Tpl<i>>
        : std::true_type
        {};


        template<template<class T, T> class Tpl, class T>
        struct is_integral_constant_layout_impl
        : std::false_type
        {};

        template<template<class T, T> class Tpl, class T, T v>
        struct is_integral_constant_layout_impl<Tpl, Tpl<T, v>>
        : std::true_type
        {};
    }

    template<template<class...> class Tpl, class T>
    using is_layout = typename detail::is_layout_impl<Tpl, T>::type;

    template<class I, template<I> class Tpl, class T>
    using is_integral_layout = typename detail::is_integral_layout_impl<I, Tpl, T>::type;

    template<template<class T, T> class Tpl, class T>
    using is_integral_constant_layout = typename detail::is_integral_constant_layout_impl<Tpl, T>::type;


    template<class...> using void_t = void;
}

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/byte.hpp"


#define PROTO_VAR(t, v)           \
    constexpr struct v            \
    : ::proto::var<v, t> {        \
      using var<v, t>::operator=; \
    } v {}

namespace proto
{
    template<std::size_t N>
    using static_size = std::integral_constant<std::size_t, N>;

    template<std::size_t n>
    struct limited_size { static const std::size_t value = n; };

    struct dyn_size {};

    template<class T>
    using is_static_size = meta::is_integral_constant_layout<std::integral_constant, T>;

    template<class T>
    using is_limited_size = meta::is_integral_layout<std::size_t, limited_size, T>;

    template<class T>
    using is_dynamic_size = typename std::is_same<T, dyn_size>::type;

    namespace tags
    {
        class static_buffer {};
        class view_buffer {};
        class limited_buffer {};
    }

    template<class Desc> using sizeof_ = typename Desc::sizeof_;

    namespace detail
    {
        template<class T> struct sizeof_to_buffer_cat;
        template<std::size_t n> struct sizeof_to_buffer_cat<static_size<n>> { using type = tags::static_buffer; };
        template<std::size_t n> struct sizeof_to_buffer_cat<limited_size<n>> { using type = tags::limited_buffer; };

        template<class T, class = void>
        struct buffer_category_impl : sizeof_to_buffer_cat<sizeof_<T>> {};

        template<class T>
        struct buffer_category_impl<T, meta::void_t<typename T::buffer_category>>
        { using type = typename T::buffer_category; };
    }

    template<class Desc> using buffer_category = typename detail::buffer_category_impl<Desc>::type;

    template<class Desc> using has_static_buffer
      = typename std::is_same<tags::static_buffer, buffer_category<Desc>>::type;
    template<class Desc> using has_limited_buffer
      = typename std::is_same<tags::limited_buffer, buffer_category<Desc>>::type;
    template<class Desc> using has_view_buffer
      = typename std::is_same<tags::view_buffer, buffer_category<Desc>>::type;

    namespace detail
    {
        template<class T, class U> struct common_size_impl;
        template<class T, class U> struct common_buffer_impl;
    }

    template<class T, class U> using common_size = typename detail::common_size_impl<T, U>::type;
    template<class T, class U> using common_buffer = typename detail::common_buffer_impl<T, U>::type;

    template<class T>
    using t_ = typename T::type;

    template<class... Ts, class F>
    void for_each(brigand::list<Ts...>, F && f)
    {
        (void)std::initializer_list<int>{
            ((void)(f(Ts{})), 1)...
        };
    }


    // clang narrowing checker with std::intgral_constant... (cf: safe_int<T> = T)
    template<class T>
    struct safe_int
    {
        T val;

        template<class U> constexpr safe_int(U x) noexcept : val{x} {}
        template<class U, U v> constexpr safe_int(std::integral_constant<U, v>) noexcept : val{v} {}

        constexpr operator T () const { return val; }
        constexpr operator T & () { return val; }
    };

    namespace types
    {
        template<class Desc, typename Desc::type val>
        struct static_value
        {
            using type = typename Desc::type;
            using sizeof_ = proto::sizeof_<Desc>;
            using buffer_category = proto::buffer_category<Desc>;

            std::size_t static_serialize(uint8_t * p) const
            {
                return Desc{val}.static_serialize(p);
            }

            std::size_t limited_serialize(uint8_t * p) const
            {
                return Desc{val}.limited_serialize(p);
            }
        };

        class le_tag {};
        class be_tag {};

        /**
        * fixed width integer types
        * @{
        */
        template<class T, class Endianess>
        struct integer
        {
            using type = T;
            using sizeof_ = static_size<sizeof(T)>;

            static_assert(std::is_integral<T>::value, "");

            safe_int<type> val;

            sizeof_ static_serialize(uint8_t * p) const
            {
                /**///std::cout << " [static_buffer] [sizeof_: " << sizeof(T) << "] {" << static_cast<void*>(p) << "}";
                using rng = brigand::range<std::size_t, 0, sizeof(T)>;
                using is_little_endian = t_<std::is_same<Endianess, le_tag>>;
                for_each(rng{}, [&p, this](auto i) {
                    // TODO std::make_unsigned
                    /**///std::cout << " { *p++ }";
                    *p++ = this->val >> ((is_little_endian{} ? i : sizeof(T)-1-i) * 8);
                });
                return sizeof_{};
            }
        };

        template<class E, class ProtoType>
        struct enum_ : ProtoType
        {
            static_assert(std::is_enum<E>::value, "");

            using type = E;

            constexpr enum_(E e) noexcept : ProtoType{typename ProtoType::type(e)} {}
        };

        using s8 = integer<int8_t, void>;
        using u8 = integer<uint8_t, void>;

        using s16_be = integer<int16_t, be_tag>;
        using s16_le = integer<int16_t, le_tag>;
        using u16_be = integer<uint16_t, be_tag>;
        using u16_le = integer<uint16_t, le_tag>;

        using s32_be = integer<int32_t, be_tag>;
        using s32_le = integer<int32_t, le_tag>;
        using u32_be = integer<uint32_t, be_tag>;
        using u32_le = integer<uint32_t, le_tag>;

        using s64_be = integer<int64_t, be_tag>;
        using s64_le = integer<int64_t, le_tag>;
        using u64_be = integer<uint64_t, be_tag>;
        using u64_le = integer<uint64_t, le_tag>;

        template<class E> using enum_s8 = enum_<E, s8>;
        template<class E> using enum_u8 = enum_<E, u8>;

        template<class E> using enum_s16_be = enum_<E, s16_be>;
        template<class E> using enum_s16_le = enum_<E, s16_le>;
        template<class E> using enum_u16_be = enum_<E, u16_be>;
        template<class E> using enum_u16_le = enum_<E, u16_le>;

        template<class E> using enum_s32_be = enum_<E, s32_be>;
        template<class E> using enum_s32_le = enum_<E, s32_le>;
        template<class E> using enum_u32_be = enum_<E, u32_be>;
        template<class E> using enum_u32_le = enum_<E, u32_le>;

        template<class E> using enum_s64_be = enum_<E, s64_be>;
        template<class E> using enum_s64_le = enum_<E, s64_le>;
        template<class E> using enum_u64_be = enum_<E, u64_be>;
        template<class E> using enum_u64_le = enum_<E, u64_le>;
        /** @} */

        /**
        * @{
        */
        struct u16_encoding
        {
            using type = uint16_t;
            using sizeof_ = limited_size<sizeof(type)>;

            safe_int<type> val;

            std::size_t limited_serialize(uint8_t * p) const
            {
                assert(!(val & 0x8000));
                return (val <= 127)
                    ? u8{uint8_t(val)}.static_serialize(p)
                    : u16_be{uint16_t(val|0x8000)}.static_serialize(p);
            }

            constexpr std::size_t reserved_size() const
            {
                // val + limited_serialize()
                return (val < 127) ? 1u : 2u;
            }

            constexpr std::size_t real_size() const
            {
                return (val <= 127) ? 1 : 2;
            }

            template<class U, U v>
            constexpr static
            std::conditional_t<(v < 127), u8, u16_be>
            make(std::integral_constant<U, v> x)
            { return {x}; }
        };

        struct u16_encoding_force_u16
        {
            using type = uint16_t;
            using sizeof_ = static_size<sizeof(type)>;

            safe_int<type> val;

            sizeof_ static_serialize(uint8_t * p) const
            {
                assert(!(val & 0x8000));
                return u16_be{uint16_t(val|0x8000)}.static_serialize(p);
            }
        };

        template<class E> using enum_u16_encoding = enum_<E, u16_encoding>;

        struct u32_encoding
        {
            using type = uint32_t;
            using sizeof_ = limited_size<sizeof(type)>;

            safe_int<type> val;

            std::size_t limited_serialize(uint8_t * p) const
            {
                assert(!(val & 0xC0000000));
                auto serial = [&p](uint8_t v) { return u8{v}.static_serialize(p++); };
                return (val <= 0x3FFF)
                ?
                    (val <= 0x3F)
                    ?
                        serial(        val       )
                    :
                        serial(0x40 | (val >> 8 )) +
                        serial(        val       )
                :
                    (val <= 0x3FFFFF)
                    ?
                        serial(0x80 | (val >> 16)) +
                        serial(        val >> 8  ) +
                        serial(        val       )
                    :
                        serial(0xC0 | (val >> 24)) +
                        serial(       (val >> 16)) +
                        serial(        val >> 8  ) +
                        serial(        val       )
                ;
            }

            // constexpr std::size_t reserved_size() const;
            // constexpr static std::size_t make() const;

//             template<class U, U v>
//             constexpr static
//             std::conditional_t<(v <= 0x3FFF),
//                 std::conditional_t<(v <= 0x3F), u8, u16_be>,
//                 std::conditional_t<(v <= 0x3FFFFF), u24_be, u32_be>
//             >
//             make(std::integral_constant<U, v> x)
//             { return {x}; }
        };

        template<class E>
        using enum_u16_encoding = enum_<E, u16_encoding>;

        template<class E>
        using enum_u32_encoding = enum_<E, u32_encoding>;
        /** @} */

        struct bytes
        {
            using type = const_buffer_t;
            using sizeof_ = dyn_size;
            using buffer_category = tags::view_buffer;

            type av;

            constexpr bytes(bytes &&) = default;
            constexpr bytes(bytes const &) = default;

            constexpr bytes(const_buffer_t b) noexcept
            : av(b)
            {}

            template<class T>
            constexpr bytes(T && a) noexcept
            : av(a)
            {}

            template<class T, class U>
            constexpr bytes(T && a, U && sz) noexcept
            : av(a, sz)
            {}

            array_view_const_u8 get_view_buffer() const
            {
                /**///std::cout << " [view_buffer] [size: " << av.size() << "]";
                return this->av;
            }
        };

        template<class ProtoType, std::size_t N>
        struct array
        {
            using type = typename ProtoType::type const (&)[N];
            using sizeof_ = static_size<ProtoType::sizeof_::value * N>;

            type av;

            constexpr array(array &&) = default;
            constexpr array(array const &) = default;

            constexpr array(type av) noexcept
            : av(av)
            {}

            sizeof_ static_serialize(uint8_t * p) const
            {
                for (auto const & v : av) {
                    p += ProtoType{v}.static_serialize(p);
                }
                return {};
            }

            static sizeof_ size()
            {
                return {};
            }
        };

        template<std::size_t N>
        struct pad
        {
            using sizeof_ = static_size<N>;

            sizeof_ static_serialize(uint8_t * p) const
            {
                return {};
            }
        };

        template<std::size_t N> using array_u8 = array<u8, N>;

        template<class T>
        struct value
        {
            using sizeof_ = void;
            using buffer_category = void;

            T val;
        };
    }


    template<class... Ts>
    struct inherits : Ts...
    {
        template<class... Us>
        constexpr inherits(Us && ... v)
        : Ts{std::forward<Us>(v)}...
        {}
    };


    namespace detail
    {
        template<std::size_t n>
        struct common_size_impl<static_size<n>, static_size<n>> { using type = static_size<n>; };

        template<std::size_t n1, std::size_t n2>
        struct common_size_impl<static_size<n1>, static_size<n2>> { using type = limited_size<std::max(n1, n2)>; };

        template<std::size_t n1, std::size_t n2>
        struct common_size_impl<limited_size<n1>, limited_size<n2>> { using type = limited_size<std::max(n1, n2)>; };
        template<std::size_t n1, std::size_t n2>
        struct common_size_impl<static_size<n1>, limited_size<n2>> { using type = limited_size<std::max(n1, n2)>; };
        template<std::size_t n1, std::size_t n2>
        struct common_size_impl<limited_size<n1>, static_size<n2>> { using type = limited_size<std::max(n1, n2)>; };

        template<class T> struct common_size_impl<T, dyn_size> { using type = dyn_size; };
        template<class U> struct common_size_impl<dyn_size, U> { using type = dyn_size; };
        template<> struct common_size_impl<dyn_size, dyn_size> { using type = dyn_size; };


        template<class T, class U> struct common_buffer_impl;
        template<class T> struct common_buffer_impl<T, T> { using type = T; };
        template<> struct common_buffer_impl<tags::static_buffer, tags::limited_buffer>
        { using type = tags::limited_buffer ; };
        template<> struct common_buffer_impl<tags::limited_buffer, tags::static_buffer>
        { using type = tags::limited_buffer ; };
    }

    namespace detail
    {
        template<class T, class = void>
        struct get_dependencies_impl
        { using type = brigand::list<T>; };

        template<>
        struct get_dependencies_impl<void, void>
        { using type = brigand::list<>; };

        template<class... T>
        struct get_dependencies_impl<brigand::list<T...>, void>
        { using type = brigand::list<T...>; };

        template<class T>
        struct get_dependencies_impl<T, meta::void_t<typename T::dependencies>>
        { using type = typename T::dependencies; };
    }
    template<class T>
    using get_dependencies = typename detail::get_dependencies_impl<T>::type;

    template<class Deps, class... ReDeps>
    using get_dependencies_if_void = std::conditional_t<
        std::is_void<Deps>::value,
        brigand::append<get_dependencies<ReDeps>...>,
        get_dependencies<Deps>
    >;

    namespace detail
    {
        template<template<class> class Tpl, class T, class R, class = void>
        struct get_or_impl
        { using type = R; };

        template<template<class> class Tpl, class T, class R>
        struct get_or_impl<Tpl, T, R, meta::void_t<Tpl<T>>>
        { using type = Tpl<T>; };
    }

    template<template<class> class Tpl, class T, class R = T>
    using get_or_t = typename detail::get_or_impl<Tpl, T, R>::type;

    template<class T>
    using desc_type_t = typename T::desc_type;

    template<class T>
    using value_type_t = typename T::value_type;

    template<class T>
    using desc_or_t = get_or_t<desc_type_t, T>;

    namespace detail
    {
        template<class Desc, class = void> struct is_reserializer_impl : std::false_type {};
        template<class Desc> struct is_reserializer_impl<Desc, meta::void_t<typename Desc::is_reserializer>>
        : brigand::bool_<Desc::is_reserializer::value> {};
    }
    template<class Desc>
    using is_reserializer = typename detail::is_reserializer_impl<Desc>::type;

    // TODO desc_type
    template<class Sizeof, class BufCat, class IsReserializer>
    struct desc_type
    {
        using sizeof_ = Sizeof;
        using buffer_category = BufCat;
        using is_reserializer = IsReserializer;
    };

    using nodesc = desc_type<void, void, std::false_type>;

    template<class Desc>
    using desc_traits_t = desc_type<
        proto::sizeof_<Desc>,
        proto::buffer_category<Desc>,
        proto::is_reserializer<Desc>
    >;

    // TODO val = val_impl<..., minimal_desc_def<Desc>>, with minimal_desc_def<u8> == minimal_desc_def<s8>, traits?
    template<class Deps, class Value, class Desc = desc_or_t<Value>>
    struct val
    {
        using deps_type = Deps;
        using dependencies = get_dependencies<Deps>;
        using value_type = Value;
        using desc_type = desc_traits_t<Desc>;

        Value desc;

        template<class Params>
        constexpr val
        to_proto_value(Params const &) const
        { return *this; }
    };

    template<class T>
    using is_integral_or_enum = brigand::bool_<(std::is_integral<T>{} or std::is_enum<T>{})>;

    template<class T>
    using is_integral_constant = meta::is_integral_constant_layout<std::integral_constant, T>;

    template<class T>
    using is_integral_or_enum_or_constant = brigand::bool_<is_integral_or_enum<T>{} or is_integral_constant<T>{}>;

    template<class T, class R = void>
    struct enable_is_integral_or_enum
    : std::enable_if<std::is_integral<T>{} or std::is_enum<T>{}, R>
    {};

    template<class T, class R = void>
    struct disable_is_integral_or_enum
    : std::enable_if<!std::is_integral<T>{} and !std::is_enum<T>{}, R>
    {};

    template<class T, class R = void>
    using enable_is_integral_or_enum_t = typename enable_is_integral_or_enum<T, R>::type;

    template<class T, class R = void>
    using disable_is_integral_or_enum_t = typename enable_is_integral_or_enum<T, R>::type;


    template<class T>
    struct safe_cast_impl
    {
        static_assert(is_integral_or_enum<T>{} or is_integral_constant<T>{}, "");
        T value_;
    };

    template<class T>
    constexpr safe_cast_impl<T>
    cast(T v)
    { return {v}; }

    template<class T>
    struct unsafe_cast_impl
    {
        static_assert(is_integral_or_enum<T>{}, "");
        T value_;
    };

    template<class T>
    constexpr unsafe_cast_impl<T>
    unsafe_cast(T v)
    { return {v}; }

    template<class T, T v>
    constexpr safe_cast_impl<std::integral_constant<T, v>>
    unsafe_cast(std::integral_constant<T, v> c)
    { return {c}; }


    namespace detail
    {
        template<class T, bool = std::is_enum<T>::value>
        struct integral_type_impl
        {
            using type = T;
        };

        template<class T>
        struct integral_type_impl<T, true>
        {
            using type = std::underlying_type_t<T>;
        };
    }

    template<class T>
    using integral_type = typename detail::integral_type_impl<T>::type;

    namespace detail
    {
        template<class T, class Desc, class = void, bool = std::is_enum<T>::value>
        struct is_enum_to_int
        : std::false_type
        {};

        template<class T, class Desc>
        struct is_enum_to_int<T, Desc, meta::void_t<typename Desc::type>, true>
        : std::is_integral<typename Desc::type>
        {};
    }

    template<class T, class Desc>
    using is_enum_to_int = typename detail::is_enum_to_int<std::decay_t<T>, Desc>::type;

    namespace detail
    {
        // Dep for more readable errors
        template<class Dep, class Desc, class T>
        constexpr auto make_val(T && x, int)
        -> decltype(Desc::make(std::forward<T>(x)))
        { return Desc::make(std::forward<T>(x)); }

        template<class Dep, class Desc, class T>
        constexpr auto make_val(T && x, char)
        { return Desc{std::forward<T>(x)}; }
    }

    // NOTE std::optional
    template<class T>
    struct maybe_impl
    {
        bool enable;
        T value;
    };

    template<class T>
    maybe_impl<std::remove_reference_t<T>>
    maybe(bool enable, T && value)
    {
        return {enable, std::forward<T>(value)};
    }

    using ignore_t = decltype(std::ignore);

    template<class Dep, class Desc>
    struct var
    {
        using dependencies = brigand::list<Dep>;
        using desc_type = Desc;
        using arguments = brigand::list<Dep>;

        template<class U>
        constexpr auto operator = (U && v) const
        { return this->lax_impl(is_enum_to_int<U, desc_type>{}, std::forward<U>(v)); }

        template<class U>
        constexpr auto operator = (ignore_t) const
        { return val<Dep, ignore_t, nodesc>{}; }

        template<class U>
        constexpr auto operator = (maybe_impl<U> && o) const = delete; // TODO unimplemented

        template<class U>
        constexpr auto operator = (maybe_impl<U> const & o) const = delete; // TODO unimplemented

        template<class U>
        constexpr auto operator = (unsafe_cast_impl<U> o) const
        { return impl(static_cast<typename desc_type::type>(o.value_)); }

        template<class U>
        constexpr auto operator = (safe_cast_impl<U> o) const
        {
            using type = typename desc_type::type;
            using scr = integral_type<U>;
            using dst = integral_type<type>;
            return impl(static_cast<type>(checked_cast<dst>(static_cast<scr>(o.value_))));
        }

        template<class U, U v>
        constexpr auto operator = (safe_cast_impl<std::integral_constant<U, v>>) const
        {
            using type = typename desc_type::type;
            using scr = integral_type<U>;
            using dst = integral_type<type>;
            return impl(static_cast<type>(dst{static_cast<scr>(v)}));
        }

        template<class Params>
        static decltype(auto) to_proto_value(Params params) noexcept
        { return params.template get_proto_value<Dep>(); }

    private:
        template<class U>
        constexpr auto lax_impl(std::true_type, U v) const
        { return *this = unsafe_cast_impl<U>{v}; }

        template<class U>
        static constexpr auto lax_impl(std::false_type, U && v)
        { return impl(std::forward<U>(v)); }

        template<class U>
        static constexpr auto impl(U && x)
        -> val<Dep, decltype(detail::make_val<Dep, Desc>(std::forward<U>(x), 1))>
        { return {detail::make_val<Dep, Desc>(std::forward<U>(x), 1)}; }
    };

    template<class T, class = void> struct check;
    template<class T> struct check<T, std::enable_if_t<T::value>>
    : std::true_type
    {};


    template<class T> using deps_type_t = typename T::deps_type;

    namespace cexp
    {
        constexpr std::size_t strlen(char const * s)
        {
            std::size_t n = 0;
            while (*s) {
                ++n;
                s++;
            }
            return n;
        }

        constexpr std::size_t strcpy(char * dst, char const * src)
        {
            char * p = dst;
            while (*src) {
                *p++ = *src++;
            }
            return p - dst;
        }

        template<class T, class... Ts>
        constexpr T fold(T a, Ts... as)
        {
            (void)std::initializer_list<int>{
                (void(a += as), 1)...
            };
            return a;
        }
    }


    namespace detail
    {
        template<class T, class = void>
        struct get_arguments_impl
        { using type = brigand::list<>; };

        template<class T>
        struct get_arguments_impl<T, meta::void_t<typename T::arguments>>
        { using type = typename T::arguments; };
    }
    template<class T>
    using get_arguments = typename detail::get_arguments_impl<T>::type;


    namespace dsl
    {
        struct current_pkts_sz {};
        struct next_pkts_sz {};
    }

    template<class Sp, class Desc>
    struct special
    {
        using desc_type = Desc;
        using arguments = brigand::list<>;

        using sizeof_ = proto::sizeof_<desc_type>;
        using buffer_category = proto::buffer_category<desc_type>;
        using is_reserializer = proto::is_reserializer<desc_type>;

        struct lazy
        {
            using desc_type = Desc;
            using arguments = brigand::list<Sp>;
            using sizeof_ = typename Desc::sizeof_;

            template<class Params>
            constexpr val<Sp, Desc> to_proto_value(Params p) const
            {
                return {checked_cast<typename Desc::type>(
                    // TODO desc() -> desc
                    p.get_proto_value(Sp{}).desc()
                )};
            }

            std::size_t reserved_size() const
            {
                return sizeof_::value;
            }
        };

        template<class Params>
        constexpr val<Sp, lazy, Desc> to_proto_value(Params) const
        {
            return {};
        }
    };

    // TODO Deps, Desc
    template<class Desc>
    using next_pkts_sz = special<dsl::next_pkts_sz, Desc>;

    // TODO Deps, Desc
    template<class Desc>
    using current_pkts_sz = special<dsl::current_pkts_sz, Desc>;

    namespace types {
        using ::proto::dsl::next_pkts_sz;
        using ::proto::dsl::current_pkts_sz;
    }

    template<class Val, class T>
    using has_argument_of = brigand::any<
        get_arguments<Val>,
        brigand::bind<std::is_same, brigand::_1, brigand::pin<T>>
    >;

    template<class Desc>
    using has_next_pkts_sz = has_argument_of<Desc, dsl::next_pkts_sz>;

    template<class Desc>
    using has_current_pkts_sz = has_argument_of<Desc, dsl::current_pkts_sz>;

    template<class Desc>
    using has_pkts_sz = brigand::bool_<has_next_pkts_sz<Desc>{} or has_current_pkts_sz<Desc>{}>;

    template<class Desc>
    using is_lazy_value = brigand::bool_<is_reserializer<Desc>{} or has_pkts_sz<Desc>{}>;

    // alias on val<>
    namespace v
    {
        template<class Val>
        using has_next_pkts_sz = proto::has_next_pkts_sz<value_type_t<Val>>;

        template<class Val>
        using has_current_pkts_sz = proto::has_current_pkts_sz<value_type_t<Val>>;

        template<class Val>
        using has_pkts_sz = proto::has_pkts_sz<value_type_t<Val>>;

        template<class Val>
        using is_reserializer = proto::is_reserializer<desc_type_t<Val>>;

        template<class Val>
        using is_lazy_value = brigand::bool_<is_reserializer<Val>{} or has_pkts_sz<Val>{}>;
    }

    // alias on trait<desc_type_t<T>>
    namespace v
    {
        template<class Val>
        using has_next_pkts_sz = proto::has_next_pkts_sz<value_type_t<Val>>;

        template<class Val>
        using has_current_pkts_sz = proto::has_current_pkts_sz<value_type_t<Val>>;

        template<class Val>
        using has_pkts_sz = proto::has_pkts_sz<value_type_t<Val>>;

        template<class Val>
        using is_reserializer = proto::is_reserializer<desc_type_t<Val>>;

        template<class Val>
        using is_lazy_value = brigand::bool_<is_reserializer<Val>{} or has_pkts_sz<Val>{}>;
    }


    template<class Var, class T, class Desc>
    constexpr T get_value(val<Var, T, Desc> v)
    { return v.desc; }

    template<class T>
    constexpr T get_value(T v)
    { return v; }


    template<class Deps, class Desc, class... Vars>
    struct lazy_creator
    {
        using dependencies = get_dependencies<Deps>;
        using desc_type = Desc;
        using arguments = brigand::append<get_arguments<Vars>...>;

        using sizeof_ = proto::sizeof_<desc_type>;
        using buffer_category = proto::buffer_category<desc_type>;
        using is_reserializer = proto::is_reserializer<desc_type>;

        // TODO tuple
        inherits<Vars...> values;

        template<class Params>
        constexpr Desc
        to_proto_value(Params params) const
        {
            return Desc{get_value(static_cast<Vars const &>(this->values).to_proto_value(params))...};
        }
    };

    template<class Deps, class Desc, class... Vars>
    struct creator
    {
        using dependencies = get_dependencies<Deps>;
        using desc_type = Desc;
        using arguments = brigand::append<get_arguments<Vars>...>;

        // TODO tuple
        inherits<Vars...> values;

        template<class Params>
        constexpr auto
        to_proto_value(Params params) const
        {
            return this->to_proto_value_(
                static_cast<Vars const &>(this->values).to_proto_value(params).desc...
            );
        }

    private:
        template<class... V>
        constexpr
        std::conditional_t<
            brigand::any<brigand::list<has_pkts_sz<V>...>>::value,
            val<Deps, lazy_creator<Deps, Desc, V...>, Desc>,
            val<Deps, Desc>
        >
        to_proto_value_(V && ... values) const
        {
            return {{std::move(values)...}};
        }
    };


    template<class Desc>
    std::enable_if_t<has_static_buffer<Desc>::value, std::size_t>
    static_or_limited_serialize(uint8_t * p, Desc const & x)
    { return x.static_serialize(p); }

    template<class Desc>
    std::enable_if_t<!has_static_buffer<Desc>::value, std::size_t>
    static_or_limited_serialize(uint8_t * p, Desc const & x)
    { return x.limited_serialize(p); }


    template<class Desc, class AV, class... Sz>
    std::enable_if_t<has_static_buffer<Desc>::value, std::size_t>
    static_or_limited_reserialize(uint8_t * p, Desc const & x, AV av, Sz... sz)
    { return x.static_reserialize(p, av, sz...); }

    template<class Desc, class AV, class... Sz>
    std::enable_if_t<!has_static_buffer<Desc>::value, std::size_t>
    static_or_limited_reserialize(uint8_t * p, Desc const & x, AV av, Sz... sz)
    { return x.limited_reserialize(p, av, sz...); }


    namespace detail
    {

        template<class, class>
        struct add_size_impl
        { using type = dyn_size; };

        template<std::size_t n1, std::size_t n2>
        struct add_size_impl<static_size<n1>, static_size<n2>> { using type = static_size<n1 + n2>; };

        template<std::size_t n1, std::size_t n2>
        struct add_size_impl<limited_size<n1>, limited_size<n2>> { using type = limited_size<n1 + n2>; };
        template<std::size_t n1, std::size_t n2>
        struct add_size_impl<static_size<n1>, limited_size<n2>> { using type = limited_size<n1 + n2>; };
        template<std::size_t n1, std::size_t n2>
        struct add_size_impl<limited_size<n1>, static_size<n2>> { using type = limited_size<n1 + n2>; };
    }

    template<class Sz1, class Sz2>
    using add_size = typename detail::add_size_impl<Sz1, Sz2>::type;

    template<class L>
    using sizeof_desc_list = brigand::fold<
        brigand::transform<L, brigand::call<sizeof_>>,
        static_size<0>,
        brigand::call<add_size>
    >;

    template<class... Desc>
    using sizeof_descs = sizeof_desc_list<brigand::list<Desc...>>;

    namespace detail
    {
        template<class Ints, class... Ts>
        struct compose_impl;

        template<std::size_t, class T>
        struct indexed_value
        { T x; };

        template<std::size_t... Ints, class... Desc>
        struct compose_impl<
            std::integer_sequence<std::size_t, Ints...>,
            Desc...
        >
        {
            using sizeof_ = sizeof_descs<Desc...>;

            static_assert(!is_dynamic_size<sizeof_>{}, "not implemented");
            static_assert(!brigand::any<brigand::list<Desc...>, brigand::call<is_lazy_value>>{}, "not implemented");

            constexpr compose_impl(Desc... d) : descs{d...} {}

            std::size_t limited_serialize(uint8_t * p) const { return serialize(p); }

            std::size_t static_serialize(uint8_t * p) const { return serialize(p); }

        private:
            inherits<indexed_value<Ints, Desc>...> descs;

            std::size_t serialize(uint8_t * p) const
            {
                std::size_t sz = 0;
                (void)std::initializer_list<int>{
                    ((sz += static_or_limited_serialize(
                        p + sz,
                        static_cast<indexed_value<Ints, Desc> const &>(this->descs).x
                    )), 1)...
                };
                return sz;
            }
        };

        template<class Ch, class Tr, class Ints, class... Ts>
        std::basic_ostream<Ch, Tr> & operator <<(
            std::basic_ostream<Ch, Tr> & os, compose_impl<Ints, Ts...> const &)
        { return os << "compose"; }
    }

    template<class... Ts>
    using compose_t = detail::compose_impl<std::index_sequence_for<Ts...>, Ts...>;

#ifdef IN_IDE_PARSER
# define PROTO_IDE_CPP(...)
#else
# define PROTO_IDE_CPP(...) __VA_ARGS__
#endif

    namespace utils
    {
        namespace detail
        {
            template<class Val>
            struct ref
            { Val & x; };

            template<class Deps, class Value, class Desc>
            constexpr val<Deps, Value, Desc>
            ref_to_val(ref<val<Deps, Value, Desc>> r)
            { return r.x; }
        }

        template<class... Values>
        struct parameters
        {
            parameters(Values & ... values) : refs{values...}
            {}

            template<class Deps>
            constexpr decltype(auto)
            operator[](Deps const &) const noexcept
            { return detail::ref_to_val<Deps>(refs).desc; }

            template<class Deps>
            constexpr decltype(auto)
            get_proto_value(Deps const &) const noexcept
            { return detail::ref_to_val<Deps>(refs); }

            template<class Deps>
            constexpr decltype(auto)
            get_proto_value() const noexcept
            { return detail::ref_to_val<Deps>(refs); }

            proto::inherits<detail::ref<Values>...> refs;
        };

        template<class... Ts>
        parameters<Ts...>
        make_parameters(Ts & ... x)
        { return {x...}; }
    }

    template<class T>
    using is_val = meta::is_layout<val, T>;

    template<class Deps, class... Vals>
    struct packet
    {
        static_assert(brigand::all<brigand::list<Vals...>, brigand::call<is_val>>::value, "is not a proto::val");

        using dependencies = get_dependencies<Deps>;
        using type_list = brigand::list<Vals...>;

        inherits<Vals...> values;

        template<class F>
        void apply_for_each(F f) const
        {
            (void)std::initializer_list<int>{
                (void(f(static_cast<Vals const &>(this->values))), 1)...
            };
        }

        template<class F>
        decltype(auto) apply(F f) const
        {
            return f(static_cast<Vals const &>(this->values)...);
        }
    };

    class dummy_ {};

    /// \brief make a proto::packet
    template<class Desc>
    constexpr auto
    value(Desc const & desc)
    {
        using value_type = val<void, Desc>;
        return packet<Desc, value_type>{value_type{desc}};
    }

    template<class Deps, class Desc>
    constexpr auto
    value(Desc const & desc)
    {
        using value_type = val<Deps, Desc>;
        return packet<Deps, value_type>{value_type{desc}};
    }

    namespace detail
    {
        template<class... I, class... Desc>
        constexpr auto
        values_impl(brigand::list<I...>, Desc const & ... desc)
        {
            return packet<void, val<I, Desc>...>{{val<I, Desc>{desc}...}};
        }
    }

    template<class... Desc>
    constexpr auto
    values(Desc const & ... desc)
    {
        static_assert(sizeof...(desc), "");
        return detail::values_impl(brigand::range<std::size_t, 0, sizeof...(Desc)>{}, desc...);
    }

    // TODO value<Deps, Desc>()

    // TODO values(...)


    template<class F, class FF = void>
    struct hook_impl
    {
        using sizeof_ = static_size<0>;
        using is_reserializer = std::true_type;

        template<class... T>
        sizeof_ static_reserialize(uint8_t *, T... args) const
        {
            auto cond = brigand::bool_<sizeof...(args) == 1>{};
            cexpr::select(cond, this->f, this->ff)(args...);
            return sizeof_{};
        }

        F f;
        FF ff;
    };

    template<class F>
    struct hook_impl<F, void>
    {
        using sizeof_ = static_size<0>;
        using is_reserializer = std::true_type;

        template<class... T>
        sizeof_ static_reserialize(uint8_t *, T... args) const
        {
            this->f(args...);
            return sizeof_{};
        }

        F f;
    };

    template<class F, class... FF>
    auto hook(F f, FF ... ff)
    {
        using desc_type = hook_impl<F, FF...>;
        using value_type = val<void, desc_type>;
        return packet<F, value_type>{value_type{f, ff...}};
    }

    template<class Deps, class F, class... FF>
    auto hook(F f, FF ... ff)
    {
        using desc_type = hook_impl<F, FF...>;
        using value_type = val<Deps, desc_type>;
        return packet<Deps, value_type>{value_type{f, ff...}};
    }

    template<class Arguments, class Val>
    using enable_if_contains_argument = std::enable_if_t<
        brigand::any<
            Arguments,
            brigand::bind<
                std::is_same,
                brigand::_1,
                brigand::pin<deps_type_t<Val>>
            >
        >::value,
        Val
    >;


    template<class Deps, class... Ts>
    struct packet_description
    {
        using dependencies = get_dependencies<Deps>;
        using arguments = brigand::append<get_arguments<Ts>...>;

        inherits<Ts...> values;

        template<class... Val>
        constexpr auto
        operator()(Val... values) const
        {
            return ordering_parameter<enable_if_contains_argument<arguments, Val>...>(
                {values...}
            );
        }

    private:
        template<class... Us>
        constexpr auto
        ordering_parameter(utils::parameters<Us...> params) const
        {
            return packet<
                Deps,
                decltype(static_cast<Ts const &>(this->values).to_proto_value(params))...
            >{{(static_cast<Ts const &>(this->values).to_proto_value(params))...}};
        }
    };

    template<class T>
    using is_packet_description = meta::is_layout<packet_description, T>;


    template<class Deps = void, class... Def>
    constexpr auto
    desc(Def... d)
    {
        return packet_description<Deps, Def...>{{d...}};
    }

    template<class Deps, class... Pkts>
    struct subpacket
    {
        inherits<Pkts...> pkts;
    };

    template<class Deps, class... Pkts>
    constexpr auto
    make_subpacket(Pkts && ... pkts)
    { return subpacket<Deps, Pkts...>{{pkts...}}; }

    template<class Deps, class... PktDescs>
    struct subpacket_description
    {
        static_assert(brigand::all<brigand::list<PktDescs...>, brigand::call<is_packet_description>>{}, "");

        using dependencies = get_dependencies<Deps>;
        using arguments = brigand::append<get_arguments<PktDescs>...>;

        inherits<PktDescs...> subpkts;

        template<class... Val>
        constexpr auto
        operator()(Val... values) const
        {
            inherits<enable_if_contains_argument<arguments, Val>...> params{values...};
            return make_subpacket<Deps>(
                this->eval_subpkt(
                    static_cast<PktDescs const &>(this->subpkts),
                    get_arguments<PktDescs>{},
                    params
                )...
            );
        }

    private:
        template<class PktDesc, class... Args, class Params>
        constexpr auto
        eval_subpkt(PktDesc const & pktdesc, brigand::list<Args...>, Params const & params) const
        {
            return pktdesc(static_cast<Args const &>(params)...);
        }
    };

    template<class Deps = void, class... PktDesc>
    constexpr auto
    desc2(PktDesc... d)
    {
        return subpacket_description<Deps, PktDesc...>{{d...}};
    }


    template<class Desc, class... Val>
    constexpr auto
    creater(Val... v)
    {
        return creator<brigand::list<Val...>, Desc, Val...>{{v...}};
    }

    template<class Deps, class Desc, class... Val>
    constexpr auto
    creater(Val... v)
    {
        return creator<Deps, Desc, Val...>{{v...}};
    }

    template<class... Val>
    constexpr auto
    composer(Val... v)
    {
        using subtype = compose_t<desc_or_t<Val>...>;
        return creator<brigand::list<Val...>, subtype, Val...>{{v...}};
    }

    template<class Deps, class... Val>
    constexpr auto
    composer(Val... v)
    {
        using subtype = compose_t<desc_or_t<Val>...>;
        return creator<Deps, subtype, Val...>{{v...}};
    }

    template<class T>
    using is_proto_packet = meta::is_layout<packet, T>;

    template<class T>
    using is_proto_subpacket = meta::is_layout<subpacket, T>;


    template<class Deps, class Desc, class Expr>
    struct retype_impl
    {
        using arguments = get_arguments<Expr>;
        using dependencies = brigand::push_back<
            get_dependencies<Expr>,
            get_dependencies_if_void<Deps, brigand::list<>>
        >;

        template<class Params>
        constexpr auto
        to_proto_value(Params p) const
        {
            using type = typename Desc::type;
            using dst = integral_type<type>;
            return val<dependencies, Desc>{{
                static_cast<type>(
                    checked_cast<dst>(this->expr_.to_proto_value(p).desc.val)
                )
            }};
        }

        Expr expr_;
    };

    template<class T> struct as {};

    // TODO redesk
    template<class Desc, class Expr>
    constexpr
    retype_impl<void, Desc, Expr>
    retype(Expr expr)
    { return {expr}; }

    // TODO desk_as
    template<class Dep, class Desc, class Expr>
    constexpr
    retype_impl<as<Dep>, Desc, Expr>
    retype_as(var<Dep, Desc>, Expr expr)
    { return {expr}; }


    namespace dsl
    {
        template<class Deps, class Op, bool MutableOperator, class T, class U>
        struct expr;

        // value
        template<class Deps, class Desc>
        struct expr<Deps, void, false, Desc, void>
        {
            using dependencies = get_dependencies<Deps>;
            using desc_type = Desc;

            template<class Params>
            constexpr expr to_proto_value(Params) const
            {
                return *this;
            }

            Desc desc;
        };

        template<class Deps, class T>
        using value = expr<Deps, void, false, types::value<T>, void>;

        // mutable binary operator
        template<class Deps, class Op, class T, class U>
        struct expr<Deps, Op, true, T, U>
        {
            using dependencies = get_dependencies_if_void<Deps, T, U>;
            using arguments = brigand::append<
                get_arguments<T>,
                get_arguments<U>
            >;

            template<class Params>
            constexpr auto
            to_proto_value(Params p) const
            {
                auto ret = this->x_.to_proto_value(p);
                // TODO integral_type2 ; safe_int<T> = T
                using type = decltype(ret.desc.val.val);
                using dst = integral_type<type>;
                ret.desc.val = static_cast<type>(
                    checked_cast<dst>(Op{}(
                        ret.desc.val,
                        this->y_.to_proto_value(p).desc.val
                    ))
                );
                return ret;
            }

            T x_;
            U y_;

        private:
            template<class Dep, class Desc>
            static constexpr
            value<brigand::append<dependencies, get_dependencies<Dep>>, Desc>
            to_value(value<Dep, Desc> v)
            { return {v.desc}; }

            template<class Dep, class Value, class Desc>
            static constexpr
            proto::val<brigand::append<dependencies, get_dependencies<Dep>>, Value, Desc>
            to_value(proto::val<Dep, Value, Desc> v)
            { return {v.desc}; }
        };

        // binary operator
        template<class Deps, class Op, class T, class U>
        struct expr<Deps, Op, false, T, U>
        {
            using dependencies = get_dependencies_if_void<Deps, T, U>;
            using arguments = brigand::append<
                get_arguments<T>,
                get_arguments<U>
            >;

            template<class Params>
            constexpr auto
            to_proto_value(Params p) const
            {
                return to_value(Op{}(
                    this->x_.to_proto_value(p).desc.val,
                    this->y_.to_proto_value(p).desc.val
                ));
            }

            T x_;
            U y_;

        private:
            template<class Dep, class Desc>
            static constexpr
            value<brigand::append<dependencies, get_dependencies<Dep>>, Desc>
            to_value(value<Dep, Desc> v)
            { return {v.desc}; }

            template<class Dep, class Value, class Desc>
            static constexpr
            proto::val<brigand::append<dependencies, get_dependencies<Dep>>, Value, Desc>
            to_value(proto::val<Dep, Value, Desc> v)
            { return {v.desc}; }

            template<class V>
            static constexpr
            value<dependencies, V>
            to_value(V v)
            { return {v}; }
        };

        // mutable unary operator
        template<class Deps, class Op, class T>
        struct expr<Deps, Op, true, T, void>
        {
            using dependencies = get_dependencies_if_void<Deps, T>;
            using arguments = get_arguments<T>;

            template<class Params>
            constexpr auto
            to_proto_value(Params p) const
            {
                auto ret = this->x_.to_proto_value(p);
                ret.desc.val = Op{}(ret.desc.val);
                return this->to_value(ret);
            }

            T x_;

        private:
            template<class Dep, class Desc>
            static constexpr
            value<brigand::append<dependencies, get_dependencies<Dep>>, Desc>
            to_value(value<Dep, Desc> v)
            { return {v.desc}; }

            template<class Dep, class Value, class Desc>
            static constexpr
            proto::val<brigand::append<dependencies, get_dependencies<Dep>>, Value, Desc>
            to_value(proto::val<Dep, Value, Desc> v)
            { return {v.desc}; }
        };

        // unary operator
        template<class Deps, class Op, class T>
        struct expr<Deps, Op, false, T, void>
        {
            using dependencies = get_dependencies_if_void<Deps, T>;
            using arguments = get_arguments<T>;

            template<class Params>
            constexpr auto
            to_proto_value(Params p) const
            {
                return to_value(Op{}(this->x_.to_proto_value(p).desc.val));
            }

            T x_;

        private:
            template<class Dep, class Desc>
            static constexpr
            value<brigand::append<dependencies, get_dependencies<Dep>>, Desc>
            to_value(value<Dep, Desc> v)
            { return {v.desc}; }

            template<class Dep, class Value, class Desc>
            static constexpr
            proto::val<brigand::append<dependencies, get_dependencies<Dep>>, Value, Desc>
            to_value(proto::val<Dep, Value, Desc> v)
            { return {v.desc}; }

            template<class V>
            static constexpr
            value<dependencies, V>
            to_value(V v)
            { return {v}; }
        };

        template<class Op, bool MutableOperator, class T>
        using unary_expr = expr<void, Op, MutableOperator, T, void>;

        // param
        template<class Dep>
        struct expr<void, Dep, true, void, void>
        {
            using dependencies = brigand::list<Dep>;
            using arguments = brigand::list<Dep>;

            template<class Params>
            constexpr decltype(auto) to_proto_value(Params p) const
            {
                return p.template get_proto_value<Dep>();
            }
        };

        template<class Dep>
        using param = expr<void, Dep, true, void, void>;


        namespace ops
        {
            struct lshift
            {
                template<class T, class U>
                constexpr auto operator()(T x, U y) const
                { return x << y; }
            };

            struct unsafe_cast
            {
                template<class T>
                constexpr auto operator()(T x) const
                {
                    return x;
                }
            };
        }
    }

#ifdef IN_IDE_PARSER
# define PROTO_DSL_BINARY_OPERATORS
# define PROTO_DSL_BINARY_OPERATOR(...)
#else
# define PROTO_DSL_BINARY_OPERATOR(mut, op_type, op)               \
    namespace dsl {                                                \
        template<                                                  \
            class Deps, class Op, bool Mut, class T, class U,      \
            class V                                                \
        > enable_is_integral_or_enum_t<                            \
            V,                                                     \
            expr<                                                  \
                void, op_type, mut,                                \
                expr<Deps, Op, Mut, T, U>,                         \
                value<void, V>                                     \
            >                                                      \
        > constexpr operator op (                                  \
            expr<Deps, Op, Mut, T, U> xexpr,                       \
            V y                                                    \
        ) { return {xexpr, {{y}}}; }                               \
                                                                   \
        template<                                                  \
            class V,                                               \
            class Deps, class Op, bool Mut, class T, class U       \
        > enable_is_integral_or_enum_t<                            \
            V,                                                     \
            expr<                                                  \
                void, op_type, mut,                                \
                value<void, V>,                                    \
                expr<Deps, Op, Mut, T, U>                          \
            >                                                      \
        > constexpr operator op (                                  \
            V x,                                                   \
            expr<Deps, Op, Mut, T, U> yexpr                        \
        ) { return {{{x}}, yexpr}; }                               \
                                                                   \
        template<                                                  \
            class Deps1, class Op1, bool Mut1, class T1, class U1, \
            class Deps2, class Op2, bool Mut2, class T2, class U2  \
        > expr<                                                    \
            void, op_type, mut,                                    \
            expr<Deps1, Op1, Mut1, T1, U1>,                        \
            expr<Deps2, Op2, Mut2, T2, U2>                         \
        > constexpr operator op (                                  \
            expr<Deps1, Op1, Mut1, T1, U1> expr1,                  \
            expr<Deps2, Op2, Mut2, T2, U2> expr2                   \
        ) { return {expr1, expr2}; }                               \
                                                                   \
        template<                                                  \
            class Deps, class Op, bool Mut, class T, class U,      \
            class Dep, class Desc                                  \
        > expr<                                                    \
            void, op_type, mut,                                    \
            expr<Deps, Op, Mut, T, U>,                             \
            param<Dep>                                             \
        > constexpr operator op (                                  \
            expr<Deps, Op, Mut, T, U> xexpr,                       \
            var<Dep, Desc>                                         \
        ) { return {xexpr, {}}; }                                  \
                                                                   \
        template<                                                  \
            class Dep, class Desc,                                 \
            class Deps, class Op, bool Mut, class T, class U       \
        > expr<                                                    \
            void, op_type, mut,                                    \
            param<Dep>,                                            \
            expr<Deps, Op, Mut, T, U>                              \
        > constexpr operator op (                                  \
            var<Dep, Desc>,                                        \
            expr<Deps, Op, Mut, T, U> yexpr                        \
        ) { return {{}, yexpr}; }                                  \
    }                                                              \
                                                                   \
    template<                                                      \
        class Dep, class Desc,                                     \
        class V                                                    \
    > enable_is_integral_or_enum_t<                                \
        V,                                                         \
        dsl::expr<                                                 \
            void, op_type, mut,                                    \
            dsl::param<Dep>,                                       \
            dsl::value<void, V>                                    \
        >                                                          \
    > constexpr operator op (                                      \
        var<Dep, Desc>,                                            \
        V y                                                        \
    ) { return {{}, {{y}}}; }                                      \
                                                                   \
    template<                                                      \
        class V,                                                   \
        class Dep, class Desc                                      \
    > enable_is_integral_or_enum_t<                                \
        V,                                                         \
        dsl::expr<                                                 \
            void, op_type, mut,                                    \
            dsl::value<void, V>,                                   \
            dsl::param<Dep>                                        \
        >                                                          \
    > constexpr operator op (                                      \
        V x,                                                       \
        var<Dep, Desc>                                             \
    ) { return {{{x}}, {}}; }                                      \
                                                                   \
    template<                                                      \
        class Dep1, class Desc1,                                   \
        class Dep2, class Desc2                                    \
    > dsl::expr<                                                   \
        void, op_type, mut,                                        \
        dsl::param<Dep1>,                                          \
        dsl::param<Dep2>                                           \
    > constexpr operator op (                                      \
        var<Dep1, Desc1>,                                          \
        var<Dep2, Desc2>                                           \
    ) { return {}; }

# define PROTO_DSL_BINARY_OPERATORS(op_type, op, op_eq) \
    PROTO_DSL_BINARY_OPERATOR(false, op_type, op)       \
    PROTO_DSL_BINARY_OPERATOR(true , op_type, op_eq)
#endif

    PROTO_DSL_BINARY_OPERATORS(std::bit_and<>, &, &= )
    PROTO_DSL_BINARY_OPERATORS(std::bit_or<>, |, |= )
    PROTO_DSL_BINARY_OPERATORS(::proto::dsl::ops::lshift, <<, <<= )

#undef PROTO_DSL_BINARY_OPERATOR
#undef PROTO_DSL_BINARY_OPERATORS


    constexpr struct params_
    {
        constexpr params_() noexcept {}

        template<class Dep, class Desc>
        constexpr dsl::param<Dep>
        operator[](var<Dep, Desc>) const noexcept
        {
            return {};
        }
    } params;

    namespace detail
    {
        template<class Val, class ValElse>
        struct if_else
        {
            using sizeof_ = proto::common_size<proto::sizeof_<Val>, proto::sizeof_<ValElse>>;
            using buffer_category = proto::common_buffer<
                proto::buffer_category<Val>,
                proto::buffer_category<ValElse>
            >;

            std::size_t static_serialize(uint8_t * p) const
            {
                return this->is_ok
                    ? this->val_ok.static_serialize(p)
                    : this->val_fail.static_serialize(p);
            }

            std::size_t limited_serialize(uint8_t * p) const
            {
                return this->is_ok
                    ? static_or_limited_serialize(p, this->val_ok)
                    : static_or_limited_serialize(p, this->val_fail);
            }

            array_view_const_u8 get_view_buffer() const
            {
                return this->is_ok
                    ? this->val_ok.get_view_buffer()
                    : this->val_fail.get_view_buffer();
            }

            bool is_ok;
            Val val_ok;
            ValElse val_fail;
        };

        template<class Deps, class Cond, class Var, class VarElse>
        struct if_else_act
        {
            using dependencies = get_dependencies_if_void<Deps, Cond, Var, VarElse>;
            using arguments = brigand::append<
                proto::get_arguments<Cond>,
                proto::get_arguments<Var>,
                proto::get_arguments<VarElse>
            >;

            template<class Params>
            constexpr auto to_proto_value(Params params) const
            {
                auto value = this->var.to_proto_value(params);
                using proto_val = decltype(value);

                auto value_else = this->var_else.to_proto_value(params);
                using proto_val_else = decltype(value_else);

                using is_same = std::is_same<
                    desc_type_t<proto_val>,
                    desc_type_t<proto_val_else>
                >;

                static_assert(!is_lazy_value<decltype(cond.to_proto_value(params))>::value, "unimplemented special value with if_else");
                static_assert(!is_lazy_value<decltype(value)>::value, "unimplemented special value with if_else");
                static_assert(!is_lazy_value<decltype(value_else)>::value, "unimplemented special value with if_else");

                return to_proto_value_(
                    typename is_same::type{},
                    bool(cond.to_proto_value(params).desc.val),
                    value,
                    value_else
                );
            }

            Cond cond;
            Var var;
            VarElse var_else;

        private:
            template<class Value>
            constexpr auto to_proto_value_(
                std::true_type, bool test,
                Value & value, Value & value_else
            ) const {
                return test ? std::move(value) : std::move(value_else);
            }

            template<class Value, class ValueElse>
            constexpr auto to_proto_value_(
                std::true_type, bool test,
                Value & value, ValueElse & value_else
            ) const {
                return val<void, Value, desc_type_t<Value>>{
                    test ? std::move(value.desc) : std::move(value_else.desc)
                };
            }

            template<class Value, class ValueElse>
            constexpr auto to_proto_value_(
                std::false_type, bool test,
                Value & value, ValueElse & value_else
            ) const {
                using new_value_type = if_else<
                    desc_type_t<Value>,
                    desc_type_t<ValueElse>
                >;

                return val<void, new_value_type>{
                    test, std::move(value.desc), std::move(value_else.desc)
                };
            }
        };

        template<class Desc>
        struct only_if_true
        {
            using sizeof_ = proto::common_size<proto::sizeof_<Desc>, proto::limited_size<0>>;
            using buffer_category = typename std::conditional_t<
                proto::has_view_buffer<Desc>::value,
                proto::detail::buffer_category_impl<Desc>,
                proto::detail::common_buffer_impl<
                    proto::tags::limited_buffer,
                    proto::buffer_category<Desc>
                >
            >::type;
            using is_reserializer = proto::is_reserializer<Desc>;

            std::size_t limited_serialize(uint8_t * p) const
            {
                return this->is_ok ? proto::static_or_limited_serialize(p, this->val_ok) : 0u;
            }

            template<class AV, class... Sz>
            std::size_t limited_reserialize(uint8_t * p, AV av, Sz... sz) const
            {
                return this->is_ok ? proto::static_or_limited_reserialize(p, this->val_ok, av, sz...) : 0u;
            }

            array_view_const_u8 get_view_buffer() const
            {
                return this->is_ok ? this->val_ok.get_view_buffer() : array_view_const_u8{};
            }

            std::size_t reserved_size() const
            {
                static_assert(proto::has_static_buffer<Desc>{}, "unimplemented");
                return this->is_ok ? proto::sizeof_<Desc>::value : 0u;
            }

            bool is_ok;
            Desc val_ok;
        };

        template<class Val>
        struct lazy_only_if_true
        {
            using desc_type = only_if_true<proto::desc_type_t<Val>>;
            using arguments = proto::get_arguments<Val>;

            using sizeof_ = proto::sizeof_<desc_type>;
            using buffer_category = proto::buffer_category<desc_type>;
            using is_reserializer = proto::is_reserializer<desc_type>;

            bool is_ok;
            Val var;

            template<class Params>
            constexpr auto
            to_proto_value(Params params) const
            {
                // TODO Deps
                return proto::val<void, desc_type>{
                    this->is_ok,
                    this->var.to_proto_value(params).desc
                };
            }

            std::size_t reserved_size() const
            {
                static_assert(proto::has_static_buffer<desc_type_t<Val>>{}, "unimplemented");
                return this->is_ok ? proto::sizeof_<desc_type_t<Val>>::value : 0u;
            }
        };

        template<class Deps, class Cond, class Var>
        struct if_act
        {
            using dependencies = get_dependencies_if_void<Deps, Cond, Var>;
            using arguments = brigand::append<
                proto::get_arguments<Cond>,
                proto::get_arguments<Var>
            >;

            template<class Params>
            constexpr auto to_proto_value(Params params) const
            {
                return this->to_proto_value_(
                    this->else_.cond.to_proto_value(params).desc,
                    this->else_.var.to_proto_value(params).desc
                );
            }

            struct
            {
                template<class VarElse>
                constexpr auto operator[](VarElse var_else) const
                {
                    return if_else_act<Deps, Cond, Var, VarElse>{cond, var, var_else};
                }

                Cond cond;
                Var var;
            } else_;

        private:
            template<class DescCond, class Desc>
            constexpr std::enable_if_t<
                !proto::has_pkts_sz<Desc>::value and
                !proto::has_pkts_sz<DescCond>::value,
                proto::val<dependencies, only_if_true<Desc>>
            >
            to_proto_value_(DescCond const & cond, Desc && value) const
            {
                return {{bool(cond.val), std::move(value)}};
            }

            template<class DescCond, class Desc>
            constexpr std::enable_if_t<
                proto::has_pkts_sz<Desc>::value or
                proto::has_pkts_sz<DescCond>::value,
                proto::val<dependencies, lazy_only_if_true<Desc>, lazy_only_if_true<Desc>>
            >
            to_proto_value_(DescCond cond, Desc && value) const
            {
                static_assert(!proto::has_pkts_sz<DescCond>::value, "unimplemented");
                return {bool(cond.val), std::move(value)};
            }
        };

        template<class Deps, class Cond>
        struct if_
        {
            template<class Var>
            constexpr auto operator[](Var var) const
            {
                return if_act<Deps, Cond, Var>{{cond, var}};
            }

            Cond cond;
        };
    }

    template<class Cond>
    constexpr auto if_(Cond cond)
    {
        return detail::if_<void, Cond>{cond};
    }

    template<class Deps, class Cond>
    constexpr auto if_(Cond cond)
    {
        return detail::if_<Deps, Cond>{cond};
    }

    /// \brief if_(cond)[var_if].else_[var_else]
    template<class Cond, class TIf, class TElse>
    constexpr auto choose(Cond cond, TIf var_if, TElse var_else)
    {
        return if_(cond)[var_if].else_[var_else];
    }

    /// \brief if_< Deps>(cond)[var_if].else_[var_else]
    template<class Deps, class Cond, class TIf, class TElse>
    constexpr auto choose(Cond cond, TIf var_if, TElse var_else)
    {
        return if_<Deps>(cond)[var_if].else_[var_else];
    }

    template<class Var>
    constexpr auto if_true(Var v)
    {
        return if_(v)[v];
    }

    template<class Deps, class Var>
    constexpr auto if_true(Var v)
    {
        return if_<Deps>(v)[v];
    }

    namespace detail
    {
        template<class T>
        using is_maybe = meta::is_layout<maybe_impl, T>;

        template<class T>
        using has_ignore_or_maybe = brigand::bool_<
            std::is_same<ignore_t, T>{} or is_maybe<T>{}
        >;

        struct ignore_desc
        {
            ignore_t ignored;

            constexpr static static_size<0> static_serialize(uint8_t *)
            {
                return {};
            }
        };

        template<class... Ts>
        struct check_seq_value
        {
            static constexpr bool check()
            {
                bool bools[]{std::is_same<ignore_t, value_type_t<Ts>>::value...};
                enum { no_ignore, has_ignore, has_error }
                st = no_ignore;

                for (bool b : bools) {
                    if (b) {
                        st = has_ignore;
                    }
                    else if (st != has_ignore) {
                        st = has_error;
                        break;
                    }
                }

                return st == has_error;
            }

            static const bool value = check();

            static_assert(value, "ignored value following a not ignored value");
        };

        template<class T>
        constexpr std::size_t
        get_value_size_impl(tags::static_buffer, T const &)
        { return sizeof_<T>{}; }

        template<class T>
        constexpr std::size_t
        get_value_size_impl(tags::limited_buffer, T const & v)
        { return v.real_size(); }

        template<std::size_t N, class T>
        constexpr std::size_t
        get_value_size_impl(tags::view_buffer, T const & v)
        { return v.get_view_buffer().size(); }

        template<class Desc>
        constexpr std::size_t
        get_value_size(Desc const & v)
        { return get_value_size_impl(buffer_category<Desc>{}, v); }

        template<class Deps, class Desc, class... Vars>
        struct optseq_sz_impl
        {
            using dependencies = get_dependencies_if_void<Deps, Vars...>;
            using arguments = brigand::append<proto::get_arguments<Vars>...>;

            inherits<Vars...> values;

            template<class Params>
            auto to_proto_value(Params params) const
            {
                std::size_t const sizes[]{
                    get_value_size(static_cast<Vars const &>(this->values).to_proto_value(params).desc)...
                };
                std::size_t total_size{};
                for (auto sz : sizes) {
                    total_size += sz;
                }
                return val<Deps, Desc>{checked_cast<t_<Desc>>(total_size)};
            }
        };

        constexpr bool is_enable_value(ignore_t const &) { return false; }
        template<class T> constexpr bool is_enable_value(T const &) { return true; }
        template<class T> constexpr bool is_enable_value(maybe_impl<T> const & m) { return m.enable; }

        template<class Deps, class... Vars>
        struct optseq_impl
        {
            using dependencies = get_dependencies_if_void<Deps, Vars...>;
            using arguments = brigand::append<proto::get_arguments<Vars>...>;

            static_assert(!is_dynamic_size<sizeof_descs<desc_or_t<Vars>...>>{}, "not implemented");
            static_assert(brigand::none<brigand::list<desc_or_t<Vars>...>, brigand::call<has_view_buffer>>{}, "");

            inherits<Vars...> values;

            template<class Params>
            constexpr auto to_proto_value(Params params) const
            {
                return impl_check(static_cast<Vars const &>(this->values).to_proto_value(params)...);
            }

            template<class Desc>
            constexpr auto get_optseq_sz() const
            {
                return optseq_sz_impl<Deps, Desc, Vars...>{this->values};
            }

        private:
            template<class... Vals>
            static auto impl_check(Vals && ... values)
            {
                #ifndef NDEBUG
                bool const is_enable_values_list[]{is_enable_value(values)...};
                bool is_ignored = false;

                for (bool is_enable : is_enable_values_list) {
                    assert((!is_ignored || !is_enable) && "previous value is not ignored");
                    is_ignored |= !is_enable;
                }
                #endif
                return impl(check<check_seq_value<Vals...>>{}, std::forward<Vals>(values)...);
            }

            template<class... Vals>
            static auto impl(std::true_type, Vals && ... values)
            {
                using value_type = compose_t<
                    std::conditional_t<
                        std::is_same<ignore_t, value_type_t<Vals>>{},
                        ignore_desc,
                        value_type_t<Vals>
                    >...
                >;
                return val<dependencies, value_type>{{std::forward<Vals>(values).desc...}};
            }
        };
    }

    template<class... Vals>
    constexpr auto optseq(Vals... vals)
    {
        return detail::optseq_impl<brigand::list<Vals...>, Vals...>{{vals...}};
    }

    template<class Dep, class... Vals>
    constexpr auto optseq(Vals... vals)
    {
        return detail::optseq_impl<Dep, Vals...>{{vals...}};
    }



    template<class T>
    struct named_dep
    {};

    template<class Ch, class Tr>
    std::basic_ostream<Ch, Tr> &
    operator<<(std::basic_ostream<Ch, Tr> & out, named_dep<brigand::list<>> const &)
    { return out << "<unamed>"; }

    namespace detail
    {
        struct view_name
        {
            array_view_const_char av;

            template<class Ch, class Tr>
            friend std::basic_ostream<Ch, Tr> &
            operator<<(std::basic_ostream<Ch, Tr> & out, view_name const & av_name)
            { return out.write(av_name.av.data(), av_name.av.size()); }
        };

        template<class T>
        constexpr view_name n()
        {
            constexpr std::size_t len = sizeof(__PRETTY_FUNCTION__);
#ifdef __clang__
            constexpr std::size_t begin_skip = 49;
#else
            constexpr std::size_t begin_skip = 64;
#endif
            constexpr std::size_t end_skip = 2;
            static_assert(begin_skip + end_skip < len, "");
            return {{__PRETTY_FUNCTION__ + begin_skip, len - begin_skip - end_skip}};
        }
    }

    template<class Ch, class Tr, class T>
    std::basic_ostream<Ch, Tr> &
    operator<<(std::basic_ostream<Ch, Tr> & out, named_dep<T> const &)
    { return out << detail::n<T>(); }

    template<class Ch, class Tr, class Desc>
    std::basic_ostream<Ch, Tr> &
    operator<<(std::basic_ostream<Ch, Tr> & out, named_dep<special<dsl::next_pkts_sz, Desc>> const &)
    { return out << named_dep<dsl::next_pkts_sz>{}; }

    template<class Ch, class Tr, class Desc>
    std::basic_ostream<Ch, Tr> &
    operator<<(std::basic_ostream<Ch, Tr> & out, named_dep<special<dsl::current_pkts_sz, Desc>> const &)
    { return out << named_dep<dsl::current_pkts_sz>{}; }

    namespace detail
    {
        template<class Ch, class Tr, class T, class... Ts>
        void named_dep_out(std::basic_ostream<Ch, Tr> & out, brigand::list<T, Ts...>)
        {
            out << "{ " << named_dep<T>{};
            (void)std::initializer_list<int>{((void)(out << " " << named_dep<Ts>{}), 1)...};
            out << " }";
        }
    }

    template<class Ch, class Tr, class... Ts>
    std::basic_ostream<Ch, Tr> &
    operator<<(std::basic_ostream<Ch, Tr> & out, named_dep<brigand::list<Ts...>> const &)
    {
        detail::named_dep_out(out, brigand::unique<brigand::list<Ts...>>{});
        return out;
    }

    namespace detail
    {
        template<class Pkt>
        std::enable_if_t<is_proto_packet<Pkt>::value, Pkt const &>
        get_if_packet(Pkt const & pkt)
        { return pkt; }
    }

    template<class F, class... Pkts>
    std::enable_if_t<
        !brigand::any<
            brigand::list<Pkts...>,
            brigand::call<is_proto_subpacket>
        >::value
    > apply(F f, Pkts const & ... pkts)
    {
        f(detail::get_if_packet(pkts)...);
    }

    namespace detail
    {
        template<class Pkt>
        struct pkt_to_list
        { using type = brigand::list<brigand::pair<Pkt, Pkt>>; };

        template<class Deps, class... Pkts>
        struct pkt_to_list<subpacket<Deps, Pkts...>>
        {
            using type = brigand::list<
                brigand::pair<
                    inherits<Pkts...>,
                    Pkts
                >...
            >;
        };

        template<class Pkt>
        Pkt const &
        pkt_to_subpkt(Pkt const & pkt)
        { return pkt; }

        template<class Deps, class... Ts>
        auto const &
        pkt_to_subpkt(subpacket<Deps, Ts...> const & pkt)
        { return pkt.pkts; }

        template<class... Ts, class F, class Pkts>
        void
        apply_subpkt(brigand::list<Ts...>, F && f, Pkts const & pkts)
        {
            apply(
                f,
                static_cast<typename Ts::second_type const &>(
                    static_cast<utils::detail::ref<typename Ts::first_type const &>>(
                        pkts
                    ).x
                )...
            );
        }
    }

    template<class F, class... Pkts>
    std::enable_if_t<
        brigand::any<
            brigand::list<Pkts...>,
            brigand::call<is_proto_subpacket>
        >::value
    > apply(F f, Pkts const & ... pkts)
    {
        inherits<utils::detail::ref<decltype(detail::pkt_to_subpkt(pkts))>...> values{
            detail::pkt_to_subpkt(pkts)...
        };
        detail::apply_subpkt(
            brigand::append<typename detail::pkt_to_list<Pkts>::type...>{},
            f,
            values
        );
    }



    // TODO proto_utils.hpp

    namespace detail
    {
        template<class L, class Add>
        struct add_size_and_push_back_impl;

        template<class... Ts, class n>
        struct add_size_and_push_back_impl<brigand::list<Ts...>, n>
        { using type = brigand::list<add_size<Ts, n>..., n>; };
    }

    template<class L, class x>
    using add_size_and_push_back = typename detail::add_size_and_push_back_impl<L, x>::type;

    template<class L>
    using accu_sizeof_list = brigand::fold<L, brigand::list<>, brigand::call<add_size_and_push_back>>;

    namespace detail
    {
        template<class T>
        struct limited_size_to_dyn_size_impl
        { using type = T; };

        template<std::size_t n>
        struct limited_size_to_dyn_size_impl<limited_size<n>>
        { using type = dyn_size; };
    }

    template<class T>
    using limited_size_to_dyn_size = typename detail::limited_size_to_dyn_size_impl<T>::type;

    template<class... Pkts>
    struct sizeof_pkt_ctx
    {
        // [ [ desc_type ... ] ... ]
        using desc_list_by_packet = brigand::list<
            brigand::transform<
                typename Pkts::type_list,
                brigand::call<desc_type_t>
            >...
        >;

        // [ static_size<n> | dyn_size ... ]
        using sizeof_by_packet = brigand::transform<
            brigand::transform<
                desc_list_by_packet,
                brigand::call<sizeof_desc_list>
            >,
            brigand::call<limited_size_to_dyn_size>
        >;

        // [ static_size<n> | dyn_size ... ]
        using accu_sizeof_by_packet = accu_sizeof_list<sizeof_by_packet>;

        // [ static_size<n> | dyn_size ... 0 ]
        using accu_next_sizeof_by_packet = brigand::push_back<
            brigand::pop_front<accu_sizeof_by_packet>,
            proto::static_size<0>
        >;
    };
}
