// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSONCONS_CONFIG_HPP
#define JSONCONS_JSONCONS_CONFIG_HPP

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
#include <limits> // std::numeric_limits

#define JSONCONS_NO_MACRO_EXP 

namespace jsoncons {

// Follow boost

#if defined (__clang__)
#   if defined(_GLIBCXX_USE_NOEXCEPT)
#      define JSONCONS_NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#   else
#      define JSONCONS_NOEXCEPT noexcept
#   endif
#elif defined(__GNUC__)
#   define JSONCONS_NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#else
#   define JSONCONS_NOEXCEPT
#endif


#ifdef _MSC_VER
#pragma warning( disable : 4290 )
inline bool is_nan(double x) { return _isnan( x ) != 0; }
inline bool is_inf(double x)
{
    return !_finite(x) && !_isnan(x);
}
inline bool is_pos_inf(double x)
{
    return is_inf(x) && x > 0;
}
inline bool is_neg_inf(double x)
{
    return is_inf(x) && x < 0;
}

inline
int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

inline
int c99_snprintf(char* str, size_t size, const char* format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(str, size, format, ap);
    va_end(ap);

    return count;
}
#else
inline bool is_nan(double x) { return std::isnan( x ); }
inline bool is_pos_inf(double x) {return std::isinf(x) && x > 0;}
inline bool is_neg_inf(double x) {return  std::isinf(x) && x > 0;}

#if __cplusplus >= 201103L
#define c99_snprintf snprintf
#else
#define c99_snprintf std::snprintf
#endif

#endif

#ifdef _MSC_VER
template <typename Char>
std::basic_string<Char> float_to_string(double val, size_t precision)
{
    std::basic_string<Char> s;
    char buf[_CVTBUFSIZE];
    int decimal_point = 0;
    int sign = 0;

    if (precision >= _CVTBUFSIZE)
    {
        precision = _CVTBUFSIZE - 1;
    }

    int err = _ecvt_s(buf, _CVTBUFSIZE, val, static_cast<int>(precision), &decimal_point, &sign);
    if (err != 0)
    {
        throw std::runtime_error("Failed attempting double to string conversion");
    }
    if (sign != 0)
    {
        s.push_back('-');
    }

    int len = static_cast<int>(precision);

    int decimal;
    int exponent;
    if (decimal_point < 0 || decimal_point > len)
    {
        decimal = 1;
        exponent = decimal_point - 1;
    }
    else
    {
        decimal = decimal_point;
        exponent = 0;
    }

    while (len >= 2 && buf[len - 1] == '0' && (len - 1) != decimal)
    {
        --len;
    }

    if (decimal == 0)
    {
        s.push_back('0');
        s.push_back('.');
    }
    s.push_back(buf[0]);
    for (int i = 1; i < len; ++i)
    {
        if (i == decimal)
        {
            s.push_back('.');
        }
        s.push_back(buf[i]);
    }
    if (exponent != 0)
    {
        s.push_back('e');
        if (exponent > 0)
        {
            s.push_back('+');
        }
        int err2 = _itoa_s(exponent,buf,_CVTBUFSIZE,10);
        if (err2 != 0)
        {
            throw std::runtime_error("Failed attempting double to string conversion");
        }
        for (int i = 0; i < _CVTBUFSIZE && buf[i]; ++i)
        {
            s.push_back(buf[i]);
        }
    }
    return s;
}
#else
template <typename Char>
std::basic_string<Char> float_to_string(double val, size_t precision)
{
    std::basic_ostringstream<Char> os;
    os.imbue(std::locale::classic());
    os << std::showpoint << std::setprecision(precision) << val;
    std::basic_string<Char> s(os.str());

    typename std::basic_string<Char>::size_type exp_pos= s.find('e');
    std::basic_string<Char> exp;
    if (exp_pos != std::basic_string<Char>::npos)
    {
        exp = s.substr(exp_pos);
        s.erase(exp_pos);
    }

    int len = s.size();
    while (len >= 2 && s[len - 1] == '0' && s[len - 2] != '.')
    {
        --len;
    }
    s.erase(len);
    if (exp_pos != std::basic_string<Char>::npos)
    {
        s.append(exp);
    }

    return s;
}
#endif

#ifdef _MSC_VER
inline
double string_to_float(const std::string& s)
{
    static _locale_t locale = _create_locale(LC_NUMERIC, "C");

    const char* begin = &s[0];
    char* end = const_cast<char*>(begin)+s.size();
    double val = _strtod_l(begin,&end,locale);
    if (begin == end)
    {
        throw std::invalid_argument("Invalid float value");
    }
    return val;
}
inline
double string_to_float(const std::wstring& s)
{
    static _locale_t locale = _create_locale(LC_NUMERIC, "C");

    const wchar_t* begin = &s[0];
    wchar_t* end = const_cast<wchar_t*>(begin)+s.size();
    double val = _wcstod_l(begin,&end,locale);
    if (begin == end)
    {
        throw std::invalid_argument("Invalid float value");
    }
    return val;
}
#else
template <typename Char> inline
double string_to_float(const std::basic_string<Char>& s)
{
    std::basic_istringstream<Char> ss(s);
    ss.imbue(std::locale::classic());
    double val;
    ss >> val;
    if (ss.fail())
    {
        throw std::invalid_argument("Invalid float value");
    }
    return val;
}
#endif

}

#endif
