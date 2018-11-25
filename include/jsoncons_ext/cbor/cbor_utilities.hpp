// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_UTILITIES_HPP
#define JSONCONS_CBOR_CBOR_UTILITIES_HPP

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
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_utilities.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <jsoncons_ext/cbor/cbor_error_category.hpp>

namespace jsoncons { namespace cbor {

enum class cbor_major_type : uint8_t
{
    unsigned_integer = 0x00,
    negative_integer = 0x01,
    byte_string = 0x02,
    text_string = 0x03,
    array = 0x04,
    map = 0x05,   
    semantic_tag = 0x06,
    simple = 0x7
};

namespace additional_info
{
    const uint8_t indefinite_length = 0x1f;
}

inline
cbor_major_type get_major_type(uint8_t type)
{
    static const uint8_t major_type_shift = 0x05;
    uint8_t value = type >> major_type_shift;
    return static_cast<cbor_major_type>(value);
}

inline
uint8_t get_additional_information_value(uint8_t type)
{
    static const uint8_t additional_information_mask = (1U << 5) - 1;
    uint8_t value = type & additional_information_mask;
    return value;
}

// 0x00..0x17 (0..23)
#define JSONCONS_CBOR_0x00_0x17 \
        0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x09:case 0x0a:case 0x0b:case 0x0c:case 0x0d:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16:case 0x17

class cbor_decode_error : public std::invalid_argument, public virtual json_exception
{
public:
    explicit cbor_decode_error(size_t pos) noexcept
        : std::invalid_argument("")
    {
        buffer_.append("Error decoding a cbor at position ");
        buffer_.append(std::to_string(pos));
    }
    ~cbor_decode_error() noexcept
    {
    }
    const char* what() const noexcept override
    {
        return buffer_.c_str();
    }
private:
    std::string buffer_;
};

namespace detail {

void walk(const uint8_t* first, const uint8_t* last, const uint8_t** endp);


inline 
size_t get_length(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
        return 0;
    }

    switch (get_major_type(*first))
    {
        case cbor_major_type::byte_string:
        case cbor_major_type::text_string:
        case cbor_major_type::array:
        case cbor_major_type::map:
            break;
        default:
            return 0;
    }

    size_t length = 0;
    const uint8_t* p = first+1;
    switch (get_additional_information_value(*first))
    {
        case JSONCONS_CBOR_0x00_0x17: // 0x00..0x17 (0..23)
        {
            length = *first & 0x1f;
            *endp = p;
            break;
        }
    case 0x18: // one-byte uint8_t for n follows
        {
            length = binary::from_big_endian<uint8_t>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
            break;
        }
    case 0x19: // two-byte uint16_t for n follow
        {
            length = binary::from_big_endian<uint16_t>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
            break;
        }
    case 0x1a: // four-byte uint32_t for n follow
        {
            length = binary::from_big_endian<uint32_t>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
            break;
        }
    case 0x1b: // eight-byte uint64_t for n follow
        {
            length = (size_t)binary::from_big_endian<uint64_t>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
            break;
        }
    case additional_info::indefinite_length: 
        {
            switch (get_major_type(*first))
            {
                case cbor_major_type::array:
                {
                    length = 0;
                    while (*p != 0xff)
                    {
                        walk(p, last, &p);
                        ++length;
                    }
                    *endp = first + 1;
                    break;
                }
                case cbor_major_type::map:
                {
                    length = 0;
                    while (*p != 0xff)
                    {
                        walk(p, last, &p);
                        walk(p, last, &p);
                        ++length;
                    }
                    *endp = first + 1;
                    break;
                }
                case cbor_major_type::text_string:
                case cbor_major_type::byte_string:
                    length = 0;
                    while (*p != 0xff)
                    {
                        size_t len = jsoncons::cbor::detail::get_length(p,last,endp);
                        if (*endp == p)
                        {
                            *endp = first;
                            break;
                        }
                        length += len;
                    }
                    *endp = first + 1;
                    break;
                default:
                    break;
            }
            break;
        }
    default: 
        {
            *endp = first;
        }
    }
    
