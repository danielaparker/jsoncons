// Copyright 2017-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_ENCODE_CBOR_HPP
#define JSONCONS_EXT_CBOR_ENCODE_CBOR_HPP

#include <ostream> // std::basic_ostream
#include <type_traits> // std::enable_if

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/reflect/encode_traits.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/write_result.hpp>

#include <jsoncons_ext/cbor/cbor_encoder.hpp>

namespace jsoncons { 
namespace cbor {

// to bytes 

template <typename T,typename ByteContainer>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_byte_container<ByteContainer>::value,void>::type 
try_encode_cbor(const T& j, 
            ByteContainer& cont, 
            const cbor_encode_options& options = cbor_encode_options())
{
    using char_type = typename T::char_type;
    basic_cbor_encoder<jsoncons::bytes_sink<ByteContainer>> encoder(cont, options);
    auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
    j.dump(adaptor);
}

template <typename T,typename ByteContainer>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_byte_container<ByteContainer>::value,void>::type 
try_encode_cbor(const T& val, ByteContainer& cont, 
            const cbor_encode_options& options = cbor_encode_options())
{
    basic_cbor_encoder<jsoncons::bytes_sink<ByteContainer>> encoder(cont, options);
    std::error_code ec;
    reflect::encode_traits<T>::try_encode(make_alloc_set(), val, encoder, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        JSONCONS_THROW(ser_error(ec));
    }
}

// stream

template <typename T>
typename std::enable_if<ext_traits::is_basic_json<T>::value,void>::type 
try_encode_cbor(const T& j, 
            std::ostream& os, 
            const cbor_encode_options& options = cbor_encode_options())
{
    using char_type = typename T::char_type;
    cbor_stream_encoder encoder(os, options);
    auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
    j.dump(adaptor);
}

template <typename T>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,void>::type 
try_encode_cbor(const T& val, 
            std::ostream& os, 
            const cbor_encode_options& options = cbor_encode_options())
{
    cbor_stream_encoder encoder(os, options);
    std::error_code ec;
    reflect::encode_traits<T>::try_encode(make_alloc_set(), val, encoder, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        JSONCONS_THROW(ser_error(ec));
    }
}

// temp_allocator_arg

// to bytes 

template <typename T,typename ByteContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_byte_container<ByteContainer>::value,void>::type 
try_encode_cbor(const allocator_set<Alloc,TempAlloc>& aset,
            const T& j, 
            ByteContainer& cont, 
            const cbor_encode_options& options = cbor_encode_options())
{
    using char_type = typename T::char_type;
    basic_cbor_encoder<bytes_sink<ByteContainer>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
    auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
    j.dump(adaptor);
}

template <typename T,typename ByteContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_byte_container<ByteContainer>::value,void>::type 
try_encode_cbor(const allocator_set<Alloc,TempAlloc>& aset,
            const T& val, 
            ByteContainer& cont, 
            const cbor_encode_options& options = cbor_encode_options())
{
    basic_cbor_encoder<jsoncons::bytes_sink<ByteContainer>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
    std::error_code ec;
    reflect::encode_traits<T>::try_encode(aset, val, encoder, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        JSONCONS_THROW(ser_error(ec));
    }
}

// stream

template <typename T,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value,void>::type 
try_encode_cbor(const allocator_set<Alloc,TempAlloc>& aset,
            const T& j, 
            std::ostream& os, 
            const cbor_encode_options& options = cbor_encode_options())
{
    using char_type = typename T::char_type;
    basic_cbor_encoder<binary_stream_sink,TempAlloc> encoder(os, options, aset.get_temp_allocator());
    auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
    j.dump(adaptor);
}

template <typename T,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value,void>::type 
try_encode_cbor(const allocator_set<Alloc,TempAlloc>& aset,
            const T& val, 
            std::ostream& os, 
            const cbor_encode_options& options = cbor_encode_options())
{
    basic_cbor_encoder<binary_stream_sink,TempAlloc> encoder(os, options, aset.get_temp_allocator());
    std::error_code ec;
    reflect::encode_traits<T>::try_encode(aset, val, encoder, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        JSONCONS_THROW(ser_error(ec));
    }
}

template <typename... Args>
void encode_cbor(Args&& ... args)
{
    /*auto result = */try_encode_cbor(std::forward<Args>(args)...); 
    /*if (!result)
    {
        JSONCONS_THROW(ser_error(result.error().code(), result.error().line(), result.error().column()));
    }*/
    //return std::move(*result);
}

} // namespace cbor
} // namespace jsoncons

#endif
