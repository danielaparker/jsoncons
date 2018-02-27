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
#include <jsoncons/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#if !defined(JSONCONS_NO_TO_CHARS)
#include <charconv>
#endif
#include <jsoncons/detail/obufferedstream.hpp>

namespace jsoncons
{

#if !defined(JSONCONS_HAS_STRING_VIEW)
template <class CharT, class Traits = std::char_traits<CharT>>
class Basic_string_view_
{
private:
    const CharT* data_;
    size_t length_;
public:
    typedef CharT value_type;
    typedef const CharT& const_reference;
    typedef Traits traits_type;
    typedef std::size_t size_type;
    static const size_type npos = size_type(-1);
    typedef const CharT* const_iterator;
    typedef const CharT* iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    Basic_string_view_()
        : data_(nullptr), length_(0)
    {
    }
    Basic_string_view_(const CharT* data, size_t length)
        : data_(data), length_(length)
    {
    }
    Basic_string_view_(const CharT* data)
        : data_(data), length_(Traits::length(data))
    {
    }
    Basic_string_view_(const Basic_string_view_& other) = default;

    template <class Allocator>
    Basic_string_view_(const std::basic_string<CharT,Traits,Allocator>& s)
        : data_(s.data()), length_(s.length())
    {
    }

    template <class Allocator>
    explicit operator std::basic_string<CharT,Traits,Allocator>() const
    { 
        return std::basic_string<CharT,Traits>(data_,length_); 
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
    const_iterator cbegin() const JSONCONS_NOEXCEPT 
    { 
        return data_; 
    }
    const_iterator cend() const JSONCONS_NOEXCEPT 
    { 
        return data_ + length_; 
    }
    const_reverse_iterator rbegin() const JSONCONS_NOEXCEPT 
    { 
        return const_reverse_iterator(end()); 
    }
    const_reverse_iterator rend() const JSONCONS_NOEXCEPT 
    { 
        return const_reverse_iterator(begin()); 
    }
    const_reverse_iterator crbegin() const JSONCONS_NOEXCEPT 
    { 
        return const_reverse_iterator(end()); 
    }
    const_reverse_iterator crend() const JSONCONS_NOEXCEPT 
    { 
        return const_reverse_iterator(begin()); 
    }

    // capacity

    size_t size() const
    {
        return length_;
    }

    size_t length() const
    {
        return length_;
    }
    size_type max_size() const JSONCONS_NOEXCEPT 
    { 
        return length_; 
    }
    bool empty() const JSONCONS_NOEXCEPT 
    { 
        return length_ == 0; 
    }

    // element access

    const_reference operator[](size_type pos) const 
    { 
        return data_[pos]; 
    }

    const_reference at(size_t pos) const 
    {
        if (pos >= length_)
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("pos exceeds length"));
        }
        return data_[pos];
    }

    const_reference front() const                
    { 
        return data_[0]; 
    }
    const_reference back()  const                
    { 
        return data_[length_-1]; 
    }

    const CharT* data() const
    {
        return data_;
    }

    // string operations

    Basic_string_view_ substr(size_type pos, size_type n=npos) const 
    {
        if (pos > length_)
        {
            JSONCONS_THROW(json_exception_impl<std::out_of_range>("pos exceeds size"));
        }
        if (n == npos || pos + n > length_)
        {
            n = length_ - pos;
        }
        return Basic_string_view_(data_ + pos, n);
    }

    int compare(Basic_string_view_ s) const 
    {
        const int rc = Traits::compare(data_, s.data_, (std::min)(length_, s.length_));
        return rc != 0 ? rc : (length_ == s.length_ ? 0 : length_ < s.length_ ? -1 : 1);
    }

    int compare(const CharT* data) const 
    {
        const size_t length = Traits::length(data);
        const int rc = Traits::compare(data_, data, (std::min)(length_, length));
        return rc != 0 ? rc : (length_ == length? 0 : length_ < length? -1 : 1);
    }

    template <class Allocator>
    int compare(const std::basic_string<CharT,Traits,Allocator>& s) const 
    {
        const int rc = Traits::compare(data_, s.data(), (std::min)(length_, s.length()));
        return rc != 0 ? rc : (length_ == s.length() ? 0 : length_ < s.length() ? -1 : 1);
    }

