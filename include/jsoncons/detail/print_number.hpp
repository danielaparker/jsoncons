// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_PRINTNUMBER_HPP
#define JSONCONS_DETAIL_PRINTNUMBER_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <locale>
#include <limits> 
#include <algorithm>
#include <exception>
#include <jsoncons/jsoncons_config.hpp>
#include <jsoncons/detail/obufferedstream.hpp>
#include <jsoncons/detail/parse_number.hpp>

namespace jsoncons { namespace detail {

// print_integer

template<class Writer> 
void print_integer(int64_t value, Writer& os)
{
    typedef typename Writer::char_type char_type;

    char_type buf[255];
    uint64_t u = (value < 0) ? static_cast<uint64_t>(-value) : static_cast<uint64_t>(value);
    char_type* p = buf;
    do
    {
        *p++ = static_cast<char_type>(48 + u%10);
    }
    while (u /= 10);
    if (value < 0)
    {
        os.put('-');
    }
    while (--p >= buf)
    {
        os.put(*p);
    }
}

// print_uinteger

template<class Writer>
void print_uinteger(uint64_t value, Writer& os)
{
    typedef typename Writer::char_type char_type;

    char_type buf[255];
    char_type* p = buf;
    do
    {
        *p++ = static_cast<char_type>(48 + value % 10);
    } while (value /= 10);
    while (--p >= buf)
    {
        os.put(*p);
    }
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

    template <class Writer>
    void operator()(double val, const floating_point_options& fmt, Writer& writer)
    {
        typedef typename Writer::char_type char_type;

        chars_format format;
        if (override_.format() != chars_format::hex)
        {
            format = override_.format();
        }
        else 
        {
            format = fmt.format();
        }

        int precision;
        int precision2 = 0;
        if (override_.precision() != 0)
        {
            precision = override_.precision();
        }
        else if (fmt.precision() != 0)
        {
            precision = fmt.precision();
        }
        else
        {
            precision = std::numeric_limits<double>::digits10;
            precision2 = std::numeric_limits<double>::max_digits10;
        }             

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
        default:
            {
                length = snprintf(number_buffer, sizeof(number_buffer), "%1.*g", precision, val);
                if (length < 0)
                {
                    JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
                }
                if (precision2 > 0)
                {
                    if (to_double_(number_buffer,sizeof(number_buffer)) != val)
                    {
                        length = snprintf(number_buffer, sizeof(number_buffer), "%1.*g", precision2, val);
                        if (length < 0)
                        {
                            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
                        }
                    }
                }
            }
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
                    writer.put(*q);
                    break;
                default:
                    if (*q == decimal_point_)
                    {
                        dot = true;
                        writer.put('.');
                    }
                    break;
                }
            }
            if (!dot)
            {
                writer.put('.');
                writer.put('0');
                dot = true;
            }
            for (const char* q = pexp; q < send; ++q)
            {
                writer.put(*q);
            }
        }
    }
};

}}

#endif
