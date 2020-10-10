/// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CONVERT_ERROR_HPP
#define JSONCONS_CONVERT_ERROR_HPP

#include <system_error>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {

    class convert_error : public std::system_error, public virtual json_exception
    {
        std::size_t line_number_;
        std::size_t column_number_;
        mutable std::string what_;
    public:
        convert_error(std::error_code ec)
            : std::system_error(ec), line_number_(0), column_number_(0)
        {
        }
        convert_error(std::error_code ec, const std::string& what_arg)
            : std::system_error(ec, what_arg), line_number_(0), column_number_(0)
        {
        }
        convert_error(std::error_code ec, std::size_t position)
            : std::system_error(ec), line_number_(0), column_number_(position)
        {
        }
        convert_error(std::error_code ec, std::size_t line, std::size_t column)
            : std::system_error(ec), line_number_(line), column_number_(column)
        {
        }
        convert_error(const convert_error& other) = default;

        convert_error(convert_error&& other) = default;

        const char* what() const noexcept override
        {
            if (what_.empty())
            {
                JSONCONS_TRY
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
                JSONCONS_CATCH(...)
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

    enum class convert_errc
    {
        success = 0,
        conversion_failed,
        not_utf8,
        not_wide_char,
        not_vector,
        not_array,
        not_map,
        not_pair,
        not_string,
        not_string_view,
        not_byte_string,
        not_byte_string_view,
        not_integer,
        not_signed_integer,
        not_unsigned_integer,
        not_bigint,
        not_double,
        not_bool,
        not_variant,
        not_nullptr,
        not_jsoncons_null_type,
        not_bitset
    };
}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::convert_errc> : public true_type
    {
    };
}

namespace jsoncons {

namespace detail {
    class convert_error_category_impl
       : public std::error_category
    {
    public:
        const char* name() const noexcept override
        {
            return "jsoncons/convert";
        }
        std::string message(int ev) const override
        {
            switch (static_cast<convert_errc>(ev))
            {
                case convert_errc::conversion_failed:
                    return "Unable to convert into the provided type";
                case convert_errc::not_utf8:
                    return "Cannot convert string to UTF-8";
                case convert_errc::not_wide_char:
                    return "Cannot convert string to wide characters";
                case convert_errc::not_vector:
                    return "Cannot convert to vector";
                case convert_errc::not_array:
                    return "Cannot convert to std::array";
                case convert_errc::not_map:
                    return "Cannot convert to map";
                case convert_errc::not_pair:
                    return "Cannot convert to std::pair";
                case convert_errc::not_string:
                    return "Cannot convert to string";
                case convert_errc::not_string_view:
                    return "Cannot convert to string_view";
                case convert_errc::not_byte_string:
                    return "Cannot convert to byte_string";
                case convert_errc::not_byte_string_view:
                    return "Cannot convert to byte_string_view";
                case convert_errc::not_integer:
                    return "Cannot convert to integer";
                case convert_errc::not_signed_integer:
                    return "Cannot convert to signed integer";
                case convert_errc::not_unsigned_integer:
                    return "Cannot convert to unsigned integer";
                case convert_errc::not_bigint:
                    return "Cannot convert to bigint";
                case convert_errc::not_double:
                    return "Cannot convert to double";
                case convert_errc::not_bool:
                    return "Cannot convert to bool";
                case convert_errc::not_variant:
                    return "Cannot convert to std::variant";
                case convert_errc::not_nullptr:
                    return "Cannot convert to std::nullptr_t";
                case convert_errc::not_jsoncons_null_type:
                    return "Cannot convert to jsoncons::null_type";
                case convert_errc::not_bitset:
                    return "Cannot convert to std::bitset";
                default:
                    return "Unknown convert error";
            }
        }
    };
} // detail

extern inline
const std::error_category& convert_error_category()
{
  static detail::convert_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(convert_errc result)
{
    return std::error_code(static_cast<int>(result),convert_error_category());
}

}

#endif
