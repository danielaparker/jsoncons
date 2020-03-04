// Copyright 2017 Daniel Parkerstd
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_HPP
#define JSONCONS_CBOR_CBOR_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/cbor/cbor_reader.hpp>
#include <jsoncons_ext/cbor/cbor_cursor.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>
#include <jsoncons/ser_traits.hpp>

namespace jsoncons { 
namespace cbor {

    // encode_cbor

    template<class T>
    void encode_cbor(const T& j, std::vector<uint8_t>& v)
    {
        encode_cbor(j,v,cbor_encode_options());
    }

    template<class T>
    void encode_cbor(const T& j, std::ostream& os)
    {
        encode_cbor(j,os,cbor_encode_options());
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

    template<class T>
    typename std::enable_if<!is_basic_json_class<T>::value, void>::type
    encode_cbor(const T& val, std::vector<uint8_t>& v, const cbor_encode_options& options, std::error_code& ec)
    {
        cbor_bytes_encoder encoder(v, options);
        ser_traits<T>::serialize(val, encoder, json(), ec);
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

    // decode_cbor 

    template<class T>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type 
    decode_cbor(const std::vector<uint8_t>& v)
    {
        jsoncons::json_decoder<T> decoder;
        auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
        basic_cbor_reader<jsoncons::bytes_source> reader(v, adaptor);
        reader.read();
        return decoder.get_result();
    }

    template<class T>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
    decode_cbor(const std::vector<uint8_t>& v)
    {
        basic_cbor_cursor<bytes_source> cursor(v);
        json_decoder<basic_json<char,sorted_policy>> decoder();

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template<class T>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type 
    decode_cbor(std::istream& is)
    {
        jsoncons::json_decoder<T> decoder;
        auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
        cbor_stream_reader reader(is, adaptor);
        reader.read();
        return decoder.get_result();
    }
     
    template<class T>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
    decode_cbor(std::istream& is)
    {
        basic_cbor_cursor<binary_stream_source> cursor(is);
        json_decoder<basic_json<char,sorted_policy>> decoder();

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    // With leading allocator parameter

    template<class T,class TempAllocator>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type 
    decode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const std::vector<uint8_t>& v)
    {
        json_decoder<T,TempAllocator> decoder(temp_alloc);
        auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
        basic_cbor_reader<jsoncons::bytes_source,TempAllocator> reader(v, adaptor, temp_alloc);
        reader.read();
        return decoder.get_result();
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
    decode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const std::vector<uint8_t>& v)
    {
        basic_cbor_cursor<bytes_source,TempAllocator> cursor(v, temp_alloc);
        json_decoder<basic_json<char,sorted_policy,TempAllocator>,TempAllocator> decoder(temp_alloc, temp_alloc);

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template<class T,class TempAllocator>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type 
    decode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                std::istream& is)
    {
        json_decoder<T,TempAllocator> decoder(temp_alloc);
        auto adaptor = make_json_content_handler_adaptor<json_content_handler>(decoder);
        basic_cbor_reader<jsoncons::binary_stream_source,TempAllocator> reader(is, adaptor, temp_alloc);
        reader.read();
        return decoder.get_result();
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
    decode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                std::istream& is)
    {
        basic_cbor_cursor<binary_stream_source,TempAllocator> cursor(is, temp_alloc);
        json_decoder<basic_json<char,sorted_policy,TempAllocator>,TempAllocator> decoder(temp_alloc, temp_alloc);

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

} // namespace cbor
} // namespace jsoncons

#endif
