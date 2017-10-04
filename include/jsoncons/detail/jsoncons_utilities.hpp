// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONS_UTILITIES_HPP
#define JSONCONS_JSONCONS_UTILITIES_HPP

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
#include <jsoncons/detail/osequencestream.hpp>

#if defined(JSONCONS_HAS_STRING_VIEW)
#include <string_view>
#endif

namespace jsoncons
{
// bytes_view

class bytes_view
{
    const uint8_t* data_;
    size_t length_; 
public:
    typedef uint8_t value_type;
    typedef const uint8_t& const_reference;
    typedef const uint8_t* const_iterator;
    typedef const uint8_t* iterator;
    typedef std::size_t size_type;

    bytes_view(const uint8_t* data, size_t length)
        : data_(data), length_(length)
    {
    }

    operator std::vector<uint8_t>() const
    { 
        return std::vector<uint8_t>(begin(),end()); 
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

    const_reference operator[](size_type pos) const 
    { 
        return data_[pos]; 
    }

    friend bool operator==(const bytes_view& lhs, 
                           const bytes_view& rhs)
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
};

// static_max

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

// type_wrapper

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

// json_literals

namespace detail {
JSONCONS_DEFINE_LITERAL(null_literal,"null");
JSONCONS_DEFINE_LITERAL(true_literal,"true");
JSONCONS_DEFINE_LITERAL(false_literal,"false");
}

inline
unsigned char to_hex_character(unsigned char c)
{
    JSONCONS_ASSERT(c <= 0xF);

    return (c < 10) ? ('0' + c) : ('A' - 10 + c);
}

inline
bool is_control_character(uint32_t c)
{
    return c <= 0x1F || c == 0x7f;
}

inline
bool is_non_ascii_codepoint(uint32_t cp)
{
    return cp >= 0x80;
}

template <typename T>
struct is_stateless
 : public std::integral_constant<bool,  
      (std::is_default_constructible<T>::value &&
      std::is_empty<T>::value)>
{};

// type traits extensions

template<class Pointer> inline
typename std::pointer_traits<Pointer>::element_type* to_plain_pointer(Pointer ptr)
{       
    return (std::addressof(*ptr));
}
namespace detail {

// is_string_like

template <class T, class Enable=void>
struct is_string_like : std::false_type {};

template <class T>
struct is_string_like<T, 
                      typename std::enable_if<!std::is_void<typename T::traits_type>::value
>::type> : std::true_type {};

// is_integer_like

template <class T, class Enable=void>
struct is_integer_like : std::false_type {};

template <class T>
struct is_integer_like<T, 
                       typename std::enable_if<std::is_integral<T>::value && 
                       std::is_signed<T>::value && 
                       !std::is_same<T,bool>::value>::type> : std::true_type {};

// is_uinteger_like

template <class T, class Enable=void>
struct is_uinteger_like : std::false_type {};

template <class T>
struct is_uinteger_like<T, 
                        typename std::enable_if<std::is_integral<T>::value && 
                        !std::is_signed<T>::value && 
                        !std::is_same<T,bool>::value>::type> : std::true_type {};

// is_floating_point_like

template <class T, class Enable=void>
struct is_floating_point_like : std::false_type {};

template <class T>
struct is_floating_point_like<T, 
                              typename std::enable_if<std::is_floating_point<T>::value>::type> : std::true_type {};

// is_map_like

template <class T, class Enable=void>
struct is_map_like : std::false_type {};

template <class T>
struct is_map_like<T, 
                   typename std::enable_if<!std::is_void<typename T::mapped_type>::value>::type> 
    : std::true_type {};

// is_array_like
template<class T>
struct is_array_like : std::false_type {};

template<class E, size_t N>
struct is_array_like<std::array<E, N>> : std::true_type {};

// is_vector_like

template <class T, class Enable=void>
struct is_vector_like : std::false_type {};

template <class T>
struct is_vector_like<T, 
                      typename std::enable_if<!std::is_void<typename T::value_type>::value &&
                                              !is_array_like<T>::value && 
                                              !is_string_like<T>::value && 
                                              !is_map_like<T>::value 
>::type> 
    : std::true_type {};

}

// to_plain_pointer

template<class T> inline
T * to_plain_pointer(T * ptr)
{       
    return (ptr);
}  

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
    operator std::basic_string<CharT,Traits,Allocator>() const
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
            JSONCONS_THROW_EXCEPTION(std::out_of_range, "pos exceeds length");
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
            JSONCONS_THROW_EXCEPTION(std::out_of_range, "pos exceeds size");
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

template <class CharT>
class buffered_output
{
    static const size_t default_buffer_length = 16384;

