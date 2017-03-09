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
#include <jsoncons/jsoncons.hpp>
#include <jsoncons/json_traits.hpp>
#include <jsoncons/json_container.hpp>
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

enum class value_type : uint8_t 
{
    empty_object_t,
    small_string_t,
    double_t,
    integer_t,
    uinteger_t,
    bool_t,
    null_t,
    string_t,
    object_t,
    array_t
};

template <class CharT, 
          class JsonTraits = json_traits<CharT>, 
          class Allocator = std::allocator<CharT>>
class basic_json
{
public:

    typedef Allocator allocator_type;

    typedef JsonTraits json_traits_type;

    typedef typename JsonTraits::parse_error_handler_type parse_error_handler_type;

    typedef CharT char_type;
    typedef typename json_traits_type::char_traits_type char_traits_type;

#if !defined(JSONCONS_HAS_STRING_VIEW)
    typedef Basic_string_view_<char_type,char_traits_type> string_view_type;
#else
    typedef std::basic_string_view<char_type,char_traits_type> string_view_type;
#endif
    // string_type is for interface only, not storage 
    typedef std::basic_string<CharT,char_traits_type> string_type;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<char_type> char_allocator_type;

    using key_storage_type = typename json_traits_type::template key_storage<char_allocator_type>;

    using string_storage_type = typename json_traits_type::template string_storage<char_allocator_type>;

    typedef basic_json<CharT,JsonTraits,Allocator> json_type;
    typedef key_value_pair<key_storage_type,json_type> kvp_type;

#if !defined(JSONCONS_NO_DEPRECATED)
    typedef kvp_type member_type;
#endif

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<json_type> val_allocator_type;
    using array_storage_type = typename json_traits_type::template array_storage<json_type, val_allocator_type>;

    typedef json_array<json_type> array;

    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<kvp_type > kvp_allocator_type;

    using object_storage_type = typename json_traits_type::template object_storage<kvp_type , kvp_allocator_type>;
    typedef json_object<key_storage_type,json_type,json_traits_type::preserve_order> object;


    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<array> array_allocator;
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<object> object_allocator;

    typedef jsoncons::null_type null_type;

    typedef typename object::iterator object_iterator;
    typedef typename object::const_iterator const_object_iterator;
    typedef typename array::iterator array_iterator;
    typedef typename array::const_iterator const_array_iterator;

    struct variant
    {
        struct base_data
        {
            value_type type_id_;

            base_data(value_type id)
                : type_id_(id)
            {}
        };

        struct null_data : public base_data
        {
            null_data()
                : base_data(value_type::null_t)
            {
            }
        };

        struct empty_object_data : public base_data
        {
            empty_object_data()
                : base_data(value_type::empty_object_t)
            {
            }
        };

        struct bool_data : public base_data
        {
            bool val_;

            bool_data(bool val)
                : base_data(value_type::bool_t),val_(val)
            {
            }

            bool_data(const bool_data& val)
                : base_data(value_type::bool_t),val_(val.val_)
            {
            }

            bool value() const
            {
                return val_;
            }

        };

        struct integer_data : public base_data
        {
            int64_t val_;

            integer_data(int64_t val)
                : base_data(value_type::integer_t),val_(val)
            {
            }

            integer_data(const integer_data& val)
                : base_data(value_type::integer_t),val_(val.val_)
            {
            }

            int64_t value() const
            {
                return val_;
            }
        };

        struct uinteger_data : public base_data
        {
            uint64_t val_;

            uinteger_data(uint64_t val)
                : base_data(value_type::uinteger_t),val_(val)
            {
            }

            uinteger_data(const uinteger_data& val)
                : base_data(value_type::uinteger_t),val_(val.val_)
            {
            }

            uint64_t value() const
            {
                return val_;
            }
        };

        struct double_data : public base_data
        {
            uint8_t precision_;
            double val_;

            double_data(double val, uint8_t precision)
                : base_data(value_type::double_t), 
                  precision_(precision), 
                  val_(val)
            {
            }

            double_data(const double_data& val)
                : base_data(value_type::double_t),
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

        struct small_string_data : public base_data
        {
            static const size_t capacity = 14/sizeof(char_type);
            static const size_t max_length = (14 / sizeof(char_type)) - 1;

            uint8_t length_;
            char_type data_[capacity];

            small_string_data(const char_type* p, uint8_t length)
                : base_data(value_type::small_string_t), length_(length)
            {
                JSONCONS_ASSERT(length <= max_length);
                std::memcpy(data_,p,length*sizeof(char_type));
                data_[length] = 0;
            }

            small_string_data(const small_string_data& val)
                : base_data(value_type::small_string_t), length_(val.length_)
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
        struct string_data : public base_data
        {
            typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<Json_string_<json_type>> string_holder_allocator_type;
            typedef typename std::allocator_traits<string_holder_allocator_type>::pointer pointer;

            pointer ptr_;

            template <typename... Args>
            void create(string_holder_allocator_type allocator, Args&& ... args)
            {
                typename std::allocator_traits<Allocator>:: template rebind_alloc<Json_string_<json_type>> alloc(allocator);
                ptr_ = alloc.allocate(1);
                try
                {
                    std::allocator_traits<string_holder_allocator_type>:: template rebind_traits<Json_string_<json_type>>::construct(alloc, to_plain_pointer(ptr_), std::forward<Args>(args)...);
                }
                catch (...)
                {
                    alloc.deallocate(ptr_,1);
                    throw;
                }
            }

            string_data(const Json_string_<json_type>& val)
                : base_data(value_type::string_t)
            {
                create(val.get_allocator(), val);
            }

            string_data(pointer ptr)
                : base_data(value_type::string_t)
            {
                ptr_ = ptr;
            }

            string_data(const Json_string_<json_type>& val, const Allocator& a)
                : base_data(value_type::string_t)
            {
                create(string_holder_allocator_type(a), val, a);
            }

