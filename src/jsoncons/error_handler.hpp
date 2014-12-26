// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_ERROR_HANDLER_HPP
#define JSONCONS_ERROR_HANDLER_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons {

class json_parse_exception : public json_exception
{
public:
    json_parse_exception(std::string message,
                         unsigned long line,
                         unsigned long column)
        : message_(message),
          line_number_(line),
          column_number_(column)
    {
    }
    json_parse_exception(const json_parse_exception& other)
        : message_(other.message_),
          line_number_(other.line_number_),
          column_number_(other.column_number_)
    {
    }
    const char* what() const JSONCONS_NOEXCEPT
    {
        std::ostringstream os;
        os << message_ << " on line " << line_number_ << " at column " << column_number_;
        const_cast<std::string&>(message_) = os.str();
        return message_.c_str();
    }

    const std::string& message() const
    {
        return message_;
    }

    unsigned long line_number() const
    {
        return line_number_;
    }

    unsigned long column_number() const
    {
        return column_number_;
    }
private:
    std::string message_;
    std::string buffer_;
    unsigned long line_number_;
    unsigned long column_number_;
};

template<typename Char>
class basic_parsing_context
{
public:
    virtual ~basic_parsing_context() {}

    Char get() 
    {
        return do_get();
    }
    Char peek() 
    {
        return do_peek();
    }
    bool eof() const
    {
        return do_eof();
    }
    unsigned long line_number() const
    {
        return do_line_number();
    }
    unsigned long column_number() const 
    {
        return do_column_number();
    }
    size_t minimum_structure_capacity() const 
    {
        return do_minimum_structure_capacity();
    }

private:
    virtual Char do_get() = 0; 
    virtual Char do_peek() = 0;
    virtual bool do_eof() = 0;
    virtual unsigned long do_line_number() const = 0;
    virtual unsigned long do_column_number() const = 0;
    virtual size_t do_minimum_structure_capacity() const = 0;
};

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

template <typename Char>
class basic_error_handler
{
public:
    virtual ~basic_error_handler()
    {
    }

    void warning(std::error_code ec,
                 const basic_parsing_context<Char>& context) throw (json_parse_exception) 
    {
        do_warning(ec,context);
    }

    void error(std::error_code ec,
               const basic_parsing_context<Char>& context) throw (json_parse_exception) 
    {
        do_error(ec,context);
    }

private:
    virtual void do_warning(std::error_code,
                            const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;

    virtual void do_error(std::error_code,
                          const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;
};

template <typename Char>
class default_error_handler_impl : public basic_error_handler<Char>
{
private:
    virtual void do_warning(std::error_code,
                            const basic_parsing_context<Char>& context) throw (json_parse_exception) 
    {
    }

    virtual void do_error(std::error_code ec,
                          const basic_parsing_context<Char>& context) throw (json_parse_exception)
    {
        throw json_parse_exception(ec.message(),context.line_number(),context.column_number());
    }
};

typedef basic_error_handler<char> error_handler;
typedef basic_error_handler<wchar_t> werror_handler;

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

template<typename Char>
basic_error_handler<Char>& default_error_handler()
{
    static default_error_handler_impl<Char> instance;
    return instance;
}

namespace json_parser_error {
enum json_parser_error_t 
{
    unexpected_eof,
    unexpected_name_separator,
    unexpected_value_separator,
    expected_value_separator,
    unexpected_end_of_object,
    unexpected_end_of_array,
    expected_name_or_value,
    expected_name_separator,
    illegal_control_character,
    illegal_escaped_character,
    invalid_codepoint_surrogate_pair,
    invalid_hex_escape_sequence,
    invalid_unicode_escape_sequence,
    invalid_number,
    value_not_found,
    eof_reading_string_value,
    eof_reading_numeric_value
};
}

class json_parser_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const
    {
        return "JSON parser";
    }
    virtual std::string message(int ev) const
    {
        switch (ev)
        {
        case json_parser_error::unexpected_eof:
            return "Unexpected end of file";
        case json_parser_error::unexpected_name_separator:
            return "Unexpected name separator ','";
        case json_parser_error::unexpected_value_separator:
            return "Unexpected value separator ','";
        case json_parser_error::expected_value_separator:
            return "Expected value separator ','";
        case json_parser_error::unexpected_end_of_object:
            return "Unexpected end of object '}'";
        case json_parser_error::unexpected_end_of_array:
            return "Unexpected end of array ']'";
        case json_parser_error::expected_name_or_value:
            return "Expected name or value";
        case json_parser_error::expected_name_separator:
            return "Expected name separator ':'";
        case json_parser_error::illegal_control_character:
            return "Illegal control character in string";
        case json_parser_error::illegal_escaped_character:
            return "Illegal escaped character in string";
        case json_parser_error::invalid_codepoint_surrogate_pair:
            return "Invalid codepoint, expected another \\u token to begin the second half of a codepoint surrogate pair.";
        case json_parser_error::invalid_hex_escape_sequence:
            return "Invalid codepoint, expected hexadecimal digit.";
        case json_parser_error::invalid_unicode_escape_sequence:
            return "Invalid codepoint, expected four hexadecimal digits.";
        case json_parser_error::invalid_number:
            return "Invalid number";
        case json_parser_error::value_not_found:
            return "Value not found";
        case json_parser_error::eof_reading_string_value:
            return "Reached end of file while reading string value";
        case json_parser_error::eof_reading_numeric_value:
            return "Reached end of file while reading numeric value";
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

inline
std::error_code make_error_code(json_parser_error::json_parser_error_t e)
{
  return std::error_code(
      static_cast<int>(e),
      json_parser_category());
}

}
#endif
