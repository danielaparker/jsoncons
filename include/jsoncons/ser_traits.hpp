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
#include <jsoncons/staj_reader.hpp>

namespace jsoncons {

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
private:
    template <class Json>
    static void encode(std::true_type,
                          const T& val, 
                          basic_json_content_handler<typename Json::char_type>& encoder,
                          const Json& /*context_j*/, 
                          std::error_code& ec)
    {
        auto j = json_type_traits<Json, T>::to_json(val);
        j.dump(encoder, ec);
    }
    template <class Json>
    static void encode(std::false_type, 
                          const T& val, 
                          basic_json_content_handler<typename Json::char_type>& encoder,
                          const Json& context_j, 
                          std::error_code& ec)
    {
        auto j = json_type_traits<Json, T>::to_json(val, context_j.get_allocator());
        j.dump(encoder, ec);
    }
};

template <class T, class Enable = void>
struct ser_traits
{
    template <class Json>
    static constexpr bool is_compatible()
    {
        return json_type_traits<Json,T>::is_compatible;
    }

    template <class Json>
    static bool is(const Json& j)
    {
        return ser_traits_default<T>::is(j);
    }

    template <class Json>
    static T as(const Json& j)
    {
        return ser_traits_default<T>::as(j);
    }

    template <class Json>
    static Json to_json(const T& val)
    {
        return ser_traits_default<T>::template to_json<Json>(val);
    }

    template <class Json>
    static Json to_json(const T& val, const typename Json::allocator_type& alloc)
    {
        return ser_traits_default<T>::template to_json<Json>(val, alloc);
    }

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

// specializations

// vector like
template <class T>
struct ser_traits<T,
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

    template <class Json,class Ty = T>
    static typename std::enable_if<!(std::is_integral<Ty>::value && !std::is_same<Ty,bool>::value),T>::type
    as(const Json& j)
    {
        if (!j.is_array())
        {
            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Attempt to cast json non-array to array"));
        }
        T result;
        result.reserve(j.size());
        for (const auto& item : j.array_range())
        {
            result.push_back(item.template as<value_type>());
        }

        return result;
    }

    template <class Json,class Ty = T>
    static typename std::enable_if<std::is_integral<Ty>::value && !std::is_same<Ty,bool>::value,T>::type
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
        else if (j.is_byte_string_view())
        {
            T v(j.as_byte_string_view().begin(),j.as_byte_string_view().end());
            return v;
        }
        else if (j.is_byte_string())
        {
            auto s = j.as_byte_string();
            T v(s.begin(),s.end());
            return v;
        }
        else
        {
            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Attempt to cast json non-array to array"));
        }
    }

    template <class Json>
    static Json to_json(const T& val, 
                        const typename Json::allocator_type& alloc = typename Json::allocator_type())
    {
        Json j(json_array_arg, semantic_tag::none, alloc);
        auto first = std::begin(val);
        auto last = std::end(val);
        size_t size = std::distance(first, last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    }

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

// std::array

template <class T, size_t N>
struct ser_traits<std::array<T,N>>
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
        for (size_t i = 0; i < N; i++)
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
        for (size_t i = 0; i < N && reader.current().event_type() != staj_event_type::end_array && !ec; ++i)
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