            string_data(const string_data & val)
                : base_data(value_type::string_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            string_data(const string_data & val, const Allocator& a)
                : base_data(value_type::string_t)
            {
                create(string_holder_allocator_type(a), *(val.ptr_), a);
            }

            template<class InputIterator>
            string_data(InputIterator first, InputIterator last, const Allocator& a)
                : base_data(value_type::string_t)
            {
                create(string_holder_allocator_type(a), first, last, a);
            }

            string_data(const char_type* data, size_t length, const Allocator& a)
                : base_data(value_type::string_t)
            {
                create(string_holder_allocator_type(a), data, length, a);
            }

            ~string_data()
            {
                typename std::allocator_traits<string_holder_allocator_type>:: template rebind_alloc<Json_string_<json_type>> alloc(ptr_->get_allocator());
                std::allocator_traits<string_holder_allocator_type>:: template rebind_traits<Json_string_<json_type>>::destroy(alloc, to_plain_pointer(ptr_));
                alloc.deallocate(ptr_,1);
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

        struct object_data : public base_data
        {
            typedef typename std::allocator_traits<object_allocator>::pointer pointer;
            pointer ptr_;

            template <typename... Args>
            void create(Allocator allocator, Args&& ... args)
            {
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

            explicit object_data(const Allocator& a)
                : base_data(value_type::object_t)
            {
                create(a,a);
            }

            explicit object_data(pointer ptr)
                : base_data(value_type::object_t)
            {
                ptr_ = ptr;
            }

            explicit object_data(const object & val)
                : base_data(value_type::object_t)
            {
                create(val.get_allocator(), val);
            }

            explicit object_data(const object & val, const Allocator& a)
                : base_data(value_type::object_t)
            {
                create(object_allocator(a), val, a);
            }

            explicit object_data(const object_data & val)
                : base_data(value_type::object_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            explicit object_data(const object_data & val, const Allocator& a)
                : base_data(value_type::object_t)
            {
                create(object_allocator(a), *(val.ptr_), a);
            }

            ~object_data()
            {
                typename std::allocator_traits<Allocator>:: template rebind_alloc<object> alloc(ptr_->get_allocator());
                std::allocator_traits<Allocator>:: template rebind_traits<object>::destroy(alloc, to_plain_pointer(ptr_));
                alloc.deallocate(ptr_,1);
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
    public:
        struct array_data : public base_data
        {
            typedef typename std::allocator_traits<array_allocator>::pointer pointer;
            pointer ptr_;

            template <typename... Args>
            void create(array_allocator allocator, Args&& ... args)
            {
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

            allocator_type get_allocator() const
            {
                return ptr_->get_allocator();
            }

            array_data(const array& val)
                : base_data(value_type::array_t)
            {
                create(val.get_allocator(), val);
            }

            array_data(pointer ptr)
                : base_data(value_type::array_t)
            {
                ptr_ = ptr;
            }

            array_data(const array& val, const Allocator& a)
                : base_data(value_type::array_t)
            {
                create(array_allocator(a), val, a);
            }

            array_data(const array_data & val)
                : base_data(value_type::array_t)
            {
                create(val.ptr_->get_allocator(), *(val.ptr_));
            }

            array_data(const array_data & val, const Allocator& a)
                : base_data(value_type::array_t)
            {
                create(array_allocator(a), *(val.ptr_), a);
            }

            template<class InputIterator>
            array_data(InputIterator first, InputIterator last, const Allocator& a)
                : base_data(value_type::array_t)
            {
                create(array_allocator(a), first, last, a);
            }

            ~array_data()
            {
                typename std::allocator_traits<array_allocator>:: template rebind_alloc<array> alloc(ptr_->get_allocator());
                std::allocator_traits<array_allocator>:: template rebind_traits<array>::destroy(alloc, to_plain_pointer(ptr_));
                alloc.deallocate(ptr_,1);
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

    private:
        static const size_t data_size = static_max<sizeof(uinteger_data),sizeof(double_data),sizeof(small_string_data), sizeof(string_data), sizeof(array_data), sizeof(object_data)>::value;
        static const size_t data_align = static_max<JSONCONS_ALIGNOF(uinteger_data),JSONCONS_ALIGNOF(double_data),JSONCONS_ALIGNOF(small_string_data),JSONCONS_ALIGNOF(string_data),JSONCONS_ALIGNOF(array_data),JSONCONS_ALIGNOF(object_data)>::value;

        typedef typename std::aligned_storage<data_size,data_align>::type data_t;

        data_t data_;
    public:
        variant()
        {
            new(reinterpret_cast<void*>(&data_))empty_object_data();
        }

        variant(const Allocator& a)
        {
            new(reinterpret_cast<void*>(&data_))object_data(a);
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
            Init_rv_(std::forward<variant&&>(val));
        }

        variant(variant&& val, const Allocator& allocator) JSONCONS_NOEXCEPT
        {
            Init_rv_(std::forward<variant&&>(val), allocator,
                     typename std::allocator_traits<Allocator>::propagate_on_container_move_assignment());
        }

        explicit variant(null_type)
        {
            new(reinterpret_cast<void*>(&data_))null_data();
        }
        explicit variant(bool val)
        {
            new(reinterpret_cast<void*>(&data_))bool_data(val);
        }
        explicit variant(int64_t val)
        {
            new(reinterpret_cast<void*>(&data_))integer_data(val);
        }
        explicit variant(uint64_t val)
        {
            new(reinterpret_cast<void*>(&data_))uinteger_data(val);
        }
        variant(double val)
        {
            new(reinterpret_cast<void*>(&data_))double_data(val,0);
        }
        variant(double val, uint8_t precision)
        {
            new(reinterpret_cast<void*>(&data_))double_data(val,precision);
        }
        variant(const char_type* s, size_t length)
        {
            if (length <= small_string_data::max_length)
            {
                new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
            }
            else
            {
                new(reinterpret_cast<void*>(&data_))string_data(s, length, char_allocator_type());
            }
        }
        variant(const char_type* s)
        {
            size_t length = char_traits_type::length(s);
            if (length <= small_string_data::max_length)
            {
                new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
            }
            else
            {
                new(reinterpret_cast<void*>(&data_))string_data(s, length, char_allocator_type());
            }
        }

        variant(const char_type* s, const Allocator& alloc)
        {
            size_t length = char_traits_type::length(s);
            if (length <= small_string_data::max_length)
            {
                new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
            }
            else
            {
                new(reinterpret_cast<void*>(&data_))string_data(s, length, alloc);
            }
        }

        variant(const char_type* s, size_t length, const Allocator& alloc)
        {
            if (length <= small_string_data::max_length)
            {
                new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
            }
            else
            {
                new(reinterpret_cast<void*>(&data_))string_data(s, length, alloc);
            }
        }
        variant(const object& val)
        {
            new(reinterpret_cast<void*>(&data_))object_data(val);
        }
        variant(const object& val, const Allocator& alloc)
        {
            new(reinterpret_cast<void*>(&data_))object_data(val, alloc);
        }
        variant(const array& val)
        {
            new(reinterpret_cast<void*>(&data_))array_data(val);
        }
        variant(const array& val, const Allocator& alloc)
        {
            new(reinterpret_cast<void*>(&data_))array_data(val,alloc);
        }
        template<class InputIterator>
        variant(InputIterator first, InputIterator last, const Allocator& a)
        {
            new(reinterpret_cast<void*>(&data_))array_data(first, last, a);
        }

        ~variant()
        {
            Destroy_();
        }

        void Destroy_()
        {
            switch (type_id())
            {
            case value_type::string_t:
                reinterpret_cast<string_data*>(&data_)->~string_data();
                break;
            case value_type::object_t:
                reinterpret_cast<object_data*>(&data_)->~object_data();
                break;
            case value_type::array_t:
                reinterpret_cast<array_data*>(&data_)->~array_data();
                break;
            default:
                break;
            }
        }

        variant& operator=(const variant& val)
        {
            if (this != &val)
            {
                Destroy_();
                switch (val.type_id())
                {
                case value_type::null_t:
                    new(reinterpret_cast<void*>(&data_))null_data();
                    break;
                case value_type::empty_object_t:
                    new(reinterpret_cast<void*>(&data_))empty_object_data();
                    break;
                case value_type::double_t:
                    new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
                    break;
                case value_type::integer_t:
                    new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
                    break;
                case value_type::uinteger_t:
                    new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
                    break;
                case value_type::bool_t:
                    new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
                    break;
                case value_type::small_string_t:
                    new(reinterpret_cast<void*>(&data_))small_string_data(*(val.small_string_data_cast()));
                    break;
                case value_type::string_t:
                    new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()));
                    break;
                case value_type::object_t:
                    new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()));
                    break;
                case value_type::array_t:
                    new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()));
                    break;
                default:
                    break;
                }
            }
            return *this;
        }

        variant& operator=(variant&& val) JSONCONS_NOEXCEPT
        {
            if (this != &val)
            {
                Destroy_();
                new(reinterpret_cast<void*>(&data_))null_data();
                swap(val);
            }
            return *this;
        }

        value_type type_id() const
        {
            return reinterpret_cast<const base_data*>(&data_)->type_id_;
        }

        const null_data* null_data_cast() const
        {
            return reinterpret_cast<const null_data*>(&data_);
        }

        const empty_object_data* empty_object_data_cast() const
        {
            return reinterpret_cast<const empty_object_data*>(&data_);
        }

        const bool_data* bool_data_cast() const
        {
            return reinterpret_cast<const bool_data*>(&data_);
        }

        const integer_data* integer_data_cast() const
        {
            return reinterpret_cast<const integer_data*>(&data_);
        }

        const uinteger_data* uinteger_data_cast() const
        {
            return reinterpret_cast<const uinteger_data*>(&data_);
        }

        const double_data* double_data_cast() const
        {
            return reinterpret_cast<const double_data*>(&data_);
        }

        const small_string_data* small_string_data_cast() const
        {
            return reinterpret_cast<const small_string_data*>(&data_);
        }

        string_data* string_data_cast()
        {
            return reinterpret_cast<string_data*>(&data_);
        }

        const string_data* string_data_cast() const
        {
            return reinterpret_cast<const string_data*>(&data_);
        }

        object_data* object_data_cast()
        {
            return reinterpret_cast<object_data*>(&data_);
        }

        const object_data* object_data_cast() const
        {
            return reinterpret_cast<const object_data*>(&data_);
        }

        array_data* array_data_cast()
        {
            return reinterpret_cast<array_data*>(&data_);
        }

        const array_data* array_data_cast() const
        {
            return reinterpret_cast<const array_data*>(&data_);
        }

        string_view_type as_string_view() const
        {
            switch (type_id())
            {
            case value_type::small_string_t:
                return string_view_type(small_string_data_cast()->data(),small_string_data_cast()->length());
            case value_type::string_t:
                return string_view_type(string_data_cast()->data(),string_data_cast()->length());
            default:
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a string");
            }
        }

        bool operator==(const variant& rhs) const
        {
            if (this == &rhs)
            {
                return true;
            }

            const value_type id = type_id();
            const value_type rhs_id = rhs.type_id();

            if (id == rhs_id)
            {
                switch (id)
                {
                case value_type::null_t:
                    return true;
                case value_type::empty_object_t:
                    return true;
                case value_type::double_t:
                    return double_data_cast()->val_ == rhs.double_data_cast()->val_;
                case value_type::integer_t:
                    return integer_data_cast()->val_ == rhs.integer_data_cast()->val_;
                case value_type::uinteger_t:
                    return uinteger_data_cast()->val_ == rhs.uinteger_data_cast()->val_;
                case value_type::bool_t:
                    return bool_data_cast()->val_ == rhs.bool_data_cast()->val_;
                case value_type::small_string_t:
                case value_type::string_t:
                    return as_string_view() == rhs.as_string_view();
                case value_type::object_t:
                    return object_data_cast()->value() == rhs.object_data_cast()->value();
                case value_type::array_t:
                    return array_data_cast()->value() == rhs.array_data_cast()->value();
                default:
                    return false;
                }
            }

            switch (id)
            {
            case value_type::integer_t:
                if (rhs_id == value_type::double_t)
                {
                    return static_cast<double>(integer_data_cast()->val_) == rhs.double_data_cast()->val_;
                }
                else if (rhs_id == value_type::uinteger_t && integer_data_cast()->val_ >= 0)
                {
                    return static_cast<uint64_t>(integer_data_cast()->val_) == rhs.uinteger_data_cast()->val_;
                }
                break;
            case value_type::uinteger_t:
                if (rhs_id == value_type::double_t)
                {
                    return static_cast<double>(uinteger_data_cast()->val_) == rhs.double_data_cast()->val_;
                }
                else if (rhs_id == value_type::integer_t && rhs.integer_data_cast()->val_ >= 0)
                {
                    return uinteger_data_cast()->val_ == static_cast<uint64_t>(rhs.integer_data_cast()->val_);
                }
                break;
            case value_type::double_t:
                if (rhs_id == value_type::integer_t)
                {
                    return double_data_cast()->val_ == static_cast<double>(rhs.integer_data_cast()->val_);
                }
                else if (rhs_id == value_type::uinteger_t)
                {
                    return double_data_cast()->val_ == static_cast<double>(rhs.uinteger_data_cast()->val_);
                }
                break;
            case value_type::empty_object_t:
                if (rhs_id == value_type::object_t && rhs.object_data_cast()->ptr_->size() == 0)
                {
                    return true;
                }
                break;
            case value_type::object_t:
                if (rhs_id == value_type::empty_object_t && object_data_cast()->ptr_->size() == 0)
                {
                    return true;
                }
                break;
            case value_type::small_string_t:
            case value_type::string_t:
                return as_string_view() == rhs.as_string_view();
                break;
            default:
                break;
            }
            
            return false;
        }

        bool operator!=(const variant& rhs) const
        {
            return !(*this == rhs);
        }

        void swap(variant& rhs) JSONCONS_NOEXCEPT
        {
            if (this != &rhs)
            {
                switch (type_id())
                {
                case value_type::string_t:
                    {
                        auto ptr = string_data_cast()->ptr_;
                        switch (rhs.type_id())
                        {
                        case value_type::object_t:
                            new(reinterpret_cast<void*>(&data_))object_data(rhs.object_data_cast()->ptr_);
                            break;
                        case value_type::array_t:
                            new(reinterpret_cast<void*>(&data_))array_data(rhs.array_data_cast()->ptr_);
                            break;
                        case value_type::string_t:
                            new(reinterpret_cast<void*>(&data_))string_data(rhs.string_data_cast()->ptr_);
                            break;
                        case value_type::null_t:
                            new(reinterpret_cast<void*>(&data_))null_data();
                            break;
                        case value_type::empty_object_t:
                            new(reinterpret_cast<void*>(&data_))empty_object_data();
                            break;
                        case value_type::double_t:
                            new(reinterpret_cast<void*>(&data_))double_data(*(rhs.double_data_cast()));
                            break;
                        case value_type::integer_t:
                            new(reinterpret_cast<void*>(&data_))integer_data(*(rhs.integer_data_cast()));
                            break;
                        case value_type::uinteger_t:
                            new(reinterpret_cast<void*>(&data_))uinteger_data(*(rhs.uinteger_data_cast()));
                            break;
                        case value_type::bool_t:
                            new(reinterpret_cast<void*>(&data_))bool_data(*(rhs.bool_data_cast()));
                            break;
                        case value_type::small_string_t:
                            new(reinterpret_cast<void*>(&data_))small_string_data(*(rhs.small_string_data_cast()));
                            break;
                        default:
                            break;
                        }
                        new(reinterpret_cast<void*>(&(rhs.data_)))string_data(ptr);
                    }
                    break;
                case value_type::object_t:
                    {
                        auto ptr = object_data_cast()->ptr_;
                        switch (rhs.type_id())
                        {
                        case value_type::object_t:
                            new(reinterpret_cast<void*>(&data_))object_data(rhs.object_data_cast()->ptr_);
                            break;
                        case value_type::array_t:
                            new(reinterpret_cast<void*>(&data_))array_data(rhs.array_data_cast()->ptr_);
                            break;
                        case value_type::string_t:
                            new(reinterpret_cast<void*>(&data_))string_data(rhs.string_data_cast()->ptr_);
                            break;
                        case value_type::null_t:
                            new(reinterpret_cast<void*>(&data_))null_data();
                            break;
                        case value_type::empty_object_t:
                            new(reinterpret_cast<void*>(&data_))empty_object_data();
                            break;
                        case value_type::double_t:
                            new(reinterpret_cast<void*>(&data_))double_data(*(rhs.double_data_cast()));
                            break;
                        case value_type::integer_t:
                            new(reinterpret_cast<void*>(&data_))integer_data(*(rhs.integer_data_cast()));
                            break;
                        case value_type::uinteger_t:
                            new(reinterpret_cast<void*>(&data_))uinteger_data(*(rhs.uinteger_data_cast()));
                            break;
                        case value_type::bool_t:
                            new(reinterpret_cast<void*>(&data_))bool_data(*(rhs.bool_data_cast()));
                            break;
                        case value_type::small_string_t:
                            new(reinterpret_cast<void*>(&data_))small_string_data(*(rhs.small_string_data_cast()));
                            break;
                        default:
                            break;
                        }
                        new(reinterpret_cast<void*>(&(rhs.data_)))object_data(ptr);
                    }
                    break;
                case value_type::array_t:
                    {
                        auto ptr = array_data_cast()->ptr_;
                        switch (rhs.type_id())
                        {
                        case value_type::object_t:
                            new(reinterpret_cast<void*>(&data_))object_data(rhs.object_data_cast()->ptr_);
                            break;
                        case value_type::array_t:
                            new(reinterpret_cast<void*>(&data_))array_data(rhs.array_data_cast()->ptr_);
                            break;
                        case value_type::string_t:
                            new(reinterpret_cast<void*>(&data_))string_data(rhs.string_data_cast()->ptr_);
                            break;
                        case value_type::null_t:
                            new(reinterpret_cast<void*>(&data_))null_data();
                            break;
                        case value_type::empty_object_t:
                            new(reinterpret_cast<void*>(&data_))empty_object_data();
                            break;
                        case value_type::double_t:
                            new(reinterpret_cast<void*>(&data_))double_data(*(rhs.double_data_cast()));
                            break;
                        case value_type::integer_t:
                            new(reinterpret_cast<void*>(&data_))integer_data(*(rhs.integer_data_cast()));
                            break;
                        case value_type::uinteger_t:
                            new(reinterpret_cast<void*>(&data_))uinteger_data(*(rhs.uinteger_data_cast()));
                            break;
                        case value_type::bool_t:
                            new(reinterpret_cast<void*>(&data_))bool_data(*(rhs.bool_data_cast()));
                            break;
                        case value_type::small_string_t:
                            new(reinterpret_cast<void*>(&data_))small_string_data(*(rhs.small_string_data_cast()));
                            break;
                        default:
                            break;
                        }
                        new(reinterpret_cast<void*>(&(rhs.data_)))array_data(ptr);
                    }
                    break;
                default:
                    switch (rhs.type_id())
                    {
                    case value_type::string_t:
                        {
                            auto ptr = rhs.string_data_cast()->ptr_;
                            switch (type_id())
                            {
                            case value_type::null_t:
                                new(reinterpret_cast<void*>(&rhs.data_))null_data();
                                break;
                            case value_type::empty_object_t:
                                new(reinterpret_cast<void*>(&rhs.data_))empty_object_data();
                                break;
                            case value_type::double_t:
                                new(reinterpret_cast<void*>(&rhs.data_))double_data(*(double_data_cast()));
                                break;
                            case value_type::integer_t:
                                new(reinterpret_cast<void*>(&rhs.data_))integer_data(*(integer_data_cast()));
                                break;
                            case value_type::uinteger_t:
                                new(reinterpret_cast<void*>(&rhs.data_))uinteger_data(*(uinteger_data_cast()));
                                break;
                            case value_type::bool_t:
                                new(reinterpret_cast<void*>(&rhs.data_))bool_data(*(bool_data_cast()));
                                break;
                            case value_type::small_string_t:
                                new(reinterpret_cast<void*>(&rhs.data_))small_string_data(*(small_string_data_cast()));
                                break;
                            default:
                                break;
                            }
                            new(reinterpret_cast<void*>(&data_))string_data(ptr);
                        }
                        break;
                    case value_type::object_t:
                        {
                            auto ptr = rhs.object_data_cast()->ptr_;
                            switch (type_id())
                            {
                            case value_type::null_t:
                                new(reinterpret_cast<void*>(&rhs.data_))null_data();
                                break;
                            case value_type::empty_object_t:
                                new(reinterpret_cast<void*>(&rhs.data_))empty_object_data();
                                break;
                            case value_type::double_t:
                                new(reinterpret_cast<void*>(&rhs.data_))double_data(*(double_data_cast()));
                                break;
                            case value_type::integer_t:
                                new(reinterpret_cast<void*>(&rhs.data_))integer_data(*(integer_data_cast()));
                                break;
                            case value_type::uinteger_t:
                                new(reinterpret_cast<void*>(&rhs.data_))uinteger_data(*(uinteger_data_cast()));
                                break;
                            case value_type::bool_t:
                                new(reinterpret_cast<void*>(&rhs.data_))bool_data(*(bool_data_cast()));
                                break;
                            case value_type::small_string_t:
                                new(reinterpret_cast<void*>(&rhs.data_))small_string_data(*(small_string_data_cast()));
                                break;
                            default:
                                break;
                            }
                            new(reinterpret_cast<void*>(&data_))object_data(ptr);
                        }
                        break;
                    case value_type::array_t:
                        {
                            auto ptr = rhs.array_data_cast()->ptr_;
                            switch (type_id())
                            {
                            case value_type::null_t:
                                new(reinterpret_cast<void*>(&rhs.data_))null_data();
                                break;
                            case value_type::empty_object_t:
                                new(reinterpret_cast<void*>(&rhs.data_))empty_object_data();
                                break;
                            case value_type::double_t:
                                new(reinterpret_cast<void*>(&rhs.data_))double_data(*(double_data_cast()));
                                break;
                            case value_type::integer_t:
                                new(reinterpret_cast<void*>(&rhs.data_))integer_data(*(integer_data_cast()));
                                break;
                            case value_type::uinteger_t:
                                new(reinterpret_cast<void*>(&rhs.data_))uinteger_data(*(uinteger_data_cast()));
                                break;
                            case value_type::bool_t:
                                new(reinterpret_cast<void*>(&rhs.data_))bool_data(*(bool_data_cast()));
                                break;
                            case value_type::small_string_t:
                                new(reinterpret_cast<void*>(&rhs.data_))small_string_data(*(small_string_data_cast()));
                                break;
                            default:
                                break;
                            }
                            new(reinterpret_cast<void*>(&data_))array_data(ptr);
                        }
                        break;
                    default:
                        {
                            std::swap(data_,rhs.data_);
                        }
                        break;
                    }
                }
            }
        }
    private:

        void Init_(const variant& val)
        {
            switch (val.type_id())
            {
            case value_type::null_t:
                new(reinterpret_cast<void*>(&data_))null_data();
                break;
            case value_type::empty_object_t:
                new(reinterpret_cast<void*>(&data_))empty_object_data();
                break;
            case value_type::double_t:
                new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
                break;
            case value_type::integer_t:
                new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
                break;
            case value_type::uinteger_t:
                new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
                break;
            case value_type::bool_t:
                new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
                break;
            case value_type::small_string_t:
                new(reinterpret_cast<void*>(&data_))small_string_data(*(val.small_string_data_cast()));
                break;
            case value_type::string_t:
                new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()));
                break;
            case value_type::object_t:
                new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()));
                break;
            case value_type::array_t:
                new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()));
                break;
            default:
                break;
            }
        }

