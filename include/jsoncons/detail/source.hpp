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

template <class Container>
class buffer_source 
{
public:
    typedef typename Container::value_type value_type;
    typedef Container input_type;
    typedef const Container& input_reference;
private:
    input_reference s_;
    size_t i_;
    bool eof_;

    // Noncopyable and nonmoveable
    buffer_source(const buffer_source&) = delete;
    buffer_source& operator=(const buffer_source&) = delete;
public:

    buffer_source(input_reference s)
        : s_(s), i_(0), eof_(s.size() == 0) 
    {
    }

    bool eof() const
    {
        return eof_; 
    }

    size_t get(value_type& c)
    {
        if (i_ < s_.size())
        {
            c = s_[i_++];
            return 1;
        }
        else
       {
            i_ = s_.size();
            eof_ = true;
            return 0;
        }
    }

    size_t read(value_type* p, size_t length)
    {
        size_t len;
        if (i_ + length >= s_.size())
        {
            len = s_.size() - i_;
            eof_ = true;
        }
        else
        {
            len = length;
        }
        memcpy(p, &s_[i_], len);
        i_ += len;
        return len;
    }

    template <class OutputIt>
    size_t read(size_t count, OutputIt d_first)
    {
        size_t len;
        if (i_ + count >= s_.size())
        {
            len = s_.size() - i_;
            eof_ = true;
        }
        else
        {
            len = count;
        }
        for (size_t i = 0; i < len; ++i)
        {
            *d_first++ = s_[i_++];
        }
        return len;
    }
};

}}

#endif
