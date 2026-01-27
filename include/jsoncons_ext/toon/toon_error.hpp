/// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_TOON_TOON_ERROR_HPP
#define JSONCONS_TOON_TOON_ERROR_HPP

#include <string>
#include <system_error>
#include <type_traits>

#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {
namespace toon {

    enum class toon_errc
    {
        success = 0,
        unexpected_eof = 1,
        source_error,
        syntax_error,
        tab_in_indentation,
        indent_not_multiple_of_indent_size,
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
        expected_comma_or_rbrace,
        expected_comma_or_rbracket,
        unexpected_rbracket,
        unexpected_rbrace,
        illegal_comment,
        expected_continuation_byte,
        over_long_utf8_sequence,
        illegal_codepoint,
        illegal_surrogate_value,
        unpaired_high_surrogate,
        illegal_unicode_character,
        unexpected_character
    };

    class toon_error_category_impl
       : public std::error_category
    {
    public:
        const char* name() const noexcept final
        {
            return "jsoncons/json";
        }
        std::string message(int ev) const final
        {
            switch (static_cast<toon_errc>(ev))
            {
                case toon_errc::unexpected_eof:
                    return "Unexpected end of file";
                case toon_errc::source_error:
                    return "Source error";
                case toon_errc::syntax_error:
                    return "TOON syntax_error";
                case toon_errc::tab_in_indentation:
                    return "Illegal tab in indentation in strict mode";
                case toon_errc::indent_not_multiple_of_indent_size:
                    return "Indent spaces must be exact multiple of indent size";
                case toon_errc::extra_character:
                    return "Unexpected non-whitespace character after TOON text";
                case toon_errc::max_nesting_depth_exceeded:
                    return "Data item nesting exceeds limit in options";
                case toon_errc::single_quote:
                    return "TOON strings cannot be quoted with single quotes";
                case toon_errc::illegal_character_in_string:
                    return "Illegal character in string";
                case toon_errc::extra_comma:
                    return "Extra comma";
                case toon_errc::expected_key:
                    return "Expected object member key";
                case toon_errc::expected_value:
                    return "Expected value";
                case toon_errc::invalid_value:
                    return "Invalid value";
                case toon_errc::expected_colon:
                    return "Expected name separator ':'";
                case toon_errc::illegal_control_character:
                    return "Illegal control character in string";
                case toon_errc::illegal_escaped_character:
                    return "Illegal escaped character in string";
                case toon_errc::expected_codepoint_surrogate_pair:
                    return "Invalid codepoint, expected another \\u token to begin the second half of a codepoint surrogate pair.";
                case toon_errc::invalid_hex_escape_sequence:
                    return "Invalid codepoint, expected hexadecimal digit.";
                case toon_errc::invalid_unicode_escape_sequence:
                    return "Invalid codepoint, expected four hexadecimal digits.";
                case toon_errc::leading_zero:
                    return "A number cannot have a leading zero";
                case toon_errc::invalid_number:
                    return "Invalid number";
                case toon_errc::expected_comma_or_rbrace:
                    return "Expected comma or right brace '}'";
                case toon_errc::expected_comma_or_rbracket:
                    return "Expected comma or right bracket ']'";
                case toon_errc::unexpected_rbrace:
                    return "Unexpected right brace '}'";
                case toon_errc::unexpected_rbracket:
                    return "Unexpected right bracket ']'";
                case toon_errc::illegal_comment:
                    return "Illegal comment";
                case toon_errc::expected_continuation_byte:
                    return "Expected continuation byte";
                case toon_errc::over_long_utf8_sequence:
                    return "Over long UTF-8 sequence";
                case toon_errc::illegal_codepoint:
                    return "Illegal codepoint (>= 0xd800 && <= 0xdfff)";
                case toon_errc::illegal_surrogate_value:
                    return "UTF-16 surrogate values are illegal in UTF-32";
                case toon_errc::unpaired_high_surrogate:
                    return "Expected low surrogate following the high surrogate";
                case toon_errc::illegal_unicode_character:
                    return "Illegal unicode character";
                case toon_errc::unexpected_character:
                    return "Unexpected character";
                default:
                    return "Unknown TOON parser error";
                }
        }
    };

    inline const std::error_category& toon_error_category() noexcept
    {
      static toon_error_category_impl instance;
      return instance;
    }

    inline std::error_code make_error_code(toon_errc result) noexcept
    {
        return std::error_code(static_cast<int>(result),toon_error_category());
    }

} // namespace toon
} // namespace jsoncons

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::toon::toon_errc> : public true_type
    {
    };

} // namespace std

#endif // JSONCONS_TOON_TOON_ERROR_HPP
