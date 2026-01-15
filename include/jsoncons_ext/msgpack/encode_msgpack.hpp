// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_MSGPACK_ENCODE_MSGPACK_HPP
#define JSONCONS_EXT_MSGPACK_ENCODE_MSGPACK_HPP

#include <ostream> // std::basic_ostream
#include <system_error> 
#include <type_traits> 

#include <jsoncons/allocator_set.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/reflect/encode_traits.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/sink.hpp>
#include <jsoncons/utility/more_type_traits.hpp>

#include <jsoncons_ext/msgpack/msgpack_encoder.hpp>
#include <jsoncons_ext/msgpack/msgpack_options.hpp>

namespace jsoncons { 
namespace msgpack {

    template <typename T,typename ByteContainer>
    typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_byte_container<ByteContainer>::value,write_result>::type 
    try_encode_msgpack(const T& j, 
        ByteContainer& cont, 
        const msgpack_encode_options& options = msgpack_encode_options())
    {
        using char_type = typename T::char_type;
        basic_msgpack_encoder<jsoncons::bytes_sink<ByteContainer>> encoder(cont, options);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        return j.try_dump(adaptor);
    }

    template <typename T,typename ByteContainer>
    typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_byte_container<ByteContainer>::value,write_result>::type 
    try_encode_msgpack(const T& val, 
        ByteContainer& cont, 
        const msgpack_encode_options& options = msgpack_encode_options())
    {
        basic_msgpack_encoder<jsoncons::bytes_sink<ByteContainer>> encoder(cont, options);
        return reflect::encode_traits<T>::try_encode(make_alloc_set(), val, encoder);
    }

    template <typename T>
    typename std::enable_if<ext_traits::is_basic_json<T>::value,write_result>::type 
    try_encode_msgpack(const T& j, 
        std::ostream& os, 
        const msgpack_encode_options& options = msgpack_encode_options())
    {
        using char_type = typename T::char_type;
        msgpack_stream_encoder encoder(os, options);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        return j.try_dump(adaptor);
    }

    template <typename T>
    typename std::enable_if<!ext_traits::is_basic_json<T>::value,write_result>::type 
    try_encode_msgpack(const T& val, 
        std::ostream& os, 
        const msgpack_encode_options& options = msgpack_encode_options())
    {
        msgpack_stream_encoder encoder(os, options);
        return reflect::encode_traits<T>::try_encode(make_alloc_set(), val, encoder);
    }

    // with temp_allocator_arg_t

    template <typename T,typename ByteContainer,typename Alloc,typename TempAlloc >
    typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_byte_container<ByteContainer>::value,write_result>::type 
    try_encode_msgpack(const allocator_set<Alloc,TempAlloc>& aset, const T& j, 
        ByteContainer& cont, 
        const msgpack_encode_options& options = msgpack_encode_options())
    {
        using char_type = typename T::char_type;
        basic_msgpack_encoder<jsoncons::bytes_sink<ByteContainer>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        return j.try_dump(adaptor);
    }

    template <typename T,typename ByteContainer,typename Alloc,typename TempAlloc >
    typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_byte_container<ByteContainer>::value,write_result>::type 
    try_encode_msgpack(const allocator_set<Alloc,TempAlloc>& aset, 
        const T& val, ByteContainer& cont, 
        const msgpack_encode_options& options = msgpack_encode_options())
    {
        basic_msgpack_encoder<jsoncons::bytes_sink<ByteContainer>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
        return reflect::encode_traits<T>::try_encode(aset, val, encoder);
    }

    template <typename T,typename Alloc,typename TempAlloc >
    typename std::enable_if<ext_traits::is_basic_json<T>::value,write_result>::type 
    try_encode_msgpack(const allocator_set<Alloc,TempAlloc>& aset, 
        const T& j, 
        std::ostream& os, 
        const msgpack_encode_options& options = msgpack_encode_options())
    {
        using char_type = typename T::char_type;
        basic_msgpack_encoder<jsoncons::binary_stream_sink,TempAlloc> encoder(os, options, aset.get_temp_allocator());
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        return j.try_dump(adaptor);
    }

    template <typename T,typename Alloc,typename TempAlloc >
    typename std::enable_if<!ext_traits::is_basic_json<T>::value,write_result>::type 
    try_encode_msgpack(const allocator_set<Alloc,TempAlloc>& aset, 
                   const T& val, 
                   std::ostream& os, 
                   const msgpack_encode_options& options = msgpack_encode_options())
    {
        basic_msgpack_encoder<jsoncons::binary_stream_sink,TempAlloc> encoder(os, options, aset.get_temp_allocator());
        return reflect::encode_traits<T>::try_encode(aset, val, encoder);
    }

template <typename... Args>
void encode_msgpack(Args&& ... args)
{
    auto r = try_encode_msgpack(std::forward<Args>(args)...); 
    if (!r)
    {
        JSONCONS_THROW(ser_error(r.error()));
    }
}

} // namespace msgpack
} // namespace jsoncons

#endif // JSONCONS_EXT_MSGPACK_ENCODE_MSGPACK_HPP
