// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_TOON_DECODE_TOON_HPP
#define JSONCONS_TOON_DECODE_TOON_HPP

#include <istream> // std::basic_istream
#include <tuple>
#include <type_traits>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/allocator_set.hpp>
#include <jsoncons/conv_error.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons_ext/toon/toon_reader.hpp>
#include <jsoncons_ext/toon/toon_options.hpp>

namespace jsoncons {
namespace toon {

// try_decode_toon

template <typename T,typename CharsLike>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_sequence_of<CharsLike,typename T::char_type>::value,read_result<T>>::type
try_decode_toon(const CharsLike& s,
    const basic_json_decode_options<typename CharsLike::value_type>& options = basic_json_decode_options<typename CharsLike::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    (s);
    (options);

    return result_type{};
}

template <typename T,typename CharsLike>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
    ext_traits::is_char_sequence<CharsLike>::value,read_result<T>>::type
try_decode_toon(const CharsLike& s,
    const basic_json_decode_options<typename CharsLike::value_type>& options = basic_json_decode_options<typename CharsLike::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_toon(std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T,typename CharT>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_toon(std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}
// With leading allocator_set parameter

template <typename T,typename CharsLike,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_sequence_of<CharsLike,typename T::char_type>::value,read_result<T>>::type
try_decode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    const CharsLike& s,
    const basic_json_decode_options<typename CharsLike::value_type>& options = basic_json_decode_options<typename CharsLike::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T,typename CharsLike,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
     ext_traits::is_char_sequence<CharsLike>::value,read_result<T>>::type
try_decode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    const CharsLike& s,
    const basic_json_decode_options<typename CharsLike::value_type>& options = basic_json_decode_options<typename CharsLike::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T,typename CharT,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T,typename CharT,typename Alloc,typename TempAlloc>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T, typename... Args>
T decode_toon(Args&& ... args)
{
    auto result = try_decode_toon<T>(std::forward<Args>(args)...); 
    if (!result)
    {
        JSONCONS_THROW(ser_error(result.error().code(), result.error().message_arg(), result.error().line(), result.error().column()));
    }
    return std::move(*result);
}

} // namespace toon
} // namespace jsoncons

#endif // JSONCONS_TOON_DECODE_TOON_HPP


