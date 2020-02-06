// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_SINK_HPP
#define JSONCONS_SINK_HPP

#include <stdexcept>
#include <string>
#include <vector>
#include <ostream>
#include <cmath>
#include <exception>
#include <memory> // std::addressof
#include <cstring> // std::memcpy
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/more_type_traits.hpp>

namespace jsoncons { 

// stream_sink

template <class CharT>
class stream_sink
{
public:
    typedef CharT value_type;
    typedef std::basic_ostream<CharT> output_type;

private:
    static const size_t default_buffer_length = 16384;

    std::basic_ostream<CharT>* os_;
    std::vector<CharT> buffer_;
    CharT * begin_buffer_;
    const CharT* end_buffer_;
    CharT* p_;

    // Noncopyable
    stream_sink(const stream_sink&) = delete;
    stream_sink& operator=(const stream_sink&) = delete;
    stream_sink& operator=(stream_sink&&) = delete;

public:
    stream_sink(stream_sink&&) = default;

    stream_sink(std::basic_ostream<CharT>& os)
        : os_(std::addressof(os)), buffer_(default_buffer_length), begin_buffer_(buffer_.data()), end_buffer_(begin_buffer_+buffer_.size()), p_(begin_buffer_)
    {
    }
    stream_sink(std::basic_ostream<CharT>& os, std::size_t buflen)
    : os_(std::addressof(os)), buffer_(buflen), begin_buffer_(buffer_.data()), end_buffer_(begin_buffer_+buffer_.size()), p_(begin_buffer_)
    {
    }
    ~stream_sink()
    {
        os_->write(begin_buffer_, buffer_length());
        os_->flush();
    }

    void flush()
    {
        os_->write(begin_buffer_, buffer_length());
        os_->flush();
        p_ = buffer_.data();
    }

    void append(const CharT* s, std::size_t length)
    {
        std::size_t diff = end_buffer_ - p_;
        if (diff >= length)
        {
            std::memcpy(p_, s, length*sizeof(CharT));
            p_ += length;
        }
        else
        {
            os_->write(begin_buffer_, buffer_length());
            os_->write(s,length);
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
            os_->write(begin_buffer_, buffer_length());
            p_ = begin_buffer_;
            push_back(ch);
        }
    }
private:

    std::size_t buffer_length() const
    {
        return p_ - begin_buffer_;
    }
};

// binary_stream_sink

class binary_stream_sink
{
public:
    typedef uint8_t value_type;
    typedef std::basic_ostream<char> output_type;
private:
    static const size_t default_buffer_length = 16384;

    std::basic_ostream<char>* os_;
    std::vector<uint8_t> buffer_;
    uint8_t * begin_buffer_;
    const uint8_t* end_buffer_;
    uint8_t* p_;

    // Noncopyable
    binary_stream_sink(const binary_stream_sink&) = delete;
    binary_stream_sink& operator=(const binary_stream_sink&) = delete;
    binary_stream_sink& operator=(binary_stream_sink&&) = delete;

public:
    binary_stream_sink(binary_stream_sink&&) = default;

    binary_stream_sink(std::basic_ostream<char>& os)
        : os_(std::addressof(os)), 
          buffer_(default_buffer_length), 
          begin_buffer_(buffer_.data()), 
          end_buffer_(begin_buffer_+buffer_.size()), 
          p_(begin_buffer_)
    {
    }
    binary_stream_sink(std::basic_ostream<char>& os, std::size_t buflen)
        : os_(std::addressof(os)), 
          buffer_(buflen), 
          begin_buffer_(buffer_.data()), 
          end_buffer_(begin_buffer_+buffer_.size()), 
          p_(begin_buffer_)
    {
    }
    ~binary_stream_sink()
    {
        os_->write((char*)begin_buffer_, buffer_length());
        os_->flush();
    }

    void flush()
    {
        os_->write((char*)begin_buffer_, buffer_length());
        p_ = buffer_.data();
    }

    void append(const uint8_t* s, std::size_t length)
    {
        std::size_t diff = end_buffer_ - p_;
        if (diff >= length)
        {
            std::memcpy(p_, s, length*sizeof(uint8_t));
            p_ += length;
        }
        else
        {
            os_->write((char*)begin_buffer_, buffer_length());
            os_->write((const char*)s,length);
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
            os_->write((char*)begin_buffer_, buffer_length());
            p_ = begin_buffer_;
            push_back(ch);
        }
    }
private:

    std::size_t buffer_length() const
    {
        return p_ - begin_buffer_;
    }
};

// string_sink

template <class StringT>
class string_sink 
{
public:
    typedef typename StringT::value_type value_type;
    typedef StringT output_type;
private:
    output_type* s_;

    // Noncopyable
    string_sink(const string_sink&) = delete;
    string_sink& operator=(const string_sink&) = delete;
    string_sink& operator=(string_sink&& val) = delete;
public:
    string_sink(string_sink&& val)
        : s_(nullptr)
    {
        std::swap(s_,val.s_);
    }

    string_sink(output_type& s)
        : s_(std::addressof(s))
    {
    }

    void flush()
    {
    }

    void append(const value_type* s, std::size_t length)
    {
        s_->insert(s_->end(), s, s+length);
    }

    void push_back(value_type ch)
    {
        s_->push_back(ch);
    }
};

// bytes_sink

class bytes_sink 
{
public:
    typedef uint8_t value_type;
    typedef std::vector<uint8_t> output_type;
private:
    output_type& s_;

    // Noncopyable
    bytes_sink(const bytes_sink&) = delete;
    bytes_sink& operator=(const bytes_sink&) = delete;
    bytes_sink& operator=(bytes_sink&&) = delete;
public:
    bytes_sink(bytes_sink&&) = default;

    bytes_sink(output_type& s)
        : s_(s)
    {
    }

    void flush()
    {
    }

    void append(const uint8_t* s, std::size_t length)
    {
        s_.insert(s_.end(), s, s+length);
    }

    void push_back(uint8_t ch)
    {
        s_.push_back(ch);
    }
};

}

#endif
