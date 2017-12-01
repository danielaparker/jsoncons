// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_HPP
#define JSONCONS_JSON_HPP

#include <limits>
#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <memory>
#include <typeinfo>
#include <cstring>
#include <jsoncons/version.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/json_structures.hpp>
#include <jsoncons/json_output_handler.hpp>
#include <jsoncons/serialization_options.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/json_error_category.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace jsoncons {

struct sorted_policy
{
    static const bool preserve_order = false;

    template <class T,class Allocator>
    using object_storage = std::vector<T,Allocator>;

    template <class T,class Allocator>
    using array_storage = std::vector<T,Allocator>;

    template <class CharT, class CharTraits, class Allocator>
    using key_storage = std::basic_string<CharT, CharTraits,Allocator>;

    template <class CharT, class CharTraits, class Allocator>
    using string_storage = std::basic_string<CharT, CharTraits,Allocator>;

    typedef default_parse_error_handler parse_error_handler_type;
};

struct preserve_order_policy : public sorted_policy
{
    static const bool preserve_order = true;
};

template <typename IteratorT>
class range 
{
    IteratorT first_;
    IteratorT last_;
public:
    range(const IteratorT& first, const IteratorT& last)
        : first_(first), last_(last)
    {
    }

public:
    IteratorT begin()
    {
        return first_;
    }
    IteratorT end()
    {
        return last_;
    }
};

enum class json_type_tag : uint8_t 
{
    null_t = 0,
    empty_object_t,
    bool_t,
    integer_t,
    uinteger_t,
    double_t,
    small_string_t,
    string_t,
    byte_string_t,
    array_t,
    object_t
};
                        
template <class CharT, 
          class ImplementationPolicy = sorted_policy, 
          class Allocator = std::allocator<CharT>>
class basic_json
{
public:

    typedef Allocator allocator_type;

    typedef ImplementationPolicy implementation_policy;

    typedef typename ImplementationPolicy::parse_error_handler_type parse_error_handler_type;

    typedef CharT char_type;
    typedef typename std::char_traits<char_type> char_traits_type;

#if !defined(JSONCONS_HAS_STRING_VIEW)
    typedef Basic_string_view_<char_type,char_traits_type> string_view_type;
#else
    typedef std::basic_string_view<char_type,char_traits_type> string_view_type;
#endif
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<char_type> char_allocator_type;
    using string_storage_type = typename implementation_policy::template string_storage<CharT,char_traits_type,char_allocator_type>;
    using key_storage_type = typename implementation_policy::template key_storage<CharT,char_traits_type,char_allocator_type>;

    // string_type is for interface only, not storage 
    typedef std::basic_string<CharT,char_traits_type,char_allocator_type> string_type;

    typedef basic_json<CharT,ImplementationPolicy,Allocator> value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef key_value_pair<key_storage_type,value_type> key_value_pair_type;

#if !defined(JSONCONS_NO_DEPRECATED)
    typedef value_type json_type;
    typedef key_value_pair_type kvp_type;
    typedef key_value_pair_type member_type;
    typedef jsoncons::null_type null_type;
#endif

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<basic_json> val_allocator_type;
    using array_storage_type = typename implementation_policy::template array_storage<basic_json, val_allocator_type>;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<uint8_t> byte_allocator_type;
    using byte_string_storage_type = typename implementation_policy::template array_storage<uint8_t, byte_allocator_type>;

    typedef json_array<basic_json> array;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<key_value_pair_type> kvp_allocator_type;

    using object_storage_type = typename implementation_policy::template object_storage<key_value_pair_type , kvp_allocator_type>;
    typedef json_object<key_storage_type,basic_json,implementation_policy::preserve_order> object;

    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<array> array_allocator;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<object> object_allocator;

    typedef typename object::iterator object_iterator;
    typedef typename object::const_iterator const_object_iterator;
    typedef typename array::iterator array_iterator;
    typedef typename array::const_iterator const_array_iterator;

    struct variant
    {
        struct base_data
        {
            json_type_tag type_id_;

            explicit base_data(json_type_tag id)
                : type_id_(id)
            {}
        };

        class null_data : public base_data
        {
        public:
            explicit null_data()
                : base_data(json_type_tag::null_t)
            {
            }
        };

        class empty_object_data : public base_data
        {
        public:
            explicit empty_object_data()
                : base_data(json_type_tag::empty_object_t)
            {
            }
        };

        class bool_data : public base_data
        {
            bool val_;
        public:
            explicit bool_data(bool val)
                : base_data(json_type_tag::bool_t),val_(val)
            {
            }

            explicit bool_data(const bool_data& val)
                : base_data(json_type_tag::bool_t),val_(val.val_)
            {
            }

            bool value() const
            {
                return val_;
            }

        };

        class integer_data : public base_data
        {
            int64_t val_;
        public:
            explicit integer_data(int64_t val)
                : base_data(json_type_tag::integer_t),val_(val)
            {
            }

            explicit integer_data(const integer_data& val)
                : base_data(json_type_tag::integer_t),val_(val.val_)
            {
            }

            int64_t value() const
            {
                return val_;
            }
        };

        class uinteger_data : public base_data
        {
            uint64_t val_;
        public:
            explicit uinteger_data(uint64_t val)
                : base_data(json_type_tag::uinteger_t),val_(val)
            {
            }

            explicit uinteger_data(const uinteger_data& val)
                : base_data(json_type_tag::uinteger_t),val_(val.val_)
            {
            }

            uint64_t value() const
            {
                return val_;
            }
        };

        class double_data : public base_data
        {
            uint8_t precision_;
            double val_;
        public:
            explicit double_data(double val, uint8_t precision)
                : base_data(json_type_tag::double_t), 
                  precision_(precision), 
                  val_(val)
            {
            }

            explicit double_data(const double_data& val)
                : base_data(json_type_tag::double_t),
                  precision_(val.precision_), 
                  val_(val.val_)
            {
            }

            double value() const
            {
                return val_;
            }

            uint8_t precision() const
            {
                return precision_;
            }
        };

        class small_string_data : public base_data
        {
            static const size_t capacity = 14/sizeof(char_type);
            uint8_t length_;
            char_type data_[capacity];
        public:
            static const size_t max_length = (14 / sizeof(char_type)) - 1;

            explicit small_string_data(const char_type* p, uint8_t length)
                : base_data(json_type_tag::small_string_t), length_(length)
            {
                JSONCONS_ASSERT(length <= max_length);
                std::memcpy(data_,p,length*sizeof(char_type));
                data_[length] = 0;
            }

            explicit small_string_data(const small_string_data& val)
                : base_data(json_type_tag::small_string_t), length_(val.length_)
            {
                std::memcpy(data_,val.data_,val.length_*sizeof(char_type));
                data_[length_] = 0;
            }

            uint8_t length() const
            {
                return length_;
            }

            const char_type* data() const
            {
                return data_;
            }

            const char_type* c_str() const
            {
                return data_;
            }
        };

        // string_data
        class string_data : public base_data
        {
            typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<string_storage_type> string_holder_allocator_type;
            typedef typename std::allocator_traits<string_holder_allocator_type>::pointer pointer;

            pointer ptr_ = nullptr;

            template <typename... Args>
            void create(string_holder_allocator_type allocator, Args&& ... args)
            {
                JSONCONS_ASSERT(ptr_ == nullptr);
                typename std::allocator_traits<Allocator>:: template rebind_alloc<string_storage_type> alloc(allocator);
                ptr_ = alloc.allocate(1);
                try
                {
                    std::allocator_traits<string_holder_allocator_type>:: template rebind_traits<string_storage_type>::construct(alloc, to_plain_pointer(ptr_), std::forward<Args>(args)...);
                }
                catch (...)
                {
                    alloc.deallocate(ptr_,1);
                    throw;
                }
            }
        public:
            explicit string_data(const string_data& val)
                : base_data(json_type_tag::string_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            explicit string_data(string_data&& val)
                : base_data(json_type_tag::string_t), ptr_(nullptr)
            {
                std::swap(val.ptr_,ptr_);
            }

            explicit string_data(const string_data& val, const Allocator& a)
                : base_data(json_type_tag::string_t)
            {
                create(string_holder_allocator_type(a), *(val.ptr_), a);
            }

            explicit string_data(const char_type* data, size_t length, const Allocator& a)
                : base_data(json_type_tag::string_t)
            {
                create(string_holder_allocator_type(a), data, length, a);
            }

            ~string_data()
            {
                if (ptr_ != nullptr)
                {
                    typename std::allocator_traits<string_holder_allocator_type>:: template rebind_alloc<string_storage_type> alloc(ptr_->get_allocator());
                    std::allocator_traits<string_holder_allocator_type>:: template rebind_traits<string_storage_type>::destroy(alloc, to_plain_pointer(ptr_));
                    alloc.deallocate(ptr_,1);
                }
            }

            void swap(string_data& val)
            {
                std::swap(val.ptr_,ptr_);
            }

            const char_type* data() const
            {
                return ptr_->data();
            }

            const char_type* c_str() const
            {
                return ptr_->c_str();
            }

            size_t length() const
            {
                return ptr_->length();
            }

            allocator_type get_allocator() const
            {
                return ptr_->get_allocator();
            }
        };

        // byte_string_data
        class byte_string_data: public base_data
        {
            typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<byte_string_storage_type> string_holder_allocator_type;
            typedef typename std::allocator_traits<string_holder_allocator_type>::pointer pointer;

            pointer ptr_ = nullptr;

            template <typename... Args>
            void create(string_holder_allocator_type allocator, Args&& ... args)
            {
                JSONCONS_ASSERT(ptr_ == nullptr);
                typename std::allocator_traits<Allocator>:: template rebind_alloc<byte_string_storage_type> alloc(allocator);
                ptr_ = alloc.allocate(1);
                try
                {
                    std::allocator_traits<string_holder_allocator_type>:: template rebind_traits<byte_string_storage_type>::construct(alloc, to_plain_pointer(ptr_), std::forward<Args>(args)...);
                }
                catch (...)
                {
                    alloc.deallocate(ptr_,1);
                    throw;
                }
            }
        public:
            explicit byte_string_data(const byte_string_data& val)
                : base_data(json_type_tag::byte_string_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            explicit byte_string_data(byte_string_data&& val)
                : base_data(json_type_tag::byte_string_t), ptr_(nullptr)
            {
                std::swap(val.ptr_,ptr_);
            }

            explicit byte_string_data(const byte_string_data& val, const Allocator& a)
                : base_data(json_type_tag::byte_string_t)
            {
                create(string_holder_allocator_type(a), *(val.ptr_), a);
            }

            explicit byte_string_data(const uint8_t* data, size_t length, const Allocator& a)
                : base_data(json_type_tag::byte_string_t)
            {
                create(string_holder_allocator_type(a), data, data+length, a);
            }

            ~byte_string_data()
            {
                if (ptr_ != nullptr)
                {
                    typename std::allocator_traits<string_holder_allocator_type>:: template rebind_alloc<byte_string_storage_type> alloc(ptr_->get_allocator());
                    std::allocator_traits<string_holder_allocator_type>:: template rebind_traits<byte_string_storage_type>::destroy(alloc, to_plain_pointer(ptr_));
                    alloc.deallocate(ptr_,1);
                }
            }

            void swap(byte_string_data& val)
            {
                std::swap(val.ptr_,ptr_);
            }

            const uint8_t* data() const
            {
                return ptr_->data();
            }

            size_t length() const
            {
                return ptr_->size();
            }

            allocator_type get_allocator() const
            {
                return ptr_->get_allocator();
            }
        };