    return length;
}

inline 
std::vector<uint8_t> get_byte_string(const uint8_t* first, const uint8_t* last, 
                                     const uint8_t** endp)
{
    std::vector<uint8_t> v;
    if (JSONCONS_UNLIKELY(last <= first || get_major_type(*first) != cbor_major_type::byte_string))
    {
        *endp = first; 
        return v;
    }

    switch (get_additional_information_value(*first))
    {
        case additional_info::indefinite_length: 
        {
            const uint8_t* p = first+1;
            while (*p != 0xff)
            {
                std::vector<uint8_t> ss = jsoncons::cbor::detail::get_byte_string(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                    return v;
                }
                p = *endp;
                v.insert(v.end(),ss.begin(),ss.end());
            }
            *endp = p+1;
            break;
        }
    default: 
        {
            size_t length = get_length(first, last, endp);
            if (*endp == first)
            {
                return v;
            }
            const uint8_t* p = *endp;
            *endp = p + length;
            v = std::vector<uint8_t>(p, *endp);
            break;
        }
    }
    
    return v;
}

inline 
std::string get_text_string(const uint8_t* first, const uint8_t* last, 
                            const uint8_t** endp)
{
    std::string s;
    if (JSONCONS_UNLIKELY(last <= first || get_major_type(*first) != cbor_major_type::text_string))
    {
        *endp = first; 
        return s;
    }

    switch (get_additional_information_value(*first))
    {
    case additional_info::indefinite_length:
        {
            const uint8_t* p = first+1;
            while (*p != 0xff)
            {
                std::string ss = jsoncons::cbor::detail::get_text_string(p,last,endp);
                if (*endp == p)
                {
                    *endp = first;
                    break;
                }
                else
                {
                    p = *endp;
                    s.append(std::move(ss));
                }
            }
            *endp = p+1;
            break;
        }
    default: // definite length
        {
            size_t length = get_length(first, last, endp);
            if (*endp == first)
            {
                return s;
            }
            const uint8_t* p = *endp;
            *endp = p + length;
            s.resize(length);
            std::copy(p,*endp,s.begin());
            break;
        }
    }
    
    return s;
}

inline
void walk_object(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (JSONCONS_UNLIKELY(last <= first || get_major_type(*first) != cbor_major_type::map))
    {
        *endp = first; 
        return;
    }

    uint8_t info = get_additional_information_value(*first);
    switch (info)
    {
    case additional_info::indefinite_length: 
        {
            const uint8_t* p = first+1;
            while (*p != 0xff)
            {
                walk(p, last, endp);
                if (*endp == p)
                {
                    *endp = first;
                    return;
                }
                p = *endp;
                walk(p, last, endp);
                if (*endp == p)
                {
                    *endp = first;
                    return;
                }
                p = *endp;
            }
            *endp = p;
            break;
        }
        default: // definite length
        {
            size_t size = jsoncons::cbor::detail::get_length(first,last,endp);
            if (*endp == first)
            {
                *endp = first;
                return;
            }
            const uint8_t* p = *endp;
            for (size_t i = 0; i < size; ++i)
            {
                walk(p, last, endp);
                if (*endp == p)
                {
                    *endp = first;
                    return;
                }
                p = *endp;
                walk(p, last, endp);
                if (*endp == p)
                {
                    *endp = first;
                    return;
                }
                p = *endp;
            }
            break;
        }
    }    
}

inline
void walk_array(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (JSONCONS_UNLIKELY(last <= first || get_major_type(*first) != cbor_major_type::array))
    {
        *endp = first; 
        return;
    }
    uint8_t info = get_additional_information_value(*first);
    switch (info)
    {
        case additional_info::indefinite_length: 
        {
            const uint8_t* p = first+1;
            while (*p != 0xff)
            {
                walk(p, last, endp);
                if (*endp == p)
                {
                    *endp = first;
                    break;
                }
                else
                {
                    p = *endp;
                }
            }
            *endp = p;
            break;
        }
        default: // definite length
        {
            size_t size = jsoncons::cbor::detail::get_length(first,last,endp);
            if (*endp == first)
            {
                *endp = first;
                return;
            }
            const uint8_t* p = *endp;
            for (size_t i = 0; i < size; ++i)
            {
                walk(p, last, endp);
                if (*endp == p)
                {
                    *endp = first;
                    return;
                }
                p = *endp;
            }
            break;
        } 
    }
}

inline
uint64_t get_uint64_value(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    uint64_t val = 0;
    if (JSONCONS_UNLIKELY(last <= first || get_major_type(*first) != cbor_major_type::unsigned_integer))
    {
        *endp = first; 
        return val;
    }

    const uint8_t* p = first+1;
    uint8_t info = get_additional_information_value(*first);
    switch (info)
    {
    case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
        val = info;
        *endp = p;
        break;

    case 0x18: // Unsigned integer (one-byte uint8_t follows)
        {
            val = binary::from_big_endian<uint8_t>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
            break;
        }

    case 0x19: // Unsigned integer (two-byte uint16_t follows)
        {
            val = binary::from_big_endian<uint16_t>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
            break;
        }

    case 0x1a: // Unsigned integer (four-byte uint32_t follows)
        {
            val = binary::from_big_endian<uint32_t>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
            break;
        }

    case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
        {
            val = binary::from_big_endian<uint64_t>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
            break;
        }
    default:
        {
            *endp = first; 
        }
    }
    return val;
}

inline
int64_t get_int64_value(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    int64_t val = 0;
    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endp = first; 
        return val;
    }

