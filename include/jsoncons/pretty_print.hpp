// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_PRETTYPRINT_HPP
#define JSONCONS_PRETTYPRINT_HPP

#include <limits>
#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <memory>
#include <typeinfo>
#include <cstring>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/json_serializing_options.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/json_error_category.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace jsoncons {

template<class Json>
class json_printable
{
public:
    typedef typename Json::char_type char_type;

    json_printable(const Json& o, indenting line_indent)
       : o_(&o), indenting_(line_indent)
    {
    }

    json_printable(const Json& o,
                   const basic_json_serializing_options<char_type>& options,
                   indenting line_indent)
       : o_(&o), options_(options), indenting_(line_indent)
    {
    }

    void dump(std::basic_ostream<char_type>& os) const
    {
        o_->dump(os, options_, indenting_);
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_printable<Json>& o)
    {
        o.dump(os);
        return os;
    }

    const Json *o_;
    basic_json_serializing_options<char_type> options_;
    indenting indenting_;
private:
    json_printable();
};

template<class Json>
json_printable<Json> print(const Json& val)
{
    return json_printable<Json>(val, indenting::no_indent);
}

template<class Json>
json_printable<Json> print(const Json& val,
                           const basic_json_serializing_options<typename Json::char_type>& options)
{
    return json_printable<Json>(val, options, indenting::no_indent);
}

template<class Json>
json_printable<Json> pretty_print(const Json& val)
{
    return json_printable<Json>(val, indenting::indent);
}

template<class Json>
json_printable<Json> pretty_print(const Json& val,
                                  const basic_json_serializing_options<typename Json::char_type>& options)
{
    return json_printable<Json>(val, options, indenting::indent);
}

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
