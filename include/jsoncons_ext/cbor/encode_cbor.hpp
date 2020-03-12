// Copyright 2017 Daniel Parkerstd
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_ENCODE_CBOR_HPP
#define JSONCONS_CBOR_ENCODE_CBOR_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/ser_traits.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>

namespace jsoncons { 
namespace cbor {

    // to bytes 

    template<class T>
    void encode_cbor(const T& j, std::vector<uint8_t>& v)
    {
        encode_cbor(j,v,cbor_encode_options());
    }

    template<class T>
    typename std::enable_if<is_basic_json_class<T>::value,void>::type 
    encode_cbor(const T& j, std::vector<uint8_t>& v, const cbor_encode_options& options)
    {
        typedef typename T::char_type char_type;
        cbor_bytes_encoder encoder(v, options);
        auto adaptor = make_json_content_handler_adaptor<basic_json_content_handler<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T>
    typename std::enable_if<!is_basic_json_class<T>::value, void>::type
    encode_cbor(const T& val, std::vector<uint8_t>& v, const cbor_encode_options& options)
    {
        std::error_code ec;
        encode_cbor(val, v, options, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    template<class T>
    typename std::enable_if<!is_basic_json_class<T>::value, void>::type
    encode_cbor(const T& val, std::vector<uint8_t>& v, const cbor_encode_options& options, std::error_code& ec)
    {
        cbor_bytes_encoder encoder(v, options);
        ser_traits<T>::serialize(val, encoder, json(), ec);
    }

    // stream

    template<class T>
    void encode_cbor(const T& j, std::ostream& os)
    {
        encode_cbor(j,os,cbor_encode_options());
    }

    template<class T>
    typename std::enable_if<is_basic_json_class<T>::value,void>::type 
    encode_cbor(const T& j, std::ostream& os, const cbor_encode_options& options)
    {
        typedef typename T::char_type char_type;
        cbor_stream_encoder encoder(os, options);
        auto adaptor = make_json_content_handler_adaptor<basic_json_content_handler<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T>
    typename std::enable_if<!is_basic_json_class<T>::value,void>::type 
    encode_cbor(const T& val, std::ostream& os, const cbor_encode_options& options)
    {
        std::error_code ec;
        encode_cbor(val, os, options, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    template<class T>
    typename std::enable_if<!is_basic_json_class<T>::value,void>::type 
    encode_cbor(const T& val, 
                std::ostream& os, 
                const cbor_encode_options& options, 
                std::error_code& ec)
    {
        cbor_stream_encoder encoder(os, options);
        ser_traits<T>::serialize(val, encoder, json(), ec);
    }

    // temp_allocator_arg

    // to bytes 

    template<class T,class TempAllocator>
    void encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                     const T& j, 
                     std::vector<uint8_t>& v)
    {
        encode_cbor(temp_allocator_arg, temp_alloc, j,v,cbor_encode_options());
    }

    template<class T,class TempAllocator>
    typename std::enable_if<is_basic_json_class<T>::value,void>::type 
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& j, 
                std::vector<uint8_t>& v, 
                const cbor_encode_options& options)
    {
        typedef typename T::char_type char_type;
        cbor_bytes_encoder encoder(v, options);
        auto adaptor = make_json_content_handler_adaptor<basic_json_content_handler<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!is_basic_json_class<T>::value, void>::type
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val, 
                std::vector<uint8_t>& v, 
                const cbor_encode_options& options)
    {
        std::error_code ec;
        encode_cbor(temp_allocator_arg, temp_alloc, val, v, options, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!is_basic_json_class<T>::value, void>::type
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val, 
                std::vector<uint8_t>& v, 
                const cbor_encode_options& options, 
                std::error_code& ec)
    {
        cbor_bytes_encoder encoder(v, options);
        ser_traits<T>::serialize(val, encoder, json(), ec);
    }

    // stream

    template<class T,class TempAllocator>
    void encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                     const T& j, std::ostream& os)
    {
        encode_cbor(temp_allocator_arg, temp_alloc, j,os,cbor_encode_options());
    }

    template<class T,class TempAllocator>
    typename std::enable_if<is_basic_json_class<T>::value,void>::type 
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& j, std::ostream& os, const cbor_encode_options& options)
    {
        typedef typename T::char_type char_type;
        cbor_stream_encoder encoder(os, options);
        auto adaptor = make_json_content_handler_adaptor<basic_json_content_handler<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!is_basic_json_class<T>::value,void>::type 
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val, std::ostream& os, const cbor_encode_options& options)
    {
        std::error_code ec;
        encode_cbor(temp_allocator_arg, temp_alloc, val, os, options, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!is_basic_json_class<T>::value,void>::type 
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val, 
                std::ostream& os, 
                const cbor_encode_options& options, 
                std::error_code& ec)
    {
        cbor_stream_encoder encoder(os, options);
        ser_traits<T>::serialize(val, encoder, json(), ec);
    }

} // namespace cbor
} // namespace jsoncons

#endif