    const uint8_t* p = first + 1;
    uint8_t info = get_additional_information_value(*first);
    switch (get_major_type(*first))
    {
        case cbor_major_type::negative_integer:
            switch (info)
            {
                case JSONCONS_CBOR_0x00_0x17: // 0x00..0x17 (0..23)
                    val = static_cast<int8_t>(- 1 - info);
                    *endp = p;
                    break;
                case 0x18: // Negative integer (one-byte uint8_t follows)
                    {
                        auto x = binary::from_big_endian<uint8_t>(p,last,endp);
                        if (*endp == p)
                        {
                            *endp = first;
                        }
                        else
                        {
                            val = static_cast<int64_t>(-1)- x;
                        }
                        break;
                    }

                case 0x19: // Negative integer -1-n (two-byte uint16_t follows)
                    {
                        auto x = binary::from_big_endian<uint16_t>(p,last,endp);
                        if (*endp == p)
                        {
                            *endp = first;
                        }
                        else
                        {
                            val = static_cast<int64_t>(-1)- x;
                        }
                        break;
                    }

                case 0x1a: // Negative integer -1-n (four-byte uint32_t follows)
                    {
                        auto x = binary::from_big_endian<uint32_t>(p,last,endp);
                        if (*endp == p)
                        {
                            *endp = first;
                        }
                        else
                        {                       
                            val = static_cast<int64_t>(-1)- x;
                        }
                        break;
                    }

                case 0x1b: // Negative integer -1-n (eight-byte uint64_t follows)
                    {
                        auto x = binary::from_big_endian<uint64_t>(p,last,endp);
                        if (*endp == p)
                        {
                            *endp = first;
                        }
                        else
                        {
                            val = static_cast<int64_t>(-1)- static_cast<int64_t>(x);
                        }
                        break;
                    }
            }
            break;

            case cbor_major_type::unsigned_integer:
            {
                uint64_t x = jsoncons::cbor::detail::get_uint64_value(first,last,endp);
                if (*endp != first)
                {
                    if (x <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()))
                    {
                        val = x;
                    }
                    else
                    {
                        *endp = first;
                    }
                }
                break;
            }
            break;
        default:
            *endp = first; 
            break;
    }

    return val;
}

inline
double get_double(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    double val = 0;
    if (JSONCONS_UNLIKELY(last <= first || get_major_type(*first) != cbor_major_type::simple))
    {
        *endp = first; 
        return val;
    }

    const uint8_t* p = first+1;

    uint8_t info = get_additional_information_value(*first);
    switch (info)
    {
    case 0x19: // Half-Precision Float (two-byte IEEE 754)
        {
            uint16_t x = binary::from_big_endian<uint16_t>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
            else
            {
                val = binary::decode_half(x);
            }
        }
        break;


    case 0x1a: // Single-Precision Float (four-byte IEEE 754)
        {
            val = binary::from_big_endian<float>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
        }
        break;

    case 0x1b: //  Double-Precision Float (eight-byte IEEE 754)
        {
            val = binary::from_big_endian<double>(p,last,endp);
            if (*endp == p)
            {
                *endp = first;
            }
        }
        break;
    default:
        {
            *endp = first; 
        }
    }
    
    return val;
}