        // array_data
        class array_data : public base_data
        {
            typedef typename std::allocator_traits<array_allocator>::pointer pointer;
            pointer ptr_ = nullptr;

            template <typename... Args>
            void create(array_allocator allocator, Args&& ... args)
            {
                JSONCONS_ASSERT(ptr_ == nullptr);
                typename std::allocator_traits<Allocator>:: template rebind_alloc<array> alloc(allocator);
                ptr_ = alloc.allocate(1);
                try
                {
                    std::allocator_traits<array_allocator>:: template rebind_traits<array>::construct(alloc, to_plain_pointer(ptr_), std::forward<Args>(args)...);
                }
                catch (...)
                {
                    alloc.deallocate(ptr_,1);
                    throw;
                }
            }
        public:
            explicit array_data(const array& val)
                : base_data(json_type_tag::array_t)
            {
                create(val.get_allocator(), val);
            }

            explicit array_data(const array& val, const Allocator& a)
                : base_data(json_type_tag::array_t)
            {
                create(array_allocator(a), val, a);
            }

            explicit array_data(const array_data& val)
                : base_data(json_type_tag::array_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            explicit array_data(array_data&& val)
                : base_data(json_type_tag::array_t), ptr_(nullptr)
            {
                std::swap(val.ptr_, ptr_);
            }

            explicit array_data(const array_data& val, const Allocator& a)
                : base_data(json_type_tag::array_t)
            {
                create(array_allocator(a), *(val.ptr_), a);
            }
            ~array_data()
            {
                if (ptr_ != nullptr)
                {
                    typename std::allocator_traits<array_allocator>:: template rebind_alloc<array> alloc(ptr_->get_allocator());
                    std::allocator_traits<array_allocator>:: template rebind_traits<array>::destroy(alloc, to_plain_pointer(ptr_));
                    alloc.deallocate(ptr_,1);
                }
            }

            allocator_type get_allocator() const
            {
                return ptr_->get_allocator();
            }

            void swap(array_data& val)
            {
                std::swap(val.ptr_,ptr_);
            }

            array& value()
            {
                return *ptr_;
            }

            const array& value() const
            {
                return *ptr_;
            }
        };

        // object_data
        class object_data : public base_data
        {
            typedef typename std::allocator_traits<object_allocator>::pointer pointer;
            pointer ptr_ = nullptr;

            template <typename... Args>
            void create(Allocator allocator, Args&& ... args)
            {
                JSONCONS_ASSERT(ptr_ == nullptr);
                typename std::allocator_traits<object_allocator>:: template rebind_alloc<object> alloc(allocator);
                ptr_ = alloc.allocate(1);
                try
                {
                    std::allocator_traits<object_allocator>:: template rebind_traits<object>::construct(alloc, to_plain_pointer(ptr_), std::forward<Args>(args)...);
                }
                catch (...)
                {
                    alloc.deallocate(ptr_,1);
                    throw;
                }
            }
        public:
            explicit object_data(const Allocator& a)
                : base_data(json_type_tag::object_t)
            {
                create(a,a);
            }

            explicit object_data(const object& val)
                : base_data(json_type_tag::object_t)
            {
                create(val.get_allocator(), val);
            }

            explicit object_data(const object& val, const Allocator& a)
                : base_data(json_type_tag::object_t)
            {
                create(object_allocator(a), val, a);
            }

            explicit object_data(const object_data& val)
                : base_data(json_type_tag::object_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            explicit object_data(object_data&& val)
                : base_data(json_type_tag::object_t), ptr_(nullptr)
            {
                std::swap(val.ptr_,ptr_);
            }

            explicit object_data(const object_data& val, const Allocator& a)
                : base_data(json_type_tag::object_t)
            {
                create(object_allocator(a), *(val.ptr_), a);
            }

            ~object_data()
            {
                if (ptr_ != nullptr)
                {
                    typename std::allocator_traits<Allocator>:: template rebind_alloc<object> alloc(ptr_->get_allocator());
                    std::allocator_traits<Allocator>:: template rebind_traits<object>::destroy(alloc, to_plain_pointer(ptr_));
                    alloc.deallocate(ptr_,1);
                }
            }

            void swap(object_data& val)
            {
                std::swap(val.ptr_,ptr_);
            }

            object& value()
            {
                return *ptr_;
            }

            const object& value() const
            {
                return *ptr_;
            }

            allocator_type get_allocator() const
            {
                return ptr_->get_allocator();
            }
        };

    private:

        union
        {
            base_data base_data_;
            null_data null_data_;
            bool_data bool_data_;
            integer_data integer_data_;
            uinteger_data uinteger_data_;
            double_data double_data_;
            small_string_data small_string_data_;
            string_data string_data_;
            byte_string_data byte_string_data_;
            array_data array_data_;
            object_data object_data_;
            empty_object_data empty_object_data_;
        };

        template <class VariantType, class... Args>
        void construct_var(Args&&... args)
        {
            new (&as<VariantType>()) VariantType(std::forward<Args>(args)...);
        }

        template <class VariantType>
        void destroy_var()
        {
            as<VariantType>().~VariantType();
        }

        template <class T>
        struct identity { using type = T; };

    public:
        template <class T>       T& as()       { return as(identity<T>()); }
        template <class T> const T& as() const { return as(identity<T>()); }

    private:
        template <class T>       T& as(identity<T>);
        template <class T> const T& as(identity<T>) const;

        base_data&         as(identity<base_data>)         { return base_data_; }
        null_data&         as(identity<null_data>)         { return null_data_; }
        bool_data&         as(identity<bool_data>)         { return bool_data_; }
        integer_data&      as(identity<integer_data>)      { return integer_data_; }
        uinteger_data&     as(identity<uinteger_data>)     { return uinteger_data_; }
        double_data&       as(identity<double_data>)       { return double_data_; }
        small_string_data& as(identity<small_string_data>) { return small_string_data_; }
        string_data&       as(identity<string_data>)       { return string_data_; }
        byte_string_data&  as(identity<byte_string_data>)  { return byte_string_data_; }
        array_data&        as(identity<array_data>)        { return array_data_; }
        object_data&       as(identity<object_data>)       { return object_data_; }
        empty_object_data& as(identity<empty_object_data>) { return empty_object_data_; }
        
        const base_data&         as(identity<base_data>)         const { return base_data_; }
        const null_data&         as(identity<null_data>)         const { return null_data_; }
        const bool_data&         as(identity<bool_data>)         const { return bool_data_; }
        const integer_data&      as(identity<integer_data>)      const { return integer_data_; }
        const uinteger_data&     as(identity<uinteger_data>)     const { return uinteger_data_; }
        const double_data&       as(identity<double_data>)       const { return double_data_; }
        const small_string_data& as(identity<small_string_data>) const { return small_string_data_; }
        const string_data&       as(identity<string_data>)       const { return string_data_; }
        const byte_string_data&  as(identity<byte_string_data>)  const { return byte_string_data_; }
        const array_data&        as(identity<array_data>)        const { return array_data_; }
        const object_data&       as(identity<object_data>)       const { return object_data_; }
        const empty_object_data& as(identity<empty_object_data>) const { return empty_object_data_; }

    public:
        const null_data*         null_data_cast()         const { return &null_data_; }
        const empty_object_data* empty_object_data_cast() const { return &empty_object_data_; }
        const bool_data*         bool_data_cast()         const { return &bool_data_; }
        const integer_data*      integer_data_cast()      const { return &integer_data_; }
        const uinteger_data*     uinteger_data_cast()     const { return &uinteger_data_; }
        const double_data*       double_data_cast()       const { return &double_data_; }
        const small_string_data* small_string_data_cast() const { return &small_string_data_; }
              string_data*       string_data_cast()             { return &string_data_; }
        const string_data*       string_data_cast()       const { return &string_data_; }
              byte_string_data*  byte_string_data_cast()        { return &byte_string_data_; }
        const byte_string_data*  byte_string_data_cast()  const { return &byte_string_data_; }
              object_data*       object_data_cast()             { return &object_data_; }
        const object_data*       object_data_cast()       const { return &object_data_; }
              array_data*        array_data_cast()              { return &array_data_; }
        const array_data*        array_data_cast()        const { return &array_data_; }

        variant()
        {
            construct_var<empty_object_data>();
        }

        variant(const Allocator& a)
        {
            construct_var<object_data>(a);
        }

        variant(const variant& val)
        {
            Init_(val);
        }

        variant(const variant& val, const Allocator& allocator)
        {
            Init_(val,allocator);
        }

        variant(variant&& val) JSONCONS_NOEXCEPT
        {
            Init_rv_(std::forward<variant>(val));
        }

        variant(variant&& val, const Allocator& allocator) JSONCONS_NOEXCEPT
        {
            Init_rv_(std::forward<variant>(val), allocator,
                     typename std::allocator_traits<Allocator>::propagate_on_container_move_assignment());
        }

        explicit variant(null_type)
        {
            construct_var<null_data>();
        }
        explicit variant(bool val)
        {
            construct_var<bool_data>(val);
        }
        explicit variant(int64_t val)
        {
            construct_var<integer_data>(val);
        }
        explicit variant(uint64_t val, const Allocator&)
        {
            construct_var<uinteger_data>(val);
        }
        explicit variant(uint64_t val)
        {
            construct_var<uinteger_data>(val);
        }
        variant(double val)
        {
            construct_var<double_data>(val,std::numeric_limits<double>::digits10);
        }
        variant(double val, uint8_t precision)
        {
            construct_var<double_data>(val,precision);
        }
        variant(const char_type* s, size_t length)
        {
            if (length <= small_string_data::max_length)
            {
                construct_var<small_string_data>(s, static_cast<uint8_t>(length));
            }
            else
            {
                construct_var<string_data>(s, length, char_allocator_type());
            }
        }
        variant(const uint8_t* s, size_t length)
        {
            construct_var<byte_string_data>(s, length, byte_allocator_type());
        }

        variant(const uint8_t* s, size_t length, const Allocator& alloc)
        {
            construct_var<byte_string_data>(s, length, alloc);
        }

        variant(const char_type* s)
        {
            size_t length = char_traits_type::length(s);
            if (length <= small_string_data::max_length)
            {
                construct_var<small_string_data>(s, static_cast<uint8_t>(length));
            }
            else
            {
                construct_var<string_data>(s, length, char_allocator_type());
            }
        }

