// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_READER_HPP
#define JSONCONS_JSON_READER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"
#include "jsoncons/json_parser.hpp"

namespace jsoncons {

template<typename Char>
class basic_json_reader 
{
public:
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler)
        : parser_(is,handler)
    {
    }
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler,
                      basic_parse_error_handler<Char>& err_handler)
        : parser_(is,handler,err_handler)
    {
    }

    size_t buffer_capacity() const
    {
        return parser_.buffer_capacity();
    }

    void buffer_capacity(size_t capacity)
    {
        parser_.buffer_capacity(capacity);
    }

    void read()
    {
        parser_.read();
    }

    void assert_done()
    {
        parser_.assert_done();
    }

    bool eof() const
    {
        return parser_.eof();
    }

private:
    basic_json_parser<Char> parser_;
};

typedef basic_json_reader<char> json_reader;
typedef basic_json_reader<wchar_t> wjson_reader;

}

#endif