inline
void walk(const uint8_t *first, const uint8_t *last, const uint8_t **endp)
{
    if (first >= last)
    {
        *endp = first;
        return;
    }

    const uint8_t *p = first + 1;

    uint8_t info = get_additional_information_value(*first);
    switch (get_major_type(*first))
    {
        case cbor_major_type::unsigned_integer:
        {
            get_uint64_value(first, last, endp);
            break;
        }
        case cbor_major_type::negative_integer:
        {
            get_int64_value(first, last, endp);
            break;
        }
        case cbor_major_type::byte_string:
        {
            if (info == additional_info::indefinite_length)
            {
                while (*p != 0xff)
                {
                    if (p == last)
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("eof"));
                    }
                    walk(p, last, &p);
                }
                *endp = p;
            } 
            else
            {
                size_t len = get_length(first, last, endp);
                if (*endp == first)
                {
                    *endp = first;
                    return;
                }
                *endp += len;
            }
            break;
        }
        case cbor_major_type::text_string:
        {
            if (info == additional_info::indefinite_length)
            {
                while (*p != 0xff)
                {
                    if (p == last)
                    {
                        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("eof"));
                    }
                    walk(p, last, &p);
                }
                *endp = p;
            } 
            else
            {
                size_t len = get_length(first, last, endp);
                if (*endp == first)
                {
                    *endp = first;
                    return;
                }
                *endp += len;
            }
            break;
        }
        case cbor_major_type::array:
        {
            walk_array(first, last, endp);
            break;
        }
        case cbor_major_type::map:
        {
            walk_object(first, last, endp);
            break;
        }
        case cbor_major_type::semantic_tag:
        {
            walk(p, last, endp);
            break;
        }
        case cbor_major_type::simple:
        {
            switch (info)
            {
                case 20: // False
                case 21: // True
                case 22: // Null
                case 23: // Undefined
                {
                    *endp = p;
                    break;
                }
                case 25: // Half-Precision Float (two-byte IEEE 754)
                {
                    p += sizeof(uint16_t);
                    *endp = p;
                    break;
                }
                case 26: // Single-Precision Float (four-byte IEEE 754)
                {
                    p += sizeof(float);
                    *endp = p;
                    break;
                }
                case 27: // Double-Precision Float (eight-byte IEEE 754)
                {
                    p += sizeof(double);
                    *endp = p;
                    break;
                }
                default:
                {
                    *endp = first;
                    break;
                }
            }
            break;
        }
        default:
        {
            *endp = first;
            break;
        }
    }

}

template <class T>
class const_array_iterator
{
    const uint8_t* p_;
    const uint8_t* last_;
    const uint8_t* base_relative_;
    T current_;
public:
    typedef typename T::difference_type difference_type;
    typedef typename T::value_type value_type;
    typedef typename T::const_reference reference;
    typedef typename T::const_pointer pointer;
    typedef std::forward_iterator_tag iterator_catagory;

    const_array_iterator()
        : p_(nullptr), last_(nullptr), base_relative_(nullptr)
    {
    }

    const_array_iterator(const uint8_t* p, const uint8_t* last, const uint8_t* base_relative)
        : p_(p), last_(last), base_relative_(base_relative)
    {
    }

    const_array_iterator(const const_array_iterator& other) = default;

    friend bool operator==(const const_array_iterator& lhs, const const_array_iterator& rhs) 
    {
        return lhs.p_ == rhs.p_; 
    }

    friend bool operator!=(const const_array_iterator& lhs, const const_array_iterator& rhs) 
    {
        return lhs.p_ != rhs.p_; 
    }

    friend bool operator<(const const_array_iterator& lhs, const const_array_iterator& rhs) 
    {
        return lhs.p_ == rhs.p_; 
    }

    const_array_iterator& operator++()
    {
        jsoncons::cbor::detail::walk(p_, last_, &p_);
        return *this;
    }

    const_array_iterator operator++(int) // postfix increment
    {
        const_array_iterator temp(*this);
        jsoncons::cbor::detail::walk(p_, last_, &p_);
        return temp;
    }

    reference operator*() const
    {
        const uint8_t* endp;
        jsoncons::cbor::detail::walk(p_, last_, &endp);
        const_cast<T*>(&current_)->first_ = p_;
        const_cast<T*>(&current_)->last_ = endp;
        const_cast<T*>(&current_)->base_relative_ = base_relative_;
        return current_;
    }