        variant(const char_type* s, const Allocator& alloc)
        {
            size_t length = char_traits_type::length(s);
            if (length <= small_string_data::max_length)
            {
                construct_var<small_string_data>(s, static_cast<uint8_t>(length));
            }
            else
            {
                construct_var<string_data>(s, length, alloc);
            }
        }

        variant(const char_type* s, size_t length, const Allocator& alloc)
        {
            if (length <= small_string_data::max_length)
            {
                construct_var<small_string_data>(s, static_cast<uint8_t>(length));
            }
            else
            {
                construct_var<string_data>(s, length, alloc);
            }
        }
        variant(const object& val)
        {
            construct_var<object_data>(val);
        }
        variant(const object& val, const Allocator& alloc)
        {
            construct_var<object_data>(val, alloc);
        }
        variant(const array& val)
        {
            construct_var<array_data>(val);
        }
        variant(const array& val, const Allocator& alloc)
        {
            construct_var<array_data>(val, alloc);
        }

        ~variant()
        {
            Destroy_();
        }

    private:
        void Destroy_()
        {
            switch (type_id())
            {
            case json_type_tag::null_t         : destroy_var<null_data>(); break;
            case json_type_tag::empty_object_t : destroy_var<empty_object_data>(); break;
            case json_type_tag::bool_t         : destroy_var<bool_data>(); break;
            case json_type_tag::integer_t      : destroy_var<integer_data>(); break;
            case json_type_tag::uinteger_t     : destroy_var<uinteger_data>(); break;
            case json_type_tag::double_t       : destroy_var<double_data>(); break;
            case json_type_tag::small_string_t : destroy_var<small_string_data>(); break;
            case json_type_tag::string_t       : destroy_var<string_data>(); break;
            case json_type_tag::byte_string_t  : destroy_var<byte_string_data>(); break;
            case json_type_tag::array_t        : destroy_var<array_data>(); break;
            case json_type_tag::object_t       : destroy_var<object_data>(); break;
            default:
                JSONCONS_UNREACHABLE();
                break;
            }
        }

    public:
        variant& operator=(const variant& val)
        {
            if (this != &val)
            {
                Destroy_();
                Init_(val);
            }
            return *this;
        }

        variant& operator=(variant&& val) JSONCONS_NOEXCEPT
        {
            if (this != &val)
            {
                Destroy_();
                Init_rv_(std::move(val));
            }
            return *this;
        }

        json_type_tag type_id() const
        {
            return base_data_.type_id_;
        }

        string_view_type as_string_view() const
        {
            switch (type_id())
            {
            case json_type_tag::small_string_t:
                return string_view_type(small_string_data_cast()->data(),small_string_data_cast()->length());
            case json_type_tag::string_t:
                return string_view_type(string_data_cast()->data(),string_data_cast()->length());
            default:
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a string");
            }
        }

        byte_string_view as_byte_string_view() const
        {
            switch (type_id())
            {
            case json_type_tag::byte_string_t:
                return byte_string_view(byte_string_data_cast()->data(),byte_string_data_cast()->length());
            default:
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a byte string");
            }
        }

        bool operator==(const variant& rhs) const
        {
            if (this ==&rhs)
            {
                return true;
            }
            switch (type_id())
            {
            case json_type_tag::null_t:
                switch (rhs.type_id())
                {
                case json_type_tag::null_t:
                    return true;
                default:
                    return false;
                }
                break;
            case json_type_tag::empty_object_t:
                switch (rhs.type_id())
                {
                case json_type_tag::empty_object_t:
                    return true;
                case json_type_tag::object_t:
                    return rhs.object_data_cast()->value().size() == 0;
                default:
                    return false;
                }
                break;
            case json_type_tag::bool_t:
                switch (rhs.type_id())
                {
                case json_type_tag::bool_t:
                    return bool_data_cast()->value() == rhs.bool_data_cast()->value();
                default:
                    return false;
                }
                break;
            case json_type_tag::integer_t:
                switch (rhs.type_id())
                {
                case json_type_tag::integer_t:
                    return integer_data_cast()->value() == rhs.integer_data_cast()->value();
                case json_type_tag::uinteger_t:
                    return integer_data_cast()->value() >= 0 ? static_cast<uint64_t>(integer_data_cast()->value()) == rhs.uinteger_data_cast()->value() : false;
                case json_type_tag::double_t:
                    return static_cast<double>(integer_data_cast()->value()) == rhs.double_data_cast()->value();
                default:
                    return false;
                }
                break;
            case json_type_tag::uinteger_t:
                switch (rhs.type_id())
                {
                case json_type_tag::integer_t:
                    return rhs.integer_data_cast()->value() >= 0 ? uinteger_data_cast()->value() == static_cast<uint64_t>(rhs.integer_data_cast()->value()) : false;
                case json_type_tag::uinteger_t:
                    return uinteger_data_cast()->value() == rhs.uinteger_data_cast()->value();
                case json_type_tag::double_t:
                    return static_cast<double>(uinteger_data_cast()->value()) == rhs.double_data_cast()->value();
                default:
                    return false;
                }
                break;
            case json_type_tag::double_t:
                switch (rhs.type_id())
                {
                case json_type_tag::integer_t:
                    return double_data_cast()->value() == static_cast<double>(rhs.integer_data_cast()->value());
                case json_type_tag::uinteger_t:
                    return double_data_cast()->value() == static_cast<double>(rhs.uinteger_data_cast()->value());
                case json_type_tag::double_t:
                    return double_data_cast()->value() == rhs.double_data_cast()->value();
                default:
                    return false;
                }
                break;
            case json_type_tag::small_string_t:
                switch (rhs.type_id())
                {
                case json_type_tag::small_string_t:
                    return as_string_view() == rhs.as_string_view();
                case json_type_tag::string_t:
                    return as_string_view() == rhs.as_string_view();
                default:
                    return false;
                }
                break;
            case json_type_tag::byte_string_t:
                switch (rhs.type_id())
                {
                case json_type_tag::byte_string_t:
                    {
                        return as_byte_string_view() == rhs.as_byte_string_view();
                    }
                default:
                    return false;
                }
                break;
            case json_type_tag::string_t:
                switch (rhs.type_id())
                {
                case json_type_tag::small_string_t:
                    return as_string_view() == rhs.as_string_view();
                case json_type_tag::string_t:
                    return as_string_view() == rhs.as_string_view();
                default:
                    return false;
                }
                break;
            case json_type_tag::array_t:
                switch (rhs.type_id())
                {
                case json_type_tag::array_t:
                    return array_data_cast()->value() == rhs.array_data_cast()->value();
                default:
                    return false;
                }
                break;
            case json_type_tag::object_t:
                switch (rhs.type_id())
                {
                case json_type_tag::empty_object_t:
                    return object_data_cast()->value().size() == 0;
                case json_type_tag::object_t:
                    return object_data_cast()->value() == rhs.object_data_cast()->value();
                default:
                    return false;
                }
                break;
            default:
                JSONCONS_UNREACHABLE();
                break;
            }
        }

        bool operator!=(const variant& rhs) const
        {
            return !(*this == rhs);
        }

    private:
        template <class TypeA, class TypeB>
        void swap_a_b(variant& other)
        {
            TypeA& curA = as<TypeA>();
            TypeB& curB = other.as<TypeB>();
            TypeB tmpB(std::move(curB));
            other.construct_var<TypeA>(std::move(curA));
            construct_var<TypeB>(std::move(tmpB));
        }

        template <class TypeA>
        void swap_a(variant& other)
        {
            switch (other.type_id())
            {
            case json_type_tag::null_t         : swap_a_b<TypeA, null_data>(other); break;
            case json_type_tag::empty_object_t : swap_a_b<TypeA, empty_object_data>(other); break;
            case json_type_tag::bool_t         : swap_a_b<TypeA, bool_data>(other); break;
            case json_type_tag::integer_t      : swap_a_b<TypeA, integer_data>(other); break;
            case json_type_tag::uinteger_t     : swap_a_b<TypeA, uinteger_data>(other); break;
            case json_type_tag::double_t       : swap_a_b<TypeA, double_data>(other); break;
            case json_type_tag::small_string_t : swap_a_b<TypeA, small_string_data>(other); break;
            case json_type_tag::string_t       : swap_a_b<TypeA, string_data>(other); break;
            case json_type_tag::byte_string_t  : swap_a_b<TypeA, byte_string_data>(other); break;
            case json_type_tag::array_t        : swap_a_b<TypeA, array_data>(other); break;
            case json_type_tag::object_t       : swap_a_b<TypeA, object_data>(other); break;
            default:
                JSONCONS_UNREACHABLE();
                break;
            }
        }

    public:
        void swap(variant& other) JSONCONS_NOEXCEPT
        {
            if (this == &other)
            {
                return;
            }

            switch (type_id())
            {
            case json_type_tag::null_t         : swap_a<null_data>(other); break;
            case json_type_tag::empty_object_t : swap_a<empty_object_data>(other); break;
            case json_type_tag::bool_t         : swap_a<bool_data>(other); break;
            case json_type_tag::integer_t      : swap_a<integer_data>(other); break;
            case json_type_tag::uinteger_t     : swap_a<uinteger_data>(other); break;
            case json_type_tag::double_t       : swap_a<double_data>(other); break;
            case json_type_tag::small_string_t : swap_a<small_string_data>(other); break;
            case json_type_tag::string_t       : swap_a<string_data>(other); break;
            case json_type_tag::byte_string_t  : swap_a<byte_string_data>(other); break;
            case json_type_tag::array_t        : swap_a<array_data>(other); break;
            case json_type_tag::object_t       : swap_a<object_data>(other); break;
            default:
                JSONCONS_UNREACHABLE();
                break;
            }
        }

    private:
        void Init_(const variant& val)
        {
            switch (val.type_id())
            {
            case json_type_tag::null_t         : construct_var<null_data        >(                           ); break;
            case json_type_tag::empty_object_t : construct_var<empty_object_data>(                           ); break;
            case json_type_tag::bool_t         : construct_var<bool_data        >(val.as<bool_data>()        ); break;
            case json_type_tag::integer_t      : construct_var<integer_data     >(val.as<integer_data>()     ); break;
            case json_type_tag::uinteger_t     : construct_var<uinteger_data    >(val.as<uinteger_data>()    ); break;
            case json_type_tag::double_t       : construct_var<double_data      >(val.as<double_data>()      ); break;
            case json_type_tag::small_string_t : construct_var<small_string_data>(val.as<small_string_data>()); break;
            case json_type_tag::string_t       : construct_var<string_data      >(val.as<string_data>()      ); break;
            case json_type_tag::byte_string_t  : construct_var<byte_string_data >(val.as<byte_string_data>() ); break;
            case json_type_tag::array_t        : construct_var<array_data       >(val.as<array_data>()       ); break;
            case json_type_tag::object_t       : construct_var<object_data      >(val.as<object_data>()      ); break;
            default:
                JSONCONS_UNREACHABLE();
                break;
            }
        }

