// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_SOURCE_HPP
#define JSONCONS_DETAIL_SOURCE_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <limits> 
#include <type_traits>
#include <algorithm>
#include <exception>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { namespace detail {

struct binary_traits
{
    static int eof() 
    {
        return std::char_traits<char>::eof();
    }
};

class binary_stream_source 
{
public:
    typedef uint8_t value_type;
    typedef binary_traits traits_type;
private:
    std::istream& is_;
    std::streambuf* sbuf_;

    // Noncopyable and nonmoveable
    binary_stream_source(const binary_stream_source&) = delete;
    binary_stream_source& operator=(const binary_stream_source&) = delete;
public:
    binary_stream_source(binary_stream_source&&) = default;

    binary_stream_source(std::istream& is)
        : is_(is), sbuf_(is.rdbuf())
    {
    }

    ~binary_stream_source()
    {
    }

    binary_stream_source& operator=(binary_stream_source&&) = default;

    bool eof() const
    {
        return is_.eof();  
    }

    size_t get(value_type& c)
    {
        int val = sbuf_->sbumpc();
        if (!(val == traits_type::eof()))
        {
            c = (value_type)val;
            return 1;
        }
        else
        {
            is_.clear(is_.rdstate() | std::ios::eofbit);
            return 0;
        }
    }

    int get()
    {
        int c = sbuf_->sbumpc();
        if (c == traits_type::eof())
        {
            is_.clear(is_.rdstate() | std::ios::eofbit);
        }
        return c;
    }

    void ignore(size_t count)
    {
        for (size_t i = 0; i < count; ++i)
        {
            int c = sbuf_->sbumpc();
            if (c == traits_type::eof())
            {
                is_.clear(is_.rdstate() | std::ios::eofbit);
                return;
            }
        }
    }

    int peek() 
    {
        int c = sbuf_->sgetc();
        if (c == traits_type::eof())
        {
            is_.clear(is_.rdstate() | std::ios::eofbit);
        }
        return c;
    }

    template <class OutputIt>
    size_t read(OutputIt p, size_t length)
    {
        size_t count = 0;
        for (count = 0; count < length; ++count)
        {
            int c = sbuf_->sbumpc();
            if (c == traits_type::eof())
            {
                is_.clear(is_.rdstate() | std::ios::eofbit);
                return count;
            }
            *p++ = (value_type)c;
        }
        return count;
    }
};

class buffer_source 
{
public:
    typedef uint8_t value_type;
    typedef binary_traits traits_type;
private:
    const uint8_t* input_ptr_;
    const uint8_t* input_end_;
    bool eof_;

    // Noncopyable and nonmoveable
    buffer_source(const buffer_source&) = delete;
    buffer_source& operator=(const buffer_source&) = delete;
public:
    buffer_source(buffer_source&&) = default;

    buffer_source(const std::vector<uint8_t>& s)
        : input_ptr_(s.data()), input_end_(s.data()+s.size()), eof_(s.size() == 0)
    {
    }

    buffer_source(const uint8_t* data, size_t size)
        : input_ptr_(data), input_end_(data+size), eof_(size == 0)  
    {
    }

    buffer_source& operator=(buffer_source&&) = default;

    bool eof() const
    {
        return eof_;  
    }

    size_t get(value_type& c)
    {
        if (input_ptr_ < input_end_)
        {
            c = *input_ptr_++;
            return 1;
        }
        else
       {
            eof_ = true;
            input_ptr_ = input_end_;
            return 0;
        }
    }

    int get()
    {
        if (input_ptr_ < input_end_)
        {
            return *input_ptr_++;
        }
        else
       {
            eof_ = true;
            input_ptr_ = input_end_;
            return traits_type::eof();
        }
    }

    void ignore(size_t count)
    {
        size_t len;
        if ((size_t)(input_end_ - input_ptr_) < count)
        {
            len = input_end_ - input_ptr_;
            eof_ = true;
        }
        else
        {
            len = count;
        }
        input_ptr_ += len;
    }

    int peek() 
    {
        return input_ptr_ < input_end_ ? *input_ptr_ : traits_type::eof();
    }

    size_t read(uint8_t* p, size_t length)
    {
        size_t len;
        if ((size_t)(input_end_ - input_ptr_) < length)
        {
            len = input_end_ - input_ptr_;
            eof_ = true;
        }
        else
        {
            len = length;
        }
        memcpy(p, input_ptr_, len);
        input_ptr_  += len;
        return len;
    }

    template <class OutputIt>
    size_t read(OutputIt d_first, size_t count)
    {
        size_t len;
        if ((size_t)(input_end_ - input_ptr_) < count)
        {
            len = input_end_ - input_ptr_;
            eof_ = true;
        }
        else
        {
            len = count;
        }
        for (size_t i = 0; i < len; ++i)
        {
            *d_first++ = *input_ptr_++;
        }
        return len;
    }
};

}}

#endif
