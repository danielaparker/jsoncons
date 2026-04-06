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
        unterminated_quoted_key,
        unterminated_fields_segment,
        extra_character,
        missing_colon_after_key,
        missing_closing_quote,
        blank_lines_in_array,
        blank_lines_in_object,
        too_many_values_in_row,
        too_few_values_in_row,
        missing_colon,
        invalid_escape_sequence,
        inline_array_length_mismatch,
        list_array_length_mismatch,
        tabular_array_length_mismatch,
        invalid_value,
        max_nesting_depth_exceeded 
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
                case toon_errc::unterminated_quoted_key:
                    return "Unterminated quoted key";
                case toon_errc::unterminated_fields_segment:
                    return "Unterminated fields segment";
                case toon_errc::extra_character:
                    return "Unexpected non-whitespace character after TOON text";
                case toon_errc::missing_colon_after_key:
                    return "Missing colon after key";
                case toon_errc::missing_closing_quote:
                    return "Unterminated string: missing closing quote";
                case toon_errc::blank_lines_in_array:
                    return "Blank lines not allowed inside arrays";
                case toon_errc::blank_lines_in_object:
                    return "Blank lines not allowed inside objects";
                case toon_errc::too_many_values_in_row:
                    return "Too many values in tabular array row";
                case toon_errc::too_few_values_in_row:
                    return "Too few values in tabular array row";
                case toon_errc::missing_colon:
                    return "Missing colon in key-value context";
                case toon_errc::invalid_escape_sequence:
                    return "Invalid escape sequence";
                case toon_errc::inline_array_length_mismatch:
                    return "Inline array length mismatch";
                case toon_errc::list_array_length_mismatch:
                    return "List array length mismatch";
                case toon_errc::tabular_array_length_mismatch:
                    return "Tabular array length mismatch";
                case toon_errc::max_nesting_depth_exceeded:
                    return "Data item nesting exceeds limit in options";
                case toon_errc::invalid_value:
                    return "Invalid value";
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