    pointer operator->() const
    {
        const uint8_t* endp;
        jsoncons::cbor::detail::walk(p_, last_, &endp);
        const_cast<T*>(&current_)->first_ = p_;
        const_cast<T*>(&current_)->last_ = endp;
        const_cast<T*>(&current_)->base_relative_ = base_relative_;
        return &current_;
    }
};

template <class T>
class const_object_iterator;

template <class T>
class key_value_view
{
    const uint8_t* key_begin_;
    const uint8_t* key_end_;
    const uint8_t* val_begin_;
    const uint8_t* val_end_;
    const uint8_t* base_relative_;

public:
    friend class const_object_iterator<T>;

    key_value_view()
        : key_begin_(nullptr), key_end_(nullptr), val_begin_(nullptr), val_end_(nullptr)
    {
    }
    key_value_view(const uint8_t* key_begin, const uint8_t* key_end, 
                        const uint8_t* val_begin, const uint8_t* val_end, 
                        const uint8_t* base_relative)
        : key_begin_(key_begin), key_end_(key_end), val_begin_(val_begin), val_end_(val_end), 
          base_relative_(base_relative)
    {
    }
    key_value_view(const key_value_view& other) = default;

    std::string key() const
    {
        const uint8_t* endp;
        return get_text_string(key_begin_, key_end_, &endp);
    }

    T value() const
    {
        return T(val_begin_, val_end_ - val_begin_, base_relative_);
    }
};

template <class T>
class const_object_iterator
{
    const uint8_t* p_;
    const uint8_t* last_;
    const uint8_t* base_relative_;
    key_value_view<T> kvpair_;
public:
    typedef typename T::difference_type difference_type;
    typedef key_value_view<T> value_type;
    typedef const key_value_view<T>& reference;
    typedef const key_value_view<T>* pointer;
    typedef std::forward_iterator_tag iterator_catagory;

    const_object_iterator()
        : p_(nullptr), last_(nullptr), base_relative_(nullptr)
    {
    }

    const_object_iterator(const uint8_t* p, const uint8_t* last, const uint8_t* base_relative)
        : p_(p), last_(last), base_relative_(base_relative)
    {
    }

    const_object_iterator(const const_object_iterator& other) = default;

    friend bool operator==(const const_object_iterator& lhs, const const_object_iterator& rhs) 
    {
        return lhs.p_ == rhs.p_; 
    }

    friend bool operator!=(const const_object_iterator& lhs, const const_object_iterator& rhs) 
    {
        return lhs.p_ != rhs.p_; 
    }

    friend bool operator<(const const_object_iterator& lhs, const const_object_iterator& rhs) 
    {
        return lhs.p_ == rhs.p_; 
    }

    const_object_iterator& operator++()
    {
        jsoncons::cbor::detail::walk(p_, last_, &p_);
        jsoncons::cbor::detail::walk(p_, last_, &p_);
        return *this;
    }

    const_object_iterator operator++(int) // postfix increment
    {
        const_object_iterator temp(*this);
        jsoncons::cbor::detail::walk(p_, last_, &p_);
        jsoncons::cbor::detail::walk(p_, last_, &p_);
        return temp;
    }

    reference operator*() const
    {
        const uint8_t* endp;

        const_cast<key_value_view<T>*>(&kvpair_)->key_begin_ = p_;
        jsoncons::cbor::detail::walk(kvpair_.key_begin_, last_, &endp);
        const_cast<key_value_view<T>*>(&kvpair_)->key_end_ = endp;
        const_cast<key_value_view<T>*>(&kvpair_)->val_begin_ = kvpair_.key_end_;
        jsoncons::cbor::detail::walk(kvpair_.val_begin_, last_, &endp);
        const_cast<key_value_view<T>*>(&kvpair_)->val_end_ = endp;
        const_cast<key_value_view<T>*>(&kvpair_)->base_relative_ = base_relative_;

        return kvpair_;
    }

