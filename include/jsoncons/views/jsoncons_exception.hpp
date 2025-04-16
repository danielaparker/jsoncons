// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS2_JSON_EXCEPTION_HPP
#define JSONCONS2_JSON_EXCEPTION_HPP

#include <string> // std::string
#include <sstream> // std::ostringstream
#include <system_error> // std::error_code
#include <jsoncons/views/jsoncons_config.hpp>
#include <jsoncons/views/more_type_traits.hpp>

namespace jsoncons2 {

    class bad_cast : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class key_not_found : public std::out_of_range
    {
        std::string name_;
        mutable std::string what_;
    public:
        explicit key_not_found(const char* key, std::size_t length) noexcept
            : std::out_of_range("Key not found"), name_(key, length)
        {
        }

        virtual ~key_not_found() noexcept
        {
        }

        const char* what() const noexcept override
        {
            if (what_.empty())
            {
                JSONCONS2_TRY
                {
                    what_.append(std::out_of_range::what());
                    what_.append(": '");
                    what_.append(name_);
                    what_.append("'");
                    return what_.c_str();
                }
                JSONCONS2_CATCH(...)
                {
                    return std::out_of_range::what();
                }
            }
            else
            {
                return what_.c_str();
            }
        }
    };

    class not_an_object : public std::runtime_error
    {
        std::string name_;
        mutable std::string what_;
    public:
        explicit not_an_object(const char* key, std::size_t length) noexcept
            : std::runtime_error("Attempting to access a member of a value that is not an object"), name_(key, length)
        {
        }

        virtual ~not_an_object() noexcept
        {
        }
        const char* what() const noexcept override
        {
            if (what_.empty())
            {
                JSONCONS2_TRY
                {
                    what_.append(std::runtime_error::what());
                    what_.append(": '");
                    what_.append(name_);
                    what_.append("'");
                    return what_.c_str();
                }
                JSONCONS2_CATCH(...)
                {
                    return std::runtime_error::what();
                }
            }
            else
            {
                return what_.c_str();
            }
        }
    };

    class ser_error : public std::system_error
    {
        std::size_t line_number_;
        std::size_t column_number_;
        mutable std::string what_;
    public:
        ser_error(std::error_code ec)
            : std::system_error(ec), line_number_(0), column_number_(0)
        {
        }
        ser_error(std::error_code ec, const std::string& what_arg)
            : std::system_error(ec, what_arg), line_number_(0), column_number_(0)
        {
        }
        ser_error(std::error_code ec, std::size_t position)
            : std::system_error(ec), line_number_(0), column_number_(position)
        {
        }
        ser_error(std::error_code ec, std::size_t line, std::size_t column)
            : std::system_error(ec), line_number_(line), column_number_(column)
        {
        }
        ser_error(const ser_error& other) = default;

        ser_error(ser_error&& other) = default;

        const char* what() const noexcept override
        {
            if (what_.empty())
            {
                JSONCONS2_TRY
                {
                    what_.append(std::system_error::what());
                    if (line_number_ != 0 && column_number_ != 0)
                    {
                        what_.append(" at line ");
                        what_.append(std::to_string(line_number_));
                        what_.append(" and column ");
                        what_.append(std::to_string(column_number_));
                    }
                    else if (column_number_ != 0)
                    {
                        what_.append(" at position ");
                        what_.append(std::to_string(column_number_));
                    }
                    return what_.c_str();
                }
                JSONCONS2_CATCH(...)
                {
                    return std::system_error::what();
                }
            }
            else
            {
                return what_.c_str();
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
    };

} // namespace jsoncons2

#endif
