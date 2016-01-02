/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_TEXT_ERROR_CATEGORY_HPP
#define JSONCONS_JSON_TEXT_ERROR_CATEGORY_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons {

namespace json_parser_errc 
{
    const int unexpected_eof = 0;
    const int invalid_json_text = 1;
    const int extra_character = 2;
    const int max_depth_exceeded = 3;
    const int single_quote = 5;
    const int illegal_character_in_string = 6;
    const int expected_comma_or_end = 7;
    const int extra_comma = 8;
    const int expected_name = 11;
    const int expected_value = 12;
    const int invalid_value = 13;
    const int expected_colon = 14;
    const int illegal_control_character = 15;
    const int illegal_escaped_character = 16;
    const int expected_codepoint_surrogate_pair = 17;
    const int invalid_hex_escape_sequence = 18;
    const int invalid_unicode_escape_sequence = 19;
    const int leading_zero = 20;
    const int invalid_number = 21;
    const int bad_state = 22;
    const int expected_comma_or_right_bracket = 23;
    const int expected_comma_or_right_brace = 24;
    const int unexpected_right_bracket = 25;
    const int unexpected_right_brace = 26;
}

class json_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "json text";
    }
    virtual std::string message(int ev) const
    {
        switch (ev)
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
        case json_parser_errc::expected_comma_or_end:
            return "Expected comma or end";
        case json_parser_errc::expected_comma_or_right_bracket:
            return "Expected comma or right bracket";
        case json_parser_errc::expected_comma_or_right_brace:
            return "Expected comma or right brace";
        case json_parser_errc::unexpected_right_bracket:
            return "Unexpected right bracket";
        case json_parser_errc::unexpected_right_brace:
            return "Unexpected right bracket";
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
        case json_parser_errc::invalid_number:
            return "Invalid number";
        case json_parser_errc::leading_zero:
            return "A number cannot have a leading zero";
        case json_parser_errc::bad_state:
            return "Bad state";
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

}
#endif