        void Init_(const variant& val, const Allocator& a)
        {
            switch (val.type_id())
            {
            case json_type_tag::null_t         : construct_var<null_data        >(                            ); break;
            case json_type_tag::empty_object_t : construct_var<empty_object_data>(                            ); break;
            case json_type_tag::bool_t         : construct_var<bool_data        >(val.as<bool_data>()         ); break;
            case json_type_tag::integer_t      : construct_var<integer_data     >(val.as<integer_data>()      ); break;
            case json_type_tag::uinteger_t     : construct_var<uinteger_data    >(val.as<uinteger_data>()     ); break;
            case json_type_tag::double_t       : construct_var<double_data      >(val.as<double_data>()       ); break;
            case json_type_tag::small_string_t : construct_var<small_string_data>(val.as<small_string_data>() ); break;
            case json_type_tag::string_t       : construct_var<string_data      >(val.as<string_data>()     ,a); break;
            case json_type_tag::byte_string_t  : construct_var<byte_string_data >(val.as<byte_string_data>(),a); break;
            case json_type_tag::array_t        : construct_var<array_data       >(val.as<array_data>()      ,a); break;
            case json_type_tag::object_t       : construct_var<object_data      >(val.as<object_data>()     ,a); break;
            default:
                JSONCONS_UNREACHABLE();
                break;
            }
        }

        void Init_rv_(variant&& val) JSONCONS_NOEXCEPT
        {
            construct_var<null_data>();
            swap(val);
        }

        void Init_rv_(variant&& val, const Allocator& a, std::true_type) JSONCONS_NOEXCEPT
        {
            Init_rv_(std::forward<variant>(val));
        }

        void Init_rv_(variant&& val, const Allocator& a, std::false_type) JSONCONS_NOEXCEPT
        {
            switch (val.type_id())
            {
            case json_type_tag::null_t:
            case json_type_tag::empty_object_t:
            case json_type_tag::double_t:
            case json_type_tag::integer_t:
            case json_type_tag::uinteger_t:
            case json_type_tag::bool_t:
            case json_type_tag::small_string_t:
                Init_(std::forward<variant>(val));
                break;
            case json_type_tag::string_t:
                {
                    if (a == val.string_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant>(val), a, std::true_type());
                    }
                    else
                    {
                        Init_(val,a);
                    }
                }
                break;
            case json_type_tag::byte_string_t:
                {
                    if (a == val.byte_string_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant>(val), a, std::true_type());
                    }
                    else
                    {
                        Init_(val,a);
                    }
                }
                break;
            case json_type_tag::object_t:
                {
                    if (a == val.object_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant>(val), a, std::true_type());
                    }
                    else
                    {
                        Init_(val,a);
                    }
                }
                break;
            case json_type_tag::array_t:
                {
                    if (a == val.array_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant>(val), a, std::true_type());
                    }
                    else
                    {
                        Init_(val,a);
                    }
                }
                break;
            default:
                break;
            }
        }
    };

    template <class ParentT>
    class json_proxy 
    {
    private:
        typedef json_proxy<ParentT> proxy_type;

        ParentT& parent_;
        key_storage_type key_;

        json_proxy() = delete;
        json_proxy& operator = (const json_proxy& other) = delete; 

        json_proxy(ParentT& parent, key_storage_type&& name)
            : parent_(parent), key_(std::forward<key_storage_type>(name))
        {
        }

        basic_json& evaluate() 
        {
            return parent_.evaluate(string_view_type(key_.data(),key_.size()));
        }

        const basic_json& evaluate() const
        {
            return parent_.evaluate(string_view_type(key_.data(),key_.size()));
        }

        basic_json& evaluate_with_default()
        {
            basic_json& val = parent_.evaluate_with_default();
            auto it = val.find(string_view_type(key_.data(),key_.size()));
            if (it == val.object_range().end())
            {
                it = val.set_(val.object_range().begin(),std::move(key_),object(val.object_value().get_allocator()));            
            }
            return it->value();
        }

        basic_json& evaluate(size_t index)
        {
            return evaluate().at(index);
        }

        const basic_json& evaluate(size_t index) const
        {
            return evaluate().at(index);
        }

        basic_json& evaluate(const string_view_type& index)
        {
            return evaluate().at(index);
        }

        const basic_json& evaluate(const string_view_type& index) const
        {
            return evaluate().at(index);
        }
    public:

        friend class basic_json<CharT,ImplementationPolicy,Allocator>;

        range<object_iterator> object_range()
        {
            return evaluate().object_range();
        }

        range<const_object_iterator> object_range() const
        {
            return evaluate().object_range();
        }

        range<array_iterator> array_range()
        {
            return evaluate().array_range();
        }

        range<const_array_iterator> array_range() const
        {
            return evaluate().array_range();
        }

        size_t size() const JSONCONS_NOEXCEPT
        {
            return evaluate().size();
        }

        json_type_tag type_id() const
        {
            return evaluate().type_id();
        }

        size_t count(const string_view_type& name) const
        {
            return evaluate().count(name);
        }

        bool has_key(const string_view_type& name) const
        {
            return evaluate().has_key(name);
        }

        bool is_null() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_null();
        }

        bool empty() const
        {
            return evaluate().empty();
        }

        size_t capacity() const
        {
            return evaluate().capacity();
        }

        void reserve(size_t n)
        {
            evaluate().reserve(n);
        }

        void resize(size_t n)
        {
            evaluate().resize(n);
        }

        template <class T>
        void resize(size_t n, T val)
        {
            evaluate().resize(n,val);
        }

        template<class T, class... Args>
        bool is(Args&&... args) const
        {
            return evaluate().template is<T>(std::forward<Args>(args)...);
        }

        bool is_string() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_string();
        }

        bool is_byte_string() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_byte_string();
        }

        bool is_number() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_number();
        }
        bool is_bool() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_bool();
        }

        bool is_object() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_object();
        }

        bool is_array() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_array();
        }
        bool is_integer() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_integer();
        }

        bool is_uinteger() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_uinteger();
        }

        bool is_double() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_double();
        }

        string_view_type as_string_view() const 
        {
            return evaluate().as_string_view();
        }

        byte_string_view as_byte_string_view() const 
        {
            return evaluate().as_byte_string_view();
        }

        string_type as_string() const JSONCONS_NOEXCEPT
        {
            return evaluate().as_string();
        }

        template <class SAllocator>
        string_type as_string(const SAllocator& allocator) const JSONCONS_NOEXCEPT
        {
            return evaluate().as_string(allocator);
        }

        string_type as_string(const basic_serialization_options<char_type>& options) const
        {
            return evaluate().as_string(options);
        }

        template <class SAllocator>
        string_type as_string(const basic_serialization_options<char_type>& options,
                              const SAllocator& allocator) const
        {
            return evaluate().as_string(options,allocator);
        }

        template<class T, class... Args>
        T as(Args&&... args) const
        {
            return evaluate().template as<T>(std::forward<Args>(args)...);
        }

        template<class T>
        typename std::enable_if<std::is_same<string_type,T>::value,T>::type 
        as(const char_allocator_type& allocator) const
        {
            return evaluate().template as<T>(allocator);
        }
        bool as_bool() const
        {
            return evaluate().as_bool();
        }

        double as_double() const
        {
            return evaluate().as_double();
        }

        int64_t as_integer() const
        {
            return evaluate().as_integer();
        }

        unsigned long long as_ulonglong() const
        {
            return evaluate().as_ulonglong();
        }

        uint64_t as_uinteger() const
        {
            return evaluate().as_uinteger();
        }

        template <class T>
        json_proxy& operator=(T&& val) 
        {
            parent_.evaluate_with_default().set_(std::move(key_), std::forward<T>(val));
            return *this;
        }

        bool operator==(const basic_json& val) const
        {
            return evaluate() == val;
        }

        bool operator!=(const basic_json& val) const
        {
            return evaluate() != val;
        }

        basic_json& operator[](size_t i)
        {
            return evaluate_with_default().at(i);
        }

        const basic_json& operator[](size_t i) const
        {
            return evaluate().at(i);
        }

        json_proxy<proxy_type> operator[](const string_view_type& name)
        {
            return json_proxy<proxy_type>(*this,key_storage_type(name.begin(),name.end(),key_.get_allocator()));
        }

        const basic_json& operator[](const string_view_type& name) const
        {
            return at(name);
        }

        basic_json& at(const string_view_type& name)
        {
            return evaluate().at(name);
        }

        const basic_json& at(const string_view_type& name) const
        {
            return evaluate().at(name);
        }

        const basic_json& at(size_t index)
        {
            return evaluate().at(index);
        }

        const basic_json& at(size_t index) const
        {
            return evaluate().at(index);
        }

        object_iterator find(const string_view_type& name)
        {
            return evaluate().find(name);
        }

        const_object_iterator find(const string_view_type& name) const
        {
            return evaluate().find(name);
        }

        template <class T>
        basic_json get(const string_view_type& name, T&& default_val) const
        {
            return evaluate().get(name,std::forward<T>(default_val));
        }

        template <class T>
        T get_with_default(const string_view_type& name, const T& default_val) const
        {
            return evaluate().get_with_default(name,default_val);
        }

        string_view_type get_with_default(const string_view_type& name, const CharT* default_val) const
        {
            return evaluate().get_with_default(name,default_val);
        }

        void shrink_to_fit()
        {
            evaluate_with_default().shrink_to_fit();
        }

        void clear()
        {
            evaluate().clear();
        }
        // Remove all elements from an array or object

        void erase(const_object_iterator pos)
        {
            evaluate().erase(pos);
        }
        // Remove a range of elements from an object 

        void erase(const_object_iterator first, const_object_iterator last)
        {
            evaluate().erase(first, last);
        }
        // Remove a range of elements from an object 

        void erase(const string_view_type& name)
        {
            evaluate().erase(name);
        }

        void erase(const_array_iterator pos)
        {
            evaluate().erase(pos);
        }
        // Removes the element at pos 

        void erase(const_array_iterator first, const_array_iterator last)
        {
            evaluate().erase(first, last);
        }
        // Remove a range of elements from an array 

        // merge

        void merge(const basic_json& source)
        {
            return evaluate().merge(source);
        }

        void merge(basic_json&& source)
        {
            return evaluate().merge(std::forward<basic_json>(source));
        }

        void merge(object_iterator hint, const basic_json& source)
        {
            return evaluate().merge(hint, source);
        }

        void merge(object_iterator hint, basic_json&& source)
        {
            return evaluate().merge(hint, std::forward<basic_json>(source));
        }

        // merge_or_update

        void merge_or_update(const basic_json& source)
        {
            return evaluate().merge_or_update(source);
        }

        void merge_or_update(basic_json&& source)
        {
            return evaluate().merge_or_update(std::forward<basic_json>(source));
        }

        void merge_or_update(object_iterator hint, const basic_json& source)
        {
            return evaluate().merge_or_update(hint, source);
        }

        void merge_or_update(object_iterator hint, basic_json&& source)
        {
            return evaluate().merge_or_update(hint, std::forward<basic_json>(source));
        }

       // set

        template <class T>
        std::pair<object_iterator,bool> set(const string_view_type& name, T&& val)
        {
            return evaluate().set(name,std::forward<T>(val));
        }

        template <class T>
        std::pair<object_iterator,bool> insert_or_assign(const string_view_type& name, T&& val)
        {
            return evaluate().insert_or_assign(name,std::forward<T>(val));
        }

        template <class T>
        void set_(key_storage_type&& name, T&& val)
        {
            evaluate().set_(std::forward<key_storage_type>(name),std::forward<T>(val));
        }

       // emplace

        template <class ... Args>
        std::pair<object_iterator,bool> try_emplace(const string_view_type& name, Args&&... args)
        {
            return evaluate().try_emplace(name,std::forward<Args>(args)...);
        }

        template <class T>
        object_iterator set(object_iterator hint, const string_view_type& name, T&& val)
        {
            return evaluate().set(hint, name, std::forward<T>(val));
        }

        template <class T>
        object_iterator insert_or_assign(object_iterator hint, const string_view_type& name, T&& val)
        {
            return evaluate().insert_or_assign(hint, name, std::forward<T>(val));
        }

        template <class ... Args>
        object_iterator try_emplace(object_iterator hint, const string_view_type& name, Args&&... args)
        {
            return evaluate().try_emplace(hint, name, std::forward<Args>(args)...);
        }

        template <class T>
        object_iterator set_(object_iterator hint, key_storage_type&& name, T&& val)
        {
            return evaluate().set_(hint, std::forward<key_storage_type>(name), std::forward<T>(val));
        }

        template <class... Args> 
        array_iterator emplace(const_array_iterator pos, Args&&... args)
        {
            evaluate_with_default().emplace(pos, std::forward<Args>(args)...);
        }

        template <class... Args> 
        basic_json& emplace_back(Args&&... args)
        {
            return evaluate_with_default().emplace_back(std::forward<Args>(args)...);
        }

        template <class T>
        void add(T&& val)
        {
            evaluate_with_default().add(std::forward<T>(val));
        }

        template <class T>
        void push_back(T&& val)
        {
            evaluate_with_default().push_back(std::forward<T>(val));
        }

        template <class T>
        array_iterator add(const_array_iterator pos, T&& val)
        {
            return evaluate_with_default().add(pos, std::forward<T>(val));
        }

        template <class T>
        array_iterator insert(const_array_iterator pos, T&& val)
        {
            return evaluate_with_default().insert(pos, std::forward<T>(val));
        }

        template <class InputIt>
        array_iterator insert(const_array_iterator pos, InputIt first, InputIt last)
        {
            return evaluate_with_default().insert(pos, first, last);
        }

        template <class SAllocator>
        void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s) const
        {
            evaluate().dump(s);
        }

        template <class SAllocator>
        void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s,
                  const basic_serialization_options<char_type>& options) const
        {
            evaluate().dump(s,options);
        }
        void dump(basic_json_output_handler<char_type>& handler) const
        {
            evaluate().dump(handler);
        }

        void dump(std::basic_ostream<char_type>& os) const
        {
            evaluate().dump(os);
        }

        void dump(std::basic_ostream<char_type>& os, bool pprint) const
        {
            evaluate().dump(os, pprint);
        }

        void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
        {
            evaluate().dump(os,options);
        }

        void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
        {
            evaluate().dump(os,options,pprint);
        }