    pointer operator->() const
    {
        const uint8_t* endp;

        const_cast<key_value_view<T>*>(&kvpair_)->key_begin_ = p_;
        jsoncons::cbor::detail::walk(kvpair_.key_begin_, last_, &endp);
        const_cast<key_value_view<T>*>(&kvpair_)->key_end_ = endp;
        const_cast<key_value_view<T>*>(&kvpair_)->val_begin_ = kvpair_.key_end_;
        jsoncons::cbor::detail::walk(kvpair_.val_begin_, last_, &endp);
        const_cast<key_value_view<T>*>(&kvpair_)->val_end_ = endp;
        const_cast<key_value_view<T>*>(&kvpair_)->base_relative_ = base_relative_;

        return &kvpair_;
    }
};

inline
std::string get_array_as_decimal_string(const uint8_t* first, const uint8_t* last, 
                                  const uint8_t** endpp)
{
    std::string s;

    if (JSONCONS_UNLIKELY(last <= first))
    {
        *endpp = first; 
        return s;
    }

    const uint8_t* p = first;
    const uint8_t* pos = p++;

    JSONCONS_ASSERT(get_major_type(*pos) == cbor_major_type::array);
    JSONCONS_ASSERT(get_additional_information_value(*pos) == 2);

    pos = p++;
    int64_t exponent = 0;
    switch (get_major_type(*pos))
    {
        case cbor_major_type::unsigned_integer:
        {
            exponent = jsoncons::cbor::detail::get_uint64_value(pos,last,endpp);
            if (*endpp == pos)
            {
                return s;
            }
            p = *endpp;
            break;
        }
        case cbor_major_type::negative_integer:
        {
            exponent = jsoncons::cbor::detail::get_int64_value(pos,last,endpp);
            if (*endpp == pos)
            {
                return s;
            }
            p = *endpp;
            break;
        }
        default:
            throw std::invalid_argument("Invalid decimal, integer expected");
    }

    pos = p++;
    switch (get_major_type(*pos))
    {
        case cbor_major_type::unsigned_integer:
        {
            uint64_t val = jsoncons::cbor::detail::get_uint64_value(pos,last,endpp);
            if (*endpp == pos)
            {
                return s;
            }
            jsoncons::detail::string_writer<std::string> writer(s);
            jsoncons::detail::print_uinteger(val, writer);
            p = *endpp;
            break;
        }
        case cbor_major_type::negative_integer:
        {
            int64_t val = jsoncons::cbor::detail::get_int64_value(pos,last,endpp);
            if (*endpp == pos)
            {
                return s;
            }
            jsoncons::detail::string_writer<std::string> writer(s);
            jsoncons::detail::print_integer(val, writer);
            p = *endpp;
            break;
        }
        case cbor_major_type::semantic_tag:
        {
            uint8_t tag = get_additional_information_value(*pos);
            pos = p++;
            if (get_major_type(*pos) == cbor_major_type::byte_string)
            {
                std::vector<uint8_t> v = jsoncons::cbor::detail::get_byte_string(pos,last,endpp);
                if (*endpp == pos)
                {
                    return s;
                }
                p = *endpp;
                if (tag == 2)
                {
                    bignum n(1, v.data(), v.size());
                    n.dump(s);
                }
                else if (tag == 3)
                {
                    bignum n(-1, v.data(), v.size());
                    n.dump(s);
                }
            }
            break;
        }
        default:
            throw std::invalid_argument("Invalid decimal, integer or bignum expected");
    }
    if (exponent < 0)
    {
        int64_t digits_length = static_cast<int64_t >(s.length()) + exponent;
        bool is_negative = false;
        if (s[0] == '-')
        {
            --digits_length;
            is_negative = true;
        }

        if (digits_length > 0)
        {
            size_t index = is_negative ? digits_length+1 : digits_length; 
            s.insert(index, ".");
        }
        else if (digits_length == 0)
        {
            size_t pos = is_negative ? 1 : 0; 
            s.insert(pos, "0.");
        }
        else 
        {
            size_t index = is_negative ? 1 : 0; 
            s.insert(index, "0.");
            s.append("e-");
            uint64_t u = static_cast<uint64_t>(-digits_length);
            do 
            {
                s.push_back(static_cast<char>(48+u%10));
            }
            while (u /= 10);
        }
    }
    else if (exponent == 0)
    {
        s.append(".0");
    }
    else if (exponent > 0)
    {
        s.append("e");
        do 
        {
            s.push_back(static_cast<char>(48+exponent%10));
        }
        while (exponent /= 10);
    }
    return s;
}

} // namespace detail

}}

#endif
