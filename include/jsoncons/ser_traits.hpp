// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_SER_TRAITS_HPP
#define JSONCONS_SER_TRAITS_HPP

#include <string>
#include <tuple>
#include <array>
#include <memory>
#include <type_traits> // std::enable_if, std::true_type, std::false_type
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_traits.hpp>
#include <jsoncons/staj_reader.hpp>
#include <jsoncons/conversion_error.hpp>

namespace jsoncons {

// is_compatible_element

template<class C, class Enable=void>
struct is_typed_array : std::false_type {};

template<class T>
struct is_typed_array
<
    T, 
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
                             std::is_same<typename T::value_type,double_t>::value)>::type
> : std::true_type{};

template <class T>
struct ser_traits_default;

template <class T, class Enable = void>
struct ser_traits
{
    template <class Json>
    static T decode(basic_staj_reader<typename Json::char_type>& reader, 
                    const Json& context_j, 
                    std::error_code& ec)
    {
        return ser_traits_default<T>::decode(reader, context_j, ec);
    }

    template <class Json>
    static void encode(const T& val, 
                       basic_json_content_handler<typename Json::char_type>& encoder,
                       const Json& context_j, 
                       std::error_code& ec)
    {
        ser_traits_default<T>::encode(val, encoder, context_j, ec);
    }
};

template <class T>
struct ser_traits_default
{
    template <class Json>
    static T decode(basic_staj_reader<typename Json::char_type>& reader, 
                    const Json& context_j, 
                    std::error_code& ec)
    {
        json_decoder<Json> decoder(context_j.get_allocator());
        reader.read(decoder, ec);
        return decoder.get_result().template as<T>();
    }

    template <class Json>
    static void encode(const T& val, 
                       basic_json_content_handler<typename Json::char_type>& encoder,
                       const Json& context_j, 
                       std::error_code& ec)
    {
        encode(std::integral_constant<bool, is_stateless<typename Json::allocator_type>::value>(),
                  val, encoder, context_j, ec);
    }
private:
    template <class Json>
    static void encode(std::true_type,
                       const T& val, 
                       basic_json_content_handler<typename Json::char_type>& encoder,
                       const Json& /*context_j*/, 
                       std::error_code& ec)
    {
        auto j = json_traits<T>::template to_json<Json>(val);
        j.dump(encoder, ec);
    }
    template <class Json>
    static void encode(std::false_type, 
                       const T& val, 
                       basic_json_content_handler<typename Json::char_type>& encoder,
                       const Json& context_j, 
                       std::error_code& ec)
    {
        auto j = json_traits<T>::template to_json<Json>(val, context_j.get_allocator());
        j.dump(encoder, ec);
    }
};

// specializations

// vector like
template <class T>
struct ser_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
             jsoncons::detail::is_vector_like<T>::value &&
             !is_typed_array<T>::value 
>::type>
{
    typedef typename T::value_type value_type;

    template <class Json>
    static T decode(basic_staj_reader<typename Json::char_type>& reader, 
                    const Json& context_j, 
                    std::error_code& ec)
    {
        T v;

        if (reader.current().event_type() != staj_event_type::begin_array)
        {
            return v;
        }
        reader.next(ec);
        while (reader.current().event_type() != staj_event_type::end_array && !ec)
        {
            v.push_back(ser_traits<value_type>::decode(reader, context_j, ec));
            reader.next(ec);
        }
        return v;
    }

    template <class Json>
    static void encode(const T& val, 
                          basic_json_content_handler<typename Json::char_type>& encoder, 
                          const Json& context_j, 
                          std::error_code& ec)
    {
        encoder.begin_array(val.size());
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            ser_traits<value_type>::encode(*it, encoder, context_j, ec);
        }
        encoder.end_array();
        encoder.flush();
    }
};

template <class T>
struct ser_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
             jsoncons::detail::is_vector_like<T>::value &&
             is_typed_array<T>::value 
>::type>
{
    typedef typename T::value_type value_type;

    template <class Json>
    static T decode(basic_staj_reader<typename Json::char_type>& reader, 
                    const Json& context_j, 
                    std::error_code& ec)
    {
        T v;

        if (reader.current().event_type() != staj_event_type::begin_array)
        {
            return v;
        }
        reader.next(ec);
        while (reader.current().event_type() != staj_event_type::end_array && !ec)
        {
            v.push_back(ser_traits<value_type>::decode(reader, context_j, ec));
            reader.next(ec);
        }
        return v;
    }

    template <class Json>
    static void encode(const T& val, 
                          json_content_handler& encoder, 
                          const Json&,
                          std::error_code& ec)
    {
        encoder.typed_array(span<const value_type>(val), semantic_tag::none, null_ser_context(), ec);
    }
};

// std::array

template <class T, std::size_t N>
struct ser_traits<std::array<T,N>>
{
    typedef typename std::array<T,N>::value_type value_type;

    template <class Json>
    static std::array<T, N> decode(basic_staj_reader<typename Json::char_type>& reader, 
                                   const Json& context_j, 
                                   std::error_code& ec)
    {
        std::array<T,N> v;
        v.fill(T{});
        if (reader.current().event_type() != staj_event_type::begin_array)
        {
            return v;
        }
        reader.next(ec);
        for (std::size_t i = 0; i < N && reader.current().event_type() != staj_event_type::end_array && !ec; ++i)
        {
            v[i] = ser_traits<value_type>::decode(reader, context_j, ec);
            reader.next(ec);
        }
        return v;
    }

    template <class Json>
    static void encode(const std::array<T, N>& val, 
                       basic_json_content_handler<typename Json::char_type>& encoder, 
                       const Json& context_j, 
                       std::error_code& ec)
    {
        encoder.begin_array(val.size());
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            ser_traits<value_type>::encode(*it, encoder, context_j, ec);
        }
        encoder.end_array();
        encoder.flush();
    }
};

// map like

template <class T>
struct ser_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                            jsoncons::detail::is_map_like<T>::value &&
                            jsoncons::detail::is_constructible_from_const_pointer_and_size<typename T::key_type>::value
>::type>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;
    typedef typename T::key_type key_type;

    template <class Json>
    static T decode(basic_staj_reader<typename Json::char_type>& reader, 
                    const Json& context_j, 
                    std::error_code& ec)
    {
        T val;
        if (reader.current().event_type() != staj_event_type::begin_object)
        {
            return val;
        }
        reader.next(ec);

        while (reader.current().event_type() != staj_event_type::end_object && !ec)
        {
            JSONCONS_ASSERT(reader.current().event_type() == staj_event_type::name);
            auto key = reader.current(). template get<key_type>();
            reader.next(ec);
            val.emplace(std::move(key),ser_traits<mapped_type>::decode(reader, context_j, ec));
            reader.next(ec);
        }
        return val;
    }

    template <class Json>
    static void encode(const T& val, 
                          basic_json_content_handler<typename Json::char_type>& encoder, 
                          const Json& context_j, 
                          std::error_code& ec)
    {
        encoder.begin_object(val.size(), semantic_tag::none, null_ser_context(), ec);
        if (ec)
        {
            return;
        }
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            encoder.name(it->first);
            ser_traits<mapped_type>::encode(it->second, encoder, context_j, ec);
        }
        encoder.end_object(null_ser_context(), ec);
        if (ec)
        {
            return;
        }
        encoder.flush();
    }
};

}

#endif

