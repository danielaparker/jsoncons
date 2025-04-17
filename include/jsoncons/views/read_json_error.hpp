// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_READ_JSON_ERROR_HPP
#define JSONCONS_READ_JSON_ERROR_HPP

#include <system_error>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {

    enum class read_json_errc
    {
        success = 0,
        invalid_utf8,
        invalid_escaped_sequence,
        invalid_high_surrogate,
        no_low_surrogate,
        invalid_escaped_character,
        invalid_low_surrogate,
        unexpected_control_character,
        unclosed_string,
        not_an_array,
        not_an_object,
        not_a_number,
        not_a_string,
        not_a_boolean,
        not_a_null,
        invalid_parameter,
        memory_allocation,
        empty_content,
        unexpected_content,
        unexpected_end_of_input,
        unexpected_character,
        invalid_structure,
        invalid_number,
        invalid_string,
        invalid_literal,
        unclosed_multiline_comment,
        file_open,
        file_read,
        inf_or_nan,
        no_digit_after_minus_sign,
        no_digit_after_decimal_point,
        leading_zero,
        no_digit_after_exponent_sign,
        unexpected_rbracket,
        unexpected_rbrace,
        conversion_error
    };

    class json_read_error_category_impl
       : public std::error_category
    {
    public:
        const char* name() const noexcept final
        {
            return "jsoncons/read_json";
        }
        std::string message(int ev) const final
        {
            switch (static_cast<read_json_errc>(ev))
            {
                case read_json_errc::not_an_array:
                    return "Not an array";
                case read_json_errc::invalid_utf8:
                    return "Invalid UTF-8 encoding in string";
                case read_json_errc::invalid_escaped_sequence:
                    return "Invalid escaped sequence in string";
                case read_json_errc::invalid_high_surrogate:
                    return "Invalid high surrogate in string";
                case read_json_errc::no_low_surrogate:
                    return "No low surrogate in string";
                case read_json_errc::invalid_escaped_character:
                    return "Invalid escaped character in string";
                case read_json_errc::invalid_low_surrogate:
                    return "Invalid low surrogate in string";
                case read_json_errc::unexpected_control_character:
                    return "Unexpected control character in string";
                case read_json_errc::unclosed_string:
                    return "Unclosed string";
                case read_json_errc::not_an_object:
                    return "Not an object";
                case read_json_errc::not_a_number:
                    return "Not a number";
                case read_json_errc::not_a_string:
                    return "Not a string";
                case read_json_errc::not_a_boolean:
                    return "Not a boolean";
                case read_json_errc::not_a_null:
                    return "Not a null";
                case read_json_errc::invalid_parameter:
                    return "Invalid parameter";
                case read_json_errc::memory_allocation:
                    return "Source error";
                case read_json_errc::empty_content:
                    return "JSON syntax_error";
                case read_json_errc::unexpected_content:
                    return "Unexpected non-whitespace character after JSON text";
                case read_json_errc::unexpected_end_of_input:
                    return "Unexpected end of input";
                case read_json_errc::unexpected_character:
                    return "JSON strings cannot be quoted with single quotes";
                case read_json_errc::invalid_structure:
                    return "Illegal character in string";
                case read_json_errc::unclosed_multiline_comment:
                    return "Unclosed multiline comment";
                case read_json_errc::invalid_number:
                    return "Expected object member key";
                case read_json_errc::invalid_string:
                    return "Expected value";
                case read_json_errc::invalid_literal:
                    return "Invalid value";
                case read_json_errc::file_open:
                    return "Expected name separator ':'";
                case read_json_errc::file_read:
                    return "Illegal control character in string";
                case read_json_errc::inf_or_nan:
                    return "Number is infinity when parsed as double";
                case read_json_errc::no_digit_after_minus_sign:
                    return "No digit after minus sign";
                case read_json_errc::no_digit_after_decimal_point:
                    return "No digit after decimal point";
                case read_json_errc::leading_zero:
                    return "Number with leading zero is not allowed";
                case read_json_errc::no_digit_after_exponent_sign:
                    return "No digit after exponent sign";
                case read_json_errc::unexpected_rbrace:
                    return "Unexpected right brace '}'";
                case read_json_errc::unexpected_rbracket:
                    return "Unexpected right bracket ']'";
                case read_json_errc::conversion_error:
                    return "Can't convert type";
                default:
                    return "Unknown JSON read error";
                }
        }
    };

    JSONCONS_FORCE_INLINE
    const std::error_category& json_read_error_category() noexcept
    {
      static json_read_error_category_impl instance;
      return instance;
    }

    JSONCONS_FORCE_INLINE 
    std::error_code make_error_code(read_json_errc result) noexcept
    {
        return std::error_code(static_cast<int>(result),json_read_error_category());
    }

} // jsoncons

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::read_json_errc> : public true_type
    {
    };
}

#endif
