// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_PARSER_HPP
#define JSONCONS_CBOR_CBOR_PARSER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_config.hpp>
#include <jsoncons_ext/cbor/cbor_content_handler.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>

namespace jsoncons { namespace cbor {

enum class parse_mode {root,before_done,array,indefinite_array,map_key,map_value,indefinite_map_key,indefinite_map_value};

struct uint8_array_arg_t {};
constexpr uint8_array_arg_t uint8_array_arg = uint8_array_arg_t();
struct uint16_array_arg_t {};
struct uint32_array_arg_t {};
constexpr uint32_array_arg_t uint32_array_arg = uint32_array_arg_t();
struct uint64_array_arg_t {};
constexpr uint64_array_arg_t uint64_array_arg = uint64_array_arg_t();
struct int8_array_arg_t {};
constexpr int8_array_arg_t int8_array_arg = int8_array_arg_t();
struct int16_array_arg_t {};
constexpr int16_array_arg_t int16_array_arg = int16_array_arg_t();
struct int32_array_arg_t {};
constexpr int32_array_arg_t int32_array_arg = int32_array_arg_t();
struct int64_array_arg_t {};
constexpr int64_array_arg_t int64_array_arg = int64_array_arg_t();
constexpr uint16_array_arg_t uint16_array_arg = uint16_array_arg_t();
struct half_array_arg_t {};
constexpr half_array_arg_t half_array_arg = half_array_arg_t();
struct float_array_arg_t {};
constexpr float_array_arg_t float_array_arg = float_array_arg_t();
struct double_array_arg_t {};
constexpr double_array_arg_t double_array_arg = double_array_arg_t();
struct float128_array_arg_t {};
constexpr float128_array_arg_t float128_array_arg = float128_array_arg_t();

enum typed_array_type {uint8_value=1,uint16_value,uint32_value,uint64_value,
                      int8_value,int16_value,int32_value,int64_value, 
                      half_value, float_value,double_value,float128_value};

template <class Float128T = std::nullptr_t, class Allocator=std::allocator<char>>
class typed_array
{
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<uint8_t> uint8_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<uint16_t> uint16_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<uint32_t> uint32_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<uint64_t> uint64_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<int8_t> int8_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<int16_t> int16_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<int32_t> int32_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<int64_t> int64_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<float> float_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<double> double_allocator_type;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<Float128T> float128_allocator_type;

    Allocator allocator_;
    typed_array_type type_;
    union 
    {
        uint8_t* uint8_data_;
        uint16_t* uint16_data_;
        uint32_t* uint32_data_;
        uint64_t* uint64_data_;
        int8_t* int8_data_;
        int16_t* int16_data_;
        int32_t* int32_data_;
        int64_t* int64_data_;
        float* float_data_;
        double* double_data_;
        Float128T* float128_data_;
    } data_;
    size_t size_;
public:
    typed_array(const Allocator& alloc)
        : allocator_(alloc), type_(), data_(), size_(0)
    {
    }

    typed_array(const typed_array& other)
        : allocator_(other.allocator_), type_(other.type_), data_(), size_(other.size())
    {
        switch (other.type_)
        {
            case typed_array_type::uint8_value:
            {
                uint8_allocator_type alloc{ allocator_ };
                data_.uint8_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::uint16_value:
            {
                uint16_allocator_type alloc{ allocator_ };
                data_.uint16_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::uint32_value:
            {
                uint32_allocator_type alloc{ allocator_ };
                data_.uint32_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::uint64_value:
            {
                uint64_allocator_type alloc{ allocator_ };
                data_.uint64_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::int8_value:
            {
                int8_allocator_type alloc{ allocator_ };
                data_.int8_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::int16_value:
            {
                int16_allocator_type alloc{ allocator_ };
                data_.int16_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::int32_value:
            {
                int32_allocator_type alloc{ allocator_ };
                data_.int32_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::int64_value:
            {
                int64_allocator_type alloc{ allocator_ };
                data_.int64_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::half_value:
            {
                uint16_allocator_type alloc{ allocator_ };
                data_.uint16_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::float_value:
            {
                float_allocator_type alloc{ allocator_ };
                data_.float_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::double_value:
            {
                double_allocator_type alloc{allocator_};
                data_.double_data_ = alloc.allocate(size_);
                break;
            }
            case typed_array_type::float128_value:
            {
                allocate_float128();
                break;
            }
            default:
                break;
        }
    }

    typed_array(typed_array&& other)
    {
        swap(*this,other);
    }
    typed_array(uint8_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::uint8_value), size_(size)
    {
        uint8_allocator_type alloc(allocator_);
        data_.uint8_data_ = alloc.allocate(size);
    }

    typed_array(uint16_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::uint16_value), size_(size)
    {
        uint16_allocator_type alloc(allocator_);
        data_.uint16_data_ = alloc.allocate(size);
    }

    typed_array(uint32_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::uint32_value), size_(size)
    {
        uint32_allocator_type alloc(allocator_);
        data_.uint32_data_ = alloc.allocate(size);
    }

    typed_array(uint64_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::uint64_value), size_(size)
    {
        uint64_allocator_type alloc(allocator_);
        data_.uint64_data_ = alloc.allocate(size);
    }

    typed_array(int8_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::int8_value), size_(size)
    {
        int8_allocator_type alloc(allocator_);
        data_.int8_data_ = alloc.allocate(size);
    }

    typed_array(int16_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::int16_value), size_(size)
    {
        int16_allocator_type alloc(allocator_);
        data_.int16_data_ = alloc.allocate(size);
    }

    typed_array(int32_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::int32_value), size_(size)
    {
        int32_allocator_type alloc(allocator_);
        data_.int32_data_ = alloc.allocate(size);
    }

    typed_array(int64_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::int64_value), size_(size)
    {
        int64_allocator_type alloc(allocator_);
        data_.int64_data_ = alloc.allocate(size);
    }

    typed_array(half_array_arg_t, size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::half_value), size_(size)
    {
        uint16_allocator_type alloc(allocator_);
        data_.uint16_data_ = alloc.allocate(size);
    }

    typed_array(float_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::float_value), size_(size)
    {
        float_allocator_type alloc(allocator_);
        data_.float_data_ = alloc.allocate(size);
    }

    typed_array(double_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::double_value), size_(size)
    {
        double_allocator_type alloc(allocator_);
        data_.double_data_ = alloc.allocate(size);
    }

    typed_array(float128_array_arg_t,size_t size, const Allocator& allocator)
        : allocator_(allocator), type_(typed_array_type::float128_value), size_(size)
    {
        float128_allocator_type alloc(allocator_);
        data_.float128_data_ = alloc.allocate(size);
    }

    ~typed_array()
    {
        switch (type_)
        {
            case typed_array_type::uint8_value:
            {
                uint8_allocator_type alloc(allocator_);
                alloc.deallocate(data_.uint8_data_, 1);
                break;
            }
            case typed_array_type::uint16_value:
            {
                uint16_allocator_type alloc(allocator_);
                alloc.deallocate(data_.uint16_data_, 1);
                break;
            }
            case typed_array_type::uint32_value:
            {
                uint32_allocator_type alloc(allocator_);
                alloc.deallocate(data_.uint32_data_, 1);
                break;
            }
            case typed_array_type::uint64_value:
            {
                uint64_allocator_type alloc(allocator_);
                alloc.deallocate(data_.uint64_data_, 1);
                break;
            }
            case typed_array_type::int8_value:
            {
                int8_allocator_type alloc(allocator_);
                alloc.deallocate(data_.int8_data_, 1);
                break;
            }
            case typed_array_type::int16_value:
            {
                int16_allocator_type alloc(allocator_);
                alloc.deallocate(data_.int16_data_, 1);
                break;
            }
            case typed_array_type::int32_value:
            {
                int32_allocator_type alloc(allocator_);
                alloc.deallocate(data_.int32_data_, 1);
                break;
            }
            case typed_array_type::int64_value:
            {
                int64_allocator_type alloc(allocator_);
                alloc.deallocate(data_.int64_data_, 1);
                break;
            }
            case typed_array_type::float_value:
            {
                float_allocator_type alloc(allocator_);
                alloc.deallocate(data_.float_data_, 1);
                break;
            }
            case typed_array_type::double_value:
            {
                double_allocator_type alloc(allocator_);
                alloc.deallocate(data_.double_data_, 1);
                break;
            }
            case typed_array_type::float128_value:
            {
                deallocate_float128();
                break;
            }
            default:
                break;
        }
        type_ = typed_array_type();
        size_ = 0;
    }

    template <class Float128T_ = Float128T>
    typename std::enable_if<!std::is_same<Float128T_,std::nullptr_t>::value, void>::type
    allocate_float128()
    {
        float128_allocator_type alloc{ allocator_ };
        data_.float128_data_ = alloc.allocate(size_);
    }

    template <class Float128T_ = Float128T>
    typename std::enable_if<std::is_same<Float128T_,std::nullptr_t>::value, void>::type
        allocate_float128()
    {
    }

    template <class Float128T_ = Float128T>
    typename std::enable_if<!std::is_same<Float128T_,std::nullptr_t>::value, void>::type
        deallocate_float128()
    {
        float128_allocator_type alloc(allocator_);
        alloc.deallocate(data_.float128_data_, 1);
    }

    template <class Float128T_ = Float128T>
    typename std::enable_if<std::is_same<Float128T_,std::nullptr_t>::value, void>::type
        deallocate_float128()
    {
    }

    typed_array& operator=(const typed_array& other)
    {
        typed_array temp(other);
        swap(*this,temp);
        return *this;
    }

    typed_array_type type() const
    {
        return type_;
    }

    size_t size() const
    {
        return size_;
    }

    span<uint8_t> data(uint8_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint8_value);
        return span<uint8_t>(data_.uint8_data_, size_);
    }

    span<const uint8_t> data(uint8_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint8_value);
        return span<const uint8_t>(data_.uint8_data_, size_);
    }
    span<uint16_t> data(uint16_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint16_value);
        return span<uint16_t>(data_.uint16_data_, size_);
    }

    span<const uint16_t> data(uint16_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint16_value);
        return span<const uint16_t>(data_.uint16_data_, size_);
    }

    span<uint32_t> data(uint32_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint32_value);
        return span<uint32_t>(data_.uint32_data_, size_);
    }

    span<const uint32_t> data(uint32_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint32_value);
        return span<const uint32_t>(data_.uint32_data_, size_);
    }

    span<uint64_t> data(uint64_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint64_value);
        return span<uint64_t>(data_.uint64_data_, size_);
    }

    span<const uint64_t> data(uint64_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint64_value);
         span<const uint64_t>(data_.uint64_data_, size_);
    }

    span<int8_t> data(int8_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int8_value);
        return span<int8_t>(data_.int8_data_, size_);
    }

    span<const int8_t> data(int8_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int8_value);
        return span<const int8_t>(data_.int8_data_, size_);
    }