    size_type find(Basic_string_view_ s, size_type pos = 0) const JSONCONS_NOEXCEPT 
    {
        if (pos > length_)
        {
            return npos;
        }
        if (s.length_ == 0)
        {
            return pos;
        }
        const_iterator it = std::search(cbegin() + pos, cend(),
                                        s.cbegin(), s.cend(), Traits::eq);
        return it == cend() ? npos : std::distance(cbegin(), it);
    }
    size_type find(CharT ch, size_type pos = 0) const JSONCONS_NOEXCEPT
    { 
        return find(Basic_string_view_(&ch, 1), pos); 
    }
    size_type find(const CharT* s, size_type pos, size_type n) const JSONCONS_NOEXCEPT
    { 
        return find(Basic_string_view_(s, n), pos); 
    }
    size_type find(const CharT* s, size_type pos = 0) const JSONCONS_NOEXCEPT
    { 
        return find(Basic_string_view_(s), pos); 
    }

    size_type rfind(Basic_string_view_ s, size_type pos = npos) const JSONCONS_NOEXCEPT 
    {
        if (length_ < s.length_)
        {
            return npos;
        }
        if (pos > length_ - s.length_)
        {
            pos = length_ - s.length_;
        }
        if (s.length_ == 0) 
        {
            return pos;
        }
        for (const CharT* p = data_ + pos; true; --p) 
        {
            if (Traits::compare(p, s.data_, s.length_) == 0)
            {
                return p - data_;
            }
            if (p == data_)
            {
                return npos;
            }
         };
    }
    size_type rfind(CharT ch, size_type pos = npos) const JSONCONS_NOEXCEPT
    { 
        return rfind(Basic_string_view_(&ch, 1), pos); 
    }
    size_type rfind(const CharT* s, size_type pos, size_type n) const JSONCONS_NOEXCEPT
    { 
        return rfind(Basic_string_view_(s, n), pos); 
    }
    size_type rfind(const CharT* s, size_type pos = npos) const JSONCONS_NOEXCEPT
    { 
        return rfind(Basic_string_view_(s), pos); 
    }

    size_type find_first_of(Basic_string_view_ s, size_type pos = 0) const JSONCONS_NOEXCEPT 
    {
        if (pos >= length_ || s.length_ == 0)
        {
            return npos;
        }
        const_iterator it = std::find_first_of
            (cbegin() + pos, cend(), s.cbegin(), s.cend(), Traits::eq);
        return it == cend() ? npos : std::distance (cbegin(), it);
    }
    size_type find_first_of(CharT ch, size_type pos = 0) const JSONCONS_NOEXCEPT
    {
         return find_first_of(Basic_string_view_(&ch, 1), pos); 
    }
    size_type find_first_of(const CharT* s, size_type pos, size_type n) const JSONCONS_NOEXCEPT
    { 
        return find_first_of(Basic_string_view_(s, n), pos); 
    }
    size_type find_first_of(const CharT* s, size_type pos = 0) const JSONCONS_NOEXCEPT
    { 
        return find_first_of(Basic_string_view_(s), pos); 
    }

    size_type find_last_of(Basic_string_view_ s, size_type pos = npos) const JSONCONS_NOEXCEPT 
    {
        if (s.length_ == 0)
        {
            return npos;
        }
        if (pos >= length_)
        {
            pos = 0;
        }
        else
        {
            pos = length_ - (pos+1);
        }
        const_reverse_iterator it = std::find_first_of
            (crbegin() + pos, crend(), s.cbegin(), s.cend(), Traits::eq);
        return it == crend() ? npos : (length_ - 1 - std::distance(crbegin(), it));
    }
    size_type find_last_of(CharT ch, size_type pos = npos) const JSONCONS_NOEXCEPT
    { 
        return find_last_of(Basic_string_view_(&ch, 1), pos); 
    }
    size_type find_last_of(const CharT* s, size_type pos, size_type n) const JSONCONS_NOEXCEPT
    { 
        return find_last_of(Basic_string_view_(s, n), pos); 
    }
    size_type find_last_of(const CharT* s, size_type pos = npos) const JSONCONS_NOEXCEPT
    { 
        return find_last_of(Basic_string_view_(s), pos); 
    }

