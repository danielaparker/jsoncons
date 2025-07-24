// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_REFLECT_ENCODE_TRAITS_HPP
#define JSONCONS_REFLECT_ENCODE_TRAITS_HPP

#include <array>
#include <cstddef>
#include <memory>
#include <utility>
#include <string>
#include <system_error>
#include <tuple>
#include <type_traits> // std::enable_if, std::true_type, std::false_type

#include <jsoncons/basic_json.hpp>
#include <jsoncons/conv_error.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/reflect/json_conv_traits.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/utility/more_type_traits.hpp>

namespace jsoncons {
namespace reflect {

// encode_traits

template <typename T, typename Enable = void>
struct encode_traits
{
public:
    template <typename CharT, typename Alloc, typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>& aset,
        const T& val, 
        basic_json_visitor<CharT>& encoder,
        std::error_code& ec)
    {
        auto j = json_conv_traits<basic_json<CharT,order_preserving_policy,TempAlloc>,T>::to_json(aset.get_temp_allocator(), val);
        j.dump(encoder, ec);
    }
};

// specializations

// bool
template <typename T>
struct encode_traits<T,
    typename std::enable_if<ext_traits::is_bool<T>::value 
>::type>
{
    template <typename CharT,typename Alloc,typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>&, const T& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.bool_value(val,semantic_tag::none,ser_context(),ec);
    }
};

// uint
template <typename T>
struct encode_traits<T,
    typename std::enable_if<ext_traits::is_u8_u16_u32_or_u64<T>::value 
>::type>
{
    template <typename CharT, typename Alloc, typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>&, const T& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.uint64_value(val,semantic_tag::none,ser_context(),ec);
    }
};

// int
template <typename T>
struct encode_traits<T,
    typename std::enable_if<ext_traits::is_i8_i16_i32_or_i64<T>::value 
>::type>
{
    template <typename CharT, typename Alloc, typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>&, const T& val,
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.int64_value(val,semantic_tag::none,ser_context(),ec);
    }
};

// float or double
template <typename T>
struct encode_traits<T,
    typename std::enable_if<ext_traits::is_float_or_double<T>::value 
>::type>
{
    template <typename CharT, typename Alloc, typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>&, const T& val,
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.double_value(val,semantic_tag::none,ser_context(),ec);
    }
};

// string
template <typename T>
struct encode_traits<T,
    typename std::enable_if<ext_traits::is_string<T>::value /*&&
                            std::is_same<typename T::value_type,CharT>::value*/ 
>::type>
{
    template <typename CharT, typename Alloc, typename TempAlloc>
    static
    typename std::enable_if<std::is_same<typename T::value_type, CharT>::value>::type
    try_encode(const allocator_set<Alloc,TempAlloc>&, const T& val,
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.string_value(val,semantic_tag::none,ser_context(),ec);
    }
    template <typename CharT, typename Alloc, typename TempAlloc>
    static
    typename std::enable_if<!std::is_same<typename T::value_type, CharT>::value>::type
        try_encode(const allocator_set<Alloc,TempAlloc>&, const T& val,
        basic_json_visitor<CharT>& encoder,
        std::error_code& ec)
    {
        std::basic_string<CharT> s;
        unicode_traits::convert(val.data(), val.size(), s);
        encoder.string_value(s, semantic_tag::none, ser_context(), ec);
    }
};

// std::pair

template <typename T1,typename T2>
struct encode_traits<std::pair<T1, T2>>
{
    using value_type = std::pair<T1, T2>;

