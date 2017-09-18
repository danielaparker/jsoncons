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
#include <jsoncons/detail/unicode_traits.hpp>
#include <jsoncons/detail/jsoncons_config.hpp>

namespace jsoncons {

// json_exception

class json_exception 
{
public:
    virtual const char* what() const JSONCONS_NOEXCEPT = 0;
};

template <class Base>
class json_exception_0 : public Base, public virtual json_exception
{
public:
    json_exception_0(std::string s) JSONCONS_NOEXCEPT
        : Base(""), message_(s)
    {
    }
    ~json_exception_0() JSONCONS_NOEXCEPT
    {
    }
    const char* what() const JSONCONS_NOEXCEPT
    {
        return message_.c_str();
    }
private:
    std::string message_;
};

template <class Base>
class json_exception_1 : public Base, public virtual json_exception
{
public:
    json_exception_1(const std::string& format, const std::string& arg1) JSONCONS_NOEXCEPT
        : Base(""), format_(format), arg1_(arg1)
    {
    }
    json_exception_1(const std::string& format, const std::wstring& arg1) JSONCONS_NOEXCEPT
        : Base(""), format_(format)
    {
        char buf[255];
        size_t retval;
#if defined(JSONCONS_HAS_WCSTOMBS_S)
        wcstombs_s(&retval, buf, sizeof(buf), arg1.c_str(), arg1.size());
#else
        retval = wcstombs(buf, arg1.c_str(), sizeof(buf));
#endif
        if (retval != static_cast<std::size_t>(-1))
        {
            arg1_ = buf;
        }
    }
    ~json_exception_1() JSONCONS_NOEXCEPT
    {
    }
    const char* what() const JSONCONS_NOEXCEPT
    {
        c99_snprintf(const_cast<char*>(message_),255, format_.c_str(),arg1_.c_str());
        return message_;
    }
private:
    std::string format_;
    std::string arg1_;
    char message_[255];
};

#define JSONCONS_STR2(x)  #x
#define JSONCONS_STR(x)  JSONCONS_STR2(x)

#define JSONCONS_THROW_EXCEPTION(Base,x) throw jsoncons::json_exception_0<Base>((x))
#define JSONCONS_THROW_EXCEPTION_1(Base,fmt,arg1) throw jsoncons::json_exception_1<Base>((fmt),(arg1))
#define JSONCONS_ASSERT(x) if (!(x)) { \
    throw jsoncons::json_exception_0<std::runtime_error>("assertion '" #x "' failed at " __FILE__ ":" \
            JSONCONS_STR(__LINE__)); }

}
#endif
