// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONSUTILITIES_HPP
#define JSONCONS_JSONCONSUTILITIES_HPP

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
#include <jsoncons/detail/jsoncons_config.hpp>
#include <jsoncons/detail/obufferedstream.hpp>
#include <jsoncons/detail/type_traits_helper.hpp>

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

    byte_string_view(const uint8_t* data, size_t length)
        : data_(data), length_(length)
    {
    }

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

    basic_byte_string(const basic_byte_string& s) = default; 

    basic_byte_string(basic_byte_string&& s) = default; 

    basic_byte_string& operator=(const basic_byte_string& s) = default;

    basic_byte_string& operator=(basic_byte_string&& s) = default;

    operator byte_string_view() const JSONCONS_NOEXCEPT
    {
        return byte_string_view(data(),length());
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

static const std::string base64_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                           "abcdefghijklmnopqrstuvwxyz"
                                           "0123456789+/"
                                           "=";
static const std::string base64url_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                              "abcdefghijklmnopqrstuvwxyz"
                                              "0123456789-_"
                                              "\0";

inline 
static bool is_base64(uint8_t c) 
{
    return isalnum(c) || c == '+' || c == '/';
}

template <class InputIt,class CharT>
void encode_base64(InputIt first, InputIt last, const std::string& alphabet, std::basic_string<CharT>& result)
{
    unsigned char a3[3];
    unsigned char a4[4];
    unsigned char fill = alphabet.back();
    int i = 0;
    int j = 0;

    while (first != last)
    {
        a3[i++] = *first++;
        if (i == 3)
        {
            a4[0] = (a3[0] & 0xfc) >> 2;
            a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
            a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
            a4[3] = a3[2] & 0x3f;

            for (i = 0; i < 4; i++) 
            {
                result.push_back(alphabet[a4[i]]);
            }
            i = 0;
        }
    }

    if (i > 0)
    {
        for (j = i; j < 3; ++j) 
        {
            a3[j] = 0;
        }

        a4[0] = (a3[0] & 0xfc) >> 2;
        a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
        a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; ++j) 
        {
            result.push_back(alphabet[a4[j]]);
        }

        if (fill != 0)
        {
            while (i++ < 3) 
            {
                result.push_back(fill);
            }
        }
    }
}

template <class InputIt,class CharT>
void encode_base64url(InputIt first, InputIt last, std::basic_string<CharT>& result)
{
    return encode_base64(first,last,base64url_alphabet,result);
}

template <class InputIt,class CharT>
void encode_base64(InputIt first, InputIt last, std::basic_string<CharT>& result)
{
    encode_base64(first,last,base64_alphabet,result);
}

inline
std::string decode_base64(const std::string& base64_string)
{
    std::string result;
    size_t buflen = base64_string.size();
    uint8_t a4[4], a3[3];
    uint8_t i = 0;
    uint8_t j = 0;

    auto first = base64_string.begin();
    auto last = base64_string.end();

    while (first != last && *first != '=')
    {
        JSONCONS_ASSERT(is_base64(*first));

        a4[i++] = *first++; 
        if (i == 4)
        {
            for (i = 0; i < 4; ++i) 
            {
                a4[i] = static_cast<uint8_t>(base64_alphabet.find(a4[i]));
            }

            a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
            a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
            a3[2] = ((a4[2] & 0x3) << 6) +   a4[3];

            for (i = 0; i < 3; i++) 
            {
                result.push_back(a3[i]);
            }
            i = 0;
        }
    }

    if (i > 0)
    {
        for (j = 0; j < i; ++j) 
        {
            a4[j] = static_cast<uint8_t>(base64_alphabet.find(a4[j]));
        }

        a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
        a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);

        for (j = 0; j < i - 1; ++j) 
        {
            result.push_back(a3[j]);
        }
    }

    return result;
}

}

#endif
