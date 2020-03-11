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
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/conversion_error.hpp>

namespace jsoncons {

    // ser_traits

    template <class T, class Enable = void>
    struct ser_traits
    {
        template <class CharT,class Json>
        static void serialize(const T& val, 
                              basic_json_content_handler<CharT>& encoder,
                              const Json& context_j, 
                              std::error_code& ec)
        {
            serialize(std::integral_constant<bool, is_stateless<typename Json::allocator_type>::value>(),
                      val, encoder, context_j, ec);
        }
    private:
        template <class CharT,class Json>
        static void serialize(std::true_type,
                              const T& val, 
                              basic_json_content_handler<CharT>& encoder,
                              const Json& /*context_j*/, 
                              std::error_code& ec)
        {
            auto j = json_type_traits<Json,T>::to_json(val);
            j.dump(encoder, ec);
        }
        template <class CharT,class Json>
        static void serialize(std::false_type, 
                              const T& val, 
                              basic_json_content_handler<CharT>& encoder,
                              const Json& context_j, 
                              std::error_code& ec)
        {
            auto j = json_type_traits<Json,T>::to_json(val, context_j.get_allocator());
            j.dump(encoder, ec);
        }
    };

    // specializations

    // vector like
    template <class T>
    struct ser_traits<T,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                 jsoncons::detail::is_vector_like<T>::value &&
                 !jsoncons::detail::is_typed_array<T>::value 
    >::type>
    {
        typedef typename T::value_type value_type;

        template <class CharT,class Json>
        static void serialize(const T& val, 
                              basic_json_content_handler<CharT>& encoder, 
                              const Json& context_j, 
                              std::error_code& ec)
        {
            encoder.begin_array(val.size());
            for (auto it = std::begin(val); it != std::end(val); ++it)
            {
                ser_traits<value_type>::serialize(*it, encoder, context_j, ec);
            }
            encoder.end_array();
            encoder.flush();
        }
    };

    template <class T>
    struct ser_traits<T,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                 jsoncons::detail::is_vector_like<T>::value &&
                 jsoncons::detail::is_typed_array<T>::value 
    >::type>
    {
        typedef typename T::value_type value_type;

        template <class CharT,class Json>
        static void serialize(const T& val, 
                              basic_json_content_handler<CharT>& encoder, 
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

        template <class CharT,class Json>
        static void serialize(const std::array<T, N>& val, 
                           basic_json_content_handler<CharT>& encoder, 
                           const Json& context_j, 
                           std::error_code& ec)
        {
            encoder.begin_array(val.size());
            for (auto it = std::begin(val); it != std::end(val); ++it)
            {
                ser_traits<value_type>::serialize(*it, encoder, context_j, ec);
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

        template <class CharT,class Json>
        static void serialize(const T& val, 
                              basic_json_content_handler<CharT>& encoder, 
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
                encoder.key(it->first);
                ser_traits<mapped_type>::serialize(it->second, encoder, context_j, ec);
            }
            encoder.end_object(null_ser_context(), ec);
            if (ec)
            {
                return;
            }
            encoder.flush();
        }
    };

    template <class T>
    struct ser_traits<T,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                                jsoncons::detail::is_map_like<T>::value &&
                                std::is_integral<typename T::key_type>::value
    >::type>
    {
        typedef typename T::mapped_type mapped_type;
        typedef typename T::value_type value_type;
        typedef typename T::key_type key_type;

        template <class CharT,class Json>
        static void serialize(const T& val, 
                              basic_json_content_handler<CharT>& encoder, 
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
                std::basic_string<typename Json::char_type> s;
                jsoncons::detail::write_integer(it->first,s);
                encoder.key(s);
                ser_traits<mapped_type>::serialize(it->second, encoder, context_j, ec);
            }
            encoder.end_object(null_ser_context(), ec);
            if (ec)
            {
                return;
            }
            encoder.flush();
        }
    };

} // jsoncons

#endif

