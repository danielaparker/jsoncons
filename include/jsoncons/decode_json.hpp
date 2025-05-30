// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DECODE_JSON_HPP
#define JSONCONS_DECODE_JSON_HPP

#include <iostream>
#include <istream> // std::basic_istream
#include <tuple>
#include <type_traits>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/allocator_set.hpp>
#include <jsoncons/conv_error.hpp>
#include <jsoncons/json_cursor.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/read_result.hpp>
#include <jsoncons/reflect/decode_traits.hpp>

namespace jsoncons {

// try_decode_json

template <typename T,typename Source>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_sequence_of<Source,typename T::char_type>::value,read_result<T>>::type
try_decode_json(const Source& s,
    const basic_json_decode_options<typename Source::value_type>& options = basic_json_decode_options<typename Source::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;
    using char_type = typename Source::value_type;

    std::error_code ec;   
    jsoncons::json_decoder<T> decoder;
    basic_json_reader<char_type, string_source<char_type>> reader(s, decoder, options);
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, reader.line(), reader.column()}};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type{ read_error{conv_errc::conversion_failed, reader.line(), reader.column()} };
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename Source>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_char_sequence<Source>::value,read_result<T>>::type
try_decode_json(const Source& s,
    const basic_json_decode_options<typename Source::value_type>& options = basic_json_decode_options<typename Source::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;
    using char_type = typename Source::value_type;

    std::error_code ec;
    basic_json_cursor<char_type,string_source<char_type>> cursor(s, options, default_json_parsing(), ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, cursor.line(), cursor.column()}};
    }
    return reflect::decode_traits<T>::try_decode(cursor);
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_json(std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   
    jsoncons::json_decoder<T> decoder;
    basic_json_reader<CharT, stream_source<CharT>> reader(is, decoder, options);
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, reader.line(), reader.column()}};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type(read_error(conv_errc::conversion_failed, reader.line(), reader.column()));
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename CharT>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_json(std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;
    basic_json_cursor<CharT> cursor(is, options, default_json_parsing(), ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, cursor.line(), cursor.column()}};
    }
    return reflect::decode_traits<T>::try_decode(cursor);
}

template <typename T,typename InputIt>
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_json(InputIt first, InputIt last,
    const basic_json_decode_options<typename std::iterator_traits<InputIt>::value_type>& options = 
    basic_json_decode_options<typename std::iterator_traits<InputIt>::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;
    using char_type = typename std::iterator_traits<InputIt>::value_type;

    std::error_code ec;   
    jsoncons::json_decoder<T> decoder;
    basic_json_reader<char_type, iterator_source<InputIt>> reader(iterator_source<InputIt>(first,last), decoder, options);
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, reader.line(), reader.column()}};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type(read_error(conv_errc::conversion_failed, reader.line(), reader.column()));
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename InputIt>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_json(InputIt first, InputIt last,
    const basic_json_decode_options<typename std::iterator_traits<InputIt>::value_type>& options = 
    basic_json_decode_options<typename std::iterator_traits<InputIt>::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;
    using char_type = typename std::iterator_traits<InputIt>::value_type;

    std::error_code ec;
    basic_json_cursor<char_type,iterator_source<InputIt>> cursor(iterator_source<InputIt>(first, last), 
        options, default_json_parsing(), ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, cursor.line(), cursor.column()}};
    }
    return reflect::decode_traits<T>::try_decode(cursor);
}

// With leading allocator_set parameter

template <typename T,typename Source,typename Allocator,typename TempAllocator >
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_sequence_of<Source,typename T::char_type>::value,read_result<T>>::type
try_decode_json(const allocator_set<Allocator,TempAllocator>& alloc_set,
    const Source& s,
    const basic_json_decode_options<typename Source::value_type>& options = basic_json_decode_options<typename Source::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;
    using char_type = typename Source::value_type;

    json_decoder<T,TempAllocator> decoder(alloc_set.get_allocator(), alloc_set.get_temp_allocator());

    std::error_code ec;   
    basic_json_reader<char_type, string_source<char_type>,TempAllocator> reader(s, decoder, options, alloc_set.get_temp_allocator());
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, reader.line(), reader.column()}};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type(read_error(conv_errc::conversion_failed, reader.line(), reader.column()));
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename Source,typename Allocator,typename TempAllocator >
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_char_sequence<Source>::value,read_result<T>>::type
try_decode_json(const allocator_set<Allocator,TempAllocator>& alloc_set,
    const Source& s,
    const basic_json_decode_options<typename Source::value_type>& options = basic_json_decode_options<typename Source::value_type>())
{
    using value_type = T;
    using result_type = read_result<value_type>;
    using char_type = typename Source::value_type;

    std::error_code ec;
    basic_json_cursor<char_type,string_source<char_type>,TempAllocator> cursor(
        std::allocator_arg, alloc_set.get_temp_allocator(), s, options, default_json_parsing(), ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, cursor.line(), cursor.column()}};
    }
    return reflect::decode_traits<T>::try_decode(cursor);
}

template <typename T,typename CharT,typename Allocator,typename TempAllocator >
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_json(const allocator_set<Allocator,TempAllocator>& alloc_set,
    std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    json_decoder<T,TempAllocator> decoder(alloc_set.get_allocator(), alloc_set.get_temp_allocator());

    std::error_code ec;   
    basic_json_reader<CharT, stream_source<CharT>,TempAllocator> reader(is, decoder, options, alloc_set.get_temp_allocator());
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, reader.line(), reader.column()}};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type(read_error(conv_errc::conversion_failed, reader.line(), reader.column()));
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename CharT,typename Allocator,typename TempAllocator >
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_json(const allocator_set<Allocator,TempAllocator>& alloc_set,
    std::basic_istream<CharT>& is,
    const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;
    using char_type = CharT;

    std::error_code ec;   
    basic_json_cursor<char_type,stream_source<char_type>,TempAllocator> cursor(
        std::allocator_arg, alloc_set.get_temp_allocator(), is, options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{read_error{ec, cursor.line(), cursor.column()}};
    }
    return reflect::decode_traits<value_type>::try_decode(cursor);
}

template <typename T, typename... Args>
T decode_json(Args&& ... args)
{
    auto result = try_decode_json<T>(std::forward<Args>(args)...); 
    if (!result)
    {
        JSONCONS_THROW(ser_error(result.error().ec(), result.error().line(), result.error().column()));
    }
    return std::move(*result);
}

} // namespace jsoncons

#endif // JSONCONS_DECODE_JSON_HPP