#if !defined(JSONCONS_NO_DEPRECATED)

        string_type to_string(const char_allocator_type& allocator = char_allocator_type()) const JSONCONS_NOEXCEPT
        {
            return evaluate().to_string(allocator);
        }
        void write(basic_json_output_handler<char_type>& handler) const
        {
            evaluate().write(handler);
        }

        void write(std::basic_ostream<char_type>& os) const
        {
            evaluate().write(os);
        }

        void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
        {
            evaluate().write(os,options);
        }

        void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
        {
            evaluate().write(os,options,pprint);
        }

        string_type to_string(const basic_serialization_options<char_type>& options, char_allocator_type& allocator = char_allocator_type()) const
        {
            return evaluate().to_string(options,allocator);
        }

        range<object_iterator> members()
        {
            return evaluate().members();
        }

        range<const_object_iterator> members() const
        {
            return evaluate().members();
        }

        range<array_iterator> elements()
        {
            return evaluate().elements();
        }

        range<const_array_iterator> elements() const
        {
            return evaluate().elements();
        }
        void to_stream(basic_json_output_handler<char_type>& handler) const
        {
            evaluate().to_stream(handler);
        }

        void to_stream(std::basic_ostream<char_type>& os) const
        {
            evaluate().to_stream(os);
        }

        void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
        {
            evaluate().to_stream(os,options);
        }

        void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
        {
            evaluate().to_stream(os,options,pprint);
        }
#endif
        void swap(basic_json& val)
        {
            evaluate_with_default().swap(val);
        }

        friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_proxy& o)
        {
            o.dump(os);
            return os;
        }

#if !defined(JSONCONS_NO_DEPRECATED)

        void resize_array(size_t n)
        {
            evaluate().resize_array(n);
        }

        template <class T>
        void resize_array(size_t n, T val)
        {
            evaluate().resize_array(n,val);
        }

        object_iterator begin_members()
        {
            return evaluate().begin_members();
        }

        const_object_iterator begin_members() const
        {
            return evaluate().begin_members();
        }

        object_iterator end_members()
        {
            return evaluate().end_members();
        }

        const_object_iterator end_members() const
        {
            return evaluate().end_members();
        }

        array_iterator begin_elements()
        {
            return evaluate().begin_elements();
        }

        const_array_iterator begin_elements() const
        {
            return evaluate().begin_elements();
        }

        array_iterator end_elements()
        {
            return evaluate().end_elements();
        }

        const_array_iterator end_elements() const
        {
            return evaluate().end_elements();
        }

        const basic_json& get(const string_view_type& name) const
        {
            return evaluate().get(name);
        }

        bool is_ulonglong() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_ulonglong();
        }

        bool is_longlong() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_longlong();
        }

        int as_int() const
        {
            return evaluate().as_int();
        }

        unsigned int as_uint() const
        {
            return evaluate().as_uint();
        }

        long as_long() const
        {
            return evaluate().as_long();
        }

        unsigned long as_ulong() const
        {
            return evaluate().as_ulong();
        }

        long long as_longlong() const
        {
            return evaluate().as_longlong();
        }

        void add(size_t index, const basic_json& value)
        {
            evaluate_with_default().add(index, value);
        }

        void add(size_t index, basic_json&& value)
        {
            evaluate_with_default().add(index, std::forward<basic_json>(value));
        }

        bool has_member(const key_storage_type& name) const
        {
            return evaluate().has_member(name);
        }

        // Remove a range of elements from an array 
        void remove_range(size_t from_index, size_t to_index)
        {
            evaluate().remove_range(from_index, to_index);
        }
        // Remove a range of elements from an array 
        void remove(const string_view_type& name)
        {
            evaluate().remove(name);
        }
        void remove_member(const string_view_type& name)
        {
            evaluate().remove(name);
        }
        bool is_empty() const JSONCONS_NOEXCEPT
        {
            return empty();
        }
        bool is_numeric() const JSONCONS_NOEXCEPT
        {
            return is_number();
        }
#endif
    };

    static basic_json parse(std::basic_istream<char_type>& is);
    static basic_json parse(std::basic_istream<char_type>& is, parse_error_handler& err_handler);

    static basic_json parse(const string_view_type& s)
    {
        parse_error_handler_type err_handler;
        return parse(s,err_handler);
    }

    static basic_json parse(const char_type* s, size_t length)
    {
        parse_error_handler_type err_handler;
        return parse(s,length,err_handler);
    }

    static basic_json parse(const char_type* s, size_t length, parse_error_handler& err_handler)
    {
        return parse(string_view_type(s,length),err_handler);
    }

    static basic_json parse(const string_view_type& s, parse_error_handler& err_handler)
    {
        json_decoder<basic_json> handler;
        basic_json_parser<char_type> parser(handler,err_handler);

        auto result = unicons::skip_bom(s.begin(), s.end());
        if (result.ec != unicons::encoding_errc())
        {
            throw parse_error(result.ec,1,1);
        }
        size_t offset = result.it - s.begin();
        parser.set_source(s.data()+offset,s.size()-offset);
        parser.parse();
        parser.end_parse();
        parser.check_done();
        if (!handler.is_valid())
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json string");
        }
        return handler.get_result();
    }

    static basic_json parse_file(const std::basic_string<char_type,char_traits_type>& filename)
    {
        parse_error_handler_type err_handler;
        return parse_file(filename,err_handler);
    }

    static basic_json parse_file(const std::basic_string<char_type,char_traits_type>& filename,
                                 parse_error_handler& err_handler)
    {
        std::basic_ifstream<CharT> is(filename);
        return parse(is,err_handler);
    }

    static basic_json make_array()
    {
        return basic_json(variant(array()));
    }

    static basic_json make_array(const array& a)
    {
        return basic_json(variant(a));
    }

    static basic_json make_array(const array& a, allocator_type allocator)
    {
        return basic_json(variant(a,allocator));
    }

    static basic_json make_array(std::initializer_list<basic_json> init, const Allocator& allocator = Allocator())
    {
        return array(std::move(init),allocator);
    }

    static basic_json make_array(size_t n, const Allocator& allocator = Allocator())
    {
        return array(n,allocator);
    }

    template <class T>
    static basic_json make_array(size_t n, const T& val, const Allocator& allocator = Allocator())
    {
        return basic_json::array(n, val,allocator);
    }

    template <size_t dim>
    static typename std::enable_if<dim==1,basic_json>::type make_array(size_t n)
    {
        return array(n);
    }

    template <size_t dim, class T>
    static typename std::enable_if<dim==1,basic_json>::type make_array(size_t n, const T& val, const Allocator& allocator = Allocator())
    {
        return array(n,val,allocator);
    }

    template <size_t dim, typename... Args>
    static typename std::enable_if<(dim>1),basic_json>::type make_array(size_t n, Args... args)
    {
        const size_t dim1 = dim - 1;

        basic_json val = make_array<dim1>(args...);
        val.resize(n);
        for (size_t i = 0; i < n; ++i)
        {
            val[i] = make_array<dim1>(args...);
        }
        return val;
    }

    static const basic_json& null()
    {
        static basic_json a_null = basic_json(variant(null_type()));
        return a_null;
    }

    variant var_;

    basic_json() 
        : var_()
    {
    }

    explicit basic_json(const Allocator& allocator) 
        : var_(allocator)
    {
    }

    basic_json(const basic_json& val)
        : var_(val.var_)
    {
    }

    basic_json(const basic_json& val, const Allocator& allocator)
        : var_(val.var_,allocator)
    {
    }

    basic_json(basic_json&& other) JSONCONS_NOEXCEPT
        : var_(std::move(other.var_))
    {
    }

    basic_json(basic_json&& other, const Allocator& allocator) JSONCONS_NOEXCEPT
        : var_(std::move(other.var_) /*,allocator*/ )
    {
    }

    basic_json(const variant& val)
        : var_(val)
    {
    }

    basic_json(variant&& other)
        : var_(std::forward<variant>(other))
    {
    }

    basic_json(const array& val)
        : var_(val)
    {
    }

    basic_json(array&& other)
        : var_(std::forward<array>(other))
    {
    }

    basic_json(const object& other)
        : var_(other)
    {
    }

    basic_json(object&& other)
        : var_(std::forward<object>(other))
    {
    }

    template <class ParentT>
    basic_json(const json_proxy<ParentT>& proxy)
        : var_(proxy.evaluate().var_)
    {
    }

    template <class ParentT>
    basic_json(const json_proxy<ParentT>& proxy, const Allocator& allocator)
        : var_(proxy.evaluate().var_,allocator)
    {
    }

    template <class T>
    basic_json(const T& val)
        : var_(json_type_traits<basic_json,T>::to_json(val).var_)
    {
    }

    template <class T>
    basic_json(const T& val, const Allocator& allocator)
        : var_(json_type_traits<basic_json,T>::to_json(val,allocator).var_)
    {
    }

    basic_json(const char_type* s)
        : var_(s)
    {
    }

    basic_json(const char_type* s, const Allocator& allocator)
        : var_(s,allocator)
    {
    }

    basic_json(double val, uint8_t precision)
        : var_(val,precision)
    {
    }

    basic_json(const char_type *s, size_t length)
        : var_(s, length)
    {
    }

    basic_json(const char_type *s, size_t length, const Allocator& allocator)
        : var_(s, length, allocator)
    {
    }

    basic_json(const uint8_t* s, size_t length)
        : var_(s, length)
    {
    }

    explicit basic_json(const byte_string_view& s)
        : var_(s.data(), s.length())
    {
    }

    basic_json(const uint8_t* s, size_t length, const Allocator& allocator)
        : var_(s, length, allocator)
    {
    }
