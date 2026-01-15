// Copyright 2017-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_DECODE_CBOR_HPP
#define JSONCONS_EXT_CBOR_DECODE_CBOR_HPP

#include <istream> // std::basic_istream
#include <type_traits> // std::enable_if
#include <vector>

#include <jsoncons/allocator_set.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/conv_error.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/reflect/decode_traits.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/source.hpp>

#include <jsoncons_ext/cbor/cbor_cursor.hpp>
#include <jsoncons_ext/cbor/cbor_reader.hpp>

namespace jsoncons { 
namespace cbor {

template <typename T,typename BytesLike>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_byte_sequence<BytesLike>::value,read_result<T>>::type 
try_decode_cbor(const BytesLike& v, 
    const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   
    jsoncons::json_decoder<T> decoder;
    auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
    basic_cbor_reader<jsoncons::bytes_source> reader(v, adaptor, options);
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

template <typename T,typename BytesLike>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_byte_sequence<BytesLike>::value,read_result<T>>::type 
try_decode_cbor(const BytesLike& v, 
    const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;
    basic_cbor_cursor<bytes_source> cursor(v, options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(make_alloc_set(), cursor);
}

template <typename T>
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type 
try_decode_cbor(std::istream& is, 
    const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;   
    jsoncons::json_decoder<T> decoder;
    auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
    cbor_stream_reader reader(is, adaptor, options);
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
 
template <typename T>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type 
try_decode_cbor(std::istream& is, 
    const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;
    basic_cbor_cursor<binary_stream_source> cursor(is, options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(make_alloc_set(), cursor);
}

template <typename T,typename InputIt>
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type 
try_decode_cbor(InputIt first, InputIt last,
    const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;
    jsoncons::json_decoder<T> decoder;
    auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
    basic_cbor_reader<binary_iterator_source<InputIt>> reader(binary_iterator_source<InputIt>(first, last), adaptor, options);
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
try_decode_cbor(InputIt first, InputIt last,
    const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;
    basic_cbor_cursor<binary_iterator_source<InputIt>> cursor(binary_iterator_source<InputIt>(first, last), options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(make_alloc_set(), cursor);
}

// With leading allocator_set parameter

template <typename T,typename BytesLike,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_byte_sequence<BytesLike>::value,read_result<T>>::type 
try_decode_cbor(const allocator_set<Alloc,TempAlloc>& aset,
    const BytesLike& v, 
    const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;
    json_decoder<T,TempAlloc> decoder(aset.get_allocator(), aset.get_temp_allocator());
    auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
    basic_cbor_reader<jsoncons::bytes_source,TempAlloc> reader(v, adaptor, options, aset.get_temp_allocator());
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

template <typename T,typename BytesLike,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_byte_sequence<BytesLike>::value,read_result<T>>::type 
try_decode_cbor(const allocator_set<Alloc,TempAlloc>& aset,
    const BytesLike& v, 
    const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;
    basic_cbor_cursor<bytes_source,TempAlloc> cursor(std::allocator_arg, aset.get_temp_allocator(), v, options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(aset, cursor);
}

template <typename T,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value,read_result<T>>::type 
try_decode_cbor(const allocator_set<Alloc,TempAlloc>& aset,
    std::istream& is, 
    const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;
    using byte_allocator_type = typename std::allocator_traits<TempAlloc>:: template rebind_alloc<uint8_t>;
    using stream_source_type = stream_source<uint8_t,byte_allocator_type>;

    std::error_code ec;
    json_decoder<T,TempAlloc> decoder(aset.get_allocator(), aset.get_temp_allocator());
    auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
    basic_cbor_reader<stream_source_type,TempAlloc> reader(stream_source_type(is,aset.get_temp_allocator()), 
        adaptor, options, aset.get_temp_allocator());
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

template <typename T,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value,read_result<T>>::type 
try_decode_cbor(const allocator_set<Alloc,TempAlloc>& aset,
            std::istream& is, 
            const cbor_decode_options& options = cbor_decode_options())
{
    using value_type = T;
    using result_type = read_result<value_type>;

    std::error_code ec;
    basic_cbor_cursor<binary_stream_source,TempAlloc> cursor(std::allocator_arg, aset.get_temp_allocator(), is, options, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        return result_type{jsoncons::unexpect, ec, cursor.line(), cursor.column()};
    }

    return reflect::decode_traits<T>::try_decode(aset, cursor);
}

template <typename T, typename... Args>
T decode_cbor(Args&& ... args)
{
    auto result = try_decode_cbor<T>(std::forward<Args>(args)...); 
    if (!result)
    {
        JSONCONS_THROW(ser_error(result.error().code(), result.error().line(), result.error().column()));
    }
    return std::move(*result);
}

} // namespace cbor
} // namespace jsoncons

#endif
