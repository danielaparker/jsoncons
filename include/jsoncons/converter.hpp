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
            (type_traits::is_basic_string<Into>::value && type_traits::is_narrow_character<typename Into::value_type>::value)>::type>
    {
        using allocator_type = typename Into::allocator_type;
    public:
        Into convert(From value, semantic_tag tag, const allocator_type& alloc, std::error_code&)
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
    };

    template <class From, class Into>
    class converter<From, Into, 
        typename std::enable_if<type_traits::is_byte_sequence<From>::value &&
            (type_traits::is_basic_string<Into>::value && type_traits::is_wide_character<typename Into::value_type>::value)>::type>
    {
        using allocator_type = typename Into::allocator_type;
    public:
        Into convert(From value, semantic_tag tag, const allocator_type& alloc, std::error_code& ec)
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

} // namespace jsoncons

#endif

