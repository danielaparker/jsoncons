// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSONCONS_CONFIG_HPP
#define JSONCONS_JSONCONS_CONFIG_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>
#include <ostream>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <limits> // std::numeric_limits

namespace jsoncons {

#ifdef _MSC_VER
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
inline bool is_pos_inf(double x) {return std::isinf() && x > 0;}
inline bool is_neg_inf(double x) {return  std::isinf() && x > 0;}

#define c99_snprintf std::snprintf

#endif

#define JSONCONS_BUFFERED_READ 1

}
#endif
