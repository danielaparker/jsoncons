// Copyright 2013 Daniel Parker
// Distributed under Boost license

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

#define JSONCONS_THROW_EXCEPTION(x) throw json_exception_0((x))
#define JSONCONS_THROW_EXCEPTION_1(fmt,arg1) throw json_exception_1<Char>((fmt),(arg1))
#define JSONCONS_ASSERT(x) if (!(x)) {std::cerr << #x; abort();}

class json_parse_exception : public json_exception
{
public:
    json_parse_exception(std::string message,
                         unsigned long line,
                         unsigned long column)
        : message_(message), 
          line_number_(line), 
          column_number_(column)
    {
    }
    json_parse_exception(const json_parse_exception& other)
        : message_(other.message_), 
          line_number_(other.line_number_), 
          column_number_(other.column_number_)
    {
    }
    const char* what() const
    {
        std::ostringstream os;
        os << message_ << " on line " << line_number_ << " at column " << column_number_;
        const_cast<std::string&>(message_) = os.str();
        return message_.c_str();
    }

    const std::string& message() const
    {
        return message_;
    }

    unsigned long line_number() const
    {
        return line_number_;
    }

    unsigned long column_number() const
    {
        return column_number_;
    }
private:
    std::string message_;
    std::string buffer_;
    unsigned long line_number_;
    unsigned long column_number_;
};

#define JSONCONS_THROW_PARSE_EXCEPTION(x,n,m) throw json_parse_exception(x,n,m)

}

#endif
