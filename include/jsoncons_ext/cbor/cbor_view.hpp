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
#include <jsoncons/pretty_print.hpp>
#include <jsoncons/config/binary_utilities.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>

namespace jsoncons { namespace cbor {

using jsoncons::pretty_print;

class cbor_view 
{
    const uint8_t* first_;
    const uint8_t* last_; 
    const uint8_t* base_relative_; 
public:
    typedef cbor_view array;
    typedef std::allocator<char> allocator_type;
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
    friend class detail::const_object_iterator<cbor_view>;

    range<const_object_iterator> object_range() const
    {
        const uint8_t* endp;
        const uint8_t* begin;

        if (major_type() == cbor_major_type::map)
        {
            detail::get_map_size(first_,last_,&begin);
            if (begin == first_)
            {
                JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Failed finding size"));
            }
        }
        else
        {
            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Not an object"));
        }
        detail::walk_object(first_,last_,&endp);

        return range<const_object_iterator>(const_object_iterator(begin,endp,base_relative_), const_object_iterator(endp, endp, base_relative_));
    }

    range<const_array_iterator> array_range() const
    {
        const uint8_t* endp;
        const uint8_t* begin;

        detail::get_array_size(first_, last_, &begin);
        if (begin == first_)
        {
            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Invalid CBOR"));
        }
        detail::walk_array(first_,last_,&endp);
        if (endp == first_)
        {
            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Invalid CBOR"));
        }
        return range<const_array_iterator>(const_array_iterator(begin,endp,base_relative_), const_array_iterator(endp, endp, base_relative_));
    }

    cbor_view()
        : first_(nullptr), last_(nullptr), base_relative_(nullptr)
    {
    }

    cbor_view(const uint8_t* data, size_t length)
        : first_(data), last_(data+length), base_relative_(data)
    {
    }

    cbor_view(const uint8_t* data, size_t length, const uint8_t* base_relative)
        : first_(data), last_(data+length), base_relative_(base_relative)
    {
    }

    cbor_view(const std::vector<uint8_t>& v)
        : first_(v.data()), last_(v.data()+v.size()), base_relative_(v.data())
    {
    }

    cbor_view(const cbor_view& other)
        : first_(other.first_), last_(other.last_), base_relative_(other.base_relative_)
    {
    }

    cbor_view& operator=(const cbor_view&) = default;

    friend bool operator==(const cbor_view& lhs, const cbor_view& rhs) 
    {
        size_t n = lhs.last_ - lhs.first_;
        size_t m = rhs.last_ - rhs.first_;
        return (n != m) ? false : memcmp(lhs.first_,rhs.first_,n) == 0; 
    }

    friend bool operator!=(const cbor_view& lhs, const cbor_view& rhs) 
    {
        return !(lhs == rhs); 
    }

    const uint8_t* buffer() const
    {
        return first_;
    }

    size_t buflen() const
    {
        return last_ - first_;
    }

    uint8_t type() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return first_[0];
    }

    cbor_major_type major_type() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return get_major_type(first_[0]);
    }

    uint8_t additional_information_value() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return get_additional_information_value(first_[0]);
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

