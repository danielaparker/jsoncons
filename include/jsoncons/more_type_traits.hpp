// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MORE_TYPE_TRAITS_HPP
#define JSONCONS_MORE_TYPE_TRAITS_HPP

#include <jsoncons/detail/more_type_traits.hpp>

namespace jsoncons {

    template <class T>
    using is_bool = jsoncons::detail::is_bool<T>;

    template <class T>
    using is_u8_u16_u32_or_u64 = jsoncons::detail::is_u8_u16_u32_or_u64<T>;

    template <class T>
    using is_i8_i16_i32_or_i64 = jsoncons::detail::is_i8_i16_i32_or_i64<T>;

    template <class T>
    using is_float_or_double = jsoncons::detail::is_float_or_double<T>;

    template <class T>
    using is_character = jsoncons::detail::is_character<T>;

    template <class T>
    using is_narrow_character = jsoncons::detail::is_narrow_character<T>;

    template <class T>
    using is_wide_character = jsoncons::detail::is_wide_character<T>;

    template <class T>
    using is_primitive = jsoncons::detail::is_primitive<T>;

    template <class T>
    using is_integer = jsoncons::detail::is_integer<T>;

    template <class T>
    using is_signed_integer = jsoncons::detail::is_signed_integer<T>;

    template <class T>
    using is_unsigned_integer = jsoncons::detail::is_unsigned_integer<T>;

    template <class Container>
    using is_list_like = jsoncons::detail::is_list_like<Container>;

    template <class Container>
    using is_map_like = jsoncons::detail::is_map_like<Container>;

    template <class Container>
    using is_constructible_from_const_pointer_and_size = jsoncons::detail::is_constructible_from_const_pointer_and_size<Container>;

    template <class Container>
    using has_push_back = jsoncons::detail::has_push_back<Container>;

    template <class Ret,class Container>
    using has_data_exact = jsoncons::detail::has_data_exact<Ret,Container>;

    template <class Container>
    using has_reserve = jsoncons::detail::has_reserve<Container>;

    template <class Container>
    using is_typed_array = jsoncons::detail::is_typed_array<Container>;

    template <class Container>
    using is_bytes = jsoncons::detail::is_bytes<Container>;

    template <class Container>
    using is_string = jsoncons::detail::is_string<Container>;

    template <class Container>
    using is_string_view = jsoncons::detail::is_string_view<Container>;

} // namespace jsoncons

#endif

