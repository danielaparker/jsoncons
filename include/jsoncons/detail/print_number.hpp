// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_PRINT_NUMBER_HPP
#define JSONCONS_DETAIL_PRINT_NUMBER_HPP

#include <stdexcept>
#include <string>
#include <cmath>
#include <locale>
#include <limits> // std::numeric_limits
#include <exception>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/parse_number.hpp>

namespace jsoncons { namespace detail {

// print_integer

template<class Result> 
size_t print_integer(int64_t value, Result& writer)
{
    typedef typename Result::value_type char_type;

    size_t count = 0;

    char_type buf[255];
    uint64_t u = (value < 0) ? static_cast<uint64_t>(-value) : static_cast<uint64_t>(value);
    char_type* p = buf;
    do
    {
        *p++ = static_cast<char_type>(48 + u%10);
    }
    while (u /= 10);
    count += (p-buf);
    if (value < 0)
    {
        writer.push_back('-');
        ++count;
    }
    while (--p >= buf)
    {
        writer.push_back(*p);
    }

    return count;
}

// print_uinteger

template<class Result>
size_t print_uinteger(uint64_t value, Result& writer)
{
    typedef typename Result::value_type char_type;

    size_t count = 0;

    char_type buf[255];
    char_type* p = buf;
    do
    {
        *p++ = static_cast<char_type>(48 + value % 10);
    } 
    while (value /= 10);
    count += (p-buf);
    while (--p >= buf)
    {
        writer.push_back(*p);
    }
    return count;
}

// print_double

class print_double
{
private:
    string_to_double to_double_;
    floating_point_options override_;
    char decimal_point_;
public:
    print_double(const floating_point_options& options)
        : override_(options)
    {
#if !defined(JSONCONS_NO_LOCALECONV)
        struct lconv * lc = localeconv();
        if (lc != nullptr && lc->decimal_point[0] != 0)
        {
            decimal_point_ = lc->decimal_point[0];    
        }
        else
#endif
        {
            decimal_point_ = '.'; 
        }
    }

    template <class Result>
    size_t operator()(double val, const floating_point_options& fmt, Result& writer)
    {
        size_t count = 0;

        chars_format format = override_.format() != chars_format() ? override_.format() : fmt.format();

        int decimal_places;
        if (override_.decimal_places() != 0)
        {
            decimal_places = override_.decimal_places();
        }
        else if (fmt.decimal_places() != 0)
        {
            decimal_places = fmt.decimal_places();
        }
        else
        {
            format = chars_format::general;
            decimal_places = 0;
        }             

        char number_buffer[200]; 
        int length = 0;

        switch (format)
        {
        case chars_format::fixed:
            {
                length = snprintf(number_buffer, sizeof(number_buffer), "%1.*f", decimal_places, val);
                if (length < 0)
                {
                    JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
                }
            }
            break;
        case chars_format::scientific:
            {
                length = snprintf(number_buffer, sizeof(number_buffer), "%1.*e", decimal_places, val);
                if (length < 0)
                {
                    JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
                }
            }
            break;
        case chars_format::general:
            {
                if (override_.precision() != 0)
                {
                    int precision = override_.precision();
                    length = snprintf(number_buffer, sizeof(number_buffer), "%1.*g", precision, val);
                    if (length < 0)
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
                    }
                }
                else if (fmt.precision() != 0)
                {
                    int precision = fmt.precision();
                    length = snprintf(number_buffer, sizeof(number_buffer), "%1.*g", precision, val);
                    if (length < 0)
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
                    }
                }
                else
                {
                    int precision = std::numeric_limits<double>::digits10;
                    length = snprintf(number_buffer, sizeof(number_buffer), "%1.*g", precision, val);
                    if (length < 0)
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
                    }
                    int precision2 = std::numeric_limits<double>::max_digits10;
                    if (to_double_(number_buffer,sizeof(number_buffer)) != val)
                    {
                        length = snprintf(number_buffer, sizeof(number_buffer), "%1.*g", precision2, val);
                        if (length < 0)
                        {
                            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
                        }
                    }
                }             
                break;
            }
            default:
                JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
                break;
        }

        const char* sbeg = number_buffer;
        const char* send = sbeg + length;
        const char* pexp = send;

        if (sbeg != send)
        {
            bool dot = false;
            for (pexp = sbeg; *pexp != 'e' && *pexp != 'E' && pexp < send; ++pexp)
            {
            }

            for (const char* q = sbeg; q < pexp; ++q)
            {
                switch (*q)
                {
                case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    writer.push_back(*q);
                    ++count;
                    break;
                default:
                    if (*q == decimal_point_)
                    {
                        dot = true;
                        writer.push_back('.');
                        ++count;
                    }
                    break;
                }
            }
            if (!dot)
            {
                writer.push_back('.');
                writer.push_back('0');
                count += 2;
                dot = true;
            }
            for (const char* q = pexp; q < send; ++q)
            {
                writer.push_back(*q);
                ++count;
            }
        }
        return count;
    }
};

}}

#endif
