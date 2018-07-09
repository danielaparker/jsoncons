/// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBORERRORCATEGORY_HPP
#define JSONCONS_CBOR_CBORERRORCATEGORY_HPP

#include <system_error>
#include <jsoncons/jsoncons_config.hpp>

namespace jsoncons { namespace cbor {

    enum class cbor_parser_errc
    {
        ok = 0,
        unexpected_eof = 1,
        source_error,
        invalid_cbor_text,
        extra_character,
        max_depth_exceeded,
        single_quote,
        illegal_character_in_string,
        extra_comma,
        expected_name,
        expected_value,
        invalid_value,
        expected_colon,
        illegal_control_character,
        illegal_escaped_character,
        expected_codepoint_surrogate_pair,
        invalid_hex_escape_sequence,
        invalid_unicode_escape_sequence,
        leading_zero,
        invalid_number,
        expected_comma_or_right_brace,
        expected_comma_or_right_bracket,
        unexpected_right_bracket,
        unexpected_right_brace,
        illegal_comment,
        expected_continuation_byte,
        over_long_utf8_sequence,
        illegal_codepoint,
        illegal_surrogate_value,
        unpaired_high_surrogate
    };

class cbor_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "cbor";
    }
    virtual std::string message(int ev) const
    {
        switch (static_cast<cbor_parser_errc>(ev))
        {
        case cbor_parser_errc::unexpected_eof:
            return "Unexpected end of file";
        case cbor_parser_errc::source_error:
            return "Source error";
        case cbor_parser_errc::invalid_cbor_text:
            return "Invalid JSON text";
        case cbor_parser_errc::extra_character:
            return "Unexpected non-whitespace character after JSON text";
        case cbor_parser_errc::max_depth_exceeded:
            return "Maximum JSON depth exceeded";
        case cbor_parser_errc::single_quote:
            return "JSON strings cannot be quoted with single quotes";
        case cbor_parser_errc::illegal_character_in_string:
            return "Illegal character in string";
        case cbor_parser_errc::extra_comma:
            return "Extra comma";
        case cbor_parser_errc::expected_name:
            return "Expected object member name";
        case cbor_parser_errc::expected_value:
            return "Expected value";
        case cbor_parser_errc::invalid_value:
            return "Invalid value";
        case cbor_parser_errc::expected_colon:
            return "Expected name separator ':'";
        case cbor_parser_errc::illegal_control_character:
            return "Illegal control character in string";
        case cbor_parser_errc::illegal_escaped_character:
            return "Illegal escaped character in string";
        case cbor_parser_errc::expected_codepoint_surrogate_pair:
            return "Invalid codepoint, expected another \\u token to begin the second half of a codepoint surrogate pair.";
        case cbor_parser_errc::invalid_hex_escape_sequence:
            return "Invalid codepoint, expected hexadecimal digit.";
        case cbor_parser_errc::invalid_unicode_escape_sequence:
            return "Invalid codepoint, expected four hexadecimal digits.";
        case cbor_parser_errc::leading_zero:
            return "A number cannot have a leading zero";
        case cbor_parser_errc::invalid_number:
            return "Invalid number";
        case cbor_parser_errc::expected_comma_or_right_brace:
            return "Expected comma or right brace '}'";
        case cbor_parser_errc::expected_comma_or_right_bracket:
            return "Expected comma or right bracket ']'";
        case cbor_parser_errc::unexpected_right_brace:
            return "Unexpected right brace '}'";
        case cbor_parser_errc::unexpected_right_bracket:
            return "Unexpected right bracket ']'";
        case cbor_parser_errc::illegal_comment:
            return "Illegal comment";
        case cbor_parser_errc::expected_continuation_byte:
            return "Expected continuation byte";
        case cbor_parser_errc::over_long_utf8_sequence:
            return "Over long UTF-8 sequence";
        case cbor_parser_errc::illegal_codepoint:
            return "Illegal codepoint (>= 0xd800 && <= 0xdfff)";
        case cbor_parser_errc::illegal_surrogate_value:
            return "UTF-16 surrogate values are illegal in UTF-32";
        case cbor_parser_errc::unpaired_high_surrogate:
            return "Expected low surrogate following the high surrogate";
       default:
            return "Unknown JSON parser error";
        }
    }
};

inline
const std::error_category& cbor_error_category()
{
  static cbor_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(cbor_parser_errc result)
{
    return std::error_code(static_cast<int>(result),cbor_error_category());
}


}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::cbor::cbor_parser_errc> : public true_type
    {
    };
}

#endif
