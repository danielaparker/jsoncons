// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BYTE_STRING_HPP
#define JSONCONS_BYTE_STRING_HPP

#include <sstream>
#include <vector>
#include <ostream>
#include <cmath>
#include <memory> // std::allocator
#include <iterator>
#include <exception>
#include <initializer_list>
#include <algorithm> // std::find
#include <utility> // std::move
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>

namespace jsoncons
{

// Algorithms

static const char* base64_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                           "abcdefghijklmnopqrstuvwxyz"
                                           "0123456789+/"
                                           "=";
static const char* base64url_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                              "abcdefghijklmnopqrstuvwxyz"
                                              "0123456789-_"
                                              "\0";

template <class Container>
size_t encode_base16(const uint8_t* data, size_t length, Container& result)
{
    static const char* characters = "0123456789ABCDEF";

    for (size_t i = 0; i < length; ++i)
    {
        uint8_t c = data[i];
        result.push_back(characters[c >> 4]);
        result.push_back(characters[c & 0xf]);
    }
    return length*2;
}

template <class Container>
size_t encode_base64_generic(const uint8_t* first, size_t length, const char* alphabet, Container& result)
{
    size_t count = 0;
    const uint8_t* last = first + length;
    unsigned char a3[3];
    unsigned char a4[4];
    unsigned char fill = alphabet[64];
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
                ++count;
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
            ++count;
        }

        if (fill != 0)
        {
            while (i++ < 3) 
            {
                result.push_back(fill);
                ++count;
            }
        }
    }

    return count;
}

template <class Container>
size_t encode_base64url(const uint8_t* first, size_t length, Container& result)
{
    return encode_base64_generic(first, length, base64url_alphabet, result);
}

template <class Container>
size_t encode_base64(const uint8_t* first, size_t length, Container& result)
{
    return encode_base64_generic(first, length, base64_alphabet, result);
}

inline 
static bool is_base64(uint8_t c) 
{
    return isalnum(c) || c == '+' || c == '/';
}

inline 
static bool is_base64url(uint8_t c) 
{
    return isalnum(c) || c == '-' || c == '_';
}

template <class CharT, class F>
std::vector<uint8_t> decode_base64_generic(const basic_string_view<CharT>& base64_string, 
                                           const char* alphabet, 
                                           const char* alphabet_end, 
                                           F f)
{
    std::vector<uint8_t> result;
    uint8_t a4[4], a3[3];
    uint8_t i = 0;
    uint8_t j = 0;

    auto first = base64_string.begin();
    auto last = base64_string.end();

    while (first != last && *first != '=')
    {
        if (!f(*first))
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Cannot decode encoded byte string"));
        }

        a4[i++] = *first++; 
        if (i == 4)
        {
            for (i = 0; i < 4; ++i) 
            {
                auto p = std::find(alphabet,alphabet_end,a4[i]);
                if (p == alphabet_end)
                {
                    a4[i] = 0xff;
                }
                else
                {
                    a4[i] = static_cast<uint8_t>(p - alphabet);
                }
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
            auto p = std::find(alphabet,alphabet_end,a4[j]);
            if (p == alphabet_end)
            {
                a4[j] = 0xff;
            }
            else
            {
                a4[j] = static_cast<uint8_t>(p - alphabet);
            }
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

template <class CharT>
std::vector<uint8_t> decode_base64(const basic_string_view<CharT>& base64_string)
{
    return decode_base64_generic(base64_string, base64_alphabet, base64_alphabet+65, is_base64);
}

template <class CharT>
std::vector<uint8_t> decode_base64url(const basic_string_view<CharT>& base64_string)
{
    return decode_base64_generic(base64_string, base64url_alphabet, base64url_alphabet+64, is_base64url);
}

template <class CharT>
std::vector<uint8_t> decode_base64(const std::basic_string<CharT>& base64_string)
{
    return decode_base64_generic(basic_string_view<CharT>(base64_string), base64_alphabet, base64_alphabet+65, is_base64);
}

template <class CharT>
std::vector<uint8_t> decode_base64url(const std::basic_string<CharT>& base64_string)
{
    return decode_base64_generic(basic_string_view<CharT>(base64_string), base64url_alphabet, base64url_alphabet+64, is_base64url);
}

template <class CharT>
std::vector<uint8_t> decode_base16(const basic_string_view<CharT>& input)
{
    static const char* const characters = "0123456789ABCDEF";
    size_t len = input.length();
    if (len & 1) 
    {
        JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Cannot decode encoded base16 string - odd length"));
    }

    std::vector<uint8_t> result;
    result.reserve(len / 2);
    for (size_t i = 0; i < len; i += 2)
    {
        char a = (char)input[i];
        const char* p = std::lower_bound(characters, characters + 16, a);
        if (*p != a) 
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Not a hex digit. Cannot decode encoded base16 string"));
        }

        char b = (char)input[i + 1];
        const char* q = std::lower_bound(characters, characters + 16, b);
        if (*q != b) 
        {
            JSONCONS_THROW(json_runtime_error<std::invalid_argument>("Not a hex digit. Cannot decode encoded base16 string"));
        }

        result.push_back((uint8_t)(((p - characters) << 4) | (q - characters)));
    }
    return result;
}


template <class CharT>
std::vector<uint8_t> decode_base16(const std::basic_string<CharT>& input)
{
    return decode_base16(basic_string_view<CharT>(input)); 
}

struct byte_traits
{
    typedef uint8_t char_type;

    static constexpr int eof() 
    {
        return std::char_traits<char>::eof();
    }
};

// byte_string_view
class byte_string_view
{
    const uint8_t* data_;
    size_t length_; 
public:
    typedef byte_traits traits_type;

    typedef const uint8_t* const_iterator;
    typedef const uint8_t* iterator;
    typedef std::size_t size_type;
    typedef uint8_t value_type;

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
    const_iterator begin() const noexcept
    {
        return data_;
    }
    const_iterator end() const noexcept
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
    typedef byte_traits traits_type;

    typedef typename std::vector<uint8_t,Allocator>::const_iterator const_iterator;
    typedef typename std::vector<uint8_t,Allocator>::const_iterator iterator;
    typedef std::size_t size_type;
    typedef uint8_t value_type;

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

    operator byte_string_view() const noexcept
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
    const_iterator begin() const noexcept
    {
        return data_.begin();
    }
    const_iterator end() const noexcept
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
