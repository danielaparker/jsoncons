// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_PRETTY_PRINT_HPP
#define JSONCONS_PRETTY_PRINT_HPP

#include <string>
#include <exception>
#include <cstring>
#include <ostream>
#include <memory>
#include <typeinfo>
#include <cstring>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/json_error.hpp>

namespace jsoncons {

template<class Json>
class json_printable
{
public:
    using char_type = typename Json::char_type;

    json_printable(const Json& j, json_indent_kind indent_kind)
       : j_(&j), indenting_(indent_kind)
    {
    }

    json_printable(const Json& j,
                   const basic_json_encode_options<char_type>& options,
                   json_indent_kind indent_kind)
       : j_(&j), options_(options), indenting_(indent_kind)
    {
    }

    void dump(std::basic_ostream<char_type>& os) const
    {
        j_->dump(os, options_, indenting_);
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_printable<Json>& pr)
    {
        pr.dump(os);
        return os;
    }

    const Json *j_;
    basic_json_encode_options<char_type> options_;
    json_indent_kind indenting_;
private:
    json_printable();
};

template<class Json>
json_printable<Json> print(const Json& j)
{
    return json_printable<Json>(j, json_indent_kind::no_indent);
}

template<class Json>
json_printable<Json> print(const Json& j,
                           const basic_json_encode_options<typename Json::char_type>& options)
{
    return json_printable<Json>(j, options, json_indent_kind::no_indent);
}

template<class Json>
json_printable<Json> pretty_print(const Json& j)
{
    return json_printable<Json>(j, json_indent_kind::indent);
}

template<class Json>
json_printable<Json> pretty_print(const Json& j,
                                  const basic_json_encode_options<typename Json::char_type>& options)
{
    return json_printable<Json>(j, options, json_indent_kind::indent);
}

}

#endif
