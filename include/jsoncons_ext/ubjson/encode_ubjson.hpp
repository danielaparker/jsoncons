// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_UBJSON_ENCODE_UBJSON_HPP
#define JSONCONS_EXT_UBJSON_ENCODE_UBJSON_HPP

#include <ostream> // std::basic_ostream
#include <type_traits> // std::enable_if

#include <jsoncons/basic_json.hpp>
#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/reflect/encode_traits.hpp>
#include <jsoncons/sink.hpp>
#include <jsoncons/utility/more_type_traits.hpp>

#include <jsoncons_ext/ubjson/ubjson_encoder.hpp>

namespace jsoncons { 
namespace ubjson {

template <typename T,typename ByteContainer>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_byte_container<ByteContainer>::value,write_result>::type 
try_encode_ubjson(const T& j, 
    ByteContainer& cont, 
    const ubjson_encode_options& options = ubjson_encode_options())
{
    using char_type = typename T::char_type;
    basic_ubjson_encoder<jsoncons::bytes_sink<ByteContainer>> encoder(cont, options);
    auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
    return j.try_dump(adaptor);
}

template <typename T,typename ByteContainer>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_byte_container<ByteContainer>::value,write_result>::type 
try_encode_ubjson(const T& val, 
    ByteContainer& cont, 
    const ubjson_encode_options& options = ubjson_encode_options())
{
    basic_ubjson_encoder<jsoncons::bytes_sink<ByteContainer>> encoder(cont, options);
    return reflect::encode_traits<T>::try_encode(make_alloc_set(), val, encoder);
}

template <typename T>
typename std::enable_if<ext_traits::is_basic_json<T>::value,write_result>::type 
try_encode_ubjson(const T& j, 
    std::ostream& os, 
    const ubjson_encode_options& options = ubjson_encode_options())
{
    using char_type = typename T::char_type;
    ubjson_stream_encoder encoder(os, options);
    auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
    return j.try_dump(adaptor);
}

template <typename T>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,write_result>::type 
try_encode_ubjson(const T& val, 
    std::ostream& os, 
    const ubjson_encode_options& options = ubjson_encode_options())
{
    ubjson_stream_encoder encoder(os, options);
    return reflect::encode_traits<T>::try_encode(make_alloc_set(), val, encoder);
}

// with temp_allocator_arg_t

template <typename T,typename ByteContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_byte_container<ByteContainer>::value,write_result>::type 
try_encode_ubjson(const allocator_set<Alloc,TempAlloc>& aset,const T& j, 
    ByteContainer& cont, 
    const ubjson_encode_options& options = ubjson_encode_options())
{
    using char_type = typename T::char_type;
    basic_ubjson_encoder<jsoncons::bytes_sink<ByteContainer>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
    auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
    return j.try_dump(adaptor);
}

template <typename T,typename ByteContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_byte_container<ByteContainer>::value,write_result>::type 
try_encode_ubjson(const allocator_set<Alloc,TempAlloc>& aset,const T& val, 
    ByteContainer& cont, 
    const ubjson_encode_options& options = ubjson_encode_options())
{
    basic_ubjson_encoder<jsoncons::bytes_sink<ByteContainer>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
    return reflect::encode_traits<T>::try_encode(aset, val, encoder);
}

template <typename T,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value,write_result>::type 
try_encode_ubjson(const allocator_set<Alloc,TempAlloc>& aset,
    const T& j, 
    std::ostream& os, 
    const ubjson_encode_options& options = ubjson_encode_options())
{
    using char_type = typename T::char_type;
    basic_ubjson_encoder<jsoncons::binary_stream_sink,TempAlloc> encoder(os, options, aset.get_temp_allocator());
    auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
    return j.try_dump(adaptor);
}

template <typename T,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value,write_result>::type 
try_encode_ubjson(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, 
    std::ostream& os, 
    const ubjson_encode_options& options = ubjson_encode_options())
{
    basic_ubjson_encoder<jsoncons::binary_stream_sink,TempAlloc> encoder(os, options, aset.get_temp_allocator());
    return reflect::encode_traits<T>::try_encode(aset, val, encoder);
}

template <typename... Args>
void encode_ubjson(Args&& ... args)
{
    auto r = try_encode_ubjson(std::forward<Args>(args)...); 
    if (!r)
    {
        JSONCONS_THROW(ser_error(r.error()));
    }
}

} // namespace ubjson
} // namespace jsoncons

#endif // JSONCONS_EXT_UBJSON_ENCODE_UBJSON_HPP
