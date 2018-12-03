// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_RESULT_HPP
#define JSONCONS_DETAIL_RESULT_HPP

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
#include <exception>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/type_traits_helper.hpp>

namespace jsoncons { namespace detail {

template <class CharT>
class char_stream_result
{
public:
    typedef CharT value_type;
    typedef std::basic_ostream<CharT> output_type;

private:
    static const size_t default_buffer_length = 16384;

    std::basic_ostream<CharT>& os_;
    std::vector<CharT> buffer_;
    CharT * begin_buffer_;
    const CharT* end_buffer_;
    CharT* p_;

    // Noncopyable and nonmoveable
    char_stream_result(const char_stream_result&) = delete;
    char_stream_result& operator=(const char_stream_result&) = delete;

public:
    char_stream_result(std::basic_ostream<CharT>& os)
        : os_(os), buffer_(default_buffer_length), begin_buffer_(buffer_.data()), end_buffer_(begin_buffer_+buffer_.size()), p_(begin_buffer_)
    {
    }
    char_stream_result(std::basic_ostream<CharT>& os, size_t buflen)
    : os_(os), buffer_(buflen), begin_buffer_(buffer_.data()), end_buffer_(begin_buffer_+buffer_.size()), p_(begin_buffer_)
    {
    }
    ~char_stream_result()
    {
        os_.write(begin_buffer_, buffer_length());
        os_.flush();
    }

    void flush()
    {
        os_.write(begin_buffer_, buffer_length());
        os_.flush();
        p_ = buffer_.data();
    }

    void insert(const CharT* s, size_t length)
    {
        size_t diff = end_buffer_ - p_;
        if (diff >= length)
        {
            std::memcpy(p_, s, length*sizeof(CharT));
            p_ += length;
        }
        else
        {
            os_.write(begin_buffer_, buffer_length());
            os_.write(s,length);
            p_ = begin_buffer_;
        }
    }

    void push_back(CharT ch)
    {
        if (p_ < end_buffer_)
        {
            *p_++ = ch;
        }
        else
        {
            os_.write(begin_buffer_, buffer_length());
            p_ = begin_buffer_;
            push_back(ch);
        }
    }
private:

    size_t buffer_length() const
    {
        return p_ - begin_buffer_;
    }
};

class byte_stream_result
{
public:
    typedef uint8_t value_type;
    typedef std::basic_ostream<char> output_type;
private:
    static const size_t default_buffer_length = 16384;

    std::basic_ostream<char>& os_;
    std::vector<uint8_t> buffer_;
    uint8_t * begin_buffer_;
    const uint8_t* end_buffer_;
    uint8_t* p_;

    // Noncopyable and nonmoveable
    byte_stream_result(const byte_stream_result&) = delete;
    byte_stream_result& operator=(const byte_stream_result&) = delete;

public:
    byte_stream_result(std::basic_ostream<char>& os)
        : os_(os), 
          buffer_(default_buffer_length), 
          begin_buffer_(buffer_.data()), 
          end_buffer_(begin_buffer_+buffer_.size()), 
          p_(begin_buffer_)
    {
    }
    byte_stream_result(std::basic_ostream<char>& os, size_t buflen)
        : os_(os), 
          buffer_(buflen), 
          begin_buffer_(buffer_.data()), 
          end_buffer_(begin_buffer_+buffer_.size()), 
          p_(begin_buffer_)
    {
    }
    ~byte_stream_result()
    {
        os_.write((char*)begin_buffer_, buffer_length());
        os_.flush();
    }

    void flush()
    {
        os_.write((char*)begin_buffer_, buffer_length());
        p_ = buffer_.data();
    }

    void insert(const uint8_t* s, size_t length)
    {
        size_t diff = end_buffer_ - p_;
        if (diff >= length)
        {
            std::memcpy(p_, s, length*sizeof(uint8_t));
            p_ += length;
        }
        else
        {
            os_.write((char*)begin_buffer_, buffer_length());
            os_.write((char*)s,length);
            p_ = begin_buffer_;
        }
    }

    void push_back(uint8_t ch)
    {
        if (p_ < end_buffer_)
        {
            *p_++ = ch;
        }
        else
        {
            os_.write((char*)begin_buffer_, buffer_length());
            p_ = begin_buffer_;
            push_back(ch);
        }
    }
private:

    size_t buffer_length() const
    {
        return p_ - begin_buffer_;
    }
};

template <class StringT>
class string_result 
{
public:
    typedef typename StringT::value_type value_type;
    typedef StringT output_type;
private:
    output_type& s_;

    // Noncopyable and nonmoveable
    string_result(const string_result&) = delete;
    string_result& operator=(const string_result&) = delete;
public:

    string_result(output_type& s)
        : s_(s)
    {
    }

    void flush()
    {
    }

    void insert(const value_type* s, size_t length)
    {
        s_.append(s,length);
    }

    void push_back(value_type ch)
    {
        s_.push_back(ch);
    }
};

class bytes_result 
{
public:
    typedef uint8_t value_type;
    typedef std::vector<uint8_t> output_type;
private:
    output_type& s_;

    // Noncopyable and nonmoveable
    bytes_result(const bytes_result&) = delete;
    bytes_result& operator=(const bytes_result&) = delete;
public:

    bytes_result(output_type& s)
        : s_(s)
    {
    }

    void flush()
    {
    }

    void insert(const uint8_t* s, size_t length)
    {
        s_.insert(s_.end(), s, s+length);
    }

    void push_back(uint8_t ch)
    {
        s_.push_back(ch);
    }
};

}}

#endif
