/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_PARSER_CATEGORY_HPP
#define JSONCONS_JSON_PARSER_CATEGORY_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons {

namespace json_parser_errc 
{
    enum json_parser_errc_t 
    {
        unexpected_eof,
        invalid_json_text,
        extra_character,
        max_depth_exceeded,
        mismatched_parentheses_or_brackets,
        single_quote,
        illegal_character_in_string,
        expected_comma_or_end,
        extra_comma,
        unexpected_end_of_object,
        unexpected_end_of_array,
        expected_name,
        expected_value,
        expected_colon,
        illegal_control_character,
        illegal_escaped_character,
        expected_codepoint_surrogate_pair,
        invalid_hex_escape_sequence,
        invalid_unicode_escape_sequence,
        leading_zero,
        invalid_number
    };
}

class json_parser_category_impl
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
            return "A jSON text must be an object or array";
        case json_parser_errc::extra_character:
            return "Unexpected non-whitespace character after JSON text";
        case json_parser_errc::max_depth_exceeded:
            return "Maximum JSON depth exceeded";
        case json_parser_errc::mismatched_parentheses_or_brackets:
            return "Mismatched parentheses or brackets";
        case json_parser_errc::single_quote:
            return "JSON strings cannot be quoted with single quotes";
        case json_parser_errc::illegal_character_in_string:
            return "Illegal character in string";
        case json_parser_errc::extra_comma:
            return "Extra comma";
        case json_parser_errc::expected_comma_or_end:
            return "Expected comma or end";
        case json_parser_errc::unexpected_end_of_object:
            return "Unexpected end of object '}'";
        case json_parser_errc::unexpected_end_of_array:
            return "Unexpected end of array ']'";
        case json_parser_errc::expected_name:
            return "Expected object member name";
        case json_parser_errc::expected_value:
            return "Expected value";
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
        default:
            return "Unknown JSON parser error";
        }
    }
};

inline
const std::error_category& json_parser_category()
{
  static json_parser_category_impl instance;
  return instance;
}

}
#endif