    size_type find_first_not_of(Basic_string_view_ s, size_type pos = 0) const JSONCONS_NOEXCEPT 
    {
        if (pos >= length_)
            return npos;
        if (s.length_ == 0)
            return pos;

        const_iterator it = cend();
        for (auto p = cbegin()+pos; p != cend(); ++p)
        {
            if (Traits::find(s.data_, s.length_, *p) == 0)
            {
                it = p;
                break;
            }
        }
        return it == cend() ? npos : std::distance (cbegin(), it);
    }
    size_type find_first_not_of(CharT ch, size_type pos = 0) const JSONCONS_NOEXCEPT
    { 
        return find_first_not_of(Basic_string_view_(&ch, 1), pos); 
    }
    size_type find_first_not_of(const CharT* s, size_type pos, size_type n) const JSONCONS_NOEXCEPT
    { 
        return find_first_not_of(Basic_string_view_(s, n), pos); 
    }
    size_type find_first_not_of(const CharT* s, size_type pos = 0) const JSONCONS_NOEXCEPT
    { 
        return find_first_not_of(Basic_string_view_(s), pos); 
    }

    size_type find_last_not_of(Basic_string_view_ s, size_type pos = npos) const JSONCONS_NOEXCEPT 
    {
        if (pos >= length_)
        {
            pos = length_ - 1;
        }
        if (s.length_ == 0)
        {
            return pos;
        }
        pos = length_ - (pos+1);

        const_iterator it = crend();
        for (auto p = crbegin()+pos; p != crend(); ++p)
        {
            if (Traits::find(s.data_, s.length_, *p) == 0)
            {
                it = p;
                break;
            }
        }
        return it == crend() ? npos : (length_ - 1 - std::distance(crbegin(), it));
    }
    size_type find_last_not_of(CharT ch, size_type pos = npos) const JSONCONS_NOEXCEPT
    { 
        return find_last_not_of(Basic_string_view_(&ch, 1), pos); 
    }
    size_type find_last_not_of(const CharT* s, size_type pos, size_type n) const JSONCONS_NOEXCEPT
    { 
        return find_last_not_of(Basic_string_view_(s, n), pos); 
    }
    size_type find_last_not_of(const CharT* s, size_type pos = npos) const JSONCONS_NOEXCEPT
    { 
        return find_last_not_of(Basic_string_view_(s), pos); 
    }

    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const Basic_string_view_& sv)
    {
        os.write(sv.data_,sv.length_);
        return os;
    }
};

// ==
template<class CharT,class Traits>
bool operator==(const Basic_string_view_<CharT,Traits>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return lhs.compare(rhs) == 0;
}
template<class CharT,class Traits,class Allocator>
bool operator==(const Basic_string_view_<CharT,Traits>& lhs, 
                const std::basic_string<CharT,Traits,Allocator>& rhs)
{
    return lhs.compare(rhs) == 0;
}
template<class CharT,class Traits,class Allocator>
bool operator==(const std::basic_string<CharT,Traits,Allocator>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return rhs.compare(lhs) == 0;
}
template<class CharT,class Traits>
bool operator==(const Basic_string_view_<CharT,Traits>& lhs, 
                const CharT* rhs)
{
    return lhs.compare(rhs) == 0;
}
template<class CharT,class Traits>
bool operator==(const CharT* lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return rhs.compare(lhs) == 0;
}

// !=
template<class CharT,class Traits>
bool operator!=(const Basic_string_view_<CharT,Traits>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return lhs.compare(rhs) != 0;
}
template<class CharT,class Traits,class Allocator>
bool operator!=(const Basic_string_view_<CharT,Traits>& lhs, 
                const std::basic_string<CharT,Traits,Allocator>& rhs)
{
    return lhs.compare(rhs) != 0;
}
template<class CharT,class Traits,class Allocator>
bool operator!=(const std::basic_string<CharT,Traits,Allocator>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return rhs.compare(lhs) != 0;
}
template<class CharT,class Traits>
bool operator!=(const Basic_string_view_<CharT,Traits>& lhs, 
                const CharT* rhs)
{
    return lhs.compare(rhs) != 0;
}
template<class CharT,class Traits>
bool operator!=(const CharT* lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return rhs.compare(lhs) != 0;
}

