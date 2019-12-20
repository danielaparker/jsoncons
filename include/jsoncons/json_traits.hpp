// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TRAITS_HPP
#define JSONCONS_JSON_TRAITS_HPP

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
#include <jsoncons/staj_reader.hpp>
#include <jsoncons/conversion_error.hpp>

namespace jsoncons {

template <class T, class Enable = void>
struct json_traits
{
    static constexpr bool is_undeclared = true;

    template <class Json>
    static constexpr bool is_compatible()
    {
        return json_type_traits<Json,T>::is_compatible;
    }

    template <class Json>
    static bool is(const Json& j)
    {
        return json_type_traits<Json,T>::is(j);
    }

    template <class Json>
    static T as(const Json& j)
    {
        return json_type_traits<Json,T>::as(j);
    }

    template <class Json>
    static Json to_json(const T& val)
    {
        return json_type_traits<Json,T>::to_json(val);
    }

    template <class Json>
    static Json to_json(const T& val, const typename Json::allocator_type& alloc)
    {
        return json_type_traits<Json, T>::to_json(val, alloc);
    }
};

template <class T, class Enable = void>
struct is_json_traits_declared : std::true_type
{};

template <class T>
struct is_json_traits_declared<T, typename std::enable_if<json_traits<T>::is_undeclared>::type> : std::false_type
{};
// specializations

// vector like
template <class T>
struct json_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
             jsoncons::detail::is_vector_like<T>::value
>::type>
{
    typedef typename T::value_type value_type;

    template <class Json>
    static bool is(const Json& j)
    {
        bool result = j.is_array();
        if (result)
        {
            for (const auto& item : j.array_range())
            {
                if (!item.template is<value_type>())
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    template <class Json,class Ty = value_type>
    static typename std::enable_if<!std::is_same<Ty,uint8_t>::value,T>::type
    as(const Json& j)
    {
        T result;
        if (!j.is_array())
        {
            JSONCONS_THROW(ser_error(conversion_errc::json_not_vector));
        }
        //result.reserve(j.size());
        for (const auto& item : j.array_range())
        {
            result.push_back(item.template as<value_type>());
        }

        return result;
    }

    template <class Json,class Ty = value_type>
    static typename std::enable_if<std::is_same<Ty,uint8_t>::value,T>::type
    as(const Json& j)
    {
        if (j.is_array())
        {
            T result;
            result.reserve(j.size());
            for (const auto& item : j.array_range())
            {
                result.push_back(item.template as<value_type>());
            }

            return result;
        }
        else if (j.is_byte_string())
        {
            T v(j.as_byte_string_view().begin(),j.as_byte_string_view().end());
            return v;
        }
        else
        {
            JSONCONS_THROW(ser_error(conversion_errc::json_not_vector));
        }
    }

    template <class Json>
    static Json to_json(const T& val, 
                        const typename Json::allocator_type& alloc = typename Json::allocator_type())
    {
        Json j(json_array_arg, semantic_tag::none, alloc);
        auto first = std::begin(val);
        auto last = std::end(val);
        std::size_t size = std::distance(first, last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    }
};

// std::array

template <class T, std::size_t N>
struct json_traits<std::array<T,N>>
{
    typedef typename std::array<T,N>::value_type value_type;

    template <class Json>
    static bool is(const Json& j)
    {
        bool result = j.is_array() && j.size() == N;
        if (result)
        {
            for (auto e : j.array_range())
            {
                if (!e.template is<value_type>())
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    template <class Json>
    static std::array<T,N> as(const Json& j)
    {
        std::array<T, N> buff;
        JSONCONS_ASSERT(j.size() == N);
        for (std::size_t i = 0; i < N; i++)
        {
            buff[i] = j[i].template as<T>();
        }
        return buff;
    }

    template <class Json>
    static Json to_json(const std::array<T,N>& val, const typename Json::allocator_type& alloc = typename Json::allocator_type())
    {
        Json j(json_array_arg, semantic_tag::none, alloc);
        j.reserve(N);
        for (auto it = val.begin(); it != val.end(); ++it)
        {
            j.push_back(*it);
        }
        return j;
    }
};

// map like

template <class T>
struct json_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                            jsoncons::detail::is_map_like<T>::value &&
                            jsoncons::detail::is_constructible_from_const_pointer_and_size<typename T::key_type>::value
>::type>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;
    typedef typename T::key_type key_type;

    template <class Json>
    static bool is(const Json& j)
    {
        bool result = j.is_object();
        for (auto member : j.object_range())
        {
            if (!member.value().template is<mapped_type>())
            {
                result = false;
            }
        }
        return result;
    }

    template <class Json>
    static T as(const Json& j)
    {
        T result;
        for (const auto& item : j.object_range())
        {
            result.emplace(key_type(item.key().data(),item.key().size()), item.value().template as<mapped_type>());
        }

        return result;
    }

    template <class Json>
    static Json to_json(const T& val, const typename Json::allocator_type& alloc = typename Json::allocator_type())
    {
        Json j = Json(json_object_arg, val.begin(), val.end(), semantic_tag::none, alloc);
        return j;
    }
};

}

#endif

