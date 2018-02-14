// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSON_EXCEPTION_HPP
#define JSON_EXCEPTION_HPP

#include <locale>
#include <string>
#include <vector>
#include <cstdlib>
#include <cwchar>
#include <cstdint> 
#include <iostream>
#include <vector>
#include <iterator>
#include <jsoncons/detail/unicode_traits.hpp>
#include <jsoncons/jsoncons_config.hpp>

namespace jsoncons {

// json_exception

class json_exception 
{
public:
    virtual const char* what() const JSONCONS_NOEXCEPT = 0;
};

template <class Base>
class json_exception_impl : public Base, public virtual json_exception
{
public:
    json_exception_impl(const std::string& s) JSONCONS_NOEXCEPT
        : Base(""), message_(s)
    {
    }
    ~json_exception_impl() JSONCONS_NOEXCEPT
    {
    }
    const char* what() const JSONCONS_NOEXCEPT override
    {
        return message_.c_str();
    }
private:
    std::string message_;
};

class key_not_found : public std::out_of_range, public virtual json_exception
{
public:
    template <class CharT>
    explicit key_not_found(const CharT* key, size_t length) JSONCONS_NOEXCEPT
        : std::out_of_range("")
    {
        buffer_.append("Key '");
        unicons::convert(key, key+length, std::back_inserter(buffer_),
                         unicons::conv_flags::strict);
        buffer_.append("' not found");
    }
    ~key_not_found() JSONCONS_NOEXCEPT
    {
    }
    const char* what() const JSONCONS_NOEXCEPT override
    {
        return buffer_.c_str();
    }
private:
    std::string buffer_;
};

class not_an_object : public std::runtime_error, public virtual json_exception
{
public:
    template <class CharT>
    explicit not_an_object(const CharT* key, size_t length) JSONCONS_NOEXCEPT
        : std::runtime_error("")
    {
        buffer_.append("Attempting to access or modify '");
        unicons::convert(key, key+length, std::back_inserter(buffer_),
                         unicons::conv_flags::strict);
        buffer_.append("' on a value that is not an object");
    }
    ~not_an_object() JSONCONS_NOEXCEPT
    {
    }
    const char* what() const JSONCONS_NOEXCEPT override
    {
        return buffer_.c_str();
    }
private:
    std::string buffer_;
};

#define JSONCONS_STR2(x)  #x
#define JSONCONS_STR(x)  JSONCONS_STR2(x)

#define JSONCONS_ASSERT(x) if (!(x)) { \
    throw jsoncons::json_exception_impl<std::runtime_error>("assertion '" #x "' failed at " __FILE__ ":" \
            JSONCONS_STR(__LINE__)); }

#define JSONCONS_THROW(x) throw (x)

}
#endif
