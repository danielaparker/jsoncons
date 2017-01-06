// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONS_HPP
#define JSONCONS_JSONCONS_HPP

#include <locale>
#include <string>
#include <vector>
#include <cstdlib>
#include <cwchar>
#include <cstdint> 
#include <iostream>
#include <vector>
#include <jsoncons/unicode_traits.hpp>
#include <jsoncons/jsoncons_config.hpp>
#include <jsoncons/jsoncons.hpp>

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

template <size_t arg1, size_t ... argn>
struct static_max;

template <size_t arg>
struct static_max<arg>
{
    static const size_t value = arg;
};

template <size_t arg1, size_t arg2, size_t ... argn>
struct static_max<arg1,arg2,argn ...>
{
    static const size_t value = arg1 >= arg2 ? 
        static_max<arg1,argn...>::value :
        static_max<arg2,argn...>::value; 
};

// null_type

struct null_type
{
};

template <class T>
struct type_wrapper
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <class T>
struct type_wrapper<const T>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <class T>
struct type_wrapper<T&>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <class T>
struct type_wrapper<const T&>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

}
#endif