#if !defined(JSONCONS_NO_DEPRECATED)
    template<class InputIterator>
    basic_json(InputIterator first, InputIterator last, const Allocator& allocator = Allocator())
        : var_(first,last,allocator)
    {
    }
#endif

    ~basic_json()
    {
    }

    basic_json& operator=(const basic_json& rhs)
    {
        var_ = rhs.var_;
        return *this;
    }

    basic_json& operator=(basic_json&& rhs) JSONCONS_NOEXCEPT
    {
        if (this !=&rhs)
        {
            var_ = std::move(rhs.var_);
        }
        return *this;
    }

    template <class T>
    basic_json& operator=(const T& val)
    {
        var_ = json_type_traits<basic_json,T>::to_json(val).var_;
        return *this;
    }

    basic_json& operator=(const char_type* s)
    {
        var_ = variant(s);
        return *this;
    }

    bool operator!=(const basic_json& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator==(const basic_json& rhs) const
    {
        return var_ == rhs.var_;
    }

    size_t size() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return 0;
        case json_type_tag::object_t:
            return object_value().size();
        case json_type_tag::array_t:
            return array_value().size();
        default:
            return 0;
        }
    }

    basic_json& operator[](size_t i)
    {
        return at(i);
    }

    const basic_json& operator[](size_t i) const
    {
        return at(i);
    }

    json_proxy<basic_json> operator[](const string_view_type& name)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t: 
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return json_proxy<basic_json>(*this, key_storage_type(name.begin(),name.end(),char_allocator_type(object_value().get_allocator())));
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    const basic_json& operator[](const string_view_type& name) const
    {
        return at(name);
    }

    template <class SAllocator>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s) const
    {
        std::basic_ostringstream<char_type,char_traits_type,SAllocator> os;
        os.imbue(std::locale::classic());
        {
            basic_json_serializer<char_type> serializer(os);
            dump(serializer);
        }
        s = os.str();
    }

    template <class SAllocator>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s,
              const basic_serialization_options<char_type>& options) const
    {
        std::basic_ostringstream<char_type,char_traits_type,SAllocator> os;
        os.imbue(std::locale::classic());
        {
            basic_json_serializer<char_type> serializer(os,options);
            dump(serializer);
        }
        s = os.str();
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    void dump_body(basic_json_output_handler<char_type>& handler) const
    {
        dump_fragment(handler);
    }
#endif
    void dump_fragment(basic_json_output_handler<char_type>& handler) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            handler.string_value(as_string_view());
            break;
        case json_type_tag::byte_string_t:
            handler.byte_string_value(var_.byte_string_data_cast()->data(), var_.byte_string_data_cast()->length());
            break;
        case json_type_tag::double_t:
            handler.double_value(var_.double_data_cast()->value(), var_.double_data_cast()->precision());
            break;
        case json_type_tag::integer_t:
            handler.integer_value(var_.integer_data_cast()->value());
            break;
        case json_type_tag::uinteger_t:
            handler.uinteger_value(var_.uinteger_data_cast()->value());
            break;
        case json_type_tag::bool_t:
            handler.bool_value(var_.bool_data_cast()->value());
            break;
        case json_type_tag::null_t:
            handler.null_value();
            break;
        case json_type_tag::empty_object_t:
            handler.begin_object();
            handler.end_object();
            break;
        case json_type_tag::object_t:
            {
                handler.begin_object();
                const object& o = object_value();
                for (const_object_iterator it = o.begin(); it != o.end(); ++it)
                {
                    handler.name(string_view_type((it->key()).data(),it->key().length()));
                    it->value().dump_fragment(handler);
                }
                handler.end_object();
            }
            break;
        case json_type_tag::array_t:
            {
                handler.begin_array();
                const array& o = array_value();
                for (const_array_iterator it = o.begin(); it != o.end(); ++it)
                {
                    it->dump_fragment(handler);
                }
                handler.end_array();
            }
            break;
        default:
            break;
        }
    }
    void dump(basic_json_output_handler<char_type>& handler) const
    {
        handler.begin_json();
        dump_fragment(handler);
        handler.end_json();
    }

    void dump(std::basic_ostream<char_type>& os) const
    {
        basic_json_serializer<char_type> serializer(os);
        dump(serializer);
    }

    void dump(std::basic_ostream<char_type>& os, bool pprint) const
    {
        basic_json_serializer<char_type> serializer(os, pprint);
        dump(serializer);
    }

    void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
    {
        basic_json_serializer<char_type> serializer(os, options);
        dump(serializer);
    }

    void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
    {
        basic_json_serializer<char_type> serializer(os, options, pprint);
        dump(serializer);
    }

    string_type to_string(const char_allocator_type& allocator=char_allocator_type()) const JSONCONS_NOEXCEPT
    {
        string_type s(allocator);
        std::basic_ostringstream<char_type,char_traits_type,char_allocator_type> os(s);
        os.imbue(std::locale::classic());
        {
            basic_json_serializer<char_type> serializer(os);
            dump_fragment(serializer);
        }
        return os.str();
    }

    string_type to_string(const basic_serialization_options<char_type>& options,
                          const char_allocator_type& allocator=char_allocator_type()) const
    {
        string_type s(allocator);
        std::basic_ostringstream<char_type> os(s);
        os.imbue(std::locale::classic());
        {
            basic_json_serializer<char_type> serializer(os, options);
            dump_fragment(serializer);
        }
        return os.str();
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    void write_body(basic_json_output_handler<char_type>& handler) const
    {
        dump(handler);
    }
    void write(basic_json_output_handler<char_type>& handler) const
    {
        dump(handler);
    }

    void write(std::basic_ostream<char_type>& os) const
    {
        dump(os);
    }

    void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
    {
        dump(os,options);
    }

    void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
    {
        dump(os,options,pprint);
    }

    void to_stream(basic_json_output_handler<char_type>& handler) const
    {
        handler.begin_json();
        dump_fragment(handler);
        handler.end_json();
    }

    void to_stream(std::basic_ostream<char_type>& os) const
    {
        basic_json_serializer<char_type> serializer(os);
        to_stream(serializer);
    }

    void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
    {
        basic_json_serializer<char_type> serializer(os, options);
        to_stream(serializer);
    }

    void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool pprint) const
    {
        basic_json_serializer<char_type> serializer(os, options, pprint);
        to_stream(serializer);
    }