    span<int16_t> data(int16_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int16_value);
        return span<int16_t>(data_.int16_data_, size_);
    }

    span<const int16_t> data(int16_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int16_value);
        return span<const int16_t>(data_.int16_data_, size_);
    }

    span<int32_t> data(int32_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int32_value);
        return span<int32_t>(data_.int32_data_, size_);
    }

    span<const int32_t> data(int32_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int32_value);
        return span<const int32_t>(data_.int32_data_, size_);
    }

    span<int64_t> data(int64_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int64_value);
        return span<int64_t>(data_.int64_data_, size_);
    }

    span<const int64_t> data(int64_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int64_value);
        return span<const int64_t>(data_.int64_data_, size_);
    }

    span<uint16_t> data(half_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::half_value);
        return span<uint16_t>(data_.uint16_data_, size_);
    }

    span<const uint16_t> data(half_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::half_value);
        return span<const uint16_t>(data_.uint16_data_, size_);
    }

    span<float> data(float_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::float_value);
        return span<float>(data_.float_data_, size_);
    }

    span<const float> data(float_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::float_value);
        return span<const float>(data_.float_data_, size_);
    }

    span<double> data(double_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::double_value);
        return span<double>(data_.double_data_, size_);
    }

    span<const double> data(double_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::double_value);
        return span<const double>(data_.double_data_, size_);
    }

    span<Float128T> data(float128_array_arg_t)
    {
        JSONCONS_ASSERT(type_ == typed_array_type::float128_value);
        return span<Float128T>(data_.float128_data_, size_);
    }

    span<const Float128T> data(float128_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::float128_value);
        return span<const Float128T>(data_.float128_data_, size_);
    }

    friend void swap(typed_array& a, typed_array& b) noexcept
    {
        std::swap(a.data_,b.data_);
        std::swap(a.allocator_,b.allocator_);
        std::swap(a.type_,b.type_);
        std::swap(a.size_,b.size_);
    }
   
};

template <class Float128T = std::nullptr_t>
class typed_array_view
{
    typed_array_type type_;
    union 
    {
        const uint8_t* uint8_data_;
        const uint16_t* uint16_data_;
        const uint32_t* uint32_data_;
        const uint64_t* uint64_data_;
        const int8_t* int8_data_;
        const int16_t* int16_data_;
        const int32_t* int32_data_;
        const int64_t* int64_data_;
        const float* float_data_;
        const double* double_data_;
        const Float128T* float128_data_;
    } data_;
    size_t size_;
public:
    typed_array_view()
        : type_(), data_(), size_(0)
    {
    }

    typed_array_view(const typed_array_view& other)
        : type_(other.type_), data_(other.data_), size_(other.size())
    {
    }

    typed_array_view(typed_array_view&& other)
    {
        swap(*this,other);
    }

