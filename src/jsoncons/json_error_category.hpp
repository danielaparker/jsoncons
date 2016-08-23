/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_ERROR_CATEGORY_HPP
#define JSONCONS_JSON_ERROR_CATEGORY_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons {

    enum class json_parser_errc
    {
        unexpected_eof = 1,
        invalid_json_text = 2,
        extra_character = 3,
        max_depth_exceeded = 4,
        single_quote = 5,
        illegal_character_in_string = 6,
        extra_comma = 7,
        expected_name = 8,
        expected_value = 9,
        invalid_value = 10,
        expected_colon = 11,
        illegal_control_character = 12,
        illegal_escaped_character = 13,
        expected_codepoint_surrogate_pair = 14,
        invalid_hex_escape_sequence = 15,
        invalid_unicode_escape_sequence = 16,
        leading_zero = 17,
        invalid_number = 18,
        expected_comma_or_right_brace = 19,
        expected_comma_or_right_bracket = 20,
        unexpected_right_bracket = 21,
        unexpected_right_brace = 22
    };

class json_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "json";
    }
    virtual std::string message(int ev) const
    {
        switch (static_cast<json_parser_errc>(ev))
        {
        case json_parser_errc::unexpected_eof:
            return "Unexpected end of file";
        case json_parser_errc::invalid_json_text:
            return "Invalid JSON text";
        case json_parser_errc::extra_character:
            return "Unexpected non-whitespace character after JSON text";
        case json_parser_errc::max_depth_exceeded:
            return "Maximum JSON depth exceeded";
        case json_parser_errc::single_quote:
            return "JSON strings cannot be quoted with single quotes";
        case json_parser_errc::illegal_character_in_string:
            return "Illegal character in string";
        case json_parser_errc::extra_comma:
            return "Extra comma";
        case json_parser_errc::expected_name:
            return "Expected object member name";
        case json_parser_errc::expected_value:
            return "Expected value";
        case json_parser_errc::invalid_value:
            return "Invalid value";
        case json_parser_errc::expected_colon:
            return "Expected name separator ':'";
        case json_parser_errc::illegal_control_character:
            return "Illegal control character in string";
        case json_parser_errc::illegal_escaped_character:
            return "Illegal escaped character in string";
        case json_parser_errc::expected_codepoint_surrogate_pair:
            return "Invalid codepoint, expected another \\u token to begin the second half of a codepoint surrogate pair.";
        case json_parser_errc::invalid_hex_escape_sequence:
            return "Invalid codepoint, expected hexadecimal digit.";
        case json_parser_errc::invalid_unicode_escape_sequence:
            return "Invalid codepoint, expected four hexadecimal digits.";
        case json_parser_errc::leading_zero:
            return "A number cannot have a leading zero";
        case json_parser_errc::invalid_number:
            return "Invalid number";
        case json_parser_errc::expected_comma_or_right_brace:
            return "Expected comma or right brace ']'";
        case json_parser_errc::expected_comma_or_right_bracket:
            return "Expected comma or right bracket ']'";
        case json_parser_errc::unexpected_right_brace:
            return "Unexpected right brace '}'";
        case json_parser_errc::unexpected_right_bracket:
            return "Unexpected right bracket ']'";
        default:
            return "Unknown JSON parser error";
        }
    }
};

inline
const std::error_category& json_error_category()
{
  static json_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(json_parser_errc result)
{
    return std::error_code(static_cast<int>(result),json_error_category());
}


}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::json_parser_errc> : public true_type
    {
    };
}

#endif
