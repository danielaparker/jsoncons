/// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_YAML_YMAL_ERROR_HPP
#define JSONCONS_YAML_YMAL_ERROR_HPP

#include <system_error>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {
namespace yaml {

    enum class yaml_errc
    {
        success = 0,
        unexpected_eof = 1,
        source_error,
        syntax_error,
        extra_character,
        max_nesting_depth_exceeded,
        single_quote,
        illegal_character_in_string,
        extra_comma,
        expected_key,
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

    class yaml_error_category_impl
       : public std::error_category
    {
    public:
        const char* name() const noexcept override
        {
            return "jsoncons/yaml";
        }
        std::string message(int ev) const override
        {
            switch (static_cast<yaml_errc>(ev))
            {
                case yaml_errc::unexpected_eof:
                    return "Unexpected end of file";
                case yaml_errc::source_error:
                    return "Source error";
                case yaml_errc::syntax_error:
                    return "JSON syntax_error";
                case yaml_errc::extra_character:
                    return "Unexpected non-whitespace character after JSON text";
                case yaml_errc::max_nesting_depth_exceeded:
                    return "Data item nesting exceeds limit in options";
                case yaml_errc::single_quote:
                    return "JSON strings cannot be quoted with single quotes";
                case yaml_errc::illegal_character_in_string:
                    return "Illegal character in string";
                case yaml_errc::extra_comma:
                    return "Extra comma";
                case yaml_errc::expected_key:
                    return "Expected object member key";
                case yaml_errc::expected_value:
                    return "Expected value";
                case yaml_errc::invalid_value:
                    return "Invalid value";
                case yaml_errc::expected_colon:
                    return "Expected name separator ':'";
                case yaml_errc::illegal_control_character:
                    return "Illegal control character in string";
                case yaml_errc::illegal_escaped_character:
                    return "Illegal escaped character in string";
                case yaml_errc::expected_codepoint_surrogate_pair:
                    return "Invalid codepoint, expected another \\u token to begin the second half of a codepoint surrogate pair.";
                case yaml_errc::invalid_hex_escape_sequence:
                    return "Invalid codepoint, expected hexadecimal digit.";
                case yaml_errc::invalid_unicode_escape_sequence:
                    return "Invalid codepoint, expected four hexadecimal digits.";
                case yaml_errc::leading_zero:
                    return "A number cannot have a leading zero";
                case yaml_errc::invalid_number:
                    return "Invalid number";
                case yaml_errc::expected_comma_or_right_brace:
                    return "Expected comma or right brace '}'";
                case yaml_errc::expected_comma_or_right_bracket:
                    return "Expected comma or right bracket ']'";
                case yaml_errc::unexpected_right_brace:
                    return "Unexpected right brace '}'";
                case yaml_errc::unexpected_right_bracket:
                    return "Unexpected right bracket ']'";
                case yaml_errc::illegal_comment:
                    return "Illegal comment";
                case yaml_errc::expected_continuation_byte:
                    return "Expected continuation byte";
                case yaml_errc::over_long_utf8_sequence:
                    return "Over long UTF-8 sequence";
                case yaml_errc::illegal_codepoint:
                    return "Illegal codepoint (>= 0xd800 && <= 0xdfff)";
                case yaml_errc::illegal_surrogate_value:
                    return "UTF-16 surrogate values are illegal in UTF-32";
                case yaml_errc::unpaired_high_surrogate:
                    return "Expected low surrogate following the high surrogate";
               default:
                    return "Unknown JSON parser error";
                }
        }
    };

    inline
    const std::error_category& yaml_error_category()
    {
      static yaml_error_category_impl instance;
      return instance;
    }

    inline 
    std::error_code make_error_code(yaml_errc result)
    {
        return std::error_code(static_cast<int>(result),yaml_error_category());
    }

} // namespace yaml
} // namespace jsoncons

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::yaml::yaml_errc> : public true_type
    {
    };
}

#endif