    std::basic_ostream<CharT>& os_;
    std::vector<CharT> buffer_;
    CharT * const begin_buffer_;
    const CharT* const end_buffer_;
    CharT* p_;


    // Noncopyable and nonmoveable
    buffered_output(const buffered_output&) = delete;
    buffered_output& operator=(const buffered_output&) = delete;

public:
    buffered_output(std::basic_ostream<CharT>& os)
        : os_(os), buffer_(default_buffer_length), begin_buffer_(buffer_.data()), end_buffer_(buffer_.data()+default_buffer_length), p_(buffer_.data())
    {
    }
    buffered_output(std::basic_ostream<CharT>& os, size_t buflen)
        : os_(os), buffer_(buflen), begin_buffer_(buffer_.data()), end_buffer_(buffer_.data()+buflen), p_(buffer_.data())
    {
    }
    ~buffered_output()
    {
        os_.write(begin_buffer_, (p_ - begin_buffer_));
        os_.flush();
    }

    void flush()
    {
        os_.write(begin_buffer_, (p_ - begin_buffer_));
        p_ = begin_buffer_;
        os_.flush();
    }

    void write(const CharT* s, size_t length)
    {
        size_t diff = end_buffer_ - p_;
        if (diff >= length)
        {
            std::memcpy(p_, s, length*sizeof(CharT));
            p_ += length;
        }
        else
        {
            os_.write(begin_buffer_, (p_ - begin_buffer_));
            os_.write(s, length);
            p_ = begin_buffer_;
        }
    }

    void write(const std::basic_string<CharT>& s)
    {
        write(s.data(),s.length());
    }

    void put(CharT ch)
    {
        if (p_ < end_buffer_)
        {
            *p_++ = ch;
        }
        else
        {
            os_.write(begin_buffer_, (p_-begin_buffer_));
            p_ = begin_buffer_;
            *p_++ = ch;
        }
    }

};

// print_double

#ifdef JSONCONS_HAS__ECVT_S

template <class CharT>
class print_double
{
    uint8_t precision_;
public:
    print_double(uint8_t precision)
        : precision_(precision)
    {
    }

    void operator()(double val, uint8_t precision, buffered_output<CharT>& os) 
    {
        char buf[_CVTBUFSIZE];
        int decimal_point = 0;
        int sign = 0;

        int prec = (precision_ == 0) ? precision : precision_;

        int err = _ecvt_s(buf, _CVTBUFSIZE, val, prec, &decimal_point, &sign);
        if (err != 0)
        {
            throw std::runtime_error("Failed attempting double to string conversion");
        }
        //std::cout << "prec:" << prec << ", buf:" << buf << std::endl;
        char* s = buf;
        char* se = s + prec;

        int i, k;
        int j;

        if (sign)
        {
            os.put('-');
        }
        if (decimal_point <= -4 || decimal_point > se - s + 5) 
        {
            os.put(*s++);
            if (s < se) 
            {
                os.put('.');
                while ((se-1) > s && *(se-1) == '0')
                {
                    --se;
                }

                while(s < se)
                {
                    os.put(*s++);
                }
            }
            os.put('e');
            /* sprintf(b, "%+.2d", decimal_point - 1); */
            if (--decimal_point < 0) {
                os.put('-');
                decimal_point = -decimal_point;
                }
            else
                os.put('+');
            for(j = 2, k = 10; 10*k <= decimal_point; j++, k *= 10);
            for(;;) 
            {
                i = decimal_point / k;
                os.put(static_cast<CharT>(i) + '0');
                if (--j <= 0)
                    break;
                decimal_point -= i*k;
                decimal_point *= 10;
            }
        }
        else if (decimal_point <= 0) 
        {
            os.put('0');
            os.put('.');
            while ((se-1) > s && *(se-1) == '0')
            {
                --se;
            }
            for(; decimal_point < 0; decimal_point++)
            {
                os.put('0');
            }
            while(s < se)
            {
                os.put(*s++);
            }
        }
        else {
            while(s < se) 
            {
                os.put(*s++);
                if ((--decimal_point == 0) && s < se)
                {
                    os.put('.');
                    while ((se-1) > s && *(se-1) == '0')
                    {
                        --se;
                    }
                }
            }
            for(; decimal_point > 0; decimal_point--)
            {
                os.put('0');
            }
        }
    }
};

#else

template <class CharT>
class print_double
{
    uint8_t precision_;
    basic_osequencestream<CharT> oss_;
public:
    print_double(uint8_t precision)
        : precision_(precision)
    {
        oss_.imbue(std::locale::classic());
        oss_.precision(precision);
    }
    void operator()(double val, uint8_t precision, buffered_output<CharT>& os)
    {
        oss_.clear_sequence();
        oss_.precision((precision_ == 0) ? precision : precision_);
        oss_ << val;

        //std::cout << "precision_:" << (int)precision_ << ", precision:" << (int)precision << ", buf:" << oss_.data() << std::endl;

        const CharT* sbeg = oss_.data();
        const CharT* send = sbeg + oss_.length();
        const CharT* pexp = send;

        if (sbeg != send)
        {
            bool dot = false;
            for (pexp = sbeg; *pexp != 'e' && *pexp != 'E' && pexp < send; ++pexp)
            {
            }

            if (pexp != send)
            {
                const CharT* p = pexp;
                while (p >= sbeg+2 && *(p-1) == '0' && *(p-2) != '.')
                {
                    --p;
                }
                for (const CharT* q = sbeg; q < p; ++q)
                {
                    if (*q == '.')
                    {
                        dot = true;
                    }
                    os.put(*q);
                }
                if (!dot)
                {
                    os.put('.');
                    os.put('0');
                    dot = true;
                }
                for (const CharT* q = pexp; q < send; ++q)
                {
                    os.put(*q);
                }
            }
            else
            {
                const CharT* p = send;
                while (p >= sbeg+2 && *(p-1) == '0' && *(p-2) != '.')
                {
                    --p;
                }
                const CharT* qend = *(p-2) == '.' ? p : send;
                for (const CharT* q = sbeg; q < qend; ++q)
                {
                    if (*q == '.')
                    {
                        dot = true;
                    }
                    os.put(*q);
                }
            }

            if (!dot)
            {
                os.put('.');
                os.put('0');
            }
        }
    }
};
#endif

#if defined(_MSC_VER)

class string_to_double
{
private:
    _locale_t locale_;
public:
    string_to_double()
    {
        locale_ = _create_locale(LC_NUMERIC, "C");
    }
    ~string_to_double()
    {
        _free_locale(locale_);
    }

