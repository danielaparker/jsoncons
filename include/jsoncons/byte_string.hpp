// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BYTESTRING_HPP
#define JSONCONS_BYTESTRING_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <locale>
#include <limits> 
#include <type_traits>
#include <algorithm>
#include <memory>
#include <iterator>
#include <exception>
#include <array>
#include <initializer_list>
#include <jsoncons/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#if !defined(JSONCONS_NO_TO_CHARS)
#include <charconv>
#endif
#include <jsoncons/detail/obufferedstream.hpp>

namespace jsoncons
{

// byte_string_view
class byte_string_view
{
    const uint8_t* data_;
    size_t length_; 
public:
    typedef const uint8_t* const_iterator;
    typedef const uint8_t* iterator;
    typedef std::size_t size_type;

    byte_string_view()
        : data_(nullptr), length_(0)
    {
    }

    byte_string_view(const uint8_t* data, size_t length)
        : data_(data), length_(length)
    {
    }

    byte_string_view(const byte_string_view&) = default;

    byte_string_view(byte_string_view&&) = default;

    byte_string_view& operator=(const byte_string_view&) = default;

    byte_string_view& operator=(byte_string_view&&) = default;

    const uint8_t* data() const
    {
        return data_;
    }

    size_t length() const
    {
        return length_;
    }

    size_t size() const
    {
        return length_;
    }

    // iterator support 
    const_iterator begin() const JSONCONS_NOEXCEPT
    {
        return data_;
    }
    const_iterator end() const JSONCONS_NOEXCEPT
    {
        return data_ + length_;
    }

    uint8_t operator[](size_type pos) const 
    { 
        return data_[pos]; 
    }

    friend bool operator==(const byte_string_view& lhs, const byte_string_view& rhs)
    {
        if (lhs.length() != rhs.length())
        {
            return false;
        }
        for (size_t i = 0; i < lhs.length(); ++i)
        {
            if (lhs[i] != rhs[i])
            {
                return false;
            }
        }
        return true;
    }

    friend bool operator!=(const byte_string_view& lhs, const byte_string_view& rhs)
    {
        return !(lhs == rhs);
    }

    template <class CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const byte_string_view& o)
    {
        std::basic_ostringstream<CharT> ss;
        ss.flags(std::ios::hex | std::ios::showbase);
        for (auto b : o)
        {
            ss << (int)b;
        }
        os << ss.str();
        return os;
    }
};

// basic_byte_string
template <class Allocator = std::allocator<uint8_t>>
class basic_byte_string
{
    std::vector<uint8_t,Allocator> data_;
public:
    typedef typename std::vector<uint8_t,Allocator>::const_iterator const_iterator;
    typedef typename std::vector<uint8_t,Allocator>::const_iterator iterator;
    typedef std::size_t size_type;

    basic_byte_string() = default;

    explicit basic_byte_string(const Allocator& alloc)
        : data_(alloc)
    {
    }

    basic_byte_string(std::initializer_list<uint8_t> init)
        : data_(std::move(init))
    {
    }

    basic_byte_string(std::initializer_list<uint8_t> init, const Allocator& alloc)
        : data_(std::move(init), alloc)
    {
    }

    explicit basic_byte_string(const byte_string_view& v)
        : data_(v.begin(),v.end())
    {
    }

    basic_byte_string(const basic_byte_string<Allocator>& v)
        : data_(v.data_)
    {
    }

    basic_byte_string(basic_byte_string<Allocator>&& v)
    {
        data_.swap(v.data_);
    }

    basic_byte_string(const byte_string_view& v, const Allocator& alloc)
        : data_(v.begin(),v.end(),alloc)
    {
    }

    basic_byte_string(const char* s)
    {
        while (*s)
        {
            data_.push_back(*s++);
        }
    }

    basic_byte_string(const uint8_t* data, size_t length)
        : data_(data, data+length)
    {
    }

    basic_byte_string& operator=(const basic_byte_string& s) = default;

    basic_byte_string& operator=(basic_byte_string&& s) = default;

    operator byte_string_view() const JSONCONS_NOEXCEPT
    {
        return byte_string_view(data(),length());
    }

    void push_back(uint8_t b)
    {
        data_.push_back(b);
    }

    void assign(const uint8_t* s, size_t count)
    {
        data_.clear();
        data_.insert(s, s+count);
    }

    void append(const uint8_t* s, size_t count)
    {
        data_.insert(s, s+count);
    }

    void clear()
    {
        data_.clear();
    }

    uint8_t operator[](size_type pos) const 
    { 
        return data_[pos]; 
    }

    // iterator support 
    const_iterator begin() const JSONCONS_NOEXCEPT
    {
        return data_.begin();
    }
    const_iterator end() const JSONCONS_NOEXCEPT
    {
        return data_.end();
    }

    const uint8_t* data() const
    {
        return data_.data();
    }

    size_t size() const
    {
        return data_.size();
    }

    size_t length() const
    {
        return data_.size();
    }

    friend bool operator==(const basic_byte_string& lhs, const basic_byte_string& rhs)
    {
        return byte_string_view(lhs) == byte_string_view(rhs);
    }

    friend bool operator!=(const basic_byte_string& lhs, const basic_byte_string& rhs)
    {
        return byte_string_view(lhs) != byte_string_view(rhs);
    }

    template <class CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const basic_byte_string& o)
    {
        os << byte_string_view(o);
        return os;
    }
};

typedef basic_byte_string<std::allocator<uint8_t>> byte_string;


}

#endif