    bool is_null() const
    {
        switch (major_type())
        {
            case cbor_major_type::simple:
                switch (additional_information_value())
                {
                    case 0xf6:
                        return true;
                    default:
                        return false;
                }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.is_null();
            }
            default:
                return false;
        }
    }

    bool is_array() const
    {
        switch (major_type())
        {
            case cbor_major_type::array:
                return true;
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.is_array();
            }
            default:
                return false;
        }
    }

    bool is_object() const
    {
        switch (major_type())
        {
            case cbor_major_type::map:
                return true;
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.is_object();
            }
            default:
                return false;
        }
    }

    bool is_string() const
    {
        switch (major_type())
        {
            case cbor_major_type::text_string:
                return true;
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.is_string();
            }
            default:
                return false;
        }
    }

    bool is_byte_string() const
    {
        switch (major_type())
        {
            case cbor_major_type::byte_string:
                return true;
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.is_byte_string();
            }
            default:
                return false;
        }
    }

    bool is_bignum() const
    {
        uint8_t info = additional_information_value();
        return major_type() == cbor_major_type::semantic_tag && (info == 2 || info == 3);
    }

    bool is_bool() const
    {
        switch (major_type())
        {
            case cbor_major_type::simple:
                switch (additional_information_value())
                {
                    case 0xf4:
                    case 0xf5:
                        return true;
                    default:
                        return false;
                }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.is_bool();
            }
            default:
                return false;
        }
    }

    bool is_double() const
    {
        switch (major_type())
        {
            case cbor_major_type::simple:
                switch (additional_information_value())
                {
                    case 25:
                    case 26:
                    case 27:
                        return true;
                    default:
                        return false;
                }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.is_double();
            }
            default:
                return false;
        }
    }

    bool is_int64() const
    {
        switch (major_type())
        {
            case cbor_major_type::negative_integer:
                return true;
            case cbor_major_type::unsigned_integer:
            {
                const uint8_t* endp;
                uint64_t x = detail::get_uint64_value(first_,last_,&endp);
                if (endp == first_)
                {
                    return false;
                }
                if (x <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()))
                {
                    return true;
                }
                else
                {
                    return false;
                }
                break;
            }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.is_int64();
            }
            default:
                return false;
        }
    }

    bool is_uint64() const
    {
        switch (major_type())
        {
            case cbor_major_type::unsigned_integer:
                return true;
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.is_uint64();
            }
            default:
                return false;
        }
    }

    size_t size() const
    {
        size_t len = 0;
        switch (major_type())
        {
            case cbor_major_type::array:
            {
                    const uint8_t* endp;
                    len = detail::get_array_size(first_,last_,&endp);
                    break;
            }
            case cbor_major_type::map:
            {
                    const uint8_t* endp;
                    len = detail::get_map_size(first_,last_,&endp);
                    break;
            }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.size();
            }
            default:
                len = 0;
                break;

        }
        return len;
    }

    cbor_view operator[](size_t i) const
    {
        return at(i);
    }

    cbor_view operator[](const string_view_type& name) const
    {
        return at(name);
    }

    cbor_view at(size_t index) const
    {
        JSONCONS_ASSERT(is_array());
        const uint8_t* it = first_;

        detail::get_array_size(it, last_, &it);

        for (size_t i = 0; i < index; ++i)
        {
            const uint8_t* endp;
            detail::walk(it, last_, &endp);
            if (endp == it)
            {
                JSONCONS_THROW(cbor_decode_error(0));
            }
            it = endp;
        }

        const uint8_t* endp;
        detail::walk(it, last_, &endp);
        if (endp == it)
        {
            JSONCONS_THROW(cbor_decode_error(0));
        }

        return cbor_view(it, endp-it, base_relative_);
    }

    cbor_view at(const string_view_type& key) const
    {
        JSONCONS_ASSERT(is_object());
        const uint8_t* it = first_;

        size_t len = detail::get_map_size(first_, last_, &it);

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
                return cbor_view(it, last-it, base_relative_);
            }
            const uint8_t* last;
            detail::walk(it, last_, &last);
            it = last;
        }
        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Key not found"));
    }

    bool contains(const string_view_type& key) const
    {
        if (!is_object())
        {
            return false;
        }
        const uint8_t* it = first_;

        size_t len = detail::get_map_size(it, last_, &it);

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

    template<class T, class... Args>
    bool is(Args&&... args) const
    {
        return json_type_traits<cbor_view,T>::is(*this,std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    T as(Args&&... args) const
    {
        return json_type_traits<cbor_view,T>::as(*this,std::forward<Args>(args)...);
    }

    template <class T
#if !defined(JSONCONS_NO_DEPRECATED)
         = int64_t
#endif
        >
    typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, T>::type
    as_integer() const
    {
        switch (major_type())
        {
            case cbor_major_type::unsigned_integer:
            case cbor_major_type::negative_integer:
            {
                const uint8_t* endp;
                int64_t val = detail::get_int64_value(first_,last_,&endp);
                if (endp == first_)
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
                }
                return (T)val;
            }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_-(first_+1));
                return v.as_integer<T>();
            }
            default:
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
        }
    }

    template <class T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_signed<T>::value,T>::type
    as_integer() const
    {
        switch (major_type())
        {
            case cbor_major_type::unsigned_integer:
            case cbor_major_type::negative_integer:
            {
                const uint8_t* endp;
                uint64_t val = detail::get_uint64_value(first_,last_,&endp);
                if (endp == first_)
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
                }
                return (T)val;
            }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_-(first_+1));
                return v.as_integer<T>();
            }
            default:
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not an integer"));
        }
    }

    bool as_bool() const
    {
        switch (major_type())
        {
            case cbor_major_type::simple:
                switch (additional_information_value())
                {
                    case 0xf4:
                        return false;
                    case 0xf5:
                        return true;
                    default:
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a bool"));
                }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_ - (first_ + 1));
                return v.as_bool();
            }
            default:
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a bool"));
        }
    }

    double as_double() const
    {
        switch (major_type())
        {
            case cbor_major_type::unsigned_integer:
            {
                const uint8_t* endp;
                uint64_t val = detail::get_uint64_value(first_, last_, &endp);
                if (endp == first_)
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a double"));
                }
                return static_cast<double>(val);
            }
            case cbor_major_type::negative_integer:
            {
                const uint8_t* endp;
                int64_t val = detail::get_int64_value(first_, last_, &endp);
                if (endp == first_)
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a double"));
                }
                return static_cast<double>(val);
            }
            case cbor_major_type::simple:
            {
                switch (additional_information_value())
                {
                    case 0x19:
                    case 0x1a:
                    case 0x1b:
                    {
                        const uint8_t* endp;
                        double val = detail::get_double(first_, last_, &endp);
                        if (endp == first_)
                        {
                            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a double"));
                        }
                        return val;
                    }
                    default:
                        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a double"));
                        break;
                }
            }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_-(first_+1));
                return v.as_double();
            }
            default:
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a double"));
        }
    }

    std::string as_string() const
    {
        switch (major_type())
        {
            case cbor_major_type::text_string:
            {
                const uint8_t* endp;
                std::string s = detail::get_text_string(first_,last_,&endp);
                if (endp == first_)
                {
                    JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a string"));
                }
                return s;
            }
            case cbor_major_type::byte_string:
            {
                const uint8_t* endp;
                std::vector<uint8_t> v = detail::get_byte_string(first_,last_,&endp);
                if (endp == first_)
                {
                    JSONCONS_THROW(cbor_decode_error(0));
                }
                std::string s;
                encode_base64url(v.data(),v.size(),s);
                return s;
            }
            case cbor_major_type::semantic_tag:
            {
                switch (additional_information_value())
                {
                    case 2:
                    {
                        const uint8_t* endp;
                        std::vector<uint8_t> v = detail::get_byte_string(first_+1,last_,&endp);
                        if (endp == first_+1)
                        {
                            JSONCONS_THROW(cbor_decode_error(0));
                        }
                        bignum n = bignum(1, v.data(), v.size());
                        std::string s;
                        n.dump(s);
                        return s;
                    }
                    case 3:
                    {
                        const uint8_t* endp;
                        std::vector<uint8_t> v = detail::get_byte_string(first_+1,last_,&endp);
                        if (endp == first_+1)
                        {
                            JSONCONS_THROW(cbor_decode_error(0));
                        }
                        bignum n = bignum(-1, v.data(), v.size());
                        std::string s;
                        n.dump(s);
                        return s;
                    }
                    case 4:
                    {
                        const uint8_t* endp;
                        std::string s = cbor::detail::get_decimal_as_string(first_,last_,&endp);
                        if (endp == first_)
                        {
                            JSONCONS_THROW(cbor_decode_error(0));
                        }
                        return s;
                    }
                    default:
                        cbor_view v(first_ + 1, last_ - (first_ + 1));
                        return v.as_string();
                }
                break;
            }
            default:
            {
                std::string s;
                dump(s);
                return s;
            }
        }
    }

    template <typename BAllocator=std::allocator<char>>
    basic_byte_string<BAllocator> as_byte_string() const
    {
        switch (major_type())
        {
            case cbor_major_type::byte_string:
            {
                const uint8_t* endp;
                std::vector<uint8_t> v = detail::get_byte_string(first_,last_,&endp);
                if (endp == first_)
                {
                    JSONCONS_THROW(cbor_decode_error(0));
                }
                return basic_byte_string<BAllocator>(v.data(),v.size());
            }
            case cbor_major_type::semantic_tag:
            {
                cbor_view v(first_ + 1, last_-(first_+1));
                return v.as_byte_string<BAllocator>();
            }
            default:
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a byte string"));
                break;
            }
        }
    }

    bignum as_bignum() const
    {
        switch (major_type())
        {
            case cbor_major_type::semantic_tag:
            {
                uint8_t tag = additional_information_value();
                switch (tag)
                {
                    case 2:
                    {
                        const uint8_t* endp;
                        std::vector<uint8_t> v = detail::get_byte_string(first_+1,last_,&endp);
                        if (endp == first_+1)
                        {
                            JSONCONS_THROW(cbor_decode_error(0));
                        }
                        bignum n = bignum(1, v.data(), v.size());
                        return n;
                    }
                    case 3:
                    {
                        const uint8_t* endp;
                        std::vector<uint8_t> v = detail::get_byte_string(first_+1,last_,&endp);
                        if (endp == first_+1)
                        {
                            JSONCONS_THROW(cbor_decode_error(0));
                        }
                        bignum n = bignum(-1, v.data(), v.size());
                        return n;
                    }
                }
            }
            default:
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Not a bignum"));
                break;
            }
        }
    }

    template <typename Traits,typename SAllocator>
    void dump(std::basic_string<char,Traits,SAllocator>& s) const
    {
        typedef std::basic_string<char,Traits,SAllocator> string_type;
        basic_json_serializer<char,jsoncons::detail::string_writer<string_type>> serializer(s);
        dump(serializer);
    }

    template <typename Traits,typename SAllocator>
    void dump(std::basic_string<char,Traits,SAllocator>& s, indenting line_indent) const
    {
        typedef std::basic_string<char,Traits,SAllocator> string_type;
        basic_json_serializer<char,string_type> serializer(s, line_indent);
        dump(serializer);
    }

    template <typename Traits,typename SAllocator>
    void dump(std::basic_string<char,Traits,SAllocator>& s,
              const json_serializing_options& options) const
    {
        typedef std::basic_string<char,Traits,SAllocator> string_type;
        basic_json_serializer<char,jsoncons::detail::string_writer<string_type>> serializer(s, options);
        dump(serializer);
    }

    template <typename Traits,typename SAllocator>
    void dump(std::basic_string<char,Traits,SAllocator>& s,
              const json_serializing_options& options,
              indenting line_indent) const
    {
        typedef std::basic_string<char,Traits,SAllocator> string_type;
        basic_json_serializer<char,jsoncons::detail::string_writer<string_type>> serializer(s, options, line_indent);
        dump(serializer);
    }

    void dump(std::ostream& os) const
    {
        json_serializer serializer(os);
        dump(serializer);
    }

    void dump(std::ostream& os, indenting line_indent) const
    {
        json_serializer serializer(os, line_indent);
        dump(serializer);
    }

    void dump(std::ostream& os, const json_serializing_options& options) const
    {
        json_serializer serializer(os, options);
        dump(serializer);
    }

    void dump(std::ostream& os, const json_serializing_options& options, indenting line_indent) const
    {
        json_serializer serializer(os, options, line_indent);
        dump(serializer);
    }

    void dump(json_content_handler& handler) const
    {
        // If it's a non indefinite length string, dump view
        // If it's an indefinite length string, dump view
        switch (major_type())
        {
            case cbor_major_type::unsigned_integer:
            {
                uint64_t value = as_integer<uint64_t>();
                handler.uint64_value(value);
                break;
            }
            case cbor_major_type::negative_integer:
            {
                int64_t value = as_integer<int64_t>();
                handler.int64_value(value);
                break;
            }
            case cbor_major_type::byte_string:
            {
                const uint8_t* endp;
                std::vector<uint8_t> s = detail::get_byte_string(first_,last_,&endp);
                if (endp == first_)
                {
                    JSONCONS_THROW(cbor_decode_error(0));
                }
                handler.byte_string_value(s.data(), s.size());
                break;
            }
            case cbor_major_type::text_string:
            {
                const uint8_t* endp;
                std::string s = detail::get_text_string(first_,last_,&endp);
                if (endp == first_)
                {
                    JSONCONS_THROW(cbor_decode_error(0));
                }
                handler.string_value(s);
                break;
            }
            case cbor_major_type::array:
            {
                const uint8_t* it;
                size_t len = detail::get_array_size(first_,last_,&it);
                handler.begin_array(len);
                for (size_t i = 0; i < len; ++i)
                {
                    const uint8_t* endp;
                    detail::walk(it, last_, &endp);
                    if (endp == it)
                    {
                        JSONCONS_THROW(cbor_decode_error(0));
                    }

                    cbor_view(it,endp-it).dump(handler);
                    it = endp;
                }
                handler.end_array();
                break;
            }
            case cbor_major_type::map:
            {
                const uint8_t* it;
                size_t len = detail::get_map_size(first_,last_,&it);
                handler.begin_object(len);
                for (size_t i = 0; i < len; ++i)
                {
                    const uint8_t* endp;
                    std::string key = detail::get_text_string(it, last_, &endp);
                    if (endp == it)
                    {
                        JSONCONS_THROW(cbor_decode_error(last_-it));
                    }
                    handler.name(key);
                    it = endp;
                    detail::walk(it, last_, &endp);
                    if (endp == it)
                    {
                        JSONCONS_THROW(cbor_decode_error(0));
                    }

                    cbor_view(it,endp-it).dump(handler);
                    it = endp;
                }
                handler.end_object();
                break;
            }
            case cbor_major_type::semantic_tag:
            {
                uint8_t tag = additional_information_value();
                if (tag == 2)
                {
                    const uint8_t* endp;
                    std::vector<uint8_t> v = detail::get_byte_string(first_+1,last_,&endp);
                    if (endp == first_+1)
                    {
                        JSONCONS_THROW(cbor_decode_error(0));
                    }
                    handler.bignum_value(1, v.data(), v.size());
                }
                else if (tag == 3)
                {
                    const uint8_t* endp;
                    std::vector<uint8_t> v = detail::get_byte_string(first_+1,last_,&endp);
                    if (endp == first_+1)
                    {
                        JSONCONS_THROW(cbor_decode_error(0));
                    }
                    handler.bignum_value(-1, v.data(), v.size());
                }
                else if (tag == 4)
                {
                    const uint8_t* endp;
                    std::string s = cbor::detail::get_decimal_as_string(first_,last_,&endp);
                    if (endp == first_)
                    {
                        JSONCONS_THROW(cbor_decode_error(0));
                    }
                    handler.string_value(s, semantic_tag_type::decimal);
                }
                break;
            }
            case cbor_major_type::simple:
            {
                switch (type())
                {
                    case 0xf5:
                        handler.bool_value(true);
                        break;
                    case 0xf4:
                        handler.bool_value(false);
                        break;
                    case 0xf6:
                        handler.null_value();
                        break;
                    case 0xf9:
                    case 0xfa:
                    case 0xfb:
                    {
                        double value = as_double();
                        handler.double_value(value);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
        }
        handler.flush();
    }

    friend std::ostream& operator<<(std::ostream& os, const cbor_view& bv)
    {
        bv.dump(os);
        return os;
    }
#if !defined(JSONCONS_NO_DEPRECATED)

    bool has_key(const string_view_type& key) const
    {
        return contains(key);
    }

    bool is_integer() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return is_int64();
    }

    bool is_uinteger() const
    {
        JSONCONS_ASSERT(buflen() > 0);
        return is_uint64();
    }

    void dump_fragment(json_content_handler& handler) const
    {
        dump(handler);
    }
#endif
};
// decode_cbor

template<class Json>
void encode_cbor(const Json& j, std::basic_ostream<typename Json::char_type>& os)
{
    typedef typename Json::char_type char_type;
    basic_cbor_serializer<char_type> serializer(os);
    j.dump(serializer);
}

template<class Json>
void encode_cbor(const Json& j, std::vector<uint8_t>& v)
{
    typedef typename Json::char_type char_type;
    basic_cbor_serializer<char_type,jsoncons::detail::bytes_writer> serializer(v);
    j.dump(serializer);
}

template<class Json>
typename std::enable_if<std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(const cbor_view& v)
{
    jsoncons::json_decoder<Json> decoder;
    cbor_parser parser(decoder);
    parser.update(v.buffer(),v.buflen());
    std::error_code ec;
    parser.parse_some(ec);
    if (ec)
    {
        throw parse_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

template<class Json>
typename std::enable_if<!std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(const cbor_view& v)
{
    jsoncons::json_decoder<Json> decoder;
    basic_utf8_adaptor<typename Json::char_type> adaptor(decoder);
    cbor_parser parser(adaptor);
    parser.update(v.buffer(),v.buflen());
    std::error_code ec;
    parser.parse_some(ec);
    if (ec)
    {
        throw parse_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

template<class Json>
typename std::enable_if<std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(std::basic_istream<typename Json::char_type>& is)
{
    typedef typename Json::char_type char_type;

    std::vector<uint8_t> v;
    is.seekg(0, std::ios::end);   
    v.resize((size_t)is.tellg());
    is.seekg(0, std::ios::beg);    
    is.read((char_type*)&v[0],v.size());

    jsoncons::json_decoder<Json> decoder;
    cbor_parser parser(decoder);
    parser.update(v.data(),v.size());
    std::error_code ec;
    parser.parse_some(ec);
    if (ec)
    {
        throw parse_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}

template<class Json>
typename std::enable_if<!std::is_same<typename Json::char_type,char>::value,Json>::type 
decode_cbor(std::basic_istream<typename Json::char_type>& is)
{
    typedef typename Json::char_type char_type;

    std::vector<uint8_t> v;
    is.seekg(0, std::ios::end);   
    v.resize(is.tellg());
    is.seekg(0, std::ios::beg);    
    is.read((char_type*)&v[0],v.size());

    jsoncons::json_decoder<Json> decoder;
    basic_utf8_adaptor<typename Json::char_type> adaptor(decoder);
    cbor_parser parser(adaptor);
    parser.update(v.data(),v.size());
    std::error_code ec;
    parser.parse_some(ec);
    if (ec)
    {
        throw parse_error(ec,parser.line_number(),parser.column_number());
    }
    return decoder.get_result();
}
  
#if !defined(JSONCONS_NO_DEPRECATED)
template<class Json>
std::vector<uint8_t> encode_cbor(const Json& j)
{
    std::vector<uint8_t> v;
    encode_cbor(j, v);
    return v;
}
#endif

}}

#endif
