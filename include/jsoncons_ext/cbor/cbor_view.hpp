// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBORVIEW_HPP
#define JSONCONS_CBOR_CBORVIEW_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <limits>
#include <cassert>
#include <iterator>
#include <jsoncons/json.hpp>
#include <jsoncons/binary/binary_utilities.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>

namespace jsoncons { namespace cbor {

class cbor_view 
{
    const uint8_t* first_;
    const uint8_t* last_; 
public:
    typedef std::ptrdiff_t difference_type;
    typedef cbor_view value_type;
    typedef cbor_view& reference;
    typedef const cbor_view& const_reference;
    typedef cbor_view* pointer;
    typedef const cbor_view* const_pointer;
    typedef std::string string_type;
    typedef char char_type;
    typedef std::char_traits<char_type> char_traits_type;
    typedef basic_string_view<char_type> string_view_type;
    typedef detail::const_object_iterator<cbor_view> object_iterator;
    typedef detail::const_object_iterator<cbor_view> const_object_iterator;
    typedef detail::const_array_iterator<cbor_view> array_iterator;
    typedef detail::const_array_iterator<cbor_view> const_array_iterator;
    typedef detail::key_value_pair_view<cbor_view> key_value_pair_type;

    friend class detail::const_array_iterator<cbor_view>;

    range<const_object_iterator> object_range() const
    {
        const uint8_t* endp;
        const uint8_t* begin;

        switch (*first_)
        {
        case JSONCONS_CBOR_0xa0_0xb7: // map (0x00..0x17 pairs of data items follow)
            // FALLTHRU
        case 0xb8: // map (one-byte uint8_t for n follows)
            // FALLTHRU
        case 0xb9: // map (two-byte uint16_t for n follow)
            // FALLTHRU
        case 0xba: // map (four-byte uint32_t for n follow)
            // FALLTHRU
        case 0xbb: // map (eight-byte uint64_t for n follow)
            // FALLTHRU
        case 0xbf:
            detail::get_size(first_,last_,&begin);
            break;
        default:
            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Not an object"));
            break;
        }
        detail::walk_object(first_,last_,&endp);

        return range<const_object_iterator>(const_object_iterator(begin,endp), const_object_iterator(endp, endp));
    }

    range<const_array_iterator> array_range() const
    {
        const uint8_t* endp;
        const uint8_t* begin;

        switch (*first_)
        {
        case JSONCONS_CBOR_0x80_0x97: // array (0x00..0x17 data items follow)
            // FALLTHRU
        case 0x98: // array (one-byte uint8_t for n follows)
            // FALLTHRU
        case 0x99: // array (two-byte uint16_t for n follow)
            // FALLTHRU
        case 0x9a: // array (four-byte uint32_t for n follow)
            // FALLTHRU
        case 0x9b: // array (eight-byte uint64_t for n follow)
            // FALLTHRU
        case 0x9f: // array (indefinite length)
            detail::get_size(first_,last_,&begin);
            break;
        default:
            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Not an array"));
            break;
        }
        detail::walk_array(first_,last_,&endp);
        return range<const_array_iterator>(const_array_iterator(begin,endp), const_array_iterator(endp, endp));
    }

    cbor_view()
        : first_(nullptr), last_(nullptr)
    {
    }

    cbor_view(const uint8_t* buffer, size_t buflen)
        : first_(buffer), last_(buffer+buflen)
    {
    }

    cbor_view(const std::vector<uint8_t>& v)
        : first_(v.data()), last_(v.data()+v.size())
    {
    }

    cbor_view(const cbor_view& other)
        : first_(other.first_), last_(other.last_)
    {
    }

    cbor_view& operator=(const cbor_view&) = default;

    friend bool operator==(const cbor_view& lhs, const cbor_view& rhs) 
    {
        return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_; 
    }

    friend bool operator!=(const cbor_view& lhs, const cbor_view& rhs) 
    {
        return !(lhs == rhs); 
    }

    const uint8_t* buffer() const
    {
        return first_;
    }

    const size_t buflen() const
    {
        return last_ - first_;
    }

    bool is_null() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return first_[0] == 0xf6;
    }

    bool empty() const
    {
        bool is_empty;
        if (is_array() || is_object())
        {
            is_empty = (size() == 0);
        }
        else if (is_string())
        {
            const uint8_t* endp;
            size_t length = detail::get_text_string_length(first_,last_,&endp);
            is_empty = (length == 0);
        }
        else if (is_byte_string())
        {
            const uint8_t* endp;
            size_t length = detail::get_byte_string_length(first_, last_, &endp);
            is_empty = (length == 0);
        }
        else
        {
            is_empty = false;
        }

        return is_empty;
    }

