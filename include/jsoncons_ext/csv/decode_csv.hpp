/// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CSV_DECODE_CSV_HPP
#define JSONCONS_EXT_CSV_DECODE_CSV_HPP

#include <istream>
#include <type_traits>

#include <jsoncons/allocator_set.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/reflect/decode_traits.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/source.hpp>

#include <jsoncons_ext/csv/csv_cursor.hpp>
#include <jsoncons_ext/csv/csv_encoder.hpp>
#include <jsoncons_ext/csv/csv_options.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>

namespace jsoncons { 
namespace csv {

template <typename T,typename CharsLike>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_sequence_of<CharsLike,typename T::char_type>::value,read_result<T>>::type 
try_decode_csv(const CharsLike& s, const basic_csv_decode_options<typename CharsLike::value_type>& options = basic_csv_decode_options<typename CharsLike::value_type>())
{
    using char_type = typename CharsLike::value_type;
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   

    json_decoder<T> decoder;

    basic_csv_reader<char_type,jsoncons::string_source<char_type>> reader(s,decoder,options);
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, reader.line(), reader.column()};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type{jsoncons::unexpect, conv_errc::conversion_failed, reader.line(), reader.column()};
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename CharsLike>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_char_sequence<CharsLike>::value,read_result<T>>::type 
try_decode_csv(const CharsLike& s, const basic_csv_decode_options<typename CharsLike::value_type>& options = basic_csv_decode_options<typename CharsLike::value_type>())
{
    using char_type = typename CharsLike::value_type;
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   

    basic_csv_cursor<char_type> cursor(s, options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(make_alloc_set(), cursor);
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type 
try_decode_csv(std::basic_istream<CharT>& is, const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
{
    using char_type = CharT;
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   

    json_decoder<T> decoder;

    basic_csv_reader<char_type,jsoncons::stream_source<char_type>> reader(is,decoder,options);
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, reader.line(), reader.column()};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type{jsoncons::unexpect, conv_errc::conversion_failed, reader.line(), reader.column()};
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename CharT>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type 
try_decode_csv(std::basic_istream<CharT>& is, const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   
    basic_csv_cursor<CharT> cursor(is, options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(make_alloc_set(), cursor);
}

template <typename T,typename InputIt>
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_csv(InputIt first, InputIt last,
            const basic_csv_decode_options<typename std::iterator_traits<InputIt>::value_type>& options = 
                basic_csv_decode_options<typename std::iterator_traits<InputIt>::value_type>())
{
    using char_type = typename std::iterator_traits<InputIt>::value_type;
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   

    jsoncons::json_decoder<T> decoder;
    basic_csv_reader<char_type, iterator_source<InputIt>> reader(iterator_source<InputIt>(first,last), decoder, options);
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, reader.line(), reader.column()};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type{jsoncons::unexpect, conv_errc::conversion_failed, reader.line(), reader.column()};
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename InputIt>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type
try_decode_csv(InputIt first, InputIt last,
           const basic_csv_decode_options<typename std::iterator_traits<InputIt>::value_type>& options = 
                basic_csv_decode_options<typename std::iterator_traits<InputIt>::value_type>())
{
    using char_type = typename std::iterator_traits<InputIt>::value_type;
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   

    basic_csv_cursor<char_type,iterator_source<InputIt>> cursor(iterator_source<InputIt>(first, last), options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(make_alloc_set(), cursor);
}

// With leading allocator_set parameter

template <typename T,typename CharsLike,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_sequence_of<CharsLike,typename T::char_type>::value,read_result<T>>::type 
try_decode_csv(const allocator_set<Alloc,TempAlloc>& aset,
           const CharsLike& s, 
           const basic_csv_decode_options<typename CharsLike::value_type>& options = basic_csv_decode_options<typename CharsLike::value_type>())
{
    using char_type = typename CharsLike::value_type;
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   

    json_decoder<T,TempAlloc> decoder(aset.get_allocator(), aset.get_temp_allocator());

    basic_csv_reader<char_type,jsoncons::string_source<char_type>,TempAlloc> reader(s,decoder,options,aset.get_temp_allocator());
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, reader.line(), reader.column()};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type{jsoncons::unexpect, conv_errc::conversion_failed, reader.line(), reader.column()};
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename CharsLike,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_char_sequence<CharsLike>::value,read_result<T>>::type 
try_decode_csv(const allocator_set<Alloc,TempAlloc>& aset,
           const CharsLike& s, 
           const basic_csv_decode_options<typename CharsLike::value_type>& options = basic_csv_decode_options<typename CharsLike::value_type>())
{
    using char_type = typename CharsLike::value_type;
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   

    basic_csv_cursor<char_type,string_source<char_type>,TempAlloc> cursor(
        std::allocator_arg, aset.get_temp_allocator(), s, options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(aset, cursor);
}

template <typename T,typename CharT,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type 
try_decode_csv(const allocator_set<Alloc,TempAlloc>& aset,
    std::basic_istream<CharT>& is, 
    const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
{
    using char_type = CharT;
    using value_type = T;
    using result_type = read_result<value_type>;
    using char_allocator_type = typename std::allocator_traits<TempAlloc>:: template rebind_alloc<char_type>;
    using stream_source_type = stream_source<char_type,char_allocator_type>;

    std::error_code ec;   

    json_decoder<T,TempAlloc> decoder(aset.get_allocator(), aset.get_temp_allocator());

    basic_csv_reader<char_type,stream_source_type,TempAlloc> reader(stream_source_type(is,aset.get_temp_allocator()),
        decoder, options, aset.get_temp_allocator());
    reader.read(ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, reader.line(), reader.column()};
    }
    if (JSONCONS_UNLIKELY(!decoder.is_valid()))
    {
        return result_type{jsoncons::unexpect, conv_errc::conversion_failed, reader.line(), reader.column()};
    }
    return result_type{decoder.get_result()};
}

template <typename T,typename CharT,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type 
try_decode_csv(const allocator_set<Alloc,TempAlloc>& aset,
           std::basic_istream<CharT>& is, 
           const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   
    basic_csv_cursor<CharT,stream_source<CharT>,TempAlloc> cursor(
        std::allocator_arg, aset.get_temp_allocator(), is, options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(aset, cursor);
}

template <typename T, typename... Args>
T decode_csv(Args&& ... args)
{
    auto result = try_decode_csv<T>(std::forward<Args>(args)...); 
    if (!result)
    {
        JSONCONS_THROW(ser_error(result.error().code(), result.error().line(), result.error().column()));
    }
    return std::move(*result);
}

} // namespace csv 
} // namespace jsoncons

#endif // JSONCONS_EXT_CSV_DECODE_CSV_HPP
