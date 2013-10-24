// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_EXCEPTION_HPP
#define JSONCONS_JSON_EXCEPTION_HPP

#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cstdio>
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/jsoncons_config.hpp"

namespace jsoncons {

class json_exception : public std::exception
{
public:
    json_exception()
    {
    }
};

class json_exception_0 : public json_exception
{
public:
    json_exception_0(std::string s)
        : message_(s)
    {
    }
    const char* what() const 
    {
        return message_.c_str();
    }
private:
    std::string message_;
};

template <class Char>
class json_exception_1 : public json_exception
{
public:
    json_exception_1(const std::string& format, const std::basic_string<Char>& arg1)
        : format_(format), arg1_(arg1)
    {
    }
    const char* what() const 
    {
        c99_snprintf(const_cast<char*>(message_),255, format_.c_str(),arg1_.c_str());
        return message_;
    }
private:
    std::string format_;
    std::basic_string<Char> arg1_;
    char message_[255];
};

#define JSONCONS_THROW_EXCEPTION(x) throw jsoncons::json_exception_0((x))
#define JSONCONS_THROW_EXCEPTION_1(fmt,arg1) throw jsoncons::json_exception_1<Char>((fmt),(arg1))
#define JSONCONS_ASSERT(x) if (!(x)) {std::cerr << #x; abort();}

}

#endif