    bool is_array() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return detail::is_array(first_[0]);
    }

    bool is_object() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return detail::is_object(first_[0]);
    }

    bool is_string() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return detail::is_string(first_[0]);
    }

    bool is_byte_string() const
    {
        JSONCONS_ASSERT(buflen() > 0);

        bool result;
        switch (first_[0])
        {
        case JSONCONS_CBOR_0x40_0x57: // byte string (0x00..0x17 bytes follow)
            // FALLTHRU
        case 0x58: // byte string (one-byte uint8_t for n follows)
            // FALLTHRU
        case 0x59: // byte string (two-byte uint16_t for n follow)
            // FALLTHRU
        case 0x5a: // byte string (four-byte uint32_t for n follow)
            // FALLTHRU
        case 0x5b: // byte string (eight-byte uint64_t for n follow)
            // FALLTHRU
        case 0x5f: // byte string, byte strings follow, terminated by "break"
            result = true;
            break;
        default: 
            result = false;
            break;
        }
        return result;
    }

    bool is_bool() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return detail::is_bool(first_[0]);
    }

    bool is_double() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return detail::is_double(first_[0]);
    }

    bool is_integer() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return detail::is_integer(first_,last_);
    }

    bool is_uinteger() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return detail::is_uinteger(first_[0]);
    }

    size_t size() const
    {
        const uint8_t* it;
        size_t len = detail::get_size(first_,last_,&it);
        return len;
    }

    cbor_view at(size_t index) const
    {
        JSONCONS_ASSERT(is_array());
        const uint8_t* it = first_;

        size_t len = detail::get_size(it, last_, &it);

        for (size_t i = 0; i < index; ++i)
        {
            detail::walk(it, last_, &it);
        }

        const uint8_t* endp;
        detail::walk(it, last_, &endp);

        return cbor_view(it,endp-it);
    }

    cbor_view at(const string_view_type& key) const
    {
        JSONCONS_ASSERT(is_object());
        const uint8_t* it = first_;

        size_t len = detail::get_size(first_, last_, &it);

        for (size_t i = 0; i < len; ++i)
        {
            const uint8_t* endp;
            string_type a_key = detail::get_text_string(it, last_, &endp);
            if (endp == it)
            {
                JSONCONS_THROW(cbor_decode_error(last_-it));
            }
            else
            {
                it = endp;
            }
            if (a_key == key)
            {
                const uint8_t* last;
                detail::walk(it, last_, &last);
                JSONCONS_ASSERT(last >= it);
                return cbor_view(it,last-it);
            }
            const uint8_t* last;
            detail::walk(it, last_, &last);
            it = last;
        }
        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Key not found"));
    }

    bool has_key(const string_view_type& key) const
    {
        if (!is_object())
        {
            return false;
        }
        const uint8_t* it = first_;

        size_t len = detail::get_size(it, last_, &it);

        for (size_t i = 0; i < len; ++i)
        {
            const uint8_t* endp;
            string_type a_key = detail::get_text_string(it, last_,&endp);
            if (endp == it)
            {
                JSONCONS_THROW(cbor_decode_error(last_-it));
            }
            else
            {
                it = endp;
            }
            if (a_key == key)
            {
                return true;
            }
            detail::walk(it, last_, &it);
        }
        return false;
    }

    int64_t as_integer() const
    {
        const uint8_t* endp;
        int64_t val = detail::get_integer(first_,last_,&endp);
        if (endp == first_)
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
        }
        return val;
    }

    bool as_bool() const
    {
        if (*first_ == 0xf5)
        {
            return true;
        }
        else if (*first_ == 0xf4)
        {
            return false;
        }
        else
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a bool"));
        }
    }

    uint64_t as_uinteger() const
    {
        const uint8_t* endp;
        uint64_t val = detail::get_uinteger(first_, last_, &endp);
        if (endp == first_)
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an unsigned integer"));
        }
        return val;
    }

    double as_double() const
    {
        double val;

        if (is_double())
        {
            const uint8_t* endp;
            val = detail::get_double(first_,last_,&endp);
            if (endp == first_)
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Invalid CBOR"));
            }
        }
        else if (is_uinteger())
        {
            val = static_cast<double>(as_uinteger());
        }
        else if (is_integer())
        {
            val = static_cast<double>(as_integer());
        }
        else
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a double"));
        }
        return val;
    }

    std::string as_string() const
    {
        const uint8_t* endp;
        std::string val = detail::get_text_string(first_,last_,&endp);
        if (endp == first_)
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a string"));
        }
        return val;
    }
protected:

    void set_data(const uint8_t* data, size_t length)
    {
        first_ = data;
        last_ = data + length;
    }    
};

}}

#endif
