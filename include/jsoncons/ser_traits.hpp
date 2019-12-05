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

} // namespace jsoncons

namespace jsoncons {

template <class T, class Enable = void>
struct ser_traits
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
}

#include <jsoncons/staj_iterator.hpp>

namespace jsoncons {
// specializations

// vector like

template <class T>
struct ser_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && jsoncons::detail::is_vector_like<T>::value
>::type>
{
    typedef typename T::value_type value_type;

    template <class Json>
    static T decode(basic_staj_reader<typename Json::char_type>& reader, 
                         const Json&, 
                         std::error_code& ec)
    {
        T v;
        staj_array_iterator<Json,Json> end;
        staj_array_iterator<Json,Json> it(reader, ec);

        while (it != end && !ec)
        {
            v.emplace_back(it->template as<value_type>());
            it.increment(ec);
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
    static std::array<T, N> decode(basic_staj_reader<typename Json::char_type>& reader, 
                                        const Json&, 
                                        std::error_code& ec)
    {
        std::array<T,N> v;
        v.fill(T{});
        staj_array_iterator<Json,Json> end;
        staj_array_iterator<Json,Json> it(reader, ec);

        for (size_t i = 0; it != end && i < N && !ec; ++i)
        {
            v[i] = it->template as<value_type>();
            it.increment(ec);
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
    typename std::enable_if<!is_json_type_traits_declared<T>::value && jsoncons::detail::is_map_like<T>::value
>::type>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;
    typedef typename T::key_type key_type;

    template <class Json>
    static T decode(basic_staj_reader<typename Json::char_type>& reader, 
                         const Json&, 
                         std::error_code& ec)
    {
        T m;
        staj_object_iterator<Json,Json> end;
        staj_object_iterator<Json,Json> it(reader, ec);

        while (it != end && !ec)
        {
            m.emplace(std::make_pair(it->first,(it->second).template as<mapped_type>()));
            it.increment(ec);
        }
        return m;
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