// <=
template<class CharT,class Traits>
bool operator<=(const Basic_string_view_<CharT,Traits>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return lhs.compare(rhs) <= 0;
}
template<class CharT,class Traits,class Allocator>
bool operator<=(const Basic_string_view_<CharT,Traits>& lhs, 
                const std::basic_string<CharT,Traits,Allocator>& rhs)
{
    return lhs.compare(rhs) <= 0;
}
template<class CharT,class Traits,class Allocator>
bool operator<=(const std::basic_string<CharT,Traits,Allocator>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return rhs.compare(lhs) >= 0;
}

// <
template<class CharT,class Traits>
bool operator<(const Basic_string_view_<CharT,Traits>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return lhs.compare(rhs) < 0;
}
template<class CharT,class Traits,class Allocator>
bool operator<(const Basic_string_view_<CharT,Traits>& lhs, 
                const std::basic_string<CharT,Traits,Allocator>& rhs)
{
    return lhs.compare(rhs) < 0;
}
template<class CharT,class Traits,class Allocator>
bool operator<(const std::basic_string<CharT,Traits,Allocator>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return rhs.compare(lhs) > 0;
}

// >=
template<class CharT,class Traits>
bool operator>=(const Basic_string_view_<CharT,Traits>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return lhs.compare(rhs) >= 0;
}
template<class CharT,class Traits,class Allocator>
bool operator>=(const Basic_string_view_<CharT,Traits>& lhs, 
                const std::basic_string<CharT,Traits,Allocator>& rhs)
{
    return lhs.compare(rhs) >= 0;
}
template<class CharT,class Traits,class Allocator>
bool operator>=(const std::basic_string<CharT,Traits,Allocator>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return rhs.compare(lhs) <= 0;
}

// >
template<class CharT,class Traits>
bool operator>(const Basic_string_view_<CharT,Traits>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return lhs.compare(rhs) > 0;
}
template<class CharT,class Traits,class Allocator>
bool operator>(const Basic_string_view_<CharT,Traits>& lhs, 
                const std::basic_string<CharT,Traits,Allocator>& rhs)
{
    return lhs.compare(rhs) > 0;
}
template<class CharT,class Traits,class Allocator>
bool operator>(const std::basic_string<CharT,Traits,Allocator>& lhs, 
                const Basic_string_view_<CharT,Traits>& rhs)
{
    return rhs.compare(lhs) < 0;
}
#endif

#if !defined(JSONCONS_HAS_STRING_VIEW)
template <class CharT, class Traits = std::char_traits<CharT>>
using basic_string_view_ext = Basic_string_view_<CharT, Traits>;
#else
#include <string_view>
template <class CharT, class Traits = std::char_traits<CharT>>
using basic_string_view_ext = std::basic_string_view<CharT, Traits>;
#endif

#if !defined(JSONCONS_NO_TO_CHARS)
using chars_format = std::chars_format;
#else
enum class chars_format : uint8_t {fixed=1,scientific=2,hex=4,general=fixed|scientific};
#endif

// number_format

class number_format
{
    chars_format format_;
    uint8_t precision_;
    uint8_t decimal_places_;
public:
    number_format()
        : format_(chars_format::general), precision_(0), decimal_places_(0)
    {
    }

    number_format(uint8_t precision, uint8_t decimal_places)
        : format_(chars_format::general), precision_(precision), decimal_places_(decimal_places)
    {
    }

    number_format(chars_format format, uint8_t precision, uint8_t decimal_places)
        : format_(format), precision_(precision), decimal_places_(decimal_places)
    {
    }

    number_format(chars_format format)
        : format_(format), precision_(0), decimal_places_(0)
    {
    }

    number_format(const number_format&) = default;
    number_format(number_format&&) = default;
    number_format& operator=(const number_format& e) = default;
    number_format& operator=(number_format&& e) = default;

    uint8_t precision() const
    {
        return precision_;
    }

    uint8_t decimal_places() const
    {
        return decimal_places_;
    }

    chars_format floating_point_format() const
    {
        return format_;
    }
};

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

    basic_byte_string(const uint8_t* data, size_t length)
    {
        data_.insert(data,data+length);
    }

    basic_byte_string(const basic_byte_string& s) = default; 

    basic_byte_string(basic_byte_string&& s) = default; 

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
