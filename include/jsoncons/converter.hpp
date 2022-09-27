// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CONVERTER_HPP
#define JSONCONS_CONVERTER_HPP

#include <system_error> // std::error_code
#include <jsoncons/more_type_traits.hpp>
#include <jsoncons/byte_string.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/conv_error.hpp>
#include <jsoncons/detail/write_number.hpp> // from_integer
#include <jsoncons/tag_type.hpp>

namespace jsoncons {

    template <class From, class Into, class Enable = void>
    class converter
    {
    };

    // From byte string, Into string
    template <class From, class Into>
    class converter<From, Into, 
        typename std::enable_if<type_traits::is_byte_sequence<From>::value &&
            type_traits::is_basic_string<Into>::value>::type>
    {
        using allocator_type = typename Into::allocator_type;
    public:
        template <class CharT = typename Into::value_type>
        JSONCONS_CPP14_CONSTEXPR 
        typename std::enable_if<type_traits::is_narrow_character<CharT>::value,Into>::type
        convert(From value, semantic_tag tag, const allocator_type& alloc, std::error_code&)
        {
            Into s(alloc);
            switch (tag)
            {
                case semantic_tag::base64:
                    encode_base64(value.begin(), value.end(), s);
                    break;
                case semantic_tag::base16:
                    encode_base16(value.begin(), value.end(), s);
                    break;
                default:
                    encode_base64url(value.begin(), value.end(), s);
                    break;
            }
            return s;
        }
        template <class CharT = typename Into::value_type>
        JSONCONS_CPP14_CONSTEXPR 
        typename std::enable_if<type_traits::is_wide_character<CharT>::value,Into>::type
        convert(From value, semantic_tag tag, const allocator_type& alloc, std::error_code& ec)
        {
            std::string s;
            switch (tag)
            {
                case semantic_tag::base64:
                    encode_base64(value.begin(), value.end(), s);
                    break;
                case semantic_tag::base16:
                    encode_base16(value.begin(), value.end(), s);
                    break;
                default:
                    encode_base64url(value.begin(), value.end(), s);
                    break;
            }

            Into ws(alloc);
            auto retval = unicode_traits::convert(s.data(), s.size(), ws);
            if (retval.ec != unicode_traits::conv_errc())
            {
                ec = conv_errc::not_wide_char;
            }

            return ws;
        }
    };

    // From string, Into byte_string
    template <class From, class Into>
    class converter<From, Into, 
        typename std::enable_if<type_traits::is_char_sequence<From>::value &&
            (type_traits::is_back_insertable_byte_container<Into>::value || type_traits::is_basic_byte_string<Into>::value)>::type>
    {
        using allocator_type = typename Into::allocator_type;
    public:
        template <class CharT = typename From::value_type>
        JSONCONS_CPP14_CONSTEXPR 
        typename std::enable_if<type_traits::is_narrow_character<CharT>::value,Into>::type
        convert(From value, semantic_tag tag, const allocator_type& alloc, std::error_code& ec)
        {
            Into bytes(alloc);
            switch (tag)
            {
                case semantic_tag::base16:
                {
                    auto res = decode_base16(value.begin(), value.end(), bytes);
                    if (res.ec != conv_errc::success)
                    {
                        ec = conv_errc::not_byte_string;
                    }
                    break;
                }
                case semantic_tag::base64:
                {
                    decode_base64(value.begin(), value.end(), bytes);
                    break;
                }
                case semantic_tag::base64url:
                {
                    decode_base64url(value.begin(), value.end(), bytes);
                    break;
                }
                default:
                {
                    ec = conv_errc::not_byte_string;
                    break;
                }
            }
            return bytes;
        }

        template <class CharT = typename From::value_type>
        JSONCONS_CPP14_CONSTEXPR 
        typename std::enable_if<type_traits::is_wide_character<CharT>::value,Into>::type
        convert(From value, semantic_tag tag, const allocator_type& alloc, std::error_code& ec)
        {
            Into bytes(alloc);

            std::string s(alloc);
            auto retval = unicode_traits::convert(value.data(), value.size(), s);
            if (retval.ec != unicode_traits::conv_errc())
            {
                ec = conv_errc::not_wide_char;
            }
            switch (tag)
            {
                case semantic_tag::base16:
                {
                    auto res = decode_base16(s.begin(), s.end(), bytes);
                    if (res.ec != conv_errc::success)
                    {
                        ec = conv_errc::not_byte_string;
                    }
                    break;
                }
                case semantic_tag::base64:
                {
                    decode_base64(s.begin(), s.end(), bytes);
                    break;
                }
                case semantic_tag::base64url:
                {
                    decode_base64url(s.begin(), s.end(), bytes);
                    break;
                }
                default:
                {
                    ec = conv_errc::not_byte_string;
                    break;
                }
            }
            return bytes;
        }
    };

    // From integer, Into string
    template <class From, class Into>
    class converter<From, Into, 
        typename std::enable_if<type_traits::is_integer<From>::value &&
            type_traits::is_basic_string<Into>::value>::type>
    {
        using allocator_type = typename Into::allocator_type;
    public:
        JSONCONS_CPP14_CONSTEXPR 
        Into convert(From value, semantic_tag, const allocator_type& alloc, std::error_code&)
        {
            Into s(alloc);
            jsoncons::detail::from_integer(value, s);
            return s;
        }
    };

    // From integer, Into string
    template <class From, class Into>
    class converter<From, Into, 
        typename std::enable_if<std::is_floating_point<From>::value &&
            type_traits::is_basic_string<Into>::value>::type>
    {
        using allocator_type = typename Into::allocator_type;
    public:
        JSONCONS_CPP14_CONSTEXPR 
        Into convert(From value, semantic_tag, const allocator_type& alloc, std::error_code&)
        {
            Into s(alloc);
            jsoncons::detail::write_double f{float_chars_format::general,0};
            f(value, s);
            return s;
        }
    };

} // namespace jsoncons

#endif

