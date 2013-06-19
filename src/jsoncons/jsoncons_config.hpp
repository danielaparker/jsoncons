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
#include "jsoncons/output_format.hpp"
#include "jsoncons/json2.hpp"
#include "jsoncons/json_char_traits.hpp"
//#include <math.h> // isnan
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
#else
inline bool is_nan(double x) { return std::isnan( x ); }
inline bool is_pos_inf(double x) {return std::isinf() && x > 0;}
inline bool is_neg_inf(double x) {return  std::isinf() && x > 0;}
#endif

}
#endif
