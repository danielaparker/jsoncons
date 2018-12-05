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
        : input_ptr_(s.data()), input_end_(s.data()+s.size()), eof_(false)
    {
    }

    buffer_source(const uint8_t* data, size_t size)
        : input_ptr_(data), input_end_(data+size), eof_(false)  
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

    void increment()
    {
        if (input_ptr_ < input_end_)
        {
            input_ptr_++;
        }
        else
       {
            eof_ = true;
            input_ptr_ = input_end_;
        }
    }

    int peek() const
    {
        return input_ptr_ < input_end_ ? *input_ptr_ : traits_type::eof();
    }

    size_t read(value_type* p, size_t length)
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
    size_t read(size_t count, OutputIt d_first)
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