#endif
    bool is_null() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::null_t;
    }

    bool has_key(const string_view_type& name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                return it != object_range().end();
            }
            break;
        default:
            return false;
        }
    }

    size_t count(const string_view_type& name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::object_t:
            {
                auto it = object_value().find(name);
                if (it == object_range().end())
                {
                    return 0;
                }
                size_t count = 0;
                while (it != object_range().end()&& it->key() == name)
                {
                    ++count;
                    ++it;
                }
                return count;
            }
            break;
        default:
            return 0;
        }
    }

    template<class T, class... Args>
    bool is(Args&&... args) const
    {
        return json_type_traits<basic_json,T>::is(*this,std::forward<Args>(args)...);
    }

    bool is_string() const JSONCONS_NOEXCEPT
    {
        return (var_.type_id() == json_type_tag::string_t) || (var_.type_id() == json_type_tag::small_string_t);
    }

    bool is_byte_string() const JSONCONS_NOEXCEPT
    {
        return (var_.type_id() == json_type_tag::byte_string_t);
    }

    bool is_bool() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::bool_t;
    }

    bool is_object() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::object_t || var_.type_id() == json_type_tag::empty_object_t;
    }

    bool is_array() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::array_t;
    }

    bool is_integer() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::integer_t || (var_.type_id() == json_type_tag::uinteger_t&& (as_uinteger() <= static_cast<uint64_t>((std::numeric_limits<long long>::max)())));
    }

    bool is_uinteger() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::uinteger_t || (var_.type_id() == json_type_tag::integer_t&& as_integer() >= 0);
    }

    bool is_double() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::double_t;
    }

    bool is_number() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::integer_t || var_.type_id() == json_type_tag::uinteger_t || var_.type_id() == json_type_tag::double_t;
    }

    bool empty() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
            return var_.small_string_data_cast()->length() == 0;
        case json_type_tag::string_t:
            return var_.string_data_cast()->length() == 0;
        case json_type_tag::array_t:
            return array_value().size() == 0;
        case json_type_tag::empty_object_t:
            return true;
        case json_type_tag::object_t:
            return object_value().size() == 0;
        default:
            return false;
        }
    }

    size_t capacity() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().capacity();
        case json_type_tag::object_t:
            return object_value().capacity();
        default:
            return 0;
        }
    }

    template<class U=Allocator>
    typename std::enable_if<is_stateless<U>::value,void>::type
    create_object_implicitly()
    {
        var_ = variant(Allocator());
    }

    template<class U=Allocator>
    typename std::enable_if<!is_stateless<U>::value,void>::type
    create_object_implicitly() const
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Cannot create object implicitly - allocator is not default constructible.");
    }

    void reserve(size_t n)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().reserve(n);
            break;
        case json_type_tag::empty_object_t:
        {
            create_object_implicitly();
            object_value().reserve(n);
        }
        break;
        case json_type_tag::object_t:
        {
            object_value().reserve(n);
        }
            break;
        default:
            break;
        }
    }

    void resize(size_t n)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().resize(n);
            break;
        default:
            break;
        }
    }

    template <class T>
    void resize(size_t n, T val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().resize(n, val);
            break;
        default:
            break;
        }
    }

    template<class T, class... Args>
    T as(Args&&... args) const
    {
        return json_type_traits<basic_json,T>::as(*this,std::forward<Args>(args)...);
    }

    template<class T>
    typename std::enable_if<std::is_same<string_type,T>::value,T>::type 
    as(const char_allocator_type& allocator) const
    {
        return json_type_traits<basic_json,T>::as(*this,allocator);
    }

    bool as_bool() const 
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            try
            {
                basic_json j = basic_json::parse(as_string_view().data(),as_string_view().length());
                return j.as_bool();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a bool");
            }
            break;
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value();
        case json_type_tag::double_t:
            return var_.double_data_cast()->value() != 0.0;
        case json_type_tag::integer_t:
            return var_.integer_data_cast()->value() != 0;
        case json_type_tag::uinteger_t:
            return var_.uinteger_data_cast()->value() != 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a bool");
        }
    }

    int64_t as_integer() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            try
            {
                basic_json j = basic_json::parse(as_string_view().data(),as_string_view().length());
                return j.as<int64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
            }
            break;
        case json_type_tag::double_t:
            return static_cast<int64_t>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<int64_t>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<int64_t>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
        }
    }

    uint64_t as_uinteger() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            try
            {
                basic_json j = basic_json::parse(as_string_view().data(),as_string_view().length());
                return j.as<uint64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
            }
            break;
        case json_type_tag::double_t:
            return static_cast<uint64_t>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<uint64_t>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<uint64_t>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
        }
    }

    size_t double_precision() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return var_.double_data_cast()->precision();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
        }
    }

    double as_double() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            try
            {
                basic_json j = basic_json::parse(as_string_view().data(),as_string_view().length());
                return j.as<double>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
            }
            break;
        case json_type_tag::double_t:
            return var_.double_data_cast()->value();
        case json_type_tag::integer_t:
            return static_cast<double>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<double>(var_.uinteger_data_cast()->value());
        case json_type_tag::null_t:
            return std::numeric_limits<double>::quiet_NaN();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
        }
    }

    string_view_type as_string_view() const
    {
        return var_.as_string_view();
    }

    byte_string_view as_byte_string_view() const
    {
        return var_.as_byte_string_view();
    }

    string_type as_string() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            return string_type(as_string_view().data(),as_string_view().length());
        default:
            return to_string();
        }
    }

    template <class SAllocator>
    string_type as_string(const SAllocator& allocator) const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            return string_type(as_string_view().data(),as_string_view().length(),allocator);
        default:
            return to_string(allocator);
        }
    }

    string_type as_string(const basic_serialization_options<char_type>& options) const 
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            return string_type(as_string_view().data(),as_string_view().length());
        default:
            return to_string(options);
        }
    }

    template <class SAllocator>
    string_type as_string(const basic_serialization_options<char_type>& options,
                          const SAllocator& allocator) const 
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
        case json_type_tag::string_t:
            return string_type(as_string_view().data(),as_string_view().length(),allocator);
        default:
            return to_string(options,allocator);
        }
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    const char_type* as_cstring() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::small_string_t:
            return var_.small_string_data_cast()->c_str();
        case json_type_tag::string_t:
            return var_.string_data_cast()->c_str();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a cstring");
        }
    }
#endif

    basic_json& at(const string_view_type& name)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case json_type_tag::object_t:
            {
                auto it = object_value().find(name);
                if (it == object_range().end())
                {
                    JSONCONS_THROW_EXCEPTION_1(std::out_of_range, "%s not found", name);
                }
                return it->value();
            }
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    basic_json& evaluate() 
    {
        return *this;
    }

    basic_json& evaluate_with_default() 
    {
        return *this;
    }

    const basic_json& evaluate() const
    {
        return *this;
    }
    basic_json& evaluate(const string_view_type& name) 
    {
        return at(name);
    }

    const basic_json& evaluate(const string_view_type& name) const
    {
        return at(name);
    }

    const basic_json& at(const string_view_type& name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case json_type_tag::object_t:
            {
                auto it = object_value().find(name);
                if (it == object_range().end())
                {
                    JSONCONS_THROW_EXCEPTION_1(std::out_of_range, "%s not found", name);
                }
                return it->value();
            }
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    basic_json& at(size_t i)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            if (i >= array_value().size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return array_value().operator[](i);
        case json_type_tag::object_t:
            return object_value().at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    const basic_json& at(size_t i) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            if (i >= array_value().size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return array_value().operator[](i);
        case json_type_tag::object_t:
            return object_value().at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    object_iterator find(const string_view_type& name)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return object_range().end();
        case json_type_tag::object_t:
            return object_value().find(name);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const_object_iterator find(const string_view_type& name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return object_range().end();
        case json_type_tag::object_t:
            return object_value().find(name);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    template<class T>
    basic_json get(const string_view_type& name, T&& default_val) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            {
                return basic_json(std::forward<T>(default_val));
            }
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                if (it != object_range().end())
                {
                    return it->value();
                }
                else
                {
                    return basic_json(std::forward<T>(default_val));
                }
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    template<class T>
    T get_with_default(const string_view_type& name, const T& default_val) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            {
                return default_val;
            }
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                if (it != object_range().end())
                {
                    return it->value().template as<T>();
                }
                else
                {
                    return default_val;
                }
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    string_view_type get_with_default(const string_view_type& name, const CharT* default_val) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            {
                return default_val;
            }
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                if (it != object_range().end())
                {
                    return it->value().as_string_view();
                }
                else
                {
                    return default_val;
                }
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    // Modifiers

    void shrink_to_fit()
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().shrink_to_fit();
            break;
        case json_type_tag::object_t:
            object_value().shrink_to_fit();
            break;
        default:
            break;
        }
    }

    void clear()
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().clear();
            break;
        case json_type_tag::object_t:
            object_value().clear();
            break;
        default:
            break;
        }
    }

    void erase(const_object_iterator pos)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            break;
        case json_type_tag::object_t:
            object_value().erase(pos);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    void erase(const_object_iterator first, const_object_iterator last)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            break;
        case json_type_tag::object_t:
            object_value().erase(first, last);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    void erase(const_array_iterator pos)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().erase(pos);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
            break;
        }
    }

    void erase(const_array_iterator first, const_array_iterator last)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().erase(first, last);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
            break;
        }
    }

    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void erase(const string_view_type& name)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            break;
        case json_type_tag::object_t:
            object_value().erase(name);
            break;
        default:
            JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to erase %s on a value that is not an object", name);
            break;
        }
    }

    template <class T>
    std::pair<object_iterator,bool> set(const string_view_type& name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().insert_or_assign(name, std::forward<T>(val));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    std::pair<object_iterator,bool> insert_or_assign(const string_view_type& name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().insert_or_assign(name, std::forward<T>(val));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class ... Args>
    std::pair<object_iterator,bool> try_emplace(const string_view_type& name, Args&&... args)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().try_emplace(name, std::forward<Args>(args)...);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    void set_(key_storage_type&& name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            object_value().set_(std::forward<key_storage_type>(name), std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to call set on a value that is not an object");
            }
        }
    }

    // merge

    void merge(const basic_json& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge(source.object_value());
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge a value that is not an object");
            }
        }
    }

    void merge(basic_json&& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge(std::move(source.object_value()));
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge a value that is not an object");
            }
        }
    }

    void merge(object_iterator hint, const basic_json& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge(hint, source.object_value());
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge a value that is not an object");
            }
        }
    }

    void merge(object_iterator hint, basic_json&& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge(hint, std::move(source.object_value()));
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge a value that is not an object");
            }
        }
    }

    // merge_or_update

    void merge_or_update(const basic_json& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge_or_update(source.object_value());
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge_or_update a value that is not an object");
            }
        }
    }

    void merge_or_update(basic_json&& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge_or_update(std::move(source.object_value()));
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge_or_update a value that is not an object");
            }
        }
    }

    void merge_or_update(object_iterator hint, const basic_json& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge_or_update(hint, source.object_value());
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge_or_update a value that is not an object");
            }
        }
    }

    void merge_or_update(object_iterator hint, basic_json&& source)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().merge_or_update(hint, std::move(source.object_value()));
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to merge_or_update a value that is not an object");
            }
        }
    }

    // set

    template <class T>
    object_iterator set(object_iterator hint, const string_view_type& name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().insert_or_assign(hint, name, std::forward<T>(val));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    object_iterator insert_or_assign(object_iterator hint, const string_view_type& name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().insert_or_assign(hint, name, std::forward<T>(val));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class ... Args>
    object_iterator try_emplace(object_iterator hint, const string_view_type& name, Args&&... args)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().try_emplace(hint, name, std::forward<Args>(args)...);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    object_iterator set_(object_iterator hint, key_storage_type&& name, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return object_value().set_(hint, std::forward<key_storage_type>(name), std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to set on a value that is not an object");
            }
        }
    }

    template <class T>
    void add(T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().push_back(std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class T>
    void push_back(T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().push_back(std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class T>
    array_iterator add(const_array_iterator pos, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().insert(pos, std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class T>
    array_iterator insert(const_array_iterator pos, T&& val)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().insert(pos, std::forward<T>(val));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class InputIt>
    array_iterator insert(const_array_iterator pos, InputIt first, InputIt last)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().insert(pos, first, last);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class... Args> 
    array_iterator emplace(const_array_iterator pos, Args&&... args)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().emplace(pos, std::forward<Args>(args)...);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class... Args> 
    basic_json& emplace_back(Args&&... args)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return array_value().emplace_back(std::forward<Args>(args)...);
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    json_type_tag type_id() const
    {
        return var_.type_id();
    }

    void swap(basic_json& b)
    {
        var_.swap(b.var_);
    }

    friend void swap(basic_json& a, basic_json& b)
    {
        a.swap(b);
    }

    static basic_json make_string(const string_view_type& s)
    {
        return basic_json(variant(s.data(),s.length()));
    }

    static basic_json make_string(const char_type* rhs, size_t length)
    {
        return basic_json(variant(rhs,length));
    }

    static basic_json make_string(const string_view_type& s, allocator_type allocator)
    {
        return basic_json(variant(s.data(),s.length(),allocator));
    }

    static basic_json from_integer(int64_t val)
    {
        return basic_json(variant(val));
    }

    static basic_json from_integer(int64_t val, allocator_type)
    {
        return basic_json(variant(val));
    }

    static basic_json from_uinteger(uint64_t val)
    {
        return basic_json(variant(val));
    }

    static basic_json from_uinteger(uint64_t val, allocator_type)
    {
        return basic_json(variant(val));
    }

    static basic_json from_floating_point(double val)
    {
        return basic_json(variant(val, std::numeric_limits<double>::digits10));
    }

    static basic_json from_floating_point(double val, allocator_type)
    {
        return basic_json(variant(val,std::numeric_limits<double>::digits10));
    }

    static basic_json from_bool(bool val)
    {
        return basic_json(variant(val));
    }

    static basic_json make_object(const object& o)
    {
        return basic_json(variant(o));
    }

    static basic_json make_object(const object& o, allocator_type allocator)
    {
        return basic_json(variant(o,allocator));
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    static basic_json parse_stream(std::basic_istream<char_type>& is)
    {
        return parse(is);
    }
    static basic_json parse_stream(std::basic_istream<char_type>& is, parse_error_handler& err_handler)
    {
        return parse(is,err_handler);
    }

    static basic_json parse_string(const string_type& s)
    {
        return parse(s);
    }

    static basic_json parse_string(const string_type& s, parse_error_handler& err_handler)
    {
        return parse(s,err_handler);
    }

    void resize_array(size_t n)
    {
        resize(n);
    }

    template <class T>
    void resize_array(size_t n, T val)
    {
        resize(n,val);
    }

    object_iterator begin_members()
    {
        return object_range().begin();
    }

    const_object_iterator begin_members() const
    {
        return object_range().begin();
    }

    object_iterator end_members()
    {
        return object_range().end();
    }

    const_object_iterator end_members() const
    {
        return object_range().end();
    }

    array_iterator begin_elements()
    {
        return array_range().begin();
    }

    const_array_iterator begin_elements() const
    {
        return array_range().begin();
    }

    array_iterator end_elements()
    {
        return array_range().end();
    }

    const_array_iterator end_elements() const
    {
        return array_range().end();
    }

    const basic_json& get(const string_view_type& name) const
    {
        static const basic_json a_null = null_type();

        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return a_null;
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                return it != object_range().end() ? it->value() : a_null;
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    bool is_longlong() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::integer_t;
    }

    bool is_ulonglong() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == json_type_tag::uinteger_t;
    }

    long long as_longlong() const
    {
        return as_integer();
    }

    unsigned long long as_ulonglong() const
    {
        return as_uinteger();
    }

    int as_int() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return static_cast<int>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<int>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<int>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an int");
        }
    }

    unsigned int as_uint() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return static_cast<unsigned int>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<unsigned int>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<unsigned int>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned int");
        }
    }

    long as_long() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return static_cast<long>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<long>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<long>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a long");
        }
    }

    unsigned long as_ulong() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::double_t:
            return static_cast<unsigned long>(var_.double_data_cast()->value());
        case json_type_tag::integer_t:
            return static_cast<unsigned long>(var_.integer_data_cast()->value());
        case json_type_tag::uinteger_t:
            return static_cast<unsigned long>(var_.uinteger_data_cast()->value());
        case json_type_tag::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned long");
        }
    }

    bool has_member(const key_storage_type& name) const
    {
        switch (var_.type_id())
        {
        case json_type_tag::object_t:
            {
                const_object_iterator it = object_value().find(name);
                return it != object_range().end();
            }
            break;
        default:
            return false;
        }
    }

    void remove_range(size_t from_index, size_t to_index)
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            array_value().remove_range(from_index, to_index);
            break;
        default:
            break;
        }
    }
    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void remove(const string_view_type& name)
    {
        erase(name);
    }
    void remove_member(const string_view_type& name)
    {
        erase(name);
    }
    // Removes a member from an object value

    bool is_empty() const JSONCONS_NOEXCEPT
    {
        return empty();
    }
    bool is_numeric() const JSONCONS_NOEXCEPT
    {
        return is_number();
    }

    template<int size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array()
    {
        return make_array();
    }
    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n)
    {
        return make_array(n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n, T val)
    {
        return make_array(n,val);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n)
    {
        return make_array<2>(m, n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n, T val)
    {
        return make_array<2>(m, n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k)
    {
        return make_array<3>(m, n, k);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k, T val)
    {
        return make_array<3>(m, n, k, val);
    }
    range<object_iterator> members()
    {
        return object_range();
    }

    range<const_object_iterator> members() const
    {
        return object_range();
    }

    range<array_iterator> elements()
    {
        return array_range();
    }

    range<const_array_iterator> elements() const
    {
        return array_range();
    }
#endif

    range<object_iterator> object_range()
    {
        static basic_json empty_object = object();
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return range<object_iterator>(empty_object.object_range().begin(), empty_object.object_range().end());
        case json_type_tag::object_t:
            return range<object_iterator>(object_value().begin(),object_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    range<const_object_iterator> object_range() const
    {
        static const basic_json empty_object = object();
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            return range<const_object_iterator>(empty_object.object_range().begin(), empty_object.object_range().end());
        case json_type_tag::object_t:
            return range<const_object_iterator>(object_value().begin(),object_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    range<array_iterator> array_range()
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return range<array_iterator>(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    range<const_array_iterator> array_range() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return range<const_array_iterator>(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    array& array_value() 
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return var_.array_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad array cast");
            break;
        }
    }

    const array& array_value() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::array_t:
            return var_.array_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad array cast");
            break;
        }
    }

    object& object_value()
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            create_object_implicitly();
            // FALLTHRU
        case json_type_tag::object_t:
            return var_.object_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad object cast");
            break;
        }
    }

    const object& object_value() const
    {
        switch (var_.type_id())
        {
        case json_type_tag::empty_object_t:
            const_cast<basic_json*>(this)->create_object_implicitly(); // HERE
            // FALLTHRU
        case json_type_tag::object_t:
            return var_.object_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad object cast");
            break;
        }
    }

private:

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const basic_json& o)
    {
        o.dump(os);
        return os;
    }

    friend std::basic_istream<char_type>& operator<<(std::basic_istream<char_type>& is, basic_json& o)
    {
        json_decoder<basic_json> handler;
        basic_json_reader<char_type> reader(is, handler);
        reader.read_next();
        reader.check_done();
        if (!handler.is_valid())
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
        }
        o = handler.get_result();
        return is;
    }
};

