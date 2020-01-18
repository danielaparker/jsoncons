/// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JMESPATH_JMESPATH_ERROR_HPP
#define JSONCONS_JMESPATH_JMESPATH_ERROR_HPP

#include <jsoncons/json_exception.hpp>
#include <system_error>

namespace jsoncons { namespace jmespath {

class jmespath_error : public std::system_error, public virtual json_exception
{
    std::string buffer_;
    std::size_t line_number_;
    std::size_t column_number_;
public:
    jmespath_error(std::error_code ec)
        : std::system_error(ec), line_number_(0), column_number_(0)
    {
    }
    jmespath_error(std::error_code ec, std::size_t position)
        : std::system_error(ec), line_number_(0), column_number_(position)
    {
    }
    jmespath_error(std::error_code ec, std::size_t line, std::size_t column)
        : std::system_error(ec), line_number_(line), column_number_(column)
    {
    }
    jmespath_error(const jmespath_error& other) = default;

    jmespath_error(jmespath_error&& other) = default;

    const char* what() const noexcept override
    {
        JSONCONS_TRY
        {
            std::ostringstream os;
            os << std::system_error::what();
            if (line_number_ != 0 && column_number_ != 0)
            {
                os << " at line " << line_number_ << " and column " << column_number_;
            }
            else if (column_number_ != 0)
            {
                os << " at position " << column_number_;
            }
            const_cast<std::string&>(buffer_) = os.str();
            return buffer_.c_str();
        }
        JSONCONS_CATCH(...)
        {
            return std::system_error::what();
        }
    }

    std::size_t line() const noexcept
    {
        return line_number_;
    }

    std::size_t column() const noexcept
    {
        return column_number_;
    }
#if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED_MSG("Instead, use line()")
    std::size_t line_number() const noexcept
    {
        return line();
    }

    JSONCONS_DEPRECATED_MSG("Instead, use column()")
    std::size_t column_number() const noexcept
    {
        return column();
    }
#endif
};

enum class jmespath_errc 
{
    success = 0,
    expected_identifier,
    expected_index,
    expected_A_Za_Z_,
    expected_right_bracket,
    expected_right_brace,
    expected_colon,
    expected_dot,
    invalid_number,
    expected_comparator,
    expected_key,

    expected_root,
    expected_current_node,
    expected_name,
    expected_separator,
    invalid_filter,
    invalid_filter_expected_slash,
    invalid_filter_unbalanced_paren,
    invalid_filter_unsupported_operator,
    invalid_filter_expected_right_brace,
    invalid_filter_expected_primary,
    expected_slice_start,
    expected_slice_end,
    expected_slice_step,
    expected_left_bracket_token,
    expected_minus_or_digit_or_colon_or_comma_or_right_bracket,
    expected_digit_or_colon_or_comma_or_right_bracket,
    expected_minus_or_digit_or_comma_or_right_bracket,
    expected_digit_or_comma_or_right_bracket,
    unexpected_operator,
    invalid_function_name,
    invalid_argument,
    function_name_not_found,
    parse_error_in_filter,
    argument_parse_error,
    unidentified_error,
    unexpected_end_of_input
};

class jmespath_error_category_impl
   : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "jsoncons/jmespath";
    }
    std::string message(int ev) const override
    {
        switch (static_cast<jmespath_errc>(ev))
        {
            case jmespath_errc::expected_identifier:
                return "Expected identifier";
            case jmespath_errc::expected_index:
                return "Expected index";
            case jmespath_errc::expected_A_Za_Z_:
                return "Expected A-Z, a-z, or _";
            case jmespath_errc::invalid_number:
                return "Invalid number";
            case jmespath_errc::expected_comparator:
                return "Expected <, <=, ==, >=, > or !=";
            case jmespath_errc::expected_key:
                return "Expected key";
            case jmespath_errc::expected_right_bracket:
                return "Expected ]";
            case jmespath_errc::expected_right_brace:
                return "Expected }";
            case jmespath_errc::expected_colon:
                return "Expected :";


            case jmespath_errc::expected_root:
                return "Expected $";
            case jmespath_errc::expected_current_node:
                return "Expected @";
            case jmespath_errc::expected_dot:
                return "Expected \".\"";
            case jmespath_errc::expected_name:
                return "Expected a name following a dot";
            case jmespath_errc::expected_slice_start:
                return "Expected slice start";
            case jmespath_errc::expected_slice_end:
                return "Expected slice end";
            case jmespath_errc::expected_slice_step:
                return "Expected slice step";
            case jmespath_errc::expected_separator:
                return "Expected dot or left bracket separator";
            case jmespath_errc::invalid_filter:
                return "Invalid path filter";
            case jmespath_errc::invalid_filter_expected_slash:
                return "Invalid path filter, expected '/'";
            case jmespath_errc::invalid_filter_unbalanced_paren:
                return "Invalid path filter, unbalanced parenthesis";
            case jmespath_errc::invalid_filter_unsupported_operator:
                return "Unsupported operator";
            case jmespath_errc::invalid_filter_expected_right_brace:
                return "Invalid path filter, expected right brace }";
            case jmespath_errc::invalid_filter_expected_primary:
                return "Invalid path filter, expected primary expression.";
            case jmespath_errc::expected_left_bracket_token:
                return "Expected ?,',\",0-9,*";
            case jmespath_errc::expected_minus_or_digit_or_colon_or_comma_or_right_bracket:
                return "Expected - or 0-9 or : or , or ]";
            case jmespath_errc::expected_minus_or_digit_or_comma_or_right_bracket:
                return "Expected - or 0-9 or , or ]";
            case jmespath_errc::expected_digit_or_comma_or_right_bracket:
                return "Expected - or 0-9 or , or ]";
            case jmespath_errc::expected_digit_or_colon_or_comma_or_right_bracket:
                return "Expected 0-9 or : or , or ]";
            case jmespath_errc::invalid_function_name:
                return "Invalid function name";
            case jmespath_errc::invalid_argument:
                return "Invalid argument type";
            case jmespath_errc::function_name_not_found:
                return "Function name not found";
            case jmespath_errc::parse_error_in_filter:
                return "Could not parse JSON expression in a JSONPath filter";
            case jmespath_errc::argument_parse_error:
                return "Could not parse JSON expression passed to JSONPath function";
            case jmespath_errc::unidentified_error:
                return "Unidentified error";
            case jmespath_errc::unexpected_end_of_input:
                return "Unexpected end of jmespath input";
            default:
                return "Unknown jmespath parser error";
        }
    }
};

inline
const std::error_category& jmespath_error_category()
{
  static jmespath_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(jmespath_errc result)
{
    return std::error_code(static_cast<int>(result),jmespath_error_category());
}

}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::jmespath::jmespath_errc> : public true_type
    {
    };
}

#endif
