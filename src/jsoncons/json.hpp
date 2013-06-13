// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_HPP
#define JSONCONS_JSON_HPP

/*! \file json.hpp
    \brief A Documented file.
    
    Details.
*/

#include "jsoncons/json1.hpp"
#include "jsoncons/json2.hpp"

/// documentation for the namespace
namespace jsoncons {

/*! \typedef basic_json<char> json
    \brief A type definition for a .
    
    Details.
*/

typedef basic_json<char> json;
typedef basic_name_value_pair<char> name_value_pair;
typedef basic_json<wchar_t> wjson;

}

#endif
