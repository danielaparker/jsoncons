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
#include <type_traits> // std::enable_if
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/staj_reader.hpp>

namespace jsoncons {

template <class T, class CharT, class Json>
T read_from(const Json& j, basic_staj_reader<CharT>& reader, std::error_code& ec);

template <class T, class CharT, class Json>
T read_from(const Json& j, basic_staj_reader<CharT>& reader)
{
    std::error_code ec;
    T val = read_from<T>(j, reader, ec);
    if (ec)
    {
        throw ser_error(ec, reader.context().line(), reader.context().column());
    }
    return val;
}

template <class T, class CharT, class Json>
void write_to(const T&val, basic_json_content_handler<CharT>& receiver);

} // namespace jsoncons

#include <jsoncons/staj_iterator.hpp>

namespace jsoncons {

template <class T, class Enable = void>
struct ser_traits
{
    template <class CharT, class Json>
    static T decode(basic_staj_reader<CharT>& reader, std::error_code& ec)
    {
        json_decoder<Json> decoder;
        reader.read_to(decoder, ec);
        return decoder.get_result().template as<T>();
    }

    template <class CharT, class Json>
    static void encode(const T& val, basic_json_content_handler<CharT>& receiver)
    {
        auto j = json_type_traits<Json, T>::to_json(val);
        j.dump(receiver);
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

    template <class CharT, class Json>
    static T decode(basic_staj_reader<CharT>& reader, std::error_code& ec)
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

    template <class CharT, class Json>
    static void encode(const T& val, basic_json_content_handler<CharT>& receiver)
    {
        receiver.begin_array(val.size());
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            ser_traits<value_type>::template encode<CharT,Json>(*it,receiver);
        }
        receiver.end_array();
        receiver.flush();
    }
};

// std::array

template <class T, size_t N>
struct ser_traits<std::array<T,N>>
{
    typedef typename std::array<T,N>::value_type value_type;

    template <class CharT,class Json>
    static std::array<T, N> decode(basic_staj_reader<CharT>& reader, std::error_code& ec)
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

    template <class CharT, class Json>
    static void encode(const std::array<T, N>& val, basic_json_content_handler<CharT>& receiver)
    {
        receiver.begin_array(val.size());
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            ser_traits<value_type>::template encode<CharT,Json>(*it,receiver);
        }
        receiver.end_array();
        receiver.flush();
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

    template <class CharT, class Json>
    static T decode(basic_staj_reader<CharT>& reader, std::error_code& ec)
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

    template <class CharT, class Json>
    static void encode(const T& val, basic_json_content_handler<CharT>& receiver)
    {
        receiver.begin_object(val.size());
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            receiver.name(it->first);
            ser_traits<mapped_type>::template encode<CharT,Json>(it->second,receiver);
        }
        receiver.end_object();
        receiver.flush();
    }
};

template <class T, class CharT, class Json>
T read_from(const Json&, basic_staj_reader<CharT>& reader, std::error_code& ec)
{
    return ser_traits<T>::template decode<CharT,Json>(reader,ec);
}

template <class T, class CharT, class Json>
void write_to(const Json&, const T&val, basic_json_content_handler<CharT>& receiver)
{
    ser_traits<T>::template encode<CharT,Json>(val, receiver);
}

}

#endif