    double operator()(const char* s, size_t)
    {
        const char *begin = s;
        char *end = nullptr;
        double val = _strtod_l(begin, &end, locale_);
        if (begin == end)
        {
            throw std::invalid_argument("Invalid float value");
        }
        return val;
    }
private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double&) = delete;
    string_to_double& operator=(const string_to_double&) = delete;
};

#elif defined(JSONCONS_HAS_STRTOD_L)

class string_to_double
{
private:
    locale_t locale_;
public:
    string_to_double()
    {
        locale_ = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
    }
    ~string_to_double()
    {
        freelocale(locale_);
    }

    double operator()(const char* s, size_t length)
    {
        const char *begin = s;
        char *end = nullptr;
        double val = strtod_l(begin, &end, locale_);
        if (begin == end)
        {
            throw std::invalid_argument("Invalid float value");
        }
        return val;
    }

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};

#else
class string_to_double
{
private:
    std::vector<char> buffer_;
    std::string decimal_point_;
    bool is_dot_;
public:
    string_to_double()
        : buffer_()
    {
        struct lconv * lc = localeconv();
        if (lc != nullptr)
        {
            decimal_point_ = std::string(lc->decimal_point);    
        }
        else
        {
            decimal_point_ = std::string("."); 
        }
        buffer_.reserve(100);
        is_dot_ = decimal_point_ == ".";
    }

    double operator()(const char* s, size_t length)
    {
        double val;
        if (is_dot_)
        {
            const char *begin = s;
            char *end = nullptr;
            val = strtod(begin, &end);
            if (begin == end)
            {
                throw std::invalid_argument("Invalid float value");
            }
        }
        else
        {
            buffer_.clear();
            size_t j = 0;
            const char* pe = s + length;
            for (const char* p = s; p < pe; ++p)
            {
                if (*p == '.')
                {
                    buffer_.insert(buffer_.begin() + j, decimal_point_.begin(), decimal_point_.end());
                    j += decimal_point_.length();
                }
                else
                {
                    buffer_.push_back(*p);
                    ++j;
                }
            }
            const char *begin = buffer_.data();
            char *end = nullptr;
            val = strtod(begin, &end);
            if (begin == end)
            {
                throw std::invalid_argument("Invalid float value");
            }
        }
        return val;
    }

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};
#endif

}

#endif
