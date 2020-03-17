// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DESER_TRAITS_HPP
#define JSONCONS_DESER_TRAITS_HPP

#include <string>
#include <tuple>
#include <array>
#include <memory>
#include <type_traits> // std::enable_if, std::true_type, std::false_type
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/staj_reader.hpp>
#include <jsoncons/conversion_error.hpp>

namespace jsoncons {

    // deser_traits

    template <class T, class Enable = void>
    struct deser_traits
    {
        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                             json_decoder<Json,TempAllocator>& decoder, 
                             std::error_code& ec)
        {
            decoder.reset();
            reader.read(decoder, ec);
            return decoder.get_result().template as<T>();
        }
    };

    // specializations

    // vector like
    template <class T>
    struct deser_traits<T,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                 jsoncons::detail::is_vector_like<T>::value &&
                 !jsoncons::detail::is_typed_array<T>::value 
    >::type>
    {
        typedef typename T::value_type value_type;

        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                             json_decoder<Json,TempAllocator>& decoder, 
                             std::error_code& ec)
        {
            T v;

            if (reader.current().event_type() != staj_event_type::begin_array)
            {
                ec = conversion_errc::json_not_vector;
                return v;
            }
            reader.next(ec);
            while (reader.current().event_type() != staj_event_type::end_array && !ec)
            {
                v.push_back(deser_traits<value_type>::deserialize(reader, decoder, ec));
                reader.next(ec);
            }
            return v;
        }
    };

    template <class T>
    struct typed_array_content_handler : public default_json_content_handler
    {
        T& v_;
        int level_;
    public:
        typedef typename T::value_type value_type;

        typed_array_content_handler(T& v)
            : default_json_content_handler(false,conversion_errc::json_not_vector), v_(v), level_(0)
        {
        }
    private:
        bool do_begin_array(semantic_tag, 
                            const ser_context&, 
                            std::error_code& ec) override
        {      
            if (++level_ != 1)
            {
                ec = conversion_errc::json_not_vector;
                return false;
            }
            return true;
        }

        bool do_begin_array(std::size_t size, 
                            semantic_tag, 
                            const ser_context&, 
                            std::error_code& ec) override
        {
            if (++level_ != 1)
            {
                ec = conversion_errc::json_not_vector;
                return false;
            }
            v_.reserve(size);
            return true;
        }

        bool do_end_array(const ser_context&, 
                          std::error_code& ec) override
        {
            if (level_ != 1)
            {
                ec = conversion_errc::json_not_vector;
                return false;
            }
            return false;
        }

        bool do_uint64(uint64_t value, 
                             semantic_tag, 
                             const ser_context&,
                             std::error_code&) override
        {
            v_.push_back(static_cast<value_type>(value));
            return true;
        }

        bool do_int64(int64_t value, 
                            semantic_tag,
                            const ser_context&,
                            std::error_code&) override
        {
            v_.push_back(static_cast<value_type>(value));
            return true;
        }

        bool do_half(uint16_t value, 
                           semantic_tag,
                           const ser_context&,
                           std::error_code&) override
        {
            return do_half_(typename std::integral_constant<bool, std::is_integral<value_type>::value>::type(), value);
        }

        bool do_half_(std::true_type, uint16_t value)
        {
            v_.push_back(static_cast<value_type>(value));
            return true;
        }

        bool do_half_(std::false_type, uint16_t value)
        {
            v_.push_back(static_cast<value_type>(jsoncons::detail::decode_half(value)));
            return true;
        }

        bool do_double(double value, 
                             semantic_tag,
                             const ser_context&,
                             std::error_code&) override
        {
            v_.push_back(static_cast<value_type>(value));
            return true;
        }

        bool do_typed_array(const span<const value_type>& data,  
                            semantic_tag,
                            const ser_context&,
                            std::error_code&) override
        {
            v_ = std::vector<value_type>(data.begin(),data.end());
            return false;
        }
    };

    template <class T>
    struct deser_traits<T,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                 jsoncons::detail::is_vector_like<T>::value &&
                 jsoncons::detail::is_typed_array<T>::value 
    >::type>
    {
        typedef typename T::value_type value_type;

        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                             json_decoder<Json,TempAllocator>&, 
                             std::error_code& ec)
        {
            T v;

            if (reader.current().event_type() != staj_event_type::begin_array)
            {
                ec = conversion_errc::json_not_vector;
                return v;
            }

            typed_array_content_handler<T> handler(v);
            reader.read(handler, ec);
            return v;
        }
    };

    // std::array

    template <class T, std::size_t N>
    struct deser_traits<std::array<T,N>>
    {
        typedef typename std::array<T,N>::value_type value_type;

        template <class Json,class TempAllocator>
        static std::array<T, N> deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                                            json_decoder<Json,TempAllocator>& decoder, 
                                            std::error_code& ec)
        {
            std::array<T,N> v;
            v.fill(T{});
            if (reader.current().event_type() != staj_event_type::begin_array)
            {
                ec = conversion_errc::json_not_vector;
            }
            reader.next(ec);
            for (std::size_t i = 0; i < N && reader.current().event_type() != staj_event_type::end_array && !ec; ++i)
            {
                v[i] = deser_traits<value_type>::deserialize(reader, decoder, ec);
                reader.next(ec);
            }
            return v;
        }
    };

    // map like

    template <class T>
    struct deser_traits<T,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                                jsoncons::detail::is_map_like<T>::value &&
                                jsoncons::detail::is_constructible_from_const_pointer_and_size<typename T::key_type>::value
    >::type>
    {
        typedef typename T::mapped_type mapped_type;
        typedef typename T::value_type value_type;
        typedef typename T::key_type key_type;

        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                             json_decoder<Json,TempAllocator>& decoder, 
                             std::error_code& ec)
        {
            T val;
            if (reader.current().event_type() != staj_event_type::begin_object)
            {
                ec = conversion_errc::json_not_map;
                return val;
            }
            reader.next(ec);

            while (reader.current().event_type() != staj_event_type::end_object && !ec)
            {
                if (reader.current().event_type() != staj_event_type::name)
                {
                    ec = json_errc::expected_name;
                    return val;
                }
                auto key = reader.current(). template get<key_type>();
                reader.next(ec);
                val.emplace(std::move(key),deser_traits<mapped_type>::deserialize(reader, decoder, ec));
                reader.next(ec);
            }
            return val;
        }
    };

    template <class T>
    struct deser_traits<T,
        typename std::enable_if<!is_json_type_traits_declared<T>::value && 
                                jsoncons::detail::is_map_like<T>::value &&
                                std::is_integral<typename T::key_type>::value
    >::type>
    {
        typedef typename T::mapped_type mapped_type;
        typedef typename T::value_type value_type;
        typedef typename T::key_type key_type;

        template <class Json,class TempAllocator>
        static T deserialize(basic_staj_reader<typename Json::char_type>& reader, 
                             json_decoder<Json,TempAllocator>& decoder, 
                             std::error_code& ec)
        {
            T val;
            if (reader.current().event_type() != staj_event_type::begin_object)
            {
                ec = conversion_errc::json_not_map;
                return val;
            }
            reader.next(ec);

            while (reader.current().event_type() != staj_event_type::end_object && !ec)
            {
                if (reader.current().event_type() != staj_event_type::name)
                {
                    ec = json_errc::expected_name;
                    return val;
                }
                auto s = reader.current().template get<basic_string_view<typename Json::char_type>>();
                auto key = jsoncons::detail::to_integer<key_type>(s.data(), s.size()); 
                reader.next(ec);
                val.emplace(key.value(),deser_traits<mapped_type>::deserialize(reader, decoder, ec));
                reader.next(ec);
            }
            return val;
        }
    };

} // jsoncons

#endif

