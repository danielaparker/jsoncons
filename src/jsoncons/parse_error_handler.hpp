/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_PARSE_ERROR_HANDLER_HPP
#define JSONCONS_PARSE_ERROR_HANDLER_HPP

#include <jsoncons/jsoncons.hpp>
#include <jsoncons/json_error_category.hpp>
#include <system_error>

namespace jsoncons {

class parse_error : public std::exception, public virtual json_exception
{
public:
    parse_error()
        : line_number_(0),
          column_number_(0)
    {
    }
    parse_error(std::error_code ec,
                size_t line,
                size_t column)
        : error_code_(ec),
          line_number_(line),
          column_number_(column)
    {
    }
    parse_error(const parse_error& other)
        : error_code_(other.error_code_),
          line_number_(other.line_number_),
          column_number_(other.column_number_)
    {
    }

    const char* what() const JSONCONS_NOEXCEPT override
    {
        try
        {
            std::ostringstream os;
            os << error_code_.message() << " at line " << line_number_ << " and column " << column_number_;
            const_cast<std::string&>(buffer_) = os.str();
            return buffer_.c_str();
        }
        catch (...)
        {
            return "";
        }
    }

    const std::error_code code() const
    {
        return error_code_;
    }

    size_t line_number() const
    {
        return line_number_;
    }

    size_t column_number() const
    {
        return column_number_;
    }

    parse_error& operator=(const parse_error& e)
    {
        error_code_ = e.error_code_;
        line_number_ = e.line_number_;
        column_number_ = e.column_number_;
        return *this;
    }
private:
    std::error_code error_code_;
    std::string buffer_;
    size_t line_number_;
    size_t column_number_;
};

#if !defined(JSONCONS_NO_DEPRECATED)
typedef parse_error json_parse_exception;
typedef parse_error parse_exception;
#endif

template<class CharT>
class basic_parsing_context
{
public:
    virtual ~basic_parsing_context() {}

    size_t line_number() const
    {
        return do_line_number();
    }
    size_t column_number() const 
    {
        return do_column_number();
    }
    CharT current_char() const
    {
        return do_current_char();
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    CharT last_char() const
    {
        return do_current_char();
    }
#endif

private:
    virtual size_t do_line_number() const = 0;
    virtual size_t do_column_number() const = 0;
    virtual CharT do_current_char() const = 0;
};

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

template <class CharT>
class basic_parse_error_handler
{
public:
    virtual ~basic_parse_error_handler()
    {
    }

    bool error(std::error_code ec,
               const basic_parsing_context<CharT>& context) JSONCONS_NOEXCEPT 
    {
        return do_error(ec,context);
    }

    void fatal_error(std::error_code ec,
                     const basic_parsing_context<CharT>& context) JSONCONS_NOEXCEPT 
    {
        do_fatal_error(ec,context);
    }

private:
    virtual bool do_error(std::error_code,
                          const basic_parsing_context<CharT>& context) JSONCONS_NOEXCEPT = 0;

    virtual void do_fatal_error(std::error_code,
                                const basic_parsing_context<CharT>&) JSONCONS_NOEXCEPT
    {
    }
};

template <class CharT>
class basic_default_parse_error_handler : public basic_parse_error_handler<CharT>
{
private:
    virtual bool do_error(std::error_code code,
                          const basic_parsing_context<CharT>&) JSONCONS_NOEXCEPT
    {
        static const std::error_code illegal_comment = make_error_code(json_parser_errc::illegal_comment);

        if (code == illegal_comment)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
};

template <class CharT>
class basic_strict_parse_error_handler : public basic_parse_error_handler<CharT>
{
private:
    virtual bool do_error(std::error_code,
                          const basic_parsing_context<CharT>&) JSONCONS_NOEXCEPT
    {
        return true;
    }
};

typedef basic_parse_error_handler<char> parse_error_handler;
typedef basic_parse_error_handler<wchar_t> wparse_error_handler;

typedef basic_default_parse_error_handler<char> default_parse_error_handler;
typedef basic_default_parse_error_handler<wchar_t> wdefault_parse_error_handler;
typedef basic_strict_parse_error_handler<char> strict_parse_error_handler;
typedef basic_strict_parse_error_handler<wchar_t> wstrict_parse_error_handler;

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

}
#endif