    typed_array_view(const typed_array<Float128T>& other)
        : type_(other.type_), data_(), size_(other.size())
    {
        switch (other.type_)
        {
            case typed_array_type::uint8_value:
            {
                data_.uint8_data_ = other.uint8_data_;
                break;
            }
            case typed_array_type::uint16_value:
            {
                data_.uint16_data_ = other.uint16_data_;
                break;
            }
            case typed_array_type::uint32_value:
            {
                data_.uint32_data_ = other.uint32_data_;
                break;
            }
            case typed_array_type::uint64_value:
            {
                data_.uint64_data_ = other.uint64_data_;
                break;
            }
            case typed_array_type::int8_value:
            {
                data_.int8_data_ = other.int8_data_;
                break;
            }
            case typed_array_type::int16_value:
            {
                data_.int16_data_ = other.int16_data_;
                break;
            }
            case typed_array_type::int32_value:
            {
                data_.int32_data_ = other.int32_data_;
                break;
            }
            case typed_array_type::int64_value:
            {
                data_.int64_data_ = other.int64_data_;
                break;
            }
            case typed_array_type::float_value:
            {
                data_.float_data_ = other.float_data_;
                break;
            }
            case typed_array_type::double_value:
            {
                data_.double_data_ = other.double_data_;
                break;
            }
            case typed_array_type::float128_value:
            {
                data_.float128_data_ = other.float128_data_;
                break;
            }
            default:
                break;
        }
    }

    typed_array_view(const uint8_t* data,size_t size)
        : type_(typed_array_type::uint8_value), size_(size)
    {
        data_.uint8_data_ = data;
    }

    typed_array_view(const uint16_t* data,size_t size)
        : type_(typed_array_type::uint16_value), size_(size)
    {
        data_.uint16_data_ = data;
    }

    typed_array_view(const uint32_t* data,size_t size)
        : type_(typed_array_type::uint32_value), size_(size)
    {
        data_.uint32_data_ = data;
    }

    typed_array_view(const uint64_t* data,size_t size)
        : type_(typed_array_type::uint64_value), size_(size)
    {
        data_.uint64_data_ = data;
    }

    typed_array_view(const int8_t* data,size_t size)
        : type_(typed_array_type::int8_value), size_(size)
    {
        data_.int8_data_ = data;
    }

    typed_array_view(const int16_t* data,size_t size)
        : type_(typed_array_type::int16_value), size_(size)
    {
        data_.int16_data_ = data;
    }

    typed_array_view(const int32_t* data,size_t size)
        : type_(typed_array_type::int32_value), size_(size)
    {
        data_.int32_data_ = data;
    }

    typed_array_view(const int64_t* data,size_t size)
        : type_(typed_array_type::int64_value), size_(size)
    {
        data_.int64_data_ = data;
    }

    typed_array_view(half_array_arg_t, const uint16_t* data, size_t size)
        : type_(typed_array_type::half_value), size_(size)
    {
        data_.uint16_data_ = data;
    }

    typed_array_view(const float* data,size_t size)
        : type_(typed_array_type::float_value), size_(size)
    {
        data_.float_data_ = data;
    }

    typed_array_view(const double* data,size_t size)
        : type_(typed_array_type::double_value), size_(size)
    {
        data_.double_data_ = data;
    }

    typed_array_view(const Float128T* data,size_t size)
        : type_(typed_array_type::float128_value), size_(size)
    {
        data_.float128_data_ = data;
    }

    typed_array_view& operator=(const typed_array_view& other)
    {
        typed_array_view temp(other);
        swap(*this,temp);
        return *this;
    }

    typed_array_type type() const {return type_;}

    size_t size() const
    {
        return size_;
    }

    span<const uint8_t> data(uint8_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint8_value);
        return span<const uint8_t>(data_.uint8_data_, size_);
    }

    span<const uint16_t> data(uint16_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint16_value);
        return span<const uint16_t>(data_.uint16_data_, size_);
    }

    span<const uint32_t> data(uint32_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint32_value);
        return span<const uint32_t>(data_.uint32_data_, size_);
    }

    span<const uint64_t> data(uint64_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::uint64_value);
        return span<const uint64_t>(data_.uint64_data_, size_);
    }

    span<const int8_t> data(int8_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int8_value);
        return span<const int8_t>(data_.int8_data_, size_);
    }

    span<const int16_t> data(int16_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int16_value);
        return span<const int16_t>(data_.int16_data_, size_);
    }

    span<const int32_t> data(int32_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int32_value);
        return span<const int32_t>(data_.int32_data_, size_);
    }

    span<const int64_t> data(int64_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::int64_value);
        return span<const int64_t>(data_.int64_data_, size_);
    }

    span<const uint16_t> data(half_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::half_value);
        return span<const uint16_t>(data_.uint16_data_, size_);
    }

    span<const float> data(float_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::float_value);
        return span<const float>(data_.float_data_, size_);
    }

    span<const double> data(double_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::double_value);
        return span<const double>(data_.double_data_, size_);
    }

    span<const Float128T> data(float128_array_arg_t) const
    {
        JSONCONS_ASSERT(type_ == typed_array_type::float128_value);
        return span<const Float128T>(data_.float128_data_, size_);
    }

    friend void swap(typed_array_view& a, typed_array_view& b) noexcept
    {
        std::swap(a.data_,b.data_);
        std::swap(a.type_,b.type_);
        std::swap(a.size_,b.size_);
    }
};

struct mapped_string
{
    jsoncons::cbor::detail::cbor_major_type type;
    std::string s;
    std::vector<uint8_t> bs;

    mapped_string(const std::string& s)
        : type(jsoncons::cbor::detail::cbor_major_type::text_string), s(s)
    {
    }

    mapped_string(std::string&& s)
        : type(jsoncons::cbor::detail::cbor_major_type::text_string), s(std::move(s))
    {
    }

    mapped_string(const std::vector<uint8_t>& bs)
        : type(jsoncons::cbor::detail::cbor_major_type::byte_string), bs(bs)
    {
    }

    mapped_string(std::vector<uint8_t>&& bs)
        : type(jsoncons::cbor::detail::cbor_major_type::byte_string), bs(std::move(bs))
    {
    }

    mapped_string(const mapped_string&) = default;

    mapped_string(mapped_string&&) = default;

    mapped_string& operator=(const mapped_string&) = default;

    mapped_string& operator=(mapped_string&&) = default;
};

typedef std::vector<mapped_string> stringref_map_type;

struct parse_state 
{
    parse_mode mode; 
    size_t length;
    size_t index;
    std::shared_ptr<stringref_map_type> stringref_map; 

    parse_state(parse_mode mode, size_t length)
        : mode(mode), length(length), index(0)
    {
    }

    parse_state(parse_mode mode, size_t length, std::shared_ptr<stringref_map_type> stringref_map)
        : mode(mode), length(length), index(0), stringref_map(stringref_map)
    {
    }

    parse_state(const parse_state&) = default;
    parse_state(parse_state&&) = default;
};

template <class Src,class Float128T = std::nullptr_t,class WorkAllocator=std::allocator<char>>
class basic_cbor_parser : public ser_context
{
    typedef char char_type;
    typedef std::char_traits<char> char_traits_type;
    typedef WorkAllocator work_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<char_type> char_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<uint8_t> byte_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<uint64_t> tag_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<parse_state> parse_state_allocator_type;

    typedef std::basic_string<char_type,char_traits_type,char_allocator_type> string_type;