    template <typename CharT,typename Alloc,typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>& aset, const value_type& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.begin_array(2,semantic_tag::none,ser_context(),ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        encode_traits<T1>::try_encode(aset, val.first, encoder, ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        encode_traits<T2>::try_encode(aset, val.second, encoder, ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        encoder.end_array(ser_context(),ec);
    }
};

// std::tuple

namespace detail
{
    template<size_t Pos, std::size_t Size,typename Tuple>
    struct json_serialize_tuple_helper
    {
        using element_type = typename std::tuple_element<Size-Pos, Tuple>::type;
        using next = json_serialize_tuple_helper<Pos-1, Size, Tuple>;

        template <typename CharT,typename Alloc,typename TempAlloc>
        static void try_encode(const allocator_set<Alloc,TempAlloc>& aset, const Tuple& tuple,
            basic_json_visitor<CharT>& encoder, 
            std::error_code& ec)
        {
            encode_traits<element_type>::try_encode(aset, std::get<Size-Pos>(tuple), encoder, ec);
            if (JSONCONS_UNLIKELY(ec)) {return;}
            next::try_encode(aset, tuple, encoder, ec);
        }
    };

    template<size_t Size,typename Tuple>
    struct json_serialize_tuple_helper<0, Size, Tuple>
    {
        template <typename CharT,typename Alloc,typename TempAlloc>
        static void try_encode(const allocator_set<Alloc,TempAlloc>&, const Tuple&,
            basic_json_visitor<CharT>&, 
            std::error_code&)
        {
        }
    };
} // namespace detail


template <typename... E>
struct encode_traits<std::tuple<E...>>
{
    using value_type = std::tuple<E...>;
    static constexpr std::size_t size = sizeof...(E);

    template <typename CharT,typename Alloc,typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>& aset, const value_type& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        using helper = detail::json_serialize_tuple_helper<size, size, std::tuple<E...>>;
        encoder.begin_array(size,semantic_tag::none,ser_context(),ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        helper::try_encode(aset, val, encoder, ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        encoder.end_array(ser_context(),ec);
    }
};

// vector like
template <typename T>
struct encode_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
             ext_traits::is_array_like_with_size<T>::value &&
             !ext_traits::is_typed_array<T>::value 
>::type>
{
    using value_type = typename T::value_type;

    template <typename CharT,typename Alloc,typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>& aset, const T& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.begin_array(val.size(),semantic_tag::none,ser_context(),ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            encode_traits<value_type>::try_encode(aset, *it, encoder, ec);
            if (JSONCONS_UNLIKELY(ec)) {return;}
        }
        encoder.end_array(ser_context(), ec);
    }
};
template <typename T>
struct encode_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
             ext_traits::is_array_like_without_size<T>::value &&
             !ext_traits::is_typed_array<T>::value 
>::type>
{
    using value_type = typename T::value_type;

    template <typename CharT,typename Alloc,typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>& aset, const T& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.begin_array(std::distance(val.begin(), val.end()), semantic_tag::none,ser_context(),ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            encode_traits<value_type>::try_encode(aset, *it, encoder, ec);
            if (JSONCONS_UNLIKELY(ec)) {return;}
        }
        encoder.end_array(ser_context(), ec);
    }
};

template <typename T>
struct encode_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
             ext_traits::is_array_like<T>::value &&
             ext_traits::is_typed_array<T>::value 
>::type>
{
    using value_type = typename T::value_type;

    template <typename CharT,typename Alloc,typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>&, const T& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.typed_array(jsoncons::span<const value_type>(val), semantic_tag::none, ser_context(), ec);
    }
};

// std::array

template <typename T, std::size_t N>
struct encode_traits<std::array<T,N>>
{
    using value_type = typename std::array<T,N>::value_type;

    template <typename CharT,typename Alloc,typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>& aset, const std::array<T, N>& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.begin_array(val.size(),semantic_tag::none,ser_context(),ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            encode_traits<value_type>::try_encode(aset, *it, encoder, ec);
            if (JSONCONS_UNLIKELY(ec)) {return;}
        }
        encoder.end_array(ser_context(),ec);
    }
};

// map like

template <typename T>
struct encode_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                            ext_traits::is_map_like<T>::value &&
                            ext_traits::is_constructible_from_const_pointer_and_size<typename T::key_type>::value
>::type>
{
    using mapped_type = typename T::mapped_type;
    using value_type = typename T::value_type;
    using key_type = typename T::key_type;

    template <typename CharT,typename Alloc,typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>& aset, const T& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.begin_object(val.size(), semantic_tag::none, ser_context(), ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            encoder.key((*it).first);
            encode_traits<mapped_type>::try_encode(aset, (*it).second, encoder, ec);
            if (JSONCONS_UNLIKELY(ec)) {return;}
        }
        encoder.end_object(ser_context(), ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
    }
};

template <typename T>
struct encode_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                            ext_traits::is_map_like<T>::value &&
                            std::is_integral<typename T::key_type>::value
>::type>
{
    using mapped_type = typename T::mapped_type;
    using value_type = typename T::value_type;
    using key_type = typename T::key_type;

    template <typename CharT,typename Alloc,typename TempAlloc>
    static void try_encode(const allocator_set<Alloc,TempAlloc>& aset, const T& val, 
        basic_json_visitor<CharT>& encoder, 
        std::error_code& ec)
    {
        encoder.begin_object(val.size(), semantic_tag::none, ser_context(), ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            std::basic_string<CharT> s;
            jsoncons::utility::from_integer((*it).first,s);
            encoder.key(s);
            encode_traits<mapped_type>::try_encode(aset, (*it).second, encoder, ec);
            if (JSONCONS_UNLIKELY(ec)) {return;}
        }
        encoder.end_object(ser_context(), ec);
        if (JSONCONS_UNLIKELY(ec)) {return;}
    }
};

} // namespace reflect
} // namespace jsoncons

#endif // JSONCONS_REFLECT_ENCODE_TRAITS_HPP