template <class Json>
void swap(typename Json::key_value_pair_type& a, typename Json::key_value_pair_type& b)
{
    a.swap(b);
}

template<class CharT,class ImplementationPolicy,class Allocator>
basic_json<CharT,ImplementationPolicy,Allocator> basic_json<CharT,ImplementationPolicy,Allocator>::parse(std::basic_istream<char_type>& is)
{
    parse_error_handler_type err_handler;
    return parse(is,err_handler);
}

template<class CharT,class ImplementationPolicy,class Allocator>
basic_json<CharT,ImplementationPolicy,Allocator> basic_json<CharT,ImplementationPolicy,Allocator>::parse(std::basic_istream<char_type>& is, 
                                                                                            parse_error_handler& err_handler)
{
    json_decoder<basic_json<CharT,ImplementationPolicy,Allocator>> handler;
    basic_json_reader<char_type> reader(is, handler, err_handler);
    reader.read_next();
    reader.check_done();
    if (!handler.is_valid())
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
    }
    return handler.get_result();
}

template <class Json>
std::basic_istream<typename Json::char_type>& operator>>(std::basic_istream<typename Json::char_type>& is, Json& o)
{
    json_decoder<Json> handler;
    basic_json_reader<typename Json::char_type> reader(is, handler);
    reader.read_next();
    reader.check_done();
    if (!handler.is_valid())
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
    }
    o = handler.get_result();
    return is;
}

template<class Json>
class json_printable
{
public:
    typedef typename Json::char_type char_type;

    json_printable(const Json& o,
                   bool is_pretty_print)
       : o_(&o), is_pretty_print_(is_pretty_print)
    {
    }

    json_printable(const Json& o,
                   bool is_pretty_print,
                   const basic_serialization_options<char_type>& options)
       : o_(&o), is_pretty_print_(is_pretty_print), options_(options)
    {
        ;
    }

    void dump(std::basic_ostream<char_type>& os) const
    {
        o_->dump(os, options_, is_pretty_print_);
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_printable<Json>& o)
    {
        o.dump(os);
        return os;
    }

    const Json *o_;
    bool is_pretty_print_;
    basic_serialization_options<char_type> options_;
private:
    json_printable();
};

template<class Json>
json_printable<Json> print(const Json& val)
{
    return json_printable<Json>(val,false);
}

template<class Json>
json_printable<Json> print(const Json& val,
                            const basic_serialization_options<typename Json::char_type>& options)
{
    return json_printable<Json>(val, false, options);
}

template<class Json>
json_printable<Json> pretty_print(const Json& val)
{
    return json_printable<Json>(val,true);
}

template<class Json>
json_printable<Json> pretty_print(const Json& val,
                                   const basic_serialization_options<typename Json::char_type>& options)
{
    return json_printable<Json>(val, true, options);
}

typedef basic_json<char,sorted_policy,std::allocator<char>> json;
typedef basic_json<wchar_t,sorted_policy,std::allocator<wchar_t>> wjson;
typedef basic_json<char, preserve_order_policy, std::allocator<char>> ojson;
typedef basic_json<wchar_t, preserve_order_policy, std::allocator<wchar_t>> wojson;

#if !defined(JSONCONS_NO_DEPRECATED)
typedef basic_json<wchar_t, preserve_order_policy, std::allocator<wchar_t>> owjson;
typedef json_decoder<json> json_deserializer;
typedef json_decoder<wjson> wjson_deserializer;
typedef json_decoder<ojson> ojson_deserializer;
typedef json_decoder<wojson> wojson_deserializer;
#endif

#if defined(JSONCONS_HAS_USER_DEFINED_LITERALS)
namespace literals {

inline 
jsoncons::json operator "" _json(const char* s, std::size_t n)
{
    return jsoncons::json::parse(s, n);
}

inline 
jsoncons::wjson operator "" _json(const wchar_t* s, std::size_t n)
{
    return jsoncons::wjson::parse(s, n);
}

inline
jsoncons::ojson operator "" _ojson(const char* s, std::size_t n)
{
    return jsoncons::ojson::parse(s, n);
}

inline
jsoncons::wojson operator "" _ojson(const wchar_t* s, std::size_t n)
{
    return jsoncons::wojson::parse(s, n);
}

}
#endif

}


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
