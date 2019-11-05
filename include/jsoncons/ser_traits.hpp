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
#include <jsoncons/basic_json.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/staj_reader.hpp>

namespace jsoncons {

template <class T, class CharT, class Json>
T read_from(basic_staj_reader<CharT>& reader, const Json& context_j, std::error_code& ec);

template <class T, class CharT, class Json>
T read_from(basic_staj_reader<CharT>& reader, const Json& context_j)
{
    std::error_code ec;
    T val = read_from<T>(reader, context_j, ec);
    if (ec)
    {
        JSONCONS_THROW(ser_error(ec, reader.context().line(), reader.context().column()));
    }
    return val;
}

} // namespace jsoncons

#include <jsoncons/staj_iterator.hpp>

namespace jsoncons {

template <class T, class Enable = void>
struct ser_traits
{
    template <class Json>
    static T deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                         const Json& context_j, 
                         std::error_code& ec)
    {
        json_decoder<Json> decoder(context_j.get_allocator());
        reader.read_to(decoder, ec);
        return decoder.get_result().template as<T>();
    }

    template <class Json>
    static void serialize(const T& val, 
                          basic_json_content_handler<typename Json::char_type>& encoder,
                          const Json& context_j, 
                          std::error_code& ec)
    {
        serialize(std::integral_constant<bool, is_stateless<typename Json::allocator_type>::value>(),
                  val, encoder, context_j, ec);
    }
private:
    template <class Json>
    static void serialize(std::true_type,
                          const T& val, 
                          basic_json_content_handler<typename Json::char_type>& encoder,
                          const Json& /*context_j*/, 
                          std::error_code& ec)
    {
        auto j = json_type_traits<Json, T>::to_json(val);
        j.dump(encoder, ec);
    }
    template <class Json>
    static void serialize(std::false_type, 
                          const T& val, 
                          basic_json_content_handler<typename Json::char_type>& encoder,
                          const Json& context_j, 
                          std::error_code& ec)
    {
        auto j = json_type_traits<Json, T>::to_json(val, context_j.get_allocator());
        j.dump(encoder, ec);
    }
};

// specializations

// vector like

template <class T>
struct ser_traits<T,
    typename std::enable_if<!is_json_type_traits_declared<T>::value && jsoncons::detail::is_vector_like<T>::value
>::type>
{
    typedef typename T::value_type value_type;

    template <class Json>
    static T deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                         const Json&, 
                         std::error_code& ec)
    {
        T v;
        staj_array_iterator<Json,value_type> end;
        staj_array_iterator<Json,value_type> it(reader, ec);

        while (it != end && !ec)
        {
            v.push_back(*it);
            it.increment(ec);
        }
        return v;
    }

    template <class Json>
    static void serialize(const T& val, 
                          basic_json_content_handler<typename Json::char_type>& encoder, 
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

// std::array

template <class T, size_t N>
struct ser_traits<std::array<T,N>>
{
    typedef typename std::array<T,N>::value_type value_type;

    template <class Json>
    static std::array<T, N> deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                                        const Json&, 
                                        std::error_code& ec)
    {
        std::array<T,N> v;
        v.fill(T{});
        staj_array_iterator<Json,value_type> end;
        staj_array_iterator<Json,value_type> it(reader, ec);

        for (size_t i = 0; it != end && i < N && !ec; ++i)
        {
            v[i] = *it;
            it.increment(ec);
        }
        return v;
    }

    template <class Json>
    static void serialize(const std::array<T, N>& val, 
                          basic_json_content_handler<typename Json::char_type>& encoder, 
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
    typename std::enable_if<!is_json_type_traits_declared<T>::value && jsoncons::detail::is_map_like<T>::value
>::type>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;
    typedef typename T::key_type key_type;

    template <class Json>
    static T deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                         const Json&, 
                         std::error_code& ec)
    {
        T m;
        staj_object_iterator<Json,mapped_type> end;
        staj_object_iterator<Json,mapped_type> it(reader, ec);

        while (it != end && !ec)
        {
            m.emplace(it->first,it->second);
            it.increment(ec);
        }
        return m;
    }

    template <class Json>
    static void serialize(const T& val, 
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

template <class T, class CharT, class Json>
T read_from(basic_staj_reader<CharT>& reader, const Json& context_j, std::error_code& ec)
{
    return ser_traits<T>::deserialize(reader, context_j, ec);
}

}

#endif

