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
#include <jsoncons/ser_utils.hpp>
#include <jsoncons_ext/toon/toon_reader.hpp>
#include <jsoncons_ext/toon/toon_options.hpp>

namespace jsoncons {
namespace toon {

// try_decode_toon

template <typename T,typename StringViewLike>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_sequence_of<StringViewLike,typename T::char_type>::value,read_result<T>>::type
try_decode_toon(const StringViewLike& s,
    const toon_decode_options& options = toon_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    json_decoder<T> decoder;
    toon_string_reader reader(s, decoder, options);
    std::error_code ec;
    reader.read(ec);
    if (ec)
    {
        return result_type{jsoncons::unexpect, ec};
    }

    return result_type{decoder.get_result()};
}

template <typename T,typename StringViewLike>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
    ext_traits::is_string_view_like<StringViewLike>::value,read_result<T>>::type
try_decode_toon(const StringViewLike& s,
    const toon_decode_options& options = toon_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T>
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_toon(std::istream& is,
    const toon_decode_options& options = toon_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   
    jsoncons::json_decoder<T> decoder;
    toon_stream_reader reader(is, decoder, options);

    reader.read(ec);
    if (ec)
    {
        return result_type{jsoncons::unexpect, ec};
    }

    return result_type{decoder.get_result()};
}

template <typename T>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_toon(std::istream& is,
    const toon_decode_options& options = toon_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}
// With leading allocator_set parameter

template <typename T,typename StringViewLike,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_sequence_of<StringViewLike,char>::value,read_result<T>>::type
try_decode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    const StringViewLike& s,
    const toon_decode_options& options = toon_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T,typename StringViewLike,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
     ext_traits::is_string_view_like<StringViewLike>::value,read_result<T>>::type
try_decode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    const StringViewLike& s,
    const toon_decode_options& options = toon_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    std::istream& is,
    const toon_decode_options& options = toon_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    return result_type{};
}

template <typename T,typename Alloc,typename TempAlloc>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    std::istream& is,
    const toon_decode_options& options = toon_decode_options())
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


