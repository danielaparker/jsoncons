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
#include <jsoncons/json_options.hpp>
#include <jsoncons/detail/grisu3.hpp>
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
template <class Result>
bool safe_dtoa(double val, Result& result)
{
    if (val == 0)
    {
        result.push_back('0');
        result.push_back('.');
        result.push_back('0');
        return true;
    }

    jsoncons::detail::string_to_double to_double_;

    char buffer[100];
    int precision = std::numeric_limits<double>::digits10;
    int length = snprintf(buffer, sizeof(buffer), "%1.*g", precision, val);
    if (length < 0)
    {
        return false;
    }
    int precision2 = std::numeric_limits<double>::max_digits10;
    if (to_double_(buffer,sizeof(buffer)) != val)
    {
        length = snprintf(buffer, sizeof(buffer), "%1.*g", precision2, val);
        if (length < 0)
        {
            return false;
        }
    }
    bool needs_dot = true;
    for (size_t i = 0; i < length; ++i)
    {
        switch (buffer[i])
        {
            case '.':
            case 'e':
            case 'E':
                needs_dot = false;
                break;
        }
        result.push_back(buffer[i]);
    }
    if (needs_dot)
    {
        result.push_back('.');
        result.push_back('0');
    }
    return true;
}

template <class Result>
bool dtoa(double v, Result& result)
{
    if (v == 0)
    {
        result.push_back('0');
        result.push_back('.');
        result.push_back('0');
        return true;
    }

    int length = 0;
    int k;

    char buffer[100];

    double u = std::signbit(v) ? -v : v;
    if (jsoncons::detail::grisu3(u, buffer, &length, &k))
    {
        if (std::signbit(v))
        {
            result.push_back('-');
        }
        jsoncons::detail::prettify_string(buffer, length, k, -6, 21, result);
        return true;
    }
    else
    {
        return safe_dtoa(v, result);
    }
}

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

        chars_format format = override_.format() != chars_format() ? override_.format() : chars_format::general;

        int decimal_places;
        if (override_.decimal_places() != 0)
        {
            decimal_places = override_.decimal_places();
        }
        /* else if (fmt.decimal_places() != 0)
        {
            decimal_places = fmt.decimal_places();
        } */
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
                else
                {
                    if (!dtoa(val,writer))
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
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
