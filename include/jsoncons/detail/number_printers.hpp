// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_NUMBERPRINTERS_HPP
#define JSONCONS_DETAIL_NUMBERPRINTERS_HPP

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

#if defined(JSONCONS_HAS__ECVT_S)

class print_double
{
private:
    uint8_t precision_override_;
public:
    print_double(uint8_t precision)
        : precision_override_(precision)
    {
    }

    template <class Writer>
    void operator()(double val, uint8_t precision, Writer& writer) 
    {
        typedef typename Writer::char_type char_type;

        char buf[_CVTBUFSIZE];
        int decimal_point = 0;
        int sign = 0;

        int prec;
        if (precision_override_ != 0)
        {
            prec = precision_override_;
        }
        else if (precision != 0)
        {
            prec = precision;
        }
        else
        {
            prec = std::numeric_limits<double>::digits10;
        }             

        int err = _ecvt_s(buf, _CVTBUFSIZE, val, prec, &decimal_point, &sign);
        if (err != 0)
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Failed attempting double to string conversion"));
        }
        //std::cout << "prec:" << prec << ", buf:" << buf << std::endl;
        char* s = buf;
        char* se = s + prec;

        int i, k;
        int j;

        if (sign)
        {
            writer.put('-');
        }
        if (decimal_point <= -4 || decimal_point > se - s + 5) 
        {
            writer.put(*s++);
            if (s < se) 
            {
                writer.put('.');
                while ((se-1) > s && *(se-1) == '0')
                {
                    --se;
                }

                while(s < se)
                {
                    writer.put(*s++);
                }
            }
            writer.put('e');
            /* sprintf(b, "%+.2d", decimal_point - 1); */
            if (--decimal_point < 0) {
                writer.put('-');
                decimal_point = -decimal_point;
                }
            else
                writer.put('+');
            for(j = 2, k = 10; 10*k <= decimal_point; j++, k *= 10);
            for(;;) 
            {
                i = decimal_point / k;
                writer.put(static_cast<char_type>(i) + '0');
                if (--j <= 0)
                    break;
                decimal_point -= i*k;
                decimal_point *= 10;
            }
        }
        else if (decimal_point <= 0) 
        {
            writer.put('0');
            writer.put('.');
            while ((se-1) > s && *(se-1) == '0')
            {
                --se;
            }
            for(; decimal_point < 0; decimal_point++)
            {
                writer.put('0');
            }
            while(s < se)
            {
                writer.put(*s++);
            }
        }
        else {
            while(s < se) 
            {
                writer.put(*s++);
                if ((--decimal_point == 0) && s < se)
                {
                    writer.put('.');
                    while ((se-1) > s && *(se-1) == '0')
                    {
                        --se;
                    }
                }
            }
            for(; decimal_point > 0; decimal_point--)
            {
                writer.put('0');
            }
        }
    }
};

#elif defined(JSONCONS_NO_LOCALECONV)

class print_double
{
private:
    uint8_t precision_override_;
    basic_obufferedstream<char> os_;
public:
    print_double(uint8_t precision)
        : precision_override_(precision)
    {
        os_.imbue(std::locale::classic());
        os_.precision(precision);
    }

    template <class Writer>
    void operator()(double val, uint8_t precision, Writer& writer)
    {
        typedef typename Writer::char_type char_type;

        int prec;
        if (precision_override_ != 0)
        {
            prec = precision_override_;
        }
        else if (precision != 0)
        {
            prec = precision;
        }
        else
        {
            prec = std::numeric_limits<double>::digits10;
        }             

        os_.clear_sequence();
        os_.precision(prec);
        os_ << val;

        //std::cout << "precision_override_:" << (int)precision_override_ << ", precision:" << (int)precision << ", buf:" << os_.data() << std::endl;

        const char_type* sbeg = os_.data();
        const char_type* send = sbeg + os_.length();
        const char_type* pexp = send;

        if (sbeg != send)
        {
            bool dot = false;
            for (pexp = sbeg; *pexp != 'e' && *pexp != 'E' && pexp < send; ++pexp)
            {
            }

            const char_type* qend = pexp;
            while (qend >= sbeg+2 && *(qend-1) == '0' && *(qend-2) != '.')
            {
                --qend;
            }
            if (pexp == send)
            {
                qend = ((qend >= sbeg+2) && *(qend-2) == '.') ? qend : send;
            }

            for (const char_type* q = sbeg; q < qend; ++q)
            {
                if (*q == '.')
                {
                    dot = true;
                }
                writer.put(*q);
            }
            if (!dot)
            {
                writer.put('.');
                writer.put('0');
                dot = true;
            }
            for (const char_type* q = pexp; q < send; ++q)
            {
                writer.put(*q);
            }
        }
    }
};
#else

class print_double
{
private:
    uint8_t precision_override_;
    char decimal_point_;
public:
    print_double(uint8_t precision)
        : precision_override_(precision)
    {
        struct lconv * lc = localeconv();
        if (lc != nullptr && lc->decimal_point[0] != 0)
        {
            decimal_point_ = lc->decimal_point[0];    
        }
        else
        {
            decimal_point_ = '.'; 
        }
    }

    template <class Writer>
    void operator()(double val, uint8_t precision, Writer& writer)
    {
        typedef typename Writer::char_type char_type;

        int prec;
        if (precision_override_ != 0)
        {
            prec = precision_override_;
        }
        else if (precision != 0)
        {
            prec = precision;
        }
        else
        {
            prec = std::numeric_limits<double>::digits10;
        }             

        char number_buffer[100]; 
        int length = snprintf(number_buffer, 100, "%1.*g", prec, val);
        if (length < 0)
        {
            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("print_double failed."));
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

#endif

}}

#endif
