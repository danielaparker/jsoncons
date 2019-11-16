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
#include <jsoncons/json_content_filter.hpp>
#include <jsoncons/config/binary_config.hpp>
#include <jsoncons_ext/cbor/cbor_reader.hpp>
#include <jsoncons_ext/cbor/cbor_cursor.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>
#include <jsoncons/ser_traits.hpp>

namespace jsoncons { namespace cbor {

template <class T, class Enable = void>
struct cbor_ser_traits
{
    template <class Json>
    static T decode(basic_staj_reader<typename Json::char_type>& cursor, 
                         const Json& context_j,
                         std::error_code& ec)
    {
        return ser_traits<T>::decode(cursor, context_j, ec);
    }

    template <class Json>
    static void encode(const T& val, 
                          cbor_content_handler& encoder, 
                          const Json& context_j,
                          std::error_code& ec)
    {
        ser_traits<T>::encode(val, encoder, context_j, ec);
    }
};

template <class T>
struct cbor_ser_traits<T,
    typename std::enable_if<jsoncons::detail::is_vector_like<T>::value && 
                            (std::is_same<typename T::value_type,uint8_t>::value ||  
                             std::is_same<typename T::value_type,uint16_t>::value ||
                             std::is_same<typename T::value_type,uint32_t>::value ||
                             std::is_same<typename T::value_type,uint64_t>::value ||
                             std::is_same<typename T::value_type,int8_t>::value ||  
                             std::is_same<typename T::value_type,int16_t>::value ||
                             std::is_same<typename T::value_type,int32_t>::value ||
                             std::is_same<typename T::value_type,int64_t>::value ||
                             std::is_same<typename T::value_type,float_t>::value ||
                             std::is_same<typename T::value_type,double_t>::value)
>::type>
{
    typedef typename T::value_type value_type;

    template <class Json>
    static T decode(basic_staj_reader<typename Json::char_type>& cursor, 
                         const Json& context_j,
                         std::error_code& ec)
    {
        return ser_traits<T>::decode(cursor, context_j, ec);
    }

    template <class Json>
    static void encode(const T& val, 
                          cbor_content_handler& encoder, 
                          const Json&,
                          std::error_code& ec)
    {
        encoder.typed_array(span<const value_type>(val), semantic_tag::none, null_ser_context(), ec);
    }
};

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
    cbor_ser_traits<T>::encode(val, encoder, json(), ec);
}

template<class T>
typename std::enable_if<!is_basic_json_class<T>::value, void>::type
encode_cbor(const T& val, std::vector<uint8_t>& v, const cbor_encode_options& options, std::error_code& ec)
{
    cbor_bytes_encoder encoder(v, options);
    cbor_ser_traits<T>::encode(val, encoder, json(), ec);
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
    cbor_to_json_content_handler_adaptor<> h(adaptor);
    basic_cbor_reader<jsoncons::bytes_source> reader(v, h);
    reader.read();
    return decoder.get_result();
}

template<class T>
typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
decode_cbor(const std::vector<uint8_t>& v)
{
    cbor_bytes_cursor cursor(v);
    std::error_code ec;
    T val = ser_traits<T>::decode(cursor, json(), ec);
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
    cbor_to_json_content_handler_adaptor<> adaptor2(adaptor);
    cbor_stream_reader reader(is, adaptor2);
    reader.read();
    return decoder.get_result();
}
 
template<class T>
typename std::enable_if<!is_basic_json_class<T>::value,T>::type 
decode_cbor(std::istream& is)
{
    cbor_stream_cursor cursor(is);
    std::error_code ec;
    T val = ser_traits<T>::decode(cursor, json(), ec);
    if (ec)
    {
        JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
    }
    return val;
}
  
#if !defined(JSONCONS_NO_DEPRECATED)
template<class Json>
JSONCONS_DEPRECATED_MSG("Instead, use encode_cbor(const T&, std::vector<uint8_t>&)")
std::vector<uint8_t> encode_cbor(const Json& j)
{
    std::vector<uint8_t> v;
    encode_cbor(j, v);
    return v;
}
#endif

}}

#endif