    work_allocator_type allocator_;
    Src source_;
    bool more_;
    bool done_;
    std::basic_string<char,std::char_traits<char>,char_allocator_type> text_buffer_;
    std::vector<uint8_t,byte_allocator_type> bytes_buffer_;
    std::vector<uint64_t,tag_allocator_type> tags_; 
    std::vector<parse_state,parse_state_allocator_type> state_stack_;
    typed_array<Float128T,WorkAllocator> typed_array_;
    size_t index_;

public:
    template <class Source>
    basic_cbor_parser(Source&& source,
                      const WorkAllocator allocator=WorkAllocator())
       : allocator_(allocator),
         source_(std::forward<Source>(source)),
         more_(true), 
         done_(false),
         text_buffer_(allocator),
         bytes_buffer_(allocator),
         tags_(allocator),
         state_stack_(allocator),
         typed_array_(allocator),
         index_(0)
    {
        state_stack_.emplace_back(parse_mode::root,0);
    }

    void restart()
    {
        more_ = true;
    }

    void reset()
    {
        state_stack_.clear();
        state_stack_.emplace_back(parse_mode::root,0);
        more_ = true;
        done_ = false;
    }

    bool done() const
    {
        return done_;
    }

    bool stopped() const
    {
        return !more_;
    }

    size_t line() const override
    {
        return 0;
    }

    size_t column() const override
    {
        return source_.position();
    }