        void Init_(const variant& val, const Allocator& a)
        {
            switch (val.type_id())
            {
            case value_type::null_t:
            case value_type::empty_object_t:
            case value_type::double_t:
            case value_type::integer_t:
            case value_type::uinteger_t:
            case value_type::bool_t:
            case value_type::small_string_t:
                Init_(val);
                break;
            case value_type::string_t:
                new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()),a);
                break;
            case value_type::object_t:
                new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()),a);
                break;
            case value_type::array_t:
                new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()),a);
                break;
            default:
                break;
            }
        }

        void Init_rv_(variant&& val) JSONCONS_NOEXCEPT
        {
            switch (val.type_id())
            {
            case value_type::null_t:
            case value_type::empty_object_t:
            case value_type::double_t:
            case value_type::integer_t:
            case value_type::uinteger_t:
            case value_type::bool_t:
            case value_type::small_string_t:
                Init_(val);
                break;
            case value_type::string_t:
                {
                    new(reinterpret_cast<void*>(&data_))string_data(val.string_data_cast()->ptr_);
                    val.string_data_cast()->type_id_ = value_type::null_t;
                }
                break;
            case value_type::object_t:
                {
                    new(reinterpret_cast<void*>(&data_))object_data(val.object_data_cast()->ptr_);
                    val.object_data_cast()->type_id_ = value_type::null_t;
                }
                break;
            case value_type::array_t:
                {
                    new(reinterpret_cast<void*>(&data_))array_data(val.array_data_cast()->ptr_);
                    val.array_data_cast()->type_id_ = value_type::null_t;
                }
                break;
            default:
                break;
            }
        }

        void Init_rv_(variant&& val, const Allocator& a, std::true_type) JSONCONS_NOEXCEPT
        {
            Init_rv_(std::forward<variant&&>(val));
        }

        void Init_rv_(variant&& val, const Allocator& a, std::false_type) JSONCONS_NOEXCEPT
        {
            switch (val.type_id())
            {
            case value_type::null_t:
            case value_type::empty_object_t:
            case value_type::double_t:
            case value_type::integer_t:
            case value_type::uinteger_t:
            case value_type::bool_t:
            case value_type::small_string_t:
                Init_(std::forward<variant&&>(val));
                break;
            case value_type::string_t:
                {
                    if (a == val.string_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant&&>(val), a, std::true_type());
                    }
                    else
                    {
                        Init_(val,a);
                    }
                }
                break;
            case value_type::object_t:
                {
                    if (a == val.object_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant&&>(val), a, std::true_type());
                    }
                    else
                    {
                        Init_(val,a);
                    }
                }
                break;
            case value_type::array_t:
                {
                    if (a == val.array_data_cast()->get_allocator())
                    {
                        Init_rv_(std::forward<variant&&>(val), a, std::true_type());
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
            : parent_(parent), key_(std::forward<key_storage_type&&>(name))
        {
        }

        json_type& evaluate() 
        {
            return parent_.evaluate(string_view_type(key_.data(),key_.size()));
        }

        const json_type& evaluate() const
        {
            return parent_.evaluate(string_view_type(key_.data(),key_.size()));
        }

        json_type& evaluate_with_default()
        {
            json_type& val = parent_.evaluate_with_default();
            auto it = val.find(string_view_type(key_.data(),key_.size()));
            if (it == val.object_range().end())
            {
                it = val.set_(val.object_range().begin(),std::move(key_),object(val.object_value().get_allocator()));            
            }
            return it->value();
        }

        json_type& evaluate(size_t index)
        {
            return evaluate().at(index);
        }

        const json_type& evaluate(size_t index) const
        {
            return evaluate().at(index);
        }

        json_type& evaluate(string_view_type index)
        {
            return evaluate().at(index);
        }

        const json_type& evaluate(string_view_type index) const
        {
            return evaluate().at(index);
        }
    public:

        friend class basic_json<CharT,JsonTraits,Allocator>;

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

        value_type type_id() const
        {
            return evaluate().type_id();
        }

        size_t count(string_view_type name) const
        {
            return evaluate().count(name);
        }

        bool has_key(string_view_type name) const
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

        template<class T>
        bool is() const
        {
            return evaluate().template is<T>();
        }

        bool is_string() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_string();
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

        template<class T>
        T as() const
        {
            return evaluate().template as<T>();
        }

        template<class T>
        typename std::enable_if<std::is_same<string_type,T>::value>::type 
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
            parent_.evaluate_with_default().set_(std::move(key_), std::forward<T&&>(val));
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

        json_type& operator[](size_t i)
        {
            return evaluate_with_default().at(i);
        }

        const json_type& operator[](size_t i) const
        {
            return evaluate().at(i);
        }

        json_proxy<proxy_type> operator[](string_view_type name)
        {
            return json_proxy<proxy_type>(*this,key_storage_type(name.begin(),name.end(),key_.get_allocator()));
        }

        const json_type& operator[](string_view_type name) const
        {
            return at(name);
        }

        json_type& at(string_view_type name)
        {
            return evaluate().at(name);
        }

        const json_type& at(string_view_type name) const
        {
            return evaluate().at(name);
        }

        const json_type& at(size_t index)
        {
            return evaluate().at(index);
        }

        const json_type& at(size_t index) const
        {
            return evaluate().at(index);
        }

        object_iterator find(string_view_type name)
        {
            return evaluate().find(name);
        }

        const_object_iterator find(string_view_type name) const
        {
            return evaluate().find(name);
        }

        template <class T>
        json_type get(string_view_type name, T&& default_val) const
        {
            return evaluate().get(name,std::forward<T>(default_val));
        }

        template <class T>
        T get_with_default(string_view_type name, const T& default_val) const
        {
            return evaluate().get_with_default(name,default_val);
        }

        const CharT* get_with_default(string_view_type name, const CharT* default_val) const
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

        void erase(object_iterator first, object_iterator last)
        {
            evaluate().erase(first, last);
        }
        // Remove a range of elements from an object 

        void erase(array_iterator first, array_iterator last)
        {
            evaluate().erase(first, last);
        }

        void erase(string_view_type name)
        {
            evaluate().erase(name);
        }

       // Remove a member from an object 

        template <class T>
        void set(string_view_type name, T&& value)
        {
            evaluate().set(name,std::forward<T&&>(value));
        }

        template <class T>
        void set_(key_storage_type&& name, T&& value)
        {
            evaluate().set_(std::forward<key_storage_type&&>(name),std::forward<T&&>(value));
        }

        template <class T>
        object_iterator set(object_iterator hint, string_view_type name, T&& value)
        {
            return evaluate().set(hint, name, std::forward<T&&>(value));
        }

        template <class T>
        object_iterator set_(object_iterator hint, key_storage_type&& name, T&& value)
        {
            return evaluate().set_(hint, std::forward<key_storage_type&&>(name), std::forward<T&&>(value));
        }

        template <class T>
        void add(T&& value)
        {
            evaluate_with_default().add(std::forward<T&&>(value));
        }

        template <class T>
        array_iterator add(const_array_iterator pos, T&& value)
        {
            return evaluate_with_default().add(pos, std::forward<T&&>(value));
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

        void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
        {
            evaluate().dump(os,options);
        }

        void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool indenting) const
        {
            evaluate().dump(os,options,indenting);
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

        void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool indenting) const
        {
            evaluate().write(os,options,indenting);
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

        void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool indenting) const
        {
            evaluate().to_stream(os,options,indenting);
        }
#endif
        void swap(json_type& val)
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

        const json_type& get(string_view_type name) const
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

        void add(size_t index, const json_type& value)
        {
            evaluate_with_default().add(index, value);
        }

        void add(size_t index, json_type&& value)
        {
            evaluate_with_default().add(index, std::forward<json_type&&>(value));
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
        void remove(string_view_type name)
        {
            evaluate().remove(name);
        }
        void remove_member(string_view_type name)
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

    static basic_json parse_stream(std::basic_istream<char_type>& is);
    static basic_json parse_stream(std::basic_istream<char_type>& is, basic_parse_error_handler<char_type>& err_handler);

    static basic_json parse(string_view_type s)
    {
        parse_error_handler_type err_handler;
        return parse(s,err_handler);
    }

    static basic_json parse(const char_type* s, size_t length)
    {
        parse_error_handler_type err_handler;
        return parse(s,length,err_handler);
    }

    static basic_json parse(string_view_type s, basic_parse_error_handler<char_type>& err_handler)
    {
        json_decoder<json_type> handler;
        basic_json_parser<char_type> parser(handler,err_handler);
        parser.set_source(s.data(),s.length());
        parser.skip_bom();
        parser.parse();
        parser.end_parse();
        parser.check_done();
        if (!handler.is_valid())
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json string");
        }
        return handler.get_result();
    }

    static basic_json parse(const char_type* s, size_t length, basic_parse_error_handler<char_type>& err_handler)
    {
        json_decoder<json_type> handler;
        basic_json_parser<char_type> parser(handler,err_handler);
        parser.set_source(s,length);
        parser.skip_bom();
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
                                 basic_parse_error_handler<char_type>& err_handler)
    {
        FILE* fp;

    #if !defined(JSONCONS_HAS_FOPEN_S)
        fp = std::fopen(filename.c_str(), "rb");
        if (fp == nullptr)
        {
            JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Cannot open file %s", filename);
        }
    #else
        errno_t err = fopen_s(&fp, filename.c_str(), "rb");
        if (err != 0) 
        {
            JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Cannot open file %s", filename);
        }
    #endif

        json_decoder<basic_json<CharT,JsonTraits,Allocator>> handler;
        try
        {
            // obtain file size:
            std::fseek (fp , 0 , SEEK_END);
            long size = std::ftell (fp);
            std::rewind(fp);

            if (size > 0)
            {
                std::vector<char_type> buffer(size);

                // copy the file into the buffer:
                size_t result = std::fread (buffer.data(),1,size,fp);
                if (result != static_cast<unsigned long long>(size))
                {
                    JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Error reading file %s", filename);
                }

                basic_json_parser<char_type> parser(handler,err_handler);
                parser.set_source(buffer.data(),buffer.size());
                parser.skip_bom();
                parser.parse();
                parser.end_parse();
                parser.check_done();
            }

            std::fclose (fp);
        }
        catch (...)
        {
            std::fclose (fp);
            throw;
        }
        if (!handler.is_valid())
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json file");
        }
        return handler.get_result();
    }

    static basic_json make_array()
    {
        return json_type(variant(array()));
    }

    static basic_json make_array(const array& a)
    {
        return json_type(variant(a));
    }

    static basic_json make_array(const array& a, allocator_type allocator)
    {
        return json_type(variant(a,allocator));
    }

    static basic_json make_array(std::initializer_list<json_type> init, const Allocator& allocator = Allocator())
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

    static const json_type& null()
    {
        static json_type a_null = json_type(variant(null_type()));
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

    basic_json(const json_type& val)
        : var_(val.var_)
    {
    }

    basic_json(const json_type& val, const Allocator& allocator)
        : var_(val.var_,allocator)
    {
    }

    basic_json(json_type&& other) JSONCONS_NOEXCEPT
        : var_(std::move(other.var_))
    {
    }

    basic_json(json_type&& other, const Allocator& allocator) JSONCONS_NOEXCEPT
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
        : var_(std::forward<array&&>(other))
    {
    }

    basic_json(const object& other)
        : var_(other)
    {
    }

    basic_json(object&& other)
        : var_(std::forward<object&&>(other))
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
        : var_(json_type_traits<json_type,T>::to_json(val).var_)
    {
    }

    template <class T>
    basic_json(const T& val, const Allocator& allocator)
        : var_(json_type_traits<json_type,T>::to_json(val,allocator).var_)
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

    basic_json(const char_type *s, size_t length, const Allocator& allocator = Allocator())
        : var_(s, length, allocator)
    {
    }
    template<class InputIterator>
    basic_json(InputIterator first, InputIterator last, const Allocator& allocator = Allocator())
        : var_(first,last,allocator)
    {
    }

    ~basic_json()
    {
    }

    basic_json& operator=(const json_type& rhs)
    {
        var_ = rhs.var_;
        return *this;
    }

    basic_json& operator=(json_type&& rhs) JSONCONS_NOEXCEPT
    {
        if (this != &rhs)
        {
            var_ = std::move(rhs.var_);
        }
        return *this;
    }

    template <class T>
    json_type& operator=(const T& val)
    {
        var_ = json_type_traits<json_type,T>::to_json(val).var_;
        return *this;
    }

    json_type& operator=(const char_type* s)
    {
        var_ = variant(s);
        return *this;
    }

    bool operator!=(const json_type& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator==(const json_type& rhs) const
    {
        return var_ == rhs.var_;
    }

    size_t size() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            return 0;
        case value_type::object_t:
            return object_value().size();
        case value_type::array_t:
            return array_value().size();
        default:
            return 0;
        }
    }

    json_type& operator[](size_t i)
    {
        return at(i);
    }

    const json_type& operator[](size_t i) const
    {
        return at(i);
    }

    json_proxy<json_type> operator[](string_view_type name)
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t: 
            create_object_implicitly();
        case value_type::object_t:
            return json_proxy<json_type>(*this, key_storage_type(name.begin(),name.end(),char_allocator_type(object_value().get_allocator())));
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    const json_type& operator[](string_view_type name) const
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

    void dump_body(basic_json_output_handler<char_type>& handler) const
    {
        switch (var_.type_id())
        {
        case value_type::small_string_t:
        case value_type::string_t:
            handler.value(as_string_view());
            break;
        case value_type::double_t:
            handler.value(var_.double_data_cast()->value(), var_.double_data_cast()->precision());
            break;
        case value_type::integer_t:
            handler.value(var_.integer_data_cast()->value());
            break;
        case value_type::uinteger_t:
            handler.value(var_.uinteger_data_cast()->value());
            break;
        case value_type::bool_t:
            handler.value(var_.bool_data_cast()->value());
            break;
        case value_type::null_t:
            handler.value(null_type());
            break;
        case value_type::empty_object_t:
            handler.begin_object();
            handler.end_object();
            break;
        case value_type::object_t:
            {
                handler.begin_object();
                const object& o = object_value();
                for (const_object_iterator it = o.begin(); it != o.end(); ++it)
                {
                    handler.name((it->key()).data(),it->key().length());
                    it->value().dump_body(handler);
                }
                handler.end_object();
            }
            break;
        case value_type::array_t:
            {
                handler.begin_array();
                const array& o = array_value();
                for (const_array_iterator it = o.begin(); it != o.end(); ++it)
                {
                    it->dump_body(handler);
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
        dump_body(handler);
        handler.end_json();
    }

    void dump(std::basic_ostream<char_type>& os) const
    {
        basic_json_serializer<char_type> serializer(os);
        dump(serializer);
    }

    void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options) const
    {
        basic_json_serializer<char_type> serializer(os, options);
        dump(serializer);
    }

    void dump(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool indenting) const
    {
        basic_json_serializer<char_type> serializer(os, options, indenting);
        dump(serializer);
    }

    string_type to_string(const char_allocator_type& allocator=char_allocator_type()) const JSONCONS_NOEXCEPT
    {
        string_type s(allocator);
        std::basic_ostringstream<char_type,char_traits_type,char_allocator_type> os(s);
        os.imbue(std::locale::classic());
        {
            basic_json_serializer<char_type> serializer(os);
            dump_body(serializer);
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
            dump_body(serializer);
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

    void write(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool indenting) const
    {
        dump(os,options,indenting);
    }

    void to_stream(basic_json_output_handler<char_type>& handler) const
    {
        handler.begin_json();
        dump_body(handler);
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

    void to_stream(std::basic_ostream<char_type>& os, const basic_serialization_options<char_type>& options, bool indenting) const
    {
        basic_json_serializer<char_type> serializer(os, options, indenting);
        to_stream(serializer);
    }
#endif
    bool is_null() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_type::null_t;
    }

    bool has_key(string_view_type name) const
    {
        switch (var_.type_id())
        {
        case value_type::object_t:
            {
                const_object_iterator it = object_value().find(name);
                return it != object_range().end();
            }
            break;
        default:
            return false;
        }
    }

    size_t count(string_view_type name) const
    {
        switch (var_.type_id())
        {
        case value_type::object_t:
            {
                auto it = object_value().find(name);
                if (it == object_range().end())
                {
                    return 0;
                }
                size_t count = 0;
                while (it != object_range().end() && it->key() == name)
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

    template<class T>
    bool is() const
    {
        return json_type_traits<json_type,T>::is(*this);
    }

    bool is_string() const JSONCONS_NOEXCEPT
    {
        return (var_.type_id() == value_type::string_t) || (var_.type_id() == value_type::small_string_t);
    }


    bool is_bool() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_type::bool_t;
    }

    bool is_object() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_type::object_t || var_.type_id() == value_type::empty_object_t;
    }

    bool is_array() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_type::array_t;
    }

    bool is_integer() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_type::integer_t || (var_.type_id() == value_type::uinteger_t && (as_uinteger() <= static_cast<uint64_t>((std::numeric_limits<long long>::max)())));
    }

    bool is_uinteger() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_type::uinteger_t || (var_.type_id() == value_type::integer_t && as_integer() >= 0);
    }

    bool is_double() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_type::double_t;
    }

    bool is_number() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_type::integer_t || var_.type_id() == value_type::uinteger_t || var_.type_id() == value_type::double_t;
    }

    bool empty() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case value_type::small_string_t:
            return var_.small_string_data_cast()->length() == 0;
        case value_type::string_t:
            return var_.string_data_cast()->length() == 0;
        case value_type::array_t:
            return array_value().size() == 0;
        case value_type::empty_object_t:
            return true;
        case value_type::object_t:
            return object_value().size() == 0;
        default:
            return false;
        }
    }

    size_t capacity() const
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            return array_value().capacity();
        case value_type::object_t:
            return object_value().capacity();
        default:
            return 0;
        }
    }

    template<class U=Allocator,
         typename std::enable_if<is_stateless<U>::value
            >::type* = nullptr>
    void create_object_implicitly()
    {
        var_ = variant(Allocator());
    }

    template<class U=Allocator,
         typename std::enable_if<!is_stateless<U>::value
            >::type* = nullptr>
    void create_object_implicitly() const
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Cannot create object implicitly - allocator is not default constructible.");
    }

    void reserve(size_t n)
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            array_value().reserve(n);
            break;
        case value_type::empty_object_t:
        {
            create_object_implicitly();
            object_value().reserve(n);
        }
        break;
        case value_type::object_t:
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
        case value_type::array_t:
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
        case value_type::array_t:
            array_value().resize(n, val);
            break;
        default:
            break;
        }
    }

    template<class T>
    T as() const
    {
        return json_type_traits<json_type,T>::as(*this);
    }

    template<class T>
    typename std::enable_if<std::is_same<string_type,T>::value>::type as(const char_allocator_type& allocator) const
    {
        return json_type_traits<json_type,T>::as(*this,allocator);
    }

    bool as_bool() const 
    {
        switch (var_.type_id())
        {
        case value_type::small_string_t:
        case value_type::string_t:
            try
            {
                json_type j = json_type::parse(as_string_view().data(),as_string_view().length());
                return j.as_bool();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a bool");
            }
            break;
        case value_type::bool_t:
            return var_.bool_data_cast()->value();
        case value_type::double_t:
            return var_.double_data_cast()->value() != 0.0;
        case value_type::integer_t:
            return var_.integer_data_cast()->value() != 0;
        case value_type::uinteger_t:
            return var_.uinteger_data_cast()->value() != 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a bool");
        }
    }

    int64_t as_integer() const
    {
        switch (var_.type_id())
        {
        case value_type::small_string_t:
        case value_type::string_t:
            try
            {
                json_type j = json_type::parse(as_string_view().data(),as_string_view().length());
                return j.as<int64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
            }
            break;
        case value_type::double_t:
            return static_cast<int64_t>(var_.double_data_cast()->value());
        case value_type::integer_t:
            return static_cast<int64_t>(var_.integer_data_cast()->value());
        case value_type::uinteger_t:
            return static_cast<int64_t>(var_.uinteger_data_cast()->value());
        case value_type::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
        }
    }

    uint64_t as_uinteger() const
    {
        switch (var_.type_id())
        {
        case value_type::small_string_t:
        case value_type::string_t:
            try
            {
                json_type j = json_type::parse(as_string_view().data(),as_string_view().length());
                return j.as<uint64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
            }
            break;
        case value_type::double_t:
            return static_cast<uint64_t>(var_.double_data_cast()->value());
        case value_type::integer_t:
            return static_cast<uint64_t>(var_.integer_data_cast()->value());
        case value_type::uinteger_t:
            return static_cast<uint64_t>(var_.uinteger_data_cast()->value());
        case value_type::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
        }
    }

    size_t double_precision() const
    {
        switch (var_.type_id())
        {
        case value_type::double_t:
            return var_.double_data_cast()->precision();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
        }
    }

    double as_double() const
    {
        switch (var_.type_id())
        {
        case value_type::small_string_t:
        case value_type::string_t:
            try
            {
                json_type j = json_type::parse(as_string_view().data(),as_string_view().length());
                return j.as<double>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
            }
            break;
        case value_type::double_t:
            return var_.double_data_cast()->value();
        case value_type::integer_t:
            return static_cast<double>(var_.integer_data_cast()->value());
        case value_type::uinteger_t:
            return static_cast<double>(var_.uinteger_data_cast()->value());
        case value_type::null_t:
            return std::numeric_limits<double>::quiet_NaN();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
        }
    }

    string_view_type as_string_view() const
    {
        return var_.as_string_view();
    }

    string_type as_string() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case value_type::small_string_t:
        case value_type::string_t:
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
        case value_type::small_string_t:
        case value_type::string_t:
            return string_type(as_string_view().data(),as_string_view().length(),allocator);
        default:
            return to_string(allocator);
        }
    }

    string_type as_string(const basic_serialization_options<char_type>& options) const 
    {
        switch (var_.type_id())
        {
        case value_type::small_string_t:
        case value_type::string_t:
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
        case value_type::small_string_t:
        case value_type::string_t:
            return string_type(as_string_view().data(),as_string_view().length(),allocator);
        default:
            return to_string(options,allocator);
        }
    }

    const char_type* as_cstring() const
    {
        switch (var_.type_id())
        {
        case value_type::small_string_t:
            return var_.small_string_data_cast()->c_str();
        case value_type::string_t:
            return var_.string_data_cast()->c_str();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a cstring");
        }
    }
    json_type& at(string_view_type name)
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case value_type::object_t:
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

    json_type& evaluate() 
    {
        return *this;
    }

    json_type& evaluate_with_default() 
    {
        return *this;
    }

    const json_type& evaluate() const
    {
        return *this;
    }
    json_type& evaluate(string_view_type name) 
    {
        return at(name);
    }

    const json_type& evaluate(string_view_type name) const
    {
        return at(name);
    }

    const json_type& at(string_view_type name) const
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case value_type::object_t:
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

    json_type& at(size_t i)
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            if (i >= array_value().size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return array_value().operator[](i);
        case value_type::object_t:
            return object_value().at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    const json_type& at(size_t i) const
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            if (i >= array_value().size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return array_value().operator[](i);
        case value_type::object_t:
            return object_value().at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    object_iterator find(string_view_type name)
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            return object_range().end();
        case value_type::object_t:
            return object_value().find(name);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const_object_iterator find(string_view_type name) const
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            return object_range().end();
        case value_type::object_t:
            return object_value().find(name);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    template<class T>
    json_type get(string_view_type name, T&& default_val) const
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            {
                return json_type(std::forward<T&&>(default_val));
            }
        case value_type::object_t:
            {
                const_object_iterator it = object_value().find(name);
                if (it != object_range().end())
                {
                    return it->value();
                }
                else
                {
                    return json_type(std::forward<T&&>(default_val));
                }
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    template<class T>
    T get_with_default(string_view_type name, const T& default_val) const
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            {
                return default_val;
            }
        case value_type::object_t:
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

    const CharT* get_with_default(string_view_type name, const CharT* default_val) const
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            {
                return default_val;
            }
        case value_type::object_t:
            {
                const_object_iterator it = object_value().find(name);
                if (it != object_range().end())
                {
                    return it->value().template as<const CharT*>();
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
        case value_type::array_t:
            array_value().shrink_to_fit();
            break;
        case value_type::object_t:
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
        case value_type::array_t:
            array_value().clear();
            break;
        case value_type::object_t:
            object_value().clear();
            break;
        default:
            break;
        }
    }

    void erase(object_iterator first, object_iterator last)
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            break;
        case value_type::object_t:
            object_value().erase(first, last);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    void erase(array_iterator first, array_iterator last)
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            array_value().erase(first, last);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
            break;
        }
    }

    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void erase(string_view_type name)
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            break;
        case value_type::object_t:
            object_value().erase(name);
            break;
        default:
            JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to erase %s on a value that is not an object", name);
            break;
        }
    }

    template <class T>
    void set(string_view_type name, T&& value)
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            create_object_implicitly();
        case value_type::object_t:
            object_value().set(name, std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    void set_(key_storage_type&& name, T&& value)
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            create_object_implicitly();
        case value_type::object_t:
            object_value().set_(std::forward<key_storage_type&&>(name), std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to call set on a value that is not an object");
            }
        }
    }

    template <class T>
    object_iterator set(object_iterator hint, string_view_type name, T&& value)
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            create_object_implicitly();
        case value_type::object_t:
            return object_value().set(hint, name, std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    object_iterator set_(object_iterator hint, key_storage_type&& name, T&& value)
    {
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            create_object_implicitly();
        case value_type::object_t:
            return object_value().set_(hint, std::forward<key_storage_type&&>(name), std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to set on a value that is not an object");
            }
        }
    }

    template <class T>
    void add(T&& value)
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            array_value().add(std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class T>
    array_iterator add(const_array_iterator pos, T&& value)
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            return array_value().add(pos, std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    value_type type_id() const
    {
        return var_.type_id();
    }

    void swap(json_type& b)
    {
        var_.swap(b.var_);
    }

    friend void swap(json_type& a, json_type& b)
    {
        a.swap(b);
    }

    static json_type make_string(string_view_type s)
    {
        return json_type(variant(s.data(),s.length()));
    }

    static json_type make_string(const char_type* rhs, size_t length)
    {
        return json_type(variant(rhs,length));
    }

    static json_type make_string(string_view_type s, allocator_type allocator)
    {
        return json_type(variant(s.data(),s.length(),allocator));
    }

    static json_type make_integer(int64_t val)
    {
        return json_type(variant(val));
    }

    static json_type make_uinteger(uint64_t val)
    {
        return json_type(variant(val));
    }

    static json_type make_double(double val)
    {
        return json_type(variant(val));
    }

    static json_type make_bool(bool val)
    {
        return json_type(variant(val));
    }

    static json_type make_object(const object& o)
    {
        return json_type(variant(o));
    }

    static json_type make_object(const object& o, allocator_type allocator)
    {
        return json_type(variant(o,allocator));
    }

    static basic_json make_2d_array(size_t m, size_t n);

    template <class T>
    static basic_json make_2d_array(size_t m, size_t n, T val);

    static basic_json make_3d_array(size_t m, size_t n, size_t k);

    template <class T>
    static basic_json make_3d_array(size_t m, size_t n, size_t k, T val);

#if !defined(JSONCONS_NO_DEPRECATED)

    static basic_json parse(std::basic_istream<char_type>& is)
    {
        return parse_stream(is);
    }
    static basic_json parse(std::basic_istream<char_type>& is, basic_parse_error_handler<char_type>& err_handler)
    {
        return parse_stream(is,err_handler);
    }

    static basic_json parse_string(const string_type& s)
    {
        return parse(s);
    }

    static basic_json parse_string(const string_type& s, basic_parse_error_handler<char_type>& err_handler)
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

    const json_type& get(string_view_type name) const
    {
        static const json_type a_null = null_type();

        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            return a_null;
        case value_type::object_t:
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
        return var_.type_id() == value_type::integer_t;
    }

    bool is_ulonglong() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_type::uinteger_t;
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
        case value_type::double_t:
            return static_cast<int>(var_.double_data_cast()->value());
        case value_type::integer_t:
            return static_cast<int>(var_.integer_data_cast()->value());
        case value_type::uinteger_t:
            return static_cast<int>(var_.uinteger_data_cast()->value());
        case value_type::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an int");
        }
    }

    unsigned int as_uint() const
    {
        switch (var_.type_id())
        {
        case value_type::double_t:
            return static_cast<unsigned int>(var_.double_data_cast()->value());
        case value_type::integer_t:
            return static_cast<unsigned int>(var_.integer_data_cast()->value());
        case value_type::uinteger_t:
            return static_cast<unsigned int>(var_.uinteger_data_cast()->value());
        case value_type::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned int");
        }
    }

    long as_long() const
    {
        switch (var_.type_id())
        {
        case value_type::double_t:
            return static_cast<long>(var_.double_data_cast()->value());
        case value_type::integer_t:
            return static_cast<long>(var_.integer_data_cast()->value());
        case value_type::uinteger_t:
            return static_cast<long>(var_.uinteger_data_cast()->value());
        case value_type::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a long");
        }
    }

    unsigned long as_ulong() const
    {
        switch (var_.type_id())
        {
        case value_type::double_t:
            return static_cast<unsigned long>(var_.double_data_cast()->value());
        case value_type::integer_t:
            return static_cast<unsigned long>(var_.integer_data_cast()->value());
        case value_type::uinteger_t:
            return static_cast<unsigned long>(var_.uinteger_data_cast()->value());
        case value_type::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned long");
        }
    }

    void add(size_t index, const json_type& value)
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            array_value().add(index, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    void add(size_t index, json_type&& value){
        switch (var_.type_id()){
        case value_type::array_t:
            array_value().add(index, std::forward<json_type&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    bool has_member(const key_storage_type& name) const
    {
        switch (var_.type_id())
        {
        case value_type::object_t:
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
        case value_type::array_t:
            array_value().remove_range(from_index, to_index);
            break;
        default:
            break;
        }
    }
    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void remove(string_view_type name)
    {
        erase(name.data(),name.length());
    }
    void remove_member(string_view_type name)
    {
        erase(name.data(),name.length());
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
        static json_type empty_object = object();
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            return range<object_iterator>(empty_object.object_range().begin(), empty_object.object_range().end());
        case value_type::object_t:
            return range<object_iterator>(object_value().begin(),object_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    range<const_object_iterator> object_range() const
    {
        static const json_type empty_object = object();
        switch (var_.type_id())
        {
        case value_type::empty_object_t:
            return range<const_object_iterator>(empty_object.object_range().begin(), empty_object.object_range().end());
        case value_type::object_t:
            return range<const_object_iterator>(object_value().begin(),object_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    range<array_iterator> array_range()
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            return range<array_iterator>(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    range<const_array_iterator> array_range() const
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
            return range<const_array_iterator>(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    array& array_value() 
    {
        switch (var_.type_id())
        {
        case value_type::array_t:
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
        case value_type::array_t:
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
        case value_type::empty_object_t:
            create_object_implicitly();
        case value_type::object_t:
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
        case value_type::empty_object_t:
            const_cast<json_type*>(this)->create_object_implicitly(); // HERE
        case value_type::object_t:
            return var_.object_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad object cast");
            break;
        }
    }

private:

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_type& o)
    {
        o.dump(os);
        return os;
    }

    friend std::basic_istream<char_type>& operator<<(std::basic_istream<char_type>& is, json_type& o)
    {
        json_decoder<json_type> handler;
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
void swap(typename Json::kvp_type & a, typename Json::kvp_type & b)
{
    a.swap(b);
}

template<class CharT,class JsonTraits,class Allocator>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::make_2d_array(size_t m, size_t n)
{
    basic_json<CharT,JsonTraits,Allocator> a = basic_json<CharT,JsonTraits,Allocator>::array();
    a.resize(m);
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<CharT,JsonTraits,Allocator>::make_array(n);
    }
    return a;
}

template<class CharT,class JsonTraits,class Allocator>
template<class T>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::make_2d_array(size_t m, size_t n, T val)
{
    basic_json<CharT,JsonTraits,Allocator> v;
    v = val;
    basic_json<CharT,JsonTraits,Allocator> a = make_array(m);
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<CharT,JsonTraits,Allocator>::make_array(n, v);
    }
    return a;
}

template<class CharT,class JsonTraits,class Allocator>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::make_3d_array(size_t m, size_t n, size_t k)
{
    basic_json<CharT,JsonTraits,Allocator> a = basic_json<CharT,JsonTraits,Allocator>::array();
    a.resize(m);
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<CharT,JsonTraits,Allocator>::make_2d_array(n, k);
    }
    return a;
}

template<class CharT,class JsonTraits,class Allocator>
template<class T>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::make_3d_array(size_t m, size_t n, size_t k, T val)
{
    basic_json<CharT,JsonTraits,Allocator> v;
    v = val;
    basic_json<CharT,JsonTraits,Allocator> a = make_array(m);
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<CharT,JsonTraits,Allocator>::make_2d_array(n, k, v);
    }
    return a;
}

template<class CharT,class JsonTraits,class Allocator>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::parse_stream(std::basic_istream<char_type>& is)
{
    parse_error_handler_type err_handler;
    return parse_stream(is,err_handler);
}

template<class CharT,class JsonTraits,class Allocator>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::parse_stream(std::basic_istream<char_type>& is, 
                                                                                            basic_parse_error_handler<char_type>& err_handler)
{
    json_decoder<basic_json<CharT,JsonTraits,Allocator>> handler;
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
       : o_(&o), is_pretty_print_(is_pretty_print), format_(options)
    {
        ;
    }

    void dump(std::basic_ostream<char_type>& os) const
    {
        o_->dump(os, format_, is_pretty_print_);
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_printable<Json>& o)
    {
        o.dump(os);
        return os;
    }

    const Json *o_;
    bool is_pretty_print_;
    basic_serialization_options<char_type> format_;
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

typedef basic_json<char,json_traits<char>,std::allocator<char>> json;
typedef basic_json<wchar_t,json_traits<wchar_t>,std::allocator<wchar_t>> wjson;
typedef basic_json<char, o_json_traits<char>, std::allocator<char>> ojson;
typedef basic_json<wchar_t, o_json_traits<wchar_t>, std::allocator<wchar_t>> wojson;

#if !defined(JSONCONS_NO_DEPRECATED)
typedef basic_json<wchar_t, o_json_traits<wchar_t>, std::allocator<wchar_t>> owjson;
typedef json_decoder<json> json_deserializer;
typedef json_decoder<wjson> wjson_deserializer;
typedef json_decoder<ojson> ojson_deserializer;
typedef json_decoder<wojson> wojson_deserializer;
#endif

}

#if defined(JSONCONS_HAS_USER_DEFINED_LITERALS)
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
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
