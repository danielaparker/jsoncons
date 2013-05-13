// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_EXCEPTION_HPP
#define JSONCONS_JSON_EXCEPTION_HPP

#include <string>
#include <vector>
#include <assert.h> 
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace jsoncons {

class json_exception : public std::exception
{
public:
    json_exception(std::string s)
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

#define JSONCONS_THROW_EXCEPTION(x) throw json_exception((x))
#define JSONCONS_ASSERT(x) if (!x) throw json_exception(#x)

}

#endif