    void advance_typed_array(basic_json_content_handler<char>& handler, std::error_code&)
    {
        while (more_ && typed_array_.type() != typed_array_type())
        {
            if (index_ < typed_array_.size())
            {
                switch (typed_array_.type())
                {
                    case typed_array_type::uint8_value:
                    {
                        more_ = handler.uint64_value(typed_array_.data(uint8_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::uint16_value:
                    {
                        more_ = handler.uint64_value(typed_array_.data(uint16_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::uint32_value:
                    {
                        more_ = handler.uint64_value(typed_array_.data(uint32_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::uint64_value:
                    {
                        more_ = handler.uint64_value(typed_array_.data(uint64_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::int8_value:
                    {
                        more_ = handler.int64_value(typed_array_.data(int8_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::int16_value:
                    {
                        more_ = handler.int64_value(typed_array_.data(int16_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::int32_value:
                    {
                        more_ = handler.int64_value(typed_array_.data(int32_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::int64_value:
                    {
                        more_ = handler.int64_value(typed_array_.data(int64_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::half_value:
                    {
                        more_ = handler.half_value(typed_array_.data(half_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::float_value:
                    {
                        more_ = handler.double_value(typed_array_.data(float_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::double_value:
                    {
                        more_ = handler.double_value(typed_array_.data(double_array_arg)[index_]);
                        break;
                    }
                    case typed_array_type::float128_value:
                    {
                        break;
                    }
                    default:
                        break;
                }
                ++index_;
            }
            else
            {
                more_ = handler.end_array();
                typed_array_ = typed_array<Float128T,WorkAllocator>(allocator_);
                index_ = 0;
            }
        }
    }

    void parse(basic_json_content_handler<char>& handler, std::error_code& ec)
    {
        if (typed_array_.type() != typed_array_type())
        {
            advance_typed_array(handler, ec);
        }
        if (more_)
        {
            cbor_to_json_content_handler_adaptor<Float128T> h(handler);
            parse(h,ec);
        }
    }

    void parse(basic_cbor_content_handler<Float128T>& handler, std::error_code& ec)
    {
        while (!done_ && more_)
        {
            switch (state_stack_.back().mode)
            {
                case parse_mode::array:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        read_item(handler, ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    else
                    {
                        end_array(handler, ec);
                    }
                    break;
                }
                case parse_mode::indefinite_array:
                {
                    int c = source_.peek();
                    switch (c)
                    {
                        case Src::traits_type::eof():
                            ec = cbor_errc::unexpected_eof;
                            more_ = false;
                            return;
                        case 0xff:
                            source_.ignore(1);
                            end_array(handler, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                        default:
                            read_item(handler, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                    }
                    break;
                }
                case parse_mode::map_key:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        read_name(handler, ec);
                        if (ec)
                        {
                            return;
                        }
                        state_stack_.back().mode = parse_mode::map_value;
                    }
                    else
                    {
                        end_map(handler, ec);
                    }
                    break;
                }
                case parse_mode::map_value:
                {
                    state_stack_.back().mode = parse_mode::map_key;
                    read_item(handler, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::indefinite_map_key:
                {
                    int c = source_.peek();
                    switch (c)
                    {
                        case Src::traits_type::eof():
                            ec = cbor_errc::unexpected_eof;
                            more_ = false;
                            return;
                        case 0xff:
                            source_.ignore(1);
                            end_map(handler, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                        default:
                            read_name(handler, ec);
                            if (ec)
                            {
                                return;
                            }
                            state_stack_.back().mode = parse_mode::indefinite_map_value;
                            break;
                    }
                    break;
                }
                case parse_mode::indefinite_map_value:
                {
                    state_stack_.back().mode = parse_mode::indefinite_map_key;
                    read_item(handler, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::root:
                {
                    state_stack_.back().mode = parse_mode::before_done;
                    read_item(handler, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::before_done:
                {
                    JSONCONS_ASSERT(state_stack_.size() == 1);
                    state_stack_.clear();
                    more_ = false;
                    done_ = true;
                    handler.flush();
                    break;
                }
            }
        }
    }
private:
    void read_item(basic_cbor_content_handler<Float128T>& handler, std::error_code& ec)
    {
        read_tags(ec);
        if (ec)
        {
            return;
        }
        int c = source_.peek();
        if (c == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);

        uint8_t info = get_additional_information_value((uint8_t)c);
        switch (major_type)
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = get_uint64_value(ec);
                if (ec)
                {
                    return;
                }
                if (state_stack_.back().stringref_map && !tags_.empty() && tags_.back() == 25)
                {
                    tags_.pop_back();
                    if (val >= state_stack_.back().stringref_map->size())
                    {
                        ec = cbor_errc::stringref_too_large;
                        more_ = false;
                        return;
                    }
                    stringref_map_type::size_type index = (stringref_map_type::size_type)val;
                    if (index != val)
                    {
                        ec = cbor_errc::number_too_large;
                        more_ = false;
                        return;
                    }
                    auto& str = state_stack_.back().stringref_map->at(index);
                    switch (str.type)
                    {
                        case jsoncons::cbor::detail::cbor_major_type::text_string:
                        {
                            handle_string(handler, basic_string_view<char>(str.s.data(),str.s.length()),ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                        }
                        case jsoncons::cbor::detail::cbor_major_type::byte_string:
                        {
                            handle_byte_string(handler, byte_string_view(str.bs.data(),str.bs.size()), ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                        }
                        default:
                            JSONCONS_UNREACHABLE();
                            break;
                    }
                }
                else
                {
                    semantic_tag tag = semantic_tag::none;
                    if (!tags_.empty())
                    {
                        if (tags_.back() == 1)
                        {
                            tag = semantic_tag::timestamp;
                        }
                        tags_.clear();
                    }
                    more_ = handler.uint64_value(val, tag, *this);
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = get_int64_value(ec);
                if (ec)
                {
                    return;
                }
                semantic_tag tag = semantic_tag::none;
                if (!tags_.empty())
                {
                    if (tags_.back() == 1)
                    {
                        tag = semantic_tag::timestamp;
                    }
                    tags_.clear();
                }
                more_ = handler.int64_value(val, tag, *this);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::byte_string:
            {
                bytes_buffer_ = get_byte_string(ec);
                if (ec)
                {
                    return;
                }
                handle_byte_string(handler, byte_string_view(bytes_buffer_.data(), bytes_buffer_.size()), ec);
                if (ec)
                {
                    return;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::text_string:
            {
                text_buffer_ = get_text_string(ec);
                auto result = unicons::validate(text_buffer_.begin(),text_buffer_.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
                handle_string(handler, basic_string_view<char>(text_buffer_.data(),text_buffer_.length()),ec);
                if (ec)
                {
                    return;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                JSONCONS_UNREACHABLE();
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::simple:
            {
                switch (info)
                {
                    case 0x14:
                        more_ = handler.bool_value(false, semantic_tag::none, *this);
                        source_.ignore(1);
                        break;
                    case 0x15:
                        more_ = handler.bool_value(true, semantic_tag::none, *this);
                        source_.ignore(1);
                        break;
                    case 0x16:
                        more_ = handler.null_value(semantic_tag::none, *this);
                        source_.ignore(1);
                        break;
                    case 0x17:
                        more_ = handler.null_value(semantic_tag::undefined, *this);
                        source_.ignore(1);
                        break;
                    case 0x19: // Half-Precision Float (two-byte IEEE 754)
                    {
                        uint64_t val = get_uint64_value(ec);
                        if (ec)
                        {
                            return;
                        }
                        more_ = handler.half_value(static_cast<uint16_t>(val), semantic_tag::none, *this, ec);
                        break;
                    }
                    case 0x1a: // Single-Precision Float (four-byte IEEE 754)
                    case 0x1b: // Double-Precision Float (eight-byte IEEE 754)
                        double val = get_double(ec);
                        if (ec)
                        {
                            return;
                        }
                        semantic_tag tag = semantic_tag::none;
                        if (!tags_.empty())
                        {
                            if (tags_.back() == 1)
                            {
                                tag = semantic_tag::timestamp;
                            }
                            tags_.clear();
                        }
                        more_ = handler.double_value(val, tag, *this);
                        break;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::array:
            {
                if (!tags_.empty() && tags_.back() == 0x04)
                {
                    text_buffer_ = get_array_as_decimal_string(ec);
                    if (ec)
                    {
                        return;
                    }
                    more_ = handler.string_value(text_buffer_, semantic_tag::bigdec);
                    tags_.pop_back();
                }
                else if (!tags_.empty() && tags_.back() == 0x05)
                {
                    text_buffer_ = get_array_as_hexfloat_string(ec);
                    if (ec)
                    {
                        return;
                    }
                    more_ = handler.string_value(text_buffer_, semantic_tag::bigfloat);
                    tags_.pop_back();
                }
                else
                {
                    begin_array(handler, info, ec);
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::map:
            {
                begin_map(handler, info, ec);
                break;
            }
            default:
                break;
        }
        tags_.clear();
    }

    void begin_array(basic_cbor_content_handler<Float128T>& handler, uint8_t info, std::error_code& ec)
    {
        semantic_tag tag = semantic_tag::none;
        auto stringref_map = state_stack_.back().stringref_map;
        for (auto t : tags_)
        {
            switch (t)
            {
                case 0x05:
                    tag = semantic_tag::bigfloat;
                    break;
                case 0x100: // 256 (stringref-namespace)
                    stringref_map = std::make_shared<stringref_map_type>();
                    break;
                default:
                    break;
            }
        }
        tags_.clear();
        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                state_stack_.emplace_back(parse_mode::indefinite_array,0,stringref_map);
                more_ = handler.begin_array(tag, *this);
                source_.ignore(1);
                break;
            }
            default: // definite length
            {
                size_t len = get_definite_length(ec);
                if (ec)
                {
                    return;
                }
                state_stack_.emplace_back(parse_mode::array,len,stringref_map);
                more_ = handler.begin_array(len, tag, *this);
                break;
            }
        }
    }

    void end_array(basic_cbor_content_handler<Float128T>& handler, std::error_code&)
    {
        more_ = handler.end_array(*this);
        state_stack_.pop_back();
    }

    void begin_map(basic_cbor_content_handler<Float128T>& handler, uint8_t info, std::error_code& ec)
    {
        auto stringref_map = state_stack_.back().stringref_map;
        for (auto t : tags_)
        {
            switch (t)
            {
                case 0x100: // 256 (stringref-namespace)
                    stringref_map = std::make_shared<stringref_map_type>();
                    break;
                default:
                    break;
            }
        }
        tags_.clear();
        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length: 
            {
                state_stack_.emplace_back(parse_mode::indefinite_map_key,0,stringref_map);
                more_ = handler.begin_object(semantic_tag::none, *this);
                source_.ignore(1);
                break;
            }
            default: // definite_length
            {
                size_t len = get_definite_length(ec);
                if (ec)
                {
                    return;
                }
                state_stack_.emplace_back(parse_mode::map_key,len,stringref_map);
                more_ = handler.begin_object(len, semantic_tag::none, *this);
                break;
            }
        }
    }

    void end_map(basic_cbor_content_handler<Float128T>& handler, std::error_code&)
    {
        more_ = handler.end_object(*this);
        state_stack_.pop_back();
    }

    void read_name(basic_cbor_content_handler<Float128T>& handler, std::error_code& ec)
    {
        read_tags(ec);
        if (ec)
        {
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type;
        int c = source_.peek();
        switch (c)
        {
            case Src::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return;
            default:
                major_type = get_major_type((uint8_t)c);
                break;
        }
        switch (major_type)
        {
            case jsoncons::cbor::detail::cbor_major_type::text_string:
            {
                text_buffer_ = get_text_string(ec);
                if (ec)
                {
                    return;
                }
                auto result = unicons::validate(text_buffer_.begin(),text_buffer_.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
                more_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::byte_string:
            {
                bytes_buffer_ = get_byte_string(ec);
                if (ec)
                {
                    return;
                }
                text_buffer_.clear();
                encode_base64url(bytes_buffer_.begin(),bytes_buffer_.end(),text_buffer_);
                more_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                if (state_stack_.back().stringref_map && !tags_.empty() && tags_.back() == 25)
                {
                    uint64_t ref = get_uint64_value(ec);
                    if (ec)
                    {
                        return;
                    }
                    if (ref >= state_stack_.back().stringref_map->size())
                    {
                        ec = cbor_errc::stringref_too_large;
                        more_ = false;
                        return;
                    }

                    stringref_map_type::size_type index = (stringref_map_type::size_type)ref;
                    if (index != ref)
                    {
                        ec = cbor_errc::number_too_large;
                        more_ = false;
                        return;
                    }
                    auto& val = state_stack_.back().stringref_map->at(index);
                    switch (val.type)
                    {
                        case jsoncons::cbor::detail::cbor_major_type::text_string:
                        {
                            more_ = handler.name(basic_string_view<char>(val.s.data(),val.s.length()), *this);
                            break;
                        }
                        case jsoncons::cbor::detail::cbor_major_type::byte_string:
                        {
                            text_buffer_.clear();
                            encode_base64url(val.bs.begin(),val.bs.end(),text_buffer_);
                            more_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
                            break;
                        }
                        default:
                            JSONCONS_UNREACHABLE();
                            break;
                    }
                    tags_.pop_back();
                    break;
                }
            }
                JSONCONS_FALLTHROUGH;
            default:
            {
                text_buffer_.clear();
                json_string_encoder encoder(text_buffer_);
                basic_cbor_parser<Src,Float128T> reader(std::move(source_));

                reader.parse(encoder, ec);
                source_ = std::move(reader.source_);
                auto result = unicons::validate(text_buffer_.begin(),text_buffer_.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
                more_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
            }
        }
    }

    string_type get_text_string(std::error_code& ec)
    {
        string_type s;

        jsoncons::cbor::detail::cbor_major_type major_type;
        uint8_t info;
        int c = source_.peek();
        switch (c)
        {
            case Src::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return s;
            default:
                major_type = get_major_type((uint8_t)c);
                info = get_additional_information_value((uint8_t)c);
                break;
        }
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::text_string);
        auto func = [&](size_t length, std::error_code& ec)
        {
            s.reserve(s.size()+length);
            source_.read(std::back_inserter(s), length);
            if (source_.eof())
            {
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return;
            }
        };
        iterate_string_chunks( func, ec);
        if (state_stack_.back().stringref_map && 
            info != jsoncons::cbor::detail::additional_info::indefinite_length &&
            s.length() >= jsoncons::cbor::detail::min_length_for_stringref(state_stack_.back().stringref_map->size()))
        {
            state_stack_.back().stringref_map->emplace_back(s);
        }
        
        return s;
    }

    size_t get_definite_length(std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(source_.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return 0;
        }
        switch (get_major_type((uint8_t)source_.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::byte_string:
            case jsoncons::cbor::detail::cbor_major_type::text_string:
            case jsoncons::cbor::detail::cbor_major_type::array:
            case jsoncons::cbor::detail::cbor_major_type::map:
                break;
            default:
                return 0;
        }

        uint64_t u = get_uint64_value(ec);
        size_t len = (size_t)u;
        if (len != u)
        {
            ec = cbor_errc::number_too_large;
            more_ = false;
        }
        return len;
    }

    std::vector<uint8_t> get_byte_string(std::error_code& ec)
    {
        std::vector<uint8_t> v;

        jsoncons::cbor::detail::cbor_major_type major_type;
        uint8_t info;
        int c = source_.peek();
        switch (c)
        {
            case Src::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return v;
            default:
                major_type = get_major_type((uint8_t)c);
                info = get_additional_information_value((uint8_t)c);
                break;
        }
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::byte_string);
        auto func = [&](size_t length, std::error_code& ec)
        {
            v.reserve(v.size()+length);
            source_.read(std::back_inserter(v), length);
            if (source_.eof())
            {
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return;
            }
        };
        iterate_string_chunks( func, ec);
        if (state_stack_.back().stringref_map && 
            info != jsoncons::cbor::detail::additional_info::indefinite_length &&
            v.size() >= jsoncons::cbor::detail::min_length_for_stringref(state_stack_.back().stringref_map->size()))
        {
            state_stack_.back().stringref_map->emplace_back(v);
        }
        return v;
    }

    template <class Function>
    void iterate_string_chunks(Function func, std::error_code& ec)
    {
        int c = source_.peek();
        if (c == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }

        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::text_string || major_type == jsoncons::cbor::detail::cbor_major_type::byte_string);
        uint8_t info = get_additional_information_value((uint8_t)c);

        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                source_.ignore(1);
                bool done = false;
                while (!done)
                {
                    int test = source_.peek();
                    switch (test)
                    {
                        case Src::traits_type::eof():
                            ec = cbor_errc::unexpected_eof;
                            more_ = false;
                            return;
                        case 0xff:
                            done = true;
                            break;
                        default:
                            iterate_string_chunks( func, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                    }
                }
                source_.ignore(1);
                break;
            }
            default: // definite length
            {
                size_t length = get_definite_length(ec);
                if (ec)
                {
                    return;
                }
                func(length, ec);
                if (ec)
                {
                    return;
                }
                break;
            }
        }
    }

    uint64_t get_uint64_value(std::error_code& ec)
    {
        uint64_t val = 0;
        if (JSONCONS_UNLIKELY(source_.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return val;
        }
        const uint8_t* endp = nullptr;

        uint8_t type{};
        if (source_.get(type) == 0)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return 0;
        }
        uint8_t info = get_additional_information_value(type);
        switch (info)
        {
            case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
            {
                val = info;
                break;
            }

            case 0x18: // Unsigned integer (one-byte uint8_t follows)
            {
                uint8_t c{};
                source_.get(c);
                if (source_.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return val;
                }
                val = c;
                break;
            }

            case 0x19: // Unsigned integer (two-byte uint16_t follows)
            {
                uint8_t buf[sizeof(uint16_t)];
                source_.read(buf, sizeof(uint16_t));
                val = jsoncons::detail::big_to_native<uint16_t>(buf,buf+sizeof(buf),&endp);
                break;
            }

            case 0x1a: // Unsigned integer (four-byte uint32_t follows)
            {
                uint8_t buf[sizeof(uint32_t)];
                source_.read(buf, sizeof(uint32_t));
                val = jsoncons::detail::big_to_native<uint32_t>(buf,buf+sizeof(buf),&endp);
                break;
            }

            case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
            {
                uint8_t buf[sizeof(uint64_t)];
                source_.read(buf, sizeof(uint64_t));
                val = jsoncons::detail::big_to_native<uint64_t>(buf,buf+sizeof(buf),&endp);
                break;
            }
            default:
                break;
        }
        return val;
    }

    int64_t get_int64_value(std::error_code& ec)
    {
        int64_t val = 0;
        if (JSONCONS_UNLIKELY(source_.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return val;
        }
        const uint8_t* endp = nullptr;

        uint8_t info = get_additional_information_value((uint8_t)source_.peek());
        switch (get_major_type((uint8_t)source_.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
                source_.ignore(1);
                switch (info)
                {
                    case JSONCONS_CBOR_0x00_0x17: // 0x00..0x17 (0..23)
                    {
                        val = static_cast<int8_t>(- 1 - info);
                        break;
                    }
                    case 0x18: // Negative integer (one-byte uint8_t follows)
                        {
                            uint8_t c{};
                            source_.get(c);
                            if (source_.eof())
                            {
                                ec = cbor_errc::unexpected_eof;
                                more_ = false;
                                return val;
                            }
                            val = static_cast<int64_t>(-1)- c;
                            break;
                        }

                    case 0x19: // Negative integer -1-n (two-byte uint16_t follows)
                        {
                            uint8_t buf[sizeof(uint16_t)];
                            if (source_.read(buf, sizeof(uint16_t)) != sizeof(uint16_t))
                            {
                                return val;
                            }
                            auto x = jsoncons::detail::big_to_native<uint16_t>(buf,buf+sizeof(buf),&endp);
                            val = static_cast<int64_t>(-1)- x;
                            break;
                        }

                    case 0x1a: // Negative integer -1-n (four-byte uint32_t follows)
                        {
                            uint8_t buf[sizeof(uint32_t)];
                            if (source_.read(buf, sizeof(uint32_t)) != sizeof(uint32_t))
                            {
                                return val;
                            }
                            auto x = jsoncons::detail::big_to_native<uint32_t>(buf,buf+sizeof(buf),&endp);
                            val = static_cast<int64_t>(-1)- x;
                            break;
                        }

                    case 0x1b: // Negative integer -1-n (eight-byte uint64_t follows)
                        {
                            uint8_t buf[sizeof(uint64_t)];
                            if (source_.read(buf, sizeof(uint64_t)) != sizeof(uint64_t))
                            {
                                return val;
                            }
                            auto x = jsoncons::detail::big_to_native<uint64_t>(buf,buf+sizeof(buf),&endp);
                            val = static_cast<int64_t>(-1)- static_cast<int64_t>(x);
                            break;
                        }
                }
                break;

                case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
                {
                    uint64_t x = get_uint64_value(ec);
                    if (ec)
                    {
                        return 0;
                    }
                    if (x <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()))
                    {
                        val = x;
                    }
                    else
                    {
                        // error;
                    }
                    
                    break;
                }
                break;
            default:
                break;
        }

        return val;
    }

    double get_double(std::error_code& ec)
    {
        double val = 0;
        if (JSONCONS_UNLIKELY(source_.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return val;
        }
        const uint8_t* endp = nullptr;

        uint8_t type{};
        if (source_.get(type) == 0)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return 0;
        }
        uint8_t info = get_additional_information_value(type);
        switch (info)
        {
        case 0x1a: // Single-Precision Float (four-byte IEEE 754)
            {
                uint8_t buf[sizeof(float)];
                source_.read(buf, sizeof(float));
                if (source_.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return 0;
                }
                val = jsoncons::detail::big_to_native<float>(buf,buf+sizeof(buf),&endp);
                break;
            }

        case 0x1b: //  Double-Precision Float (eight-byte IEEE 754)
            {
                uint8_t buf[sizeof(double)];
                source_.read(buf, sizeof(double));
                if (source_.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return 0;
                }
                val = jsoncons::detail::big_to_native<double>(buf,buf+sizeof(buf),&endp);
                break;
            }
            default:
                break;
        }
        
        return val;
    }

    string_type get_array_as_decimal_string(std::error_code& ec)
    {
        string_type s;

        int c;
        if ((c=source_.get()) == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return s;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        uint8_t info = get_additional_information_value((uint8_t)c);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::array);
        JSONCONS_ASSERT(info == 2);

        if ((c=source_.peek()) == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return s;
        }
        int64_t exponent = 0;
        switch (get_major_type((uint8_t)c))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                exponent = get_uint64_value(ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                exponent = get_int64_value(ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_bigdec;
                more_ = false;
                return s;
            }
        }

        switch (get_major_type((uint8_t)source_.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = get_uint64_value(ec);
                if (ec)
                {
                    return s;
                }
                jsoncons::detail::print_uinteger(val, s);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = get_int64_value(ec);
                if (ec)
                {
                    return s;
                }
                jsoncons::detail::print_integer(val, s);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                if ((c=source_.get()) == Src::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return s;
                }
                uint8_t tag = get_additional_information_value((uint8_t)c);
                if ((c=source_.peek()) == Src::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return s;
                }

                if (get_major_type((uint8_t)c) == jsoncons::cbor::detail::cbor_major_type::byte_string)
                {
                    std::vector<uint8_t> v = get_byte_string(ec);
                    if (ec)
                    {
                        return s;
                    }
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
            {
                ec = cbor_errc::invalid_bigdec;
                more_ = false;
                return s;
            }
        }

        string_type result;
        if (s.size() > 0)
        {
            if (s[0] == '-')
            {
                result.push_back('-');
                jsoncons::detail::prettify_string(s.c_str()+1, s.size()-1, (int)exponent, -4, 17, result);
            }
            else
            {
                jsoncons::detail::prettify_string(s.c_str(), s.size(), (int)exponent, -4, 17, result);
            }
        }
        return result;
    }

    string_type get_array_as_hexfloat_string(std::error_code& ec)
    {
        string_type s;

        int c;
        if ((c=source_.get()) == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return s;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        uint8_t info = get_additional_information_value((uint8_t)c);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::array);
        JSONCONS_ASSERT(info == 2);

        if ((c=source_.peek()) == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return s;
        }
        int64_t exponent = 0;
        switch (get_major_type((uint8_t)c))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                exponent = get_uint64_value(ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                exponent = get_int64_value(ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_bigfloat;
                more_ = false;
                return s;
            }
        }

        switch (get_major_type((uint8_t)source_.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = get_uint64_value(ec);
                if (ec)
                {
                    return s;
                }
                s.push_back('0');
                s.push_back('x');
                jsoncons::detail::uinteger_to_hex_string(val, s);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = get_int64_value(ec);
                if (ec)
                {
                    return s;
                }
                s.push_back('-');
                s.push_back('0');
                s.push_back('x');
                jsoncons::detail::uinteger_to_hex_string(static_cast<uint64_t>(-val), s);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                if ((c=source_.get()) == Src::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return s;
                }
                uint8_t tag = get_additional_information_value((uint8_t)c);
                if ((c=source_.peek()) == Src::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return s;
                }

                if (get_major_type((uint8_t)c) == jsoncons::cbor::detail::cbor_major_type::byte_string)
                {
                    std::vector<uint8_t> v = get_byte_string(ec);
                    if (ec)
                    {
                        return s;
                    }
                    if (tag == 2)
                    {
                        s.push_back('-');
                        s.push_back('0');
                        s.push_back('x');
                        bignum n(1, v.data(), v.size());
                        n.dump_hex_string(s);
                    }
                    else if (tag == 3)
                    {
                        s.push_back('-');
                        s.push_back('0');
                        bignum n(-1, v.data(), v.size());
                        n.dump_hex_string(s);
                        s[2] = 'x';
                    }
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_bigfloat;
                more_ = false;
                return s;
            }
        }

        s.push_back('p');
        if (exponent >=0)
        {
            jsoncons::detail::uinteger_to_hex_string(static_cast<uint64_t>(exponent), s);
        }
        else
        {
            s.push_back('-');
            jsoncons::detail::uinteger_to_hex_string(static_cast<uint64_t>(-exponent), s);
        }
        return s;
    }

    static jsoncons::cbor::detail::cbor_major_type get_major_type(uint8_t type)
    {
        static const uint8_t major_type_shift = 0x05;
        uint8_t value = type >> major_type_shift;
        return static_cast<jsoncons::cbor::detail::cbor_major_type>(value);
    }

    static uint8_t get_additional_information_value(uint8_t type)
    {
        static const uint8_t additional_information_mask = (1U << 5) - 1;
        uint8_t value = type & additional_information_mask;
        return value;
    }

    void read_tags(std::error_code& ec)
    {
        int c = source_.peek();
        if (c == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        while (major_type == jsoncons::cbor::detail::cbor_major_type::semantic_tag)
        {
            uint64_t val = get_uint64_value(ec);
            if (ec)
            {
                return;
            } 
            tags_.push_back(val);
            c = source_.peek();
            if (c == Src::traits_type::eof())
            {
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return;
            }
            major_type = get_major_type((uint8_t)c);
        }
    }

    void handle_string(basic_cbor_content_handler<Float128T>& handler, const basic_string_view<char>& v, std::error_code&)
    {
        semantic_tag tag = semantic_tag::none;
        if (!tags_.empty())
        {
            switch (tags_.back())
            {
                case 0:
                    tag = semantic_tag::datetime;
                    break;
                case 32:
                    tag = semantic_tag::uri;
                    break;
                case 33:
                    tag = semantic_tag::base64url;
                    break;
                case 34:
                    tag = semantic_tag::base64;
                    break;
                default:
                    break;
            }
            tags_.clear();
        }
        more_ = handler.string_value(v, tag, *this);
    }

    void handle_byte_string(basic_cbor_content_handler<Float128T>& handler, const byte_string_view& v, std::error_code& ec)
    {
        if (!tags_.empty())
        {
            switch (tags_.back())
            {
                case 0x2:
                {
                    bignum n(1, v.data(), v.size());
                    text_buffer_.clear();
                    n.dump(text_buffer_);
                    more_ = handler.string_value(text_buffer_, semantic_tag::bigint, *this);
                    break;
                }
                case 0x3:
                {
                    bignum n(-1, v.data(), v.size());
                    text_buffer_.clear();
                    n.dump(text_buffer_);
                    more_ = handler.string_value(text_buffer_, semantic_tag::bigint, *this);
                    break;
                }
                case 0x15:
                {
                    more_ = handler.byte_string_value(v, semantic_tag::base64url, *this);
                    break;
                }
                case 0x16:
                {
                    more_ = handler.byte_string_value(v, semantic_tag::base64, *this);
                    break;
                }
                case 0x17:
                {
                    more_ = handler.byte_string_value(v, semantic_tag::base16, *this);
                    break;
                }
                case 0x40:
                {
                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size();
                    typed_array_ = typed_array<Float128T,WorkAllocator>(uint8_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; ++p, ++i)
                    {
                        typed_array_.data(uint8_array_arg)[i] = *p;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint8_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x44:
                {
                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size();
                    typed_array_ = typed_array<Float128T,WorkAllocator>(uint8_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; ++p, ++i)
                    {
                        typed_array_.data(uint8_array_arg)[i] = *p;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint8_array_arg), semantic_tag::clamped, *this);
                    break;
                }
                case 0x41:
                case 0x45:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<Float128T,WorkAllocator>(uint16_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        uint16_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<uint16_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<uint16_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(uint16_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint16_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x42:
                case 0x46:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<Float128T,WorkAllocator>(uint32_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        uint32_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<uint32_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<uint32_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(uint32_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint32_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x43:
                case 0x47:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<Float128T,WorkAllocator>(uint64_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        uint64_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<uint64_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<uint64_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(uint64_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint64_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x48:
                {
                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size();
                    typed_array_ = typed_array<Float128T,WorkAllocator>(int8_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; ++p, ++i)
                    {
                        typed_array_.data(int8_array_arg)[i] = (int8_t)*p;
                    }
                    more_ = handler.typed_array(typed_array_.data(int8_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x49:
                case 0x4d:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<Float128T,WorkAllocator>(int16_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        int16_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<int16_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<int16_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(int16_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(int16_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x4a:
                case 0x4e:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<Float128T,WorkAllocator>(int32_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        int32_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<int32_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<int32_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(int32_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(int32_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x4b:
                case 0x4f:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<Float128T,WorkAllocator>(int64_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        int64_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<int64_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<int64_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(int64_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(int64_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x50:
                case 0x54:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<Float128T,WorkAllocator>(half_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        uint16_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<uint16_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<uint16_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(half_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(half_arg, typed_array_.data(half_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x51:
                case 0x55:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<Float128T,WorkAllocator>(float_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        float val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<float>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<float>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(float_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(float_array_arg), semantic_tag::none, *this);
                    break;
                }
                case 0x52:
                case 0x56:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<Float128T,WorkAllocator>(double_array_arg,size,allocator_);
                    for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        double val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<double>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<double>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(double_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(double_array_arg), semantic_tag::none, *this);
                    break;
                }

                case 0x53:
                case 0x57:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    handle_float128(handler,v,tag,ec);
                    break;
                }

                default:
                    more_ = handler.byte_string_value(v, semantic_tag::none, *this);
                    break;
            }
            tags_.clear();
        }
        else
        {
            more_ = handler.byte_string_value(v, semantic_tag::none, *this);
        }
    }

    template <class Float128T_ = Float128T>
    typename std::enable_if<std::is_same<Float128T_,std::nullptr_t>::value,void>::type
    handle_float128(basic_cbor_content_handler<Float128T>& handler, const byte_string_view&, const uint8_t, std::error_code&)
    {
        more_ = handler.begin_array(semantic_tag::none, *this);
        more_ = handler.end_array(*this);
    }

    template <class Float128T_ = Float128T>
    typename std::enable_if<!std::is_same<Float128T_,std::nullptr_t>::value,void>::type
    handle_float128(basic_cbor_content_handler<Float128T>& handler, const byte_string_view& v, const uint8_t tag, std::error_code&)
    {
        const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
        const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
        const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

        const size_t bytes_per_elem = size_t(1) << (f + ll);
        if (bytes_per_elem == sizeof(Float128T))
        {

            const uint8_t* p = v.data();
            const uint8_t* last = v.data() + v.size();

            size_t size = v.size()/bytes_per_elem;
            typed_array_ = typed_array<Float128T,WorkAllocator>(float128_array_arg,size,allocator_);
            for (size_t i = 0; p < last; p += bytes_per_elem, ++i)
            {
                const uint8_t* endp = nullptr;
                Float128T val{ 0 };
                switch (e)
                {
                    case 0: val = jsoncons::detail::big_to_native<Float128T>(p,p+bytes_per_elem,&endp);
                        //std::cout << "big_to_native: " << val << "\n";
                        break;
                    case 1: val = jsoncons::detail::little_to_native<Float128T>(p,p+bytes_per_elem,&endp);
                        //std::cout << "little_to_native: " << val << "\n";
                        break;
                    default: break;
                }
                typed_array_.data(float128_array_arg)[i] = val;
            }
            more_ = handler.typed_array(typed_array_.data(float128_array_arg), semantic_tag::none, *this);
        }
    }

};

}}

#endif
