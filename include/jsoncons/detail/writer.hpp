// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_WRITERS_HPP
#define JSONCONS_DETAIL_WRITERS_HPP

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
#include <jsoncons/jsoncons_config.hpp>
#include <jsoncons/detail/type_traits_helper.hpp>

namespace jsoncons { namespace detail {

template <class CharT>
class stream_char_writer
{
public:
    typedef CharT char_type;
    typedef std::basic_ostream<CharT> output_type;
private:
    static const size_t default_buffer_length = 16384;

    std::basic_ostream<CharT>& os_;
    std::vector<CharT> buffer_;
    CharT * begin_buffer_;
    const CharT* end_buffer_;
    CharT* p_;

    // Noncopyable and nonmoveable
    stream_char_writer(const stream_char_writer&) = delete;
    stream_char_writer& operator=(const stream_char_writer&) = delete;

public:
    stream_char_writer(std::basic_ostream<CharT>& os)
        : os_(os), buffer_(default_buffer_length), begin_buffer_(buffer_.data()), end_buffer_(begin_buffer_+buffer_.size()), p_(begin_buffer_)
    {
    }
    stream_char_writer(std::basic_ostream<CharT>& os, size_t buflen)
    : os_(os), buffer_(buflen), begin_buffer_(buffer_.data()), end_buffer_(begin_buffer_+buffer_.size()), p_(begin_buffer_)
    {
    }
    ~stream_char_writer()
    {
        os_.write(begin_buffer_, buffer_length());
        os_.flush();
    }

    void flush()
    {
        os_.write(begin_buffer_, buffer_length());
        p_ = buffer_.data();
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
            os_.write(begin_buffer_, buffer_length());
            os_.write(s,length);
            p_ = begin_buffer_;
        }
    }

    void put(CharT ch)
    {
        if (p_ < end_buffer_)
        {
            *p_++ = ch;
        }
        else
        {
            os_.write(begin_buffer_, buffer_length());
            p_ = begin_buffer_;
            put(ch);
        }
    }
private:

    size_t buffer_length() const
    {
        return p_ - begin_buffer_;
    }
};

class stream_byte_writer
{
public:
    typedef std::basic_ostream<char> output_type;
private:
    static const size_t default_buffer_length = 16384;

    std::basic_ostream<char>& os_;
    std::vector<uint8_t> buffer_;
    uint8_t * begin_buffer_;
    const uint8_t* end_buffer_;
    uint8_t* p_;

    // Noncopyable and nonmoveable
    stream_byte_writer(const stream_byte_writer&) = delete;
    stream_byte_writer& operator=(const stream_byte_writer&) = delete;

public:
    stream_byte_writer(std::basic_ostream<char>& os)
        : os_(os), 
          buffer_(default_buffer_length), 
          begin_buffer_(buffer_.data()), 
          end_buffer_(begin_buffer_+buffer_.size()), 
          p_(begin_buffer_)
    {
    }
    stream_byte_writer(std::basic_ostream<char>& os, size_t buflen)
        : os_(os), 
          buffer_(buflen), 
          begin_buffer_(buffer_.data()), 
          end_buffer_(begin_buffer_+buffer_.size()), 
          p_(begin_buffer_)
    {
    }
    ~stream_byte_writer()
    {
        os_.write((char*)begin_buffer_, buffer_length());
        os_.flush();
    }

    void flush()
    {
        os_.write((char*)begin_buffer_, buffer_length());
        p_ = buffer_.data();
    }

    void write(const uint8_t* s, size_t length)
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

    void put(uint8_t ch)
    {
        if (p_ < end_buffer_)
        {
            *p_++ = ch;
        }
        else
        {
            os_.write((char*)begin_buffer_, buffer_length());
            p_ = begin_buffer_;
            put(ch);
        }
    }
private:

    size_t buffer_length() const
    {
        return p_ - begin_buffer_;
    }
};

template <class StringT>
class string_writer 
{
public:
    typedef typename StringT::value_type char_type;
    typedef StringT output_type;
private:
    output_type& s_;

    // Noncopyable and nonmoveable
    string_writer(const string_writer&) = delete;
    string_writer& operator=(const string_writer&) = delete;
public:

    string_writer(output_type& s)
        : s_(s)
    {
    }

    void flush()
    {
    }

    void write(const char_type* s, size_t length)
    {
        s_.append(s,length);
    }

    void put(char_type ch)
    {
        s_.push_back(ch);
    }
};

class bytes_writer 
{
public:
    typedef std::vector<uint8_t> output_type;
private:
    output_type& s_;

    // Noncopyable and nonmoveable
    bytes_writer(const bytes_writer&) = delete;
    bytes_writer& operator=(const bytes_writer&) = delete;
public:

    bytes_writer(output_type& s)
        : s_(s)
    {
    }

    void flush()
    {
    }

    void write(const uint8_t* s, size_t length)
    {
        s_.insert(s_.end(), s, s+length);
    }

    void put(uint8_t ch)
    {
        s_.push_back(ch);
    }
};

}}

#endif
