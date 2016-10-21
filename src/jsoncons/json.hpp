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
#include "jsoncons/json_traits.hpp"
#include "jsoncons/json_structures.hpp"
#include "jsoncons/json_text_traits.hpp"
#include "jsoncons/json_output_handler.hpp"
#include "jsoncons/output_format.hpp"
#include "jsoncons/json_serializer.hpp"
#include "jsoncons/json_deserializer.hpp"
#include "jsoncons/json_reader.hpp"
#include "jsoncons/json_type_traits.hpp"

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

template <class T, class Allocator, typename... Args>
T* create_impl(const Allocator& allocator, Args&& ... args)
{
    typename std::allocator_traits<Allocator>:: template rebind_alloc<T> alloc(allocator);
    T* storage = alloc.allocate(1);
    try
    {
        std::allocator_traits<Allocator>:: template rebind_traits<T>::construct(alloc, storage, std::forward<Args>(args)...);
    }
    catch (...)
    {
        alloc.deallocate(storage,1);
        throw;
    }
    return storage;
}

template <class T, class Allocator>
void destroy_impl(const Allocator& allocator, T* p)
{
    typename std::allocator_traits<Allocator>:: template rebind_alloc<T> alloc(allocator);
    std::allocator_traits<Allocator>:: template rebind_traits<T>::destroy(alloc, p);
    alloc.deallocate(p,1);
}

enum class value_types : uint8_t 
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
    typedef typename std::char_traits<CharT> char_traits_type;

    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<CharT> string_allocator;
    typedef std::basic_string<CharT,char_traits_type,string_allocator> string_type;
    typedef basic_json<CharT,JsonTraits,Allocator> json_type;
    typedef name_value_pair<string_type,json_type> member_type;

    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<json_type> array_allocator;

    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<member_type> object_allocator;

    typedef json_array<json_type,array_allocator> array;
    typedef json_object<string_type,json_type,json_traits_type::is_object_sorted,object_allocator> object;

    typedef jsoncons::null_type null_type;

    typedef typename object::iterator object_iterator;
    typedef typename object::const_iterator const_object_iterator;
    typedef typename array::iterator array_iterator;
    typedef typename array::const_iterator const_array_iterator;

    typedef range<object_iterator> object_range;
    typedef range<const_object_iterator> const_object_range;
    typedef range<array_iterator> array_range;
    typedef range<const_array_iterator> const_array_range;

    struct variant
    {
        struct base_data
        {
            value_types type_id_;

            base_data(value_types id)
                : type_id_(id)
            {}
        };

        struct null_data : public base_data
        {
            null_data()
                : base_data(value_types::null_t)
            {
            }
        };

        struct empty_object_data : public base_data
        {
            empty_object_data()
                : base_data(value_types::empty_object_t)
            {
            }
        };

        struct bool_data : public base_data
        {
            bool val_;

            bool_data(bool val)
                : base_data(value_types::bool_t),val_(val)
            {
            }

            bool_data(const bool_data& val)
                : base_data(value_types::bool_t),val_(val.val_)
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
                : base_data(value_types::integer_t),val_(val)
            {
            }

            integer_data(const integer_data& val)
                : base_data(value_types::integer_t),val_(val.val_)
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
                : base_data(value_types::uinteger_t),val_(val)
            {
            }

            uinteger_data(const uinteger_data& val)
                : base_data(value_types::uinteger_t),val_(val.val_)
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
                : base_data(value_types::double_t), 
                  precision_(precision), 
                  val_(val)
            {
            }

            double_data(const double_data& val)
                : base_data(value_types::double_t),
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
                : base_data(value_types::small_string_t), length_(length)
            {
                JSONCONS_ASSERT(length <= max_length);
                std::memcpy(data_,p,length*sizeof(char_type));
                data_[length] = 0;
            }

            small_string_data(const small_string_data& val)
                : base_data(value_types::small_string_t), length_(val.length_)
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
            struct string_holder : public string_allocator
            {
                const char_type* c_str() const { return p_; }
                const char_type* data() const { return p_; }
                size_t length() const { return length_; }
                string_allocator get_allocator() const
                {
                    return *this;
                }

                bool operator==(const string_holder& rhs) const
                {
                    return length() == rhs.length() ? std::char_traits<char_type>::compare(data(), rhs.data(), length()) == 0 : false;
                }

                string_holder(const string_allocator& allocator)
                    : string_allocator(allocator), length_(0), p_(nullptr)
                {
                }

                size_t length_;
                char_type* p_;
            private:
                string_holder(const string_holder&);
                string_holder& operator=(const string_holder&);
            };

            struct string_holderA
            {
                string_holder data;
                char_type c[1];
            };

            typedef typename std::aligned_storage<sizeof(string_holderA), JSONCONS_ALIGNOF(string_holderA)>::type storage_type;

            static size_t aligned_size(size_t n)
            {
                return sizeof(storage_type) + n;
            }

            string_holder* create_string_holder(const char_type* s, size_t length, const string_allocator& allocator)
            {
                size_t mem_size = aligned_size(length*sizeof(char_type));

                typename std::allocator_traits<string_allocator>:: template rebind_alloc<char> alloc(allocator);

                char* storage = alloc.allocate(mem_size);
                string_holder* ps = new(storage)string_holder(allocator);
                auto psa = reinterpret_cast<string_holderA*>(storage); 

                ps->p_ = new(&psa->c)char_type[length + 1];
                memcpy(ps->p_, s, length*sizeof(char_type));
                ps->p_[length] = 0;
                ps->length_ = length;
                return ps;
            }

            void destroy_string_holder(const string_allocator& allocator, string_holder* p)
            {
                size_t mem_size = aligned_size(p->length_*sizeof(char_type));
                typename std::allocator_traits<string_allocator>:: template rebind_alloc<char> alloc(allocator);
                alloc.deallocate(reinterpret_cast<char*>(p),mem_size);
            }
            string_holder* holder_;

            string_data(const char_type* s, size_t length, const string_allocator& alloc)
                : base_data(value_types::string_t)
            {
                holder_ = create_string_holder(s, length, alloc);
            }

            string_data(const string_data& val)
                : base_data(value_types::string_t)
            {
                holder_ = create_string_holder(val.holder_->p_, 
                                             val.holder_->length_, 
                                             val.holder_->get_allocator());
            }

            string_data(const string_data& val, string_allocator allocator)
                : base_data(value_types::string_t)
            {
                holder_ = create_string_holder(val.holder_->p_, 
                                               val.holder_->length_, 
                                               allocator);
            }
            ~string_data()
            {
                destroy_string_holder(holder_->get_allocator(), holder_);
            }

            size_t length() const
            {
                return holder_->length_;
            }

            const char_type* data() const
            {
                return holder_->p_;
            }

            const char_type* c_str() const
            {
                return holder_->p_;
            }

            string_allocator get_allocator() const
            {
                return holder_->get_allocator();
            }
        };


        struct object_data : public base_data
        {
            object* data_;

            explicit object_data(const Allocator& a)
                : base_data(value_types::object_t)
            {
                data_ = create_impl<object>(a, object_allocator(a));
            }

            explicit object_data(const object & val)
                : base_data(value_types::object_t)
            {
                data_ = create_impl<object>(val.get_allocator(), val);
            }

            explicit object_data(const object & val, const Allocator& a)
                : base_data(value_types::object_t)
            {
                data_ = create_impl<object>(a, val, object_allocator(a));
            }

            explicit object_data(const object_data & val)
                : base_data(value_types::object_t)
            {
                data_ = create_impl<object>(val.data_->get_allocator(), *(val.data_));
            }

            explicit object_data(const object_data & val, const Allocator& a)
                : base_data(value_types::object_t)
            {
                data_ = create_impl<object>(a, *(val.data_), object_allocator(a));
            }

            ~object_data()
            {
                destroy_impl(data_->get_allocator(), data_);
            }

            object& value()
            {
                return *data_;
            }

            const object& value() const
            {
                return *data_;
            }
        };

        struct array_data : public base_data
        {
            array* data_;

            array_data(const array & val)
                : base_data(value_types::array_t)
            {
                data_ = create_impl<array>(val.get_allocator(), val);
            }

            array_data(const array & val, const Allocator& a)
                : base_data(value_types::array_t)
            {
                data_ = create_impl<array>(a, val, array_allocator(a));
            }

            array_data(const array_data & val)
                : base_data(value_types::array_t)
            {
                data_ = create_impl<array>(val.data_->get_allocator(), *(val.data_));
            }

            array_data(const array_data & val, const Allocator& a)
                : base_data(value_types::array_t)
            {
                data_ = create_impl<array>(a, *(val.data_), array_allocator(a));
            }

            template<class InputIterator>
            array_data(InputIterator first, InputIterator last, const Allocator& a)
                : base_data(value_types::array_t)
            {
                data_ = create_impl<array>(a, first, last, array_allocator(a));
            }
            ~array_data()
            {
                destroy_impl(data_->get_allocator(), data_);
            }

            array& value()
            {
                return *data_;
            }

            const array& value() const
            {
                return *data_;
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
            switch (val.type_id())
            {
            case value_types::null_t:
                new(reinterpret_cast<void*>(&data_))null_data();
                break;
            case value_types::empty_object_t:
                new(reinterpret_cast<void*>(&data_))empty_object_data();
                break;
            case value_types::double_t:
                new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
                break;
            case value_types::integer_t:
                new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
                break;
            case value_types::uinteger_t:
                new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
                break;
            case value_types::bool_t:
                new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
                break;
            case value_types::small_string_t:
                new(reinterpret_cast<void*>(&data_))small_string_data(*(val.small_string_data_cast()));
                break;
            case value_types::string_t:
                new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()));
                break;
            case value_types::object_t:
                new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()));
                break;
            case value_types::array_t:
                new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()));
                break;
            default:
                break;
            }
        }

        variant(const variant& val, const Allocator& allocator)
        {
            switch (val.type_id())
            {
            case value_types::null_t:
                new(reinterpret_cast<void*>(&data_))null_data();
                break;
            case value_types::empty_object_t:
                new(reinterpret_cast<void*>(&data_))empty_object_data();
                break;
            case value_types::double_t:
                new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
                break;
            case value_types::integer_t:
                new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
                break;
            case value_types::uinteger_t:
                new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
                break;
            case value_types::bool_t:
                new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
                break;
            case value_types::small_string_t:
                new(reinterpret_cast<void*>(&data_))string_data(val.small_string_data_cast()->data(), val.small_string_data_cast()->length(),allocator);
                break;
            case value_types::string_t:
                new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()),allocator);
                break;
            case value_types::object_t:
                new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()),allocator);
                break;
            case value_types::array_t:
                new(reinterpret_cast<void*>(&data_))array_data(*(val.array_data_cast()),allocator);
                break;
            default:
                break;
            }
        }

        variant(variant&& val)
        {
            new(reinterpret_cast<void*>(&data_))null_data();
            swap(val);
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
                new(reinterpret_cast<void*>(&data_))string_data(s, length, string_allocator());
            }
        }
        variant(const char_type* s)
        {
            size_t length = std::char_traits<char_type>::length(s);
            if (length <= small_string_data::max_length)
            {
                new(reinterpret_cast<void*>(&data_))small_string_data(s, static_cast<uint8_t>(length));
            }
            else
            {
                new(reinterpret_cast<void*>(&data_))string_data(s, length, string_allocator());
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
                new(reinterpret_cast<void*>(&data_))string_data(s, length, string_allocator(alloc));
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
        template<class InputIterator>
        variant(InputIterator first, InputIterator last, const Allocator& a)
        {
            new(reinterpret_cast<void*>(&data_))array_data(first, last, a);
        }
        variant(const array& val, const Allocator& alloc)
        {
            new(reinterpret_cast<void*>(&data_))array_data(val,alloc);
        }

        ~variant()
        {
            switch (type_id())
            {
            case value_types::string_t:
                reinterpret_cast<string_data*>(&data_)->~string_data();
                break;
            case value_types::object_t:
                reinterpret_cast<object_data*>(&data_)->~object_data();
                break;
            case value_types::array_t:
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
                switch (type_id())
                {
                case value_types::string_t:
                    reinterpret_cast<string_data*>(&data_)->~string_data();
                    break;
                case value_types::object_t:
                    reinterpret_cast<object_data*>(&data_)->~object_data();
                    break;
                case value_types::array_t:
                    reinterpret_cast<array_data*>(&data_)->~array_data();
                    break;
                default:
                    break;
                }
                switch (val.type_id())
                {
                case value_types::null_t:
                    new(reinterpret_cast<void*>(&data_))null_data();
                    break;
                case value_types::empty_object_t:
                    new(reinterpret_cast<void*>(&data_))empty_object_data();
                    break;
                case value_types::double_t:
                    new(reinterpret_cast<void*>(&data_))double_data(*(val.double_data_cast()));
                    break;
                case value_types::integer_t:
                    new(reinterpret_cast<void*>(&data_))integer_data(*(val.integer_data_cast()));
                    break;
                case value_types::uinteger_t:
                    new(reinterpret_cast<void*>(&data_))uinteger_data(*(val.uinteger_data_cast()));
                    break;
                case value_types::bool_t:
                    new(reinterpret_cast<void*>(&data_))bool_data(*(val.bool_data_cast()));
                    break;
                case value_types::small_string_t:
                    new(reinterpret_cast<void*>(&data_))small_string_data(*(val.small_string_data_cast()));
                    break;
                case value_types::string_t:
                    new(reinterpret_cast<void*>(&data_))string_data(*(val.string_data_cast()));
                    break;
                case value_types::object_t:
                    new(reinterpret_cast<void*>(&data_))object_data(*(val.object_data_cast()));
                    break;
                case value_types::array_t:
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
                swap(val);
            }
            return *this;
        }

        value_types type_id() const
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

        bool operator==(const variant& rhs) const
        {
            if (this == &rhs)
            {
                return true;
            }

            const value_types id = type_id();
            const value_types rhs_id = rhs.type_id();

            if (id == rhs_id)
            {
                switch (id)
                {
                case value_types::null_t:
                    return true;
                case value_types::empty_object_t:
                    return true;
                case value_types::double_t:
                    return double_data_cast()->val_ == rhs.double_data_cast()->val_;
                case value_types::integer_t:
                    return integer_data_cast()->val_ == rhs.integer_data_cast()->val_;
                case value_types::uinteger_t:
                    return uinteger_data_cast()->val_ == rhs.uinteger_data_cast()->val_;
                case value_types::bool_t:
                    return bool_data_cast()->val_ == rhs.bool_data_cast()->val_;
                case value_types::small_string_t:
                    return small_string_data_cast()->length() == rhs.small_string_data_cast()->length() &&
                           std::char_traits<char_type>::compare(small_string_data_cast()->data(),rhs.small_string_data_cast()->data(),small_string_data_cast()->length()) == 0;
                case value_types::string_t:
                    return string_data_cast()->length() == rhs.string_data_cast()->length() &&
                           std::char_traits<char_type>::compare(string_data_cast()->data(),rhs.string_data_cast()->data(),string_data_cast()->length()) == 0;
                case value_types::object_t:
                    return object_data_cast()->value() == rhs.object_data_cast()->value();
                case value_types::array_t:
                    return array_data_cast()->value() == rhs.array_data_cast()->value();
                default:
                    return false;
                }
            }

            switch (id)
            {
            case value_types::integer_t:
                if (rhs_id == value_types::double_t)
                {
                    return static_cast<double>(integer_data_cast()->val_) == rhs.double_data_cast()->val_;
                }
                else if (rhs_id == value_types::uinteger_t && integer_data_cast()->val_ >= 0)
                {
                    return static_cast<uint64_t>(integer_data_cast()->val_) == rhs.uinteger_data_cast()->val_;
                }
                break;
            case value_types::uinteger_t:
                if (rhs_id == value_types::double_t)
                {
                    return static_cast<double>(uinteger_data_cast()->val_) == rhs.double_data_cast()->val_;
                }
                else if (rhs_id == value_types::integer_t && rhs.integer_data_cast()->val_ >= 0)
                {
                    return uinteger_data_cast()->val_ == static_cast<uint64_t>(rhs.integer_data_cast()->val_);
                }
                break;
            case value_types::double_t:
                if (rhs_id == value_types::integer_t)
                {
                    return double_data_cast()->val_ == static_cast<double>(rhs.integer_data_cast()->val_);
                }
                else if (rhs_id == value_types::uinteger_t)
                {
                    return double_data_cast()->val_ == static_cast<double>(rhs.uinteger_data_cast()->val_);
                }
                break;
            case value_types::empty_object_t:
                if (rhs_id == value_types::object_t && rhs.object_data_cast()->data_->size() == 0)
                {
                    return true;
                }
                break;
            case value_types::object_t:
                if (rhs_id == value_types::empty_object_t && object_data_cast()->data_->size() == 0)
                {
                    return true;
                }
                break;
            case value_types::small_string_t:
                if ((rhs_id == value_types::string_t) && 
                    (small_string_data_cast()->length() == rhs.string_data_cast()->length()) &&
                     std::char_traits<char_type>::compare(small_string_data_cast()->data(),rhs.string_data_cast()->data(),small_string_data_cast()->length()) == 0)
                {
                    return true;
                }
                break;
            case value_types::string_t:
                if ((rhs_id == value_types::small_string_t) && 
                    (string_data_cast()->length() == rhs.small_string_data_cast()->length()) &&
                     std::char_traits<char_type>::compare(string_data_cast()->data(),rhs.small_string_data_cast()->data(),string_data_cast()->length()) == 0)
                {
                    return true;
                }
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


        void swap(variant& rhs)
        {
            std::swap(data_,rhs.data_);
        }
    };

    template <class ParentT>
    class json_proxy 
    {
    private:
        typedef json_proxy<ParentT> proxy_type;

        ParentT& parent_;
        const string_type& name_;

        json_proxy() = delete;
        json_proxy& operator = (const json_proxy& other) = delete; 

        json_proxy(ParentT& parent, const string_type& name)
            : parent_(parent), name_(name)
        {
        }

        json_type& evaluate() 
        {
            return parent_.evaluate(name_);
        }

        const json_type& evaluate() const
        {
            return parent_.evaluate(name_);
        }

        json_type& evaluate_with_default()
        {
            json_type& val = parent_.evaluate_with_default();
            auto it = val.find(name_.data(),name_.length());
            if (it == val.members().end())
            {
                it = val.set(val.members().begin(),name_,object(val.object_value().get_allocator()));            
            }
            return it->value();
        }

        json_type& evaluate(size_t index)
        {
            return parent_.evaluate(name_).at(index);
        }

        const json_type& evaluate(size_t index) const
        {
            return parent_.evaluate(name_).at(index);
        }

        json_type& evaluate(const string_type& index)
        {
            return parent_.evaluate(name_).at(index);
        }

        const json_type& evaluate(const string_type& index) const
        {
            return parent_.evaluate(name_).at(index);
        }
    public:

        friend class basic_json<CharT,JsonTraits,Allocator>;

        object_range members()
        {
            return evaluate().members();
        }

        const_object_range members() const
        {
            return evaluate().members();
        }

        array_range elements()
        {
            return evaluate().elements();
        }

        const_array_range elements() const
        {
            return evaluate().elements();
        }

        size_t size() const JSONCONS_NOEXCEPT
        {
            return evaluate().size();
        }

        value_types type_id() const
        {
            return evaluate().type_id();
        }

        size_t count(const string_type& name) const
        {
            return evaluate().count(name);
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
#if !defined(JSONCONS_NO_DEPRECATED)
 
        bool is_any() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_any();
        }
#endif
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

        string_type as_string() const JSONCONS_NOEXCEPT
        {
            return evaluate().as_string();
        }

        string_type as_string(const string_allocator& allocator) const JSONCONS_NOEXCEPT
        {
            return evaluate().as_string(allocator);
        }

        string_type as_string(const basic_output_format<char_type>& format) const
        {
            return evaluate().as_string(format);
        }

        string_type as_string(const basic_output_format<char_type>& format,
                              const string_allocator& allocator) const
        {
            return evaluate().as_string(format,allocator);
        }

        template<class T>
        T as() const
        {
            return evaluate().template as<T>();
        }

        template<class T>
        typename std::enable_if<std::is_same<string_type,T>::value>::type as(const string_allocator& allocator) const
        {
            return evaluate().template as<T>(allocator);
        }
        bool as_bool() const
        {
            return evaluate().as_bool();
        }

        template <class T>
        std::vector<T> as_vector() const
        {
            return evaluate().template as_vector<T>();
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
            parent_.evaluate_with_default().set(name_, std::forward<T&&>(val));
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

        json_proxy<proxy_type> operator[](const string_type& name)
        {
            return json_proxy<proxy_type>(*this,name);
        }

        const json_proxy<proxy_type> operator[](const string_type& name) const
        {
            return json_proxy<proxy_type>(*this,name);
        }

        json_type& at(const string_type& name)
        {
            return evaluate().at(name);
        }

        const json_type& at(const string_type& name) const
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

        object_iterator find(const string_type& name)
        {
            return evaluate().find(name);
        }

        const_object_iterator find(const string_type& name) const
        {
            return evaluate().find(name);
        }

        object_iterator find(const char_type* name)
        {
            return evaluate().find(name);
        }

        const_object_iterator find(const char_type* name) const
        {
            return evaluate().find(name);
        }

        object_iterator find(const char_type* name, size_t length)
        {
            return evaluate().find(name,length);
        }

        const_object_iterator find(const char_type* name, size_t length) const
        {
            return evaluate().find(name,length);
        }

        template <class T>
        json_type get(const string_type& name, T&& default_val) const
        {
            return evaluate().get(name,std::forward<T>(default_val));
        }

        template <class T>
        T get_with_default(const string_type& name, const T& default_val) const
        {
            return evaluate().get_with_default(name,default_val);
        }

        const CharT* get_with_default(const string_type& name, const CharT* default_val) const
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

        void erase(const string_type& name)
        {
            evaluate().erase(name);
        }

       // Remove a member from an object 

        template <class T>
        void set(const string_type& name, T&& value)
        {
            evaluate().set(name,std::forward<T&&>(value));
        }

        template <class T>
        void set(string_type&& name, T&& value)

        {
            evaluate().set(std::forward<string_type&&>(name),std::forward<T&&>(value));
        }

        template <class T>
        object_iterator set(object_iterator hint, const string_type& name, T&& value)
        {
            return evaluate().set(hint, name, std::forward<T&&>(value));
        }

        template <class T>
        object_iterator set(object_iterator hint, string_type&& name, T&& value)

        {
            return evaluate().set(hint, std::forward<string_type&&>(name),std::forward<T&&>(value));
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

        string_type to_string(const string_allocator& allocator = string_allocator()) const JSONCONS_NOEXCEPT
        {
            return evaluate().to_string(allocator);
        }

        string_type to_string(const basic_output_format<char_type>& format, string_allocator& allocator = string_allocator()) const
        {
            return evaluate().to_string(format,allocator);
        }
        void write(basic_json_output_handler<char_type>& handler) const
        {
            evaluate().write(handler);
        }

        void write(std::basic_ostream<char_type>& os) const
        {
            evaluate().write(os);
        }

        void write(std::basic_ostream<char_type>& os, const basic_output_format<char_type>& format) const
        {
            evaluate().write(os,format);
        }

        void write(std::basic_ostream<char_type>& os, const basic_output_format<char_type>& format, bool indenting) const
        {
            evaluate().write(os,format,indenting);
        }
#if !defined(JSONCONS_NO_DEPRECATED)
        void to_stream(basic_json_output_handler<char_type>& handler) const
        {
            evaluate().to_stream(handler);
        }

        void to_stream(std::basic_ostream<char_type>& os) const
        {
            evaluate().to_stream(os);
        }

        void to_stream(std::basic_ostream<char_type>& os, const basic_output_format<char_type>& format) const
        {
            evaluate().to_stream(os,format);
        }

        void to_stream(std::basic_ostream<char_type>& os, const basic_output_format<char_type>& format, bool indenting) const
        {
            evaluate().to_stream(os,format,indenting);
        }
#endif
        void swap(json_type& val)
        {
            evaluate_with_default().swap(val);
        }

        friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_proxy& o)
        {
            o.write(os);
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

        const json_type& get(const string_type& name) const
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

        bool has_member(const string_type& name) const
        {
            return evaluate().has_member(name);
        }

        // Remove a range of elements from an array 
        void remove_range(size_t from_index, size_t to_index)
        {
            evaluate().remove_range(from_index, to_index);
        }
        // Remove a range of elements from an array 
        void remove(const string_type& name)
        {
            evaluate().remove(name);
        }
        void remove_member(const string_type& name)
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

    static basic_json parse(const string_type& s)
    {
        parse_error_handler_type err_handler;
        return parse(s,err_handler);
    }

    static basic_json parse(const char_type* s, size_t length)
    {
        parse_error_handler_type err_handler;
        return parse(s,length,err_handler);
    }

    static basic_json parse(const string_type& s, basic_parse_error_handler<char_type>& err_handler)
    {
        basic_json_deserializer<json_type> handler;
        basic_json_parser<char_type> parser(handler,err_handler);
        parser.begin_parse();
        parser.parse(s.data(),0,s.length());
        parser.end_parse();
        parser.check_done(s.data(),parser.index(),s.length());
        if (!handler.is_valid())
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json string");
        }
        return handler.get_result();
    }

    static basic_json parse(const char_type* s, size_t length, basic_parse_error_handler<char_type>& err_handler)
    {
        basic_json_deserializer<json_type> handler;
        basic_json_parser<char_type> parser(handler,err_handler);
        parser.begin_parse();
        parser.parse(s,0,length);
        parser.end_parse();
        parser.check_done(s,parser.index(),length);
        if (!handler.is_valid())
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json string");
        }
        return handler.get_result();
    }

    static basic_json parse_file(const std::string& s);

    static basic_json parse_file(const std::string& s, basic_parse_error_handler<char_type>& err_handler);

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

    static basic_json make_array(size_t n, const array_allocator& allocator = array_allocator())
    {
        return array(n,allocator);
    }

    template <class T>
    static basic_json make_array(size_t n, const T& val, const array_allocator& allocator = array_allocator())
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

    basic_json(json_type&& other, const Allocator& allocator)
        : var_(std::move(other.var_),allocator)
    {
    }

    basic_json(const array& val)
        : var_(val)
    {
    }

    basic_json(variant&& other)
        : var_(std::forward<variant>(other))
    {
    }

    basic_json(const variant& val)
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
    basic_json(const json_proxy<ParentT>& proxy, const Allocator& allocator = Allocator())
        : var_(proxy.evaluate().var_,allocator)
    {
    }

    template <class T>
    basic_json(const T& val)
        : var_(json_type_traits<json_type,T>::to_json(val).var_)
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

    template <class T>
    basic_json(const T& val, const Allocator& allocator)
        : var_(json_type_traits<json_type,T>::to_json(val,allocator).var_)
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
        size_t length = std::char_traits<char_type>::length(s);
        var_ = make_string(s,length).var_;
        return *this;
    }

    bool operator!=(const json_type& rhs) const;

    bool operator==(const json_type& rhs) const;

    size_t size() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return 0;
        case value_types::object_t:
            return var_.object_data_cast()->value().size();
        case value_types::array_t:
            return var_.array_data_cast()->value().size();
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

    json_proxy<json_type> operator[](const string_type& name)
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t: 
            create_object_implicitly();
        case value_types::object_t:
            return json_proxy<json_type>(*this, name);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    const json_type& operator[](const string_type& name) const
    {
        return at(name);
    }

    string_type to_string(const string_allocator& allocator=string_allocator()) const JSONCONS_NOEXCEPT
    {
        string_type s(allocator);
        std::basic_ostringstream<char_type,char_traits_type,string_allocator> os(s);
        {
            basic_json_serializer<char_type> serializer(os);
            write_body(serializer);
        }
        return os.str();
    }

    string_type to_string(const basic_output_format<char_type>& format,
                          const string_allocator& allocator=string_allocator()) const
    {
        string_type s(allocator);
        std::basic_ostringstream<char_type> os(s);
        {
            basic_json_serializer<char_type> serializer(os, format);
            write_body(serializer);
        }
        return os.str();
    }

    void write_body(basic_json_output_handler<char_type>& handler) const
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            handler.value(var_.small_string_data_cast()->data(),var_.small_string_data_cast()->length());
            break;
        case value_types::string_t:
            handler.value(var_.string_data_cast()->data(),var_.string_data_cast()->length());
            break;
        case value_types::double_t:
            handler.value(var_.double_data_cast()->value(), var_.double_data_cast()->precision());
            break;
        case value_types::integer_t:
            handler.value(var_.integer_data_cast()->value());
            break;
        case value_types::uinteger_t:
            handler.value(var_.uinteger_data_cast()->value());
            break;
        case value_types::bool_t:
            handler.value(var_.bool_data_cast()->value());
            break;
        case value_types::null_t:
            handler.value(null_type());
            break;
        case value_types::empty_object_t:
            handler.begin_object();
            handler.end_object();
            break;
        case value_types::object_t:
            {
                handler.begin_object();
                const object& o = var_.object_data_cast()->value();
                for (const_object_iterator it = o.begin(); it != o.end(); ++it)
                {
                    handler.name((it->name()).data(),it->name().length());
                    it->value().write_body(handler);
                }
                handler.end_object();
            }
            break;
        case value_types::array_t:
            {
                handler.begin_array();
                const array& o = var_.array_data_cast()->value();
                for (const_array_iterator it = o.begin(); it != o.end(); ++it)
                {
                    it->write_body(handler);
                }
                handler.end_array();
            }
            break;
        default:
            break;
        }
    }
    void write(basic_json_output_handler<char_type>& handler) const
    {
        handler.begin_json();
        write_body(handler);
        handler.end_json();
    }

    void write(std::basic_ostream<char_type>& os) const
    {
        basic_json_serializer<char_type> serializer(os);
        write(serializer);
    }

    void write(std::basic_ostream<char_type>& os, const basic_output_format<char_type>& format) const
    {
        basic_json_serializer<char_type> serializer(os, format);
        write(serializer);
    }

    void write(std::basic_ostream<char_type>& os, const basic_output_format<char_type>& format, bool indenting) const
    {
        basic_json_serializer<char_type> serializer(os, format, indenting);
        write(serializer);
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    void to_stream(basic_json_output_handler<char_type>& handler) const
    {
        handler.begin_json();
        write_body(handler);
        handler.end_json();
    }

    void to_stream(std::basic_ostream<char_type>& os) const
    {
        basic_json_serializer<char_type> serializer(os);
        to_stream(serializer);
    }

    void to_stream(std::basic_ostream<char_type>& os, const basic_output_format<char_type>& format) const
    {
        basic_json_serializer<char_type> serializer(os, format);
        to_stream(serializer);
    }

    void to_stream(std::basic_ostream<char_type>& os, const basic_output_format<char_type>& format, bool indenting) const
    {
        basic_json_serializer<char_type> serializer(os, format, indenting);
        to_stream(serializer);
    }
#endif
    bool is_null() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::null_t;
    }

    size_t count(const string_type& name) const
    {
        switch (var_.type_id())
        {
        case value_types::object_t:
            {
                auto it = var_.object_data_cast()->value().find(name.data(),name.length());
                if (it == members().end())
                {
                    return 0;
                }
                size_t count = 0;
                while (it != members().end() && it->name() == name)
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
        return (var_.type_id() == value_types::string_t) || (var_.type_id() == value_types::small_string_t);
    }


    bool is_bool() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::bool_t;
    }

    bool is_object() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::object_t || var_.type_id() == value_types::empty_object_t;
    }

    bool is_array() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::array_t;
    }

    bool is_integer() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::integer_t || (var_.type_id() == value_types::uinteger_t && (as_uinteger() <= static_cast<unsigned long long>(std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP())));
    }

    bool is_uinteger() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::uinteger_t || (var_.type_id() == value_types::integer_t && as_integer() >= 0);
    }

    bool is_double() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::double_t;
    }

    bool is_number() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::integer_t || var_.type_id() == value_types::uinteger_t || var_.type_id() == value_types::double_t;
    }

    bool empty() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            return var_.small_string_data_cast()->length() == 0;
        case value_types::string_t:
            return var_.string_data_cast()->length() == 0;
        case value_types::array_t:
            return var_.array_data_cast()->value().size() == 0;
        case value_types::empty_object_t:
            return true;
        case value_types::object_t:
            return var_.object_data_cast()->value().size() == 0;
        default:
            return false;
        }
    }

    size_t capacity() const
    {
        switch (var_.type_id())
        {
        case value_types::array_t:
            return var_.array_data_cast()->value().capacity();
        case value_types::object_t:
            return var_.object_data_cast()->value().capacity();
        default:
            return 0;
        }
    }

    template<class U=Allocator,
         typename std::enable_if<std::is_default_constructible<U>::value
            >::type* = nullptr>
    void create_object_implicitly()
    {
        var_ = variant(Allocator());
    }

    template<class U=Allocator,
         typename std::enable_if<!std::is_default_constructible<U>::value
            >::type* = nullptr>
    void create_object_implicitly() const
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Cannot create object implicitly - allocator is not default constructible.");
    }

    void reserve(size_t n)
    {
        switch (var_.type_id())
        {
        case value_types::array_t:
            var_.array_data_cast()->value().reserve(n);
            break;
        case value_types::empty_object_t:
        {
            create_object_implicitly();
            var_.object_data_cast()->value().reserve(n);
        }
        break;
        case value_types::object_t:
        {
            var_.object_data_cast()->value().reserve(n);
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
        case value_types::array_t:
            var_.array_data_cast()->value().resize(n);
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
        case value_types::array_t:
            var_.array_data_cast()->value().resize(n, val);
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
    typename std::enable_if<std::is_same<string_type,T>::value>::type as(const string_allocator& allocator) const
    {
        return json_type_traits<json_type,T>::as(*this,allocator);
    }

    bool as_bool() const 
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            try
            {
                json_type j = json_type::parse(var_.small_string_data_cast()->data(),var_.small_string_data_cast()->length());
                return j.as_bool();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a bool");
            }
        case value_types::string_t:
            try
            {
                json_type j = json_type::parse(var_.string_data_cast()->data(),var_.string_data_cast()->length());
                return j.as_bool();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a bool");
            }
        case value_types::bool_t:
            return var_.bool_data_cast()->value();
        case value_types::double_t:
            return var_.double_data_cast()->value() != 0.0;
        case value_types::integer_t:
            return var_.integer_data_cast()->value() != 0;
        case value_types::uinteger_t:
            return var_.uinteger_data_cast()->value() != 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a bool");
        }
    }

    int64_t as_integer() const
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            try
            {
                json_type j = json_type::parse(var_.small_string_data_cast()->data(),var_.small_string_data_cast()->length());
                return j.as<int64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
            }
        case value_types::string_t:
            try
            {
                json_type j = json_type::parse(var_.string_data_cast()->data(),var_.string_data_cast()->length());
                return j.as<int64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
            }
        case value_types::double_t:
            return static_cast<int64_t>(var_.double_data_cast()->value());
        case value_types::integer_t:
            return static_cast<int64_t>(var_.integer_data_cast()->value());
        case value_types::uinteger_t:
            return static_cast<int64_t>(var_.uinteger_data_cast()->value());
        case value_types::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
        }
    }

    uint64_t as_uinteger() const
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            try
            {
                json_type j = json_type::parse(var_.small_string_data_cast()->data(),var_.small_string_data_cast()->length());
                return j.as<uint64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
            }
        case value_types::string_t:
            try
            {
                json_type j = json_type::parse(var_.string_data_cast()->data(),var_.string_data_cast()->length());
                return j.as<uint64_t>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
            }
        case value_types::double_t:
            return static_cast<uint64_t>(var_.double_data_cast()->value());
        case value_types::integer_t:
            return static_cast<uint64_t>(var_.integer_data_cast()->value());
        case value_types::uinteger_t:
            return static_cast<uint64_t>(var_.uinteger_data_cast()->value());
        case value_types::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
        }
    }

    size_t double_precision() const
    {
        switch (var_.type_id())
        {
        case value_types::double_t:
            return var_.double_data_cast()->precision();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
        }
    }

    double as_double() const
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            try
            {
                json_type j = json_type::parse(var_.small_string_data_cast()->data(),var_.small_string_data_cast()->length());
                return j.as<double>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
            }
        case value_types::string_t:
            try
            {
                json_type j = json_type::parse(var_.string_data_cast()->data(),var_.string_data_cast()->length());
                return j.as<double>();
            }
            catch (...)
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
            }
        case value_types::double_t:
            return var_.double_data_cast()->value();
        case value_types::integer_t:
            return static_cast<double>(var_.integer_data_cast()->value());
        case value_types::uinteger_t:
            return static_cast<double>(var_.uinteger_data_cast()->value());
        case value_types::null_t:
            return std::numeric_limits<double>::quiet_NaN();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
        }
    }

    string_type as_string() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            return string_type(var_.small_string_data_cast()->data(),var_.small_string_data_cast()->length());
        case value_types::string_t:
            return string_type(var_.string_data_cast()->data(),var_.string_data_cast()->length(),var_.string_data_cast()->get_allocator());
        default:
            return to_string();
        }
    }

    string_type as_string(const string_allocator& allocator) const JSONCONS_NOEXCEPT
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            return string_type(var_.small_string_data_cast()->data(),var_.small_string_data_cast()->length(),allocator);
        case value_types::string_t:
            return string_type(var_.string_data_cast()->data(),var_.string_data_cast()->length(),allocator);
        default:
            return to_string(allocator);
        }
    }

    string_type as_string(const basic_output_format<char_type>& format) const 
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            return string_type(var_.small_string_data_cast()->data(),var_.small_string_data_cast()->length());
        case value_types::string_t:
            return string_type(var_.string_data_cast()->data(),var_.string_data_cast()->length(),var_.string_data_cast()->get_allocator());
        default:
            return to_string(format);
        }
    }

    string_type as_string(const basic_output_format<char_type>& format,
                          const string_allocator& allocator) const 
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            return string_type(var_.small_string_data_cast()->data(),var_.small_string_data_cast()->length(),allocator);
        case value_types::string_t:
            return string_type(var_.string_data_cast()->data(),var_.string_data_cast()->length(),allocator);
        default:
            return to_string(format,allocator);
        }
    }

    const char_type* as_cstring() const
    {
        switch (var_.type_id())
        {
        case value_types::small_string_t:
            return var_.small_string_data_cast()->c_str();
        case value_types::string_t:
            return var_.string_data_cast()->c_str();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a cstring");
        }
    }
    json_type& at(const string_type& name)
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case value_types::object_t:
            {
                auto it = var_.object_data_cast()->value().find(name.data(),name.length());
                if (it == members().end())
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

    json_type& evaluate(size_t i) 
    {
        return at(i);
    }

    const json_type& evaluate(size_t i) const
    {
        return at(i);
    }

    json_type& evaluate(const string_type& name) 
    {
        return at(name);
    }

    const json_type& evaluate(const string_type& name) const
    {
        return at(name);
    }

    const json_type& at(const string_type& name) const
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case value_types::object_t:
            {
                auto it = var_.object_data_cast()->value().find(name.data(),name.length());
                if (it == members().end())
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
        case value_types::array_t:
            if (i >= var_.array_data_cast()->value().size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return var_.array_data_cast()->value().operator[](i);
        case value_types::object_t:
            return var_.object_data_cast()->value().at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    const json_type& at(size_t i) const
    {
        switch (var_.type_id())
        {
        case value_types::array_t:
            if (i >= var_.array_data_cast()->value().size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return var_.array_data_cast()->value().operator[](i);
        case value_types::object_t:
            return var_.object_data_cast()->value().at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    object_iterator find(const string_type& name)
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.object_data_cast()->value().find(name.data(),name.length());
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const_object_iterator find(const string_type& name) const
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.object_data_cast()->value().find(name.data(),name.length());
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    object_iterator find(const char_type* name)
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.object_data_cast()->value().find(name, std::char_traits<char_type>::length(name));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const_object_iterator find(const char_type* name) const
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.object_data_cast()->value().find(name, std::char_traits<char_type>::length(name));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    object_iterator find(const char_type* name, size_t length)
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.object_data_cast()->value().find(name, length);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const_object_iterator find(const char_type* name, size_t length) const
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.object_data_cast()->value().find(name, length);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    template<class T>
    json_type get(const string_type& name, T&& default_val) const
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            {
                return json_type(std::forward<T&&>(default_val));
            }
        case value_types::object_t:
            {
                const_object_iterator it = var_.object_data_cast()->value().find(name.data(),name.length());
                if (it != members().end())
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
    T get_with_default(const string_type& name, const T& default_val) const
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            {
                return default_val;
            }
        case value_types::object_t:
            {
                const_object_iterator it = var_.object_data_cast()->value().find(name.data(),name.length());
                if (it != members().end())
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

    const CharT* get_with_default(const string_type& name, const CharT* default_val) const
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            {
                return default_val;
            }
        case value_types::object_t:
            {
                const_object_iterator it = var_.object_data_cast()->value().find(name.data(),name.length());
                if (it != members().end())
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
        case value_types::array_t:
            var_.array_data_cast()->value().shrink_to_fit();
            break;
        case value_types::object_t:
            var_.object_data_cast()->value().shrink_to_fit();
            break;
        default:
            break;
        }
    }

    void clear()
    {
        switch (var_.type_id())
        {
        case value_types::array_t:
            var_.array_data_cast()->value().clear();
            break;
        case value_types::object_t:
            var_.object_data_cast()->value().clear();
            break;
        default:
            break;
        }
    }

    void erase(object_iterator first, object_iterator last)
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            break;
        case value_types::object_t:
            var_.object_data_cast()->value().erase(first, last);
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
        case value_types::array_t:
            var_.array_data_cast()->value().erase(first, last);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
            break;
        }
    }

    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void erase(const string_type& name)
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            break;
        case value_types::object_t:
            var_.object_data_cast()->value().erase(name.data(),name.length());
            break;
        default:
            JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to erase %s on a value that is not an object", name);
            break;
        }
    }

    template <class T>
    void set(const string_type& name, T&& value)
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            var_.object_data_cast()->value().set(name, std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    void set(string_type&& name, T&& value){
        switch (var_.type_id()){
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            var_.object_data_cast()->value().set(std::forward<string_type&&>(name),std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    template <class T>
    object_iterator set(object_iterator hint, const string_type& name, T&& value)
    {
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            return var_.object_data_cast()->value().set(hint, name, std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    template <class T>
    object_iterator set(object_iterator hint, string_type&& name, T&& value){
        switch (var_.type_id()){
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            return var_.object_data_cast()->value().set(hint, std::forward<string_type&&>(name),std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    template <class T>
    void add(T&& value)
    {
        switch (var_.type_id())
        {
        case value_types::array_t:
            var_.array_data_cast()->value().add(std::forward<T&&>(value));
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
        case value_types::array_t:
            return var_.array_data_cast()->value().add(pos, std::forward<T&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    value_types type_id() const
    {
        return var_.type_id();
    }

    void swap(json_type& b)
    {
        var_.swap(b.var_);
    }

    template <class T>
    std::vector<T> as_vector() const
    {
        std::vector<T> v(size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = json_type_traits<json_type,T>::as(at(i));
        }
        return v;
    }

    friend void swap(json_type& a, json_type& b)
    {
        a.swap(b);
    }

    static json_type make_string(const string_type& s)
    {
        return json_type(variant(s.data(),s.length()));
    }

    static json_type make_string(const string_type& s, allocator_type allocator)
    {
        return json_type(variant(s.data(),s.length(),allocator));
    }

    static json_type make_string(const char_type* rhs, size_t length)
    {
        return json_type(variant(rhs,length));
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
        return members().begin();
    }

    const_object_iterator begin_members() const
    {
        return members().begin();
    }

    object_iterator end_members()
    {
        return members().end();
    }

    const_object_iterator end_members() const
    {
        return members().end();
    }

    array_iterator begin_elements()
    {
        return elements().begin();
    }

    const_array_iterator begin_elements() const
    {
        return elements().begin();
    }

    array_iterator end_elements()
    {
        return elements().end();
    }

    const_array_iterator end_elements() const
    {
        return elements().end();
    }

    const json_type& get(const string_type& name) const
    {
        static const json_type a_null = null_type();

        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return a_null;
        case value_types::object_t:
            {
                const_object_iterator it = var_.object_data_cast()->value().find(name.data(),name.length());
                return it != members().end() ? it->value() : a_null;
            }
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    bool is_longlong() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::integer_t;
    }

    bool is_ulonglong() const JSONCONS_NOEXCEPT
    {
        return var_.type_id() == value_types::uinteger_t;
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
        case value_types::double_t:
            return static_cast<int>(var_.double_data_cast()->value());
        case value_types::integer_t:
            return static_cast<int>(var_.integer_data_cast()->value());
        case value_types::uinteger_t:
            return static_cast<int>(var_.uinteger_data_cast()->value());
        case value_types::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an int");
        }
    }

    unsigned int as_uint() const
    {
        switch (var_.type_id())
        {
        case value_types::double_t:
            return static_cast<unsigned int>(var_.double_data_cast()->value());
        case value_types::integer_t:
            return static_cast<unsigned int>(var_.integer_data_cast()->value());
        case value_types::uinteger_t:
            return static_cast<unsigned int>(var_.uinteger_data_cast()->value());
        case value_types::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned int");
        }
    }

    long as_long() const
    {
        switch (var_.type_id())
        {
        case value_types::double_t:
            return static_cast<long>(var_.double_data_cast()->value());
        case value_types::integer_t:
            return static_cast<long>(var_.integer_data_cast()->value());
        case value_types::uinteger_t:
            return static_cast<long>(var_.uinteger_data_cast()->value());
        case value_types::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a long");
        }
    }

    unsigned long as_ulong() const
    {
        switch (var_.type_id())
        {
        case value_types::double_t:
            return static_cast<unsigned long>(var_.double_data_cast()->value());
        case value_types::integer_t:
            return static_cast<unsigned long>(var_.integer_data_cast()->value());
        case value_types::uinteger_t:
            return static_cast<unsigned long>(var_.uinteger_data_cast()->value());
        case value_types::bool_t:
            return var_.bool_data_cast()->value() ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned long");
        }
    }

    void add(size_t index, const json_type& value)
    {
        switch (var_.type_id())
        {
        case value_types::array_t:
            var_.array_data_cast()->value().add(index, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    void add(size_t index, json_type&& value){
        switch (var_.type_id()){
        case value_types::array_t:
            var_.array_data_cast()->value().add(index, std::forward<json_type&&>(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    bool has_member(const string_type& name) const
    {
        switch (var_.type_id())
        {
        case value_types::object_t:
            {
                const_object_iterator it = var_.object_data_cast()->value().find(name.data(),name.length());
                return it != members().end();
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
        case value_types::array_t:
            var_.array_data_cast()->value().remove_range(from_index, to_index);
            break;
        default:
            break;
        }
    }
    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void remove(const string_type& name)
    {
        erase(name.data(),name.length());
    }
    void remove_member(const string_type& name)
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
#endif

    object_range members()
    {
        static json_type empty_object = object();
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return object_range(empty_object.members().begin(), empty_object.members().end());
        case value_types::object_t:
            return object_range(object_value().begin(),object_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    const_object_range members() const
    {
        static const json_type empty_object = object();
        switch (var_.type_id())
        {
        case value_types::empty_object_t:
            return const_object_range(empty_object.members().begin(), empty_object.members().end());
        case value_types::object_t:
            return const_object_range(object_value().begin(),object_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    array_range elements()
    {
        switch (var_.type_id())
        {
        case value_types::array_t:
            return array_range(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    const_array_range elements() const
    {
        switch (var_.type_id())
        {
        case value_types::array_t:
            return const_array_range(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    array& array_value() 
    {
        switch (var_.type_id())
        {
        case value_types::array_t:
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
        case value_types::array_t:
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
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
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
        case value_types::empty_object_t:
            const_cast<json_type*>(this)->create_object_implicitly(); // HERE
        case value_types::object_t:
            return var_.object_data_cast()->value();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad object cast");
            break;
        }
    }

private:

    friend std::basic_ostream<typename string_type::value_type>& operator<<(std::basic_ostream<typename string_type::value_type>& os, const json_type& o)
    {
        o.write(os);
        return os;
    }

    friend std::basic_istream<typename string_type::value_type>& operator<<(std::basic_istream<typename string_type::value_type>& is, json_type& o)
    {
        basic_json_deserializer<json_type> handler;
        basic_json_reader<typename string_type::value_type> reader(is, handler);
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
void swap(typename Json::member_type& a, typename Json::member_type& b)
{
    a.swap(b);
}

template<class CharT,class JsonTraits,class Allocator>
bool basic_json<CharT,JsonTraits,Allocator>::operator!=(const basic_json<CharT,JsonTraits,Allocator>& rhs) const
{
    return !(*this == rhs);
}

template<class CharT,class JsonTraits,class Allocator>
bool basic_json<CharT,JsonTraits,Allocator>::operator==(const basic_json<CharT,JsonTraits,Allocator>& rhs) const
{
    return var_ == rhs.var_;
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
    basic_json_deserializer<basic_json<CharT,JsonTraits,Allocator>> handler;
    basic_json_reader<char_type> reader(is, handler, err_handler);
    reader.read_next();
    reader.check_done();
    if (!handler.is_valid())
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
    }
    return handler.get_result();
}

template<class CharT,class JsonTraits,class Allocator>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::parse_file(const std::string& filename)
{
    parse_error_handler_type err_handler;
    return parse_file(filename,err_handler);
}

template<class CharT,class JsonTraits,class Allocator>
basic_json<CharT,JsonTraits,Allocator> basic_json<CharT,JsonTraits,Allocator>::parse_file(const std::string& filename, 
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

    basic_json_deserializer<basic_json<CharT,JsonTraits,Allocator>> handler;
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
            parser.begin_parse();
            parser.parse(buffer.data(),0,buffer.size());
            parser.end_parse();
            parser.check_done(buffer.data(),parser.index(),buffer.size());
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

template <class Json>
std::basic_istream<typename Json::char_type>& operator>>(std::basic_istream<typename Json::char_type>& is, Json& o)
{
    basic_json_deserializer<Json> handler;
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
                   const basic_output_format<char_type>& format)
       : o_(&o), is_pretty_print_(is_pretty_print), format_(format)
    {
        ;
    }

    void write(std::basic_ostream<char_type>& os) const
    {
        o_->write(os, format_, is_pretty_print_);
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_printable<Json>& o)
    {
        o.write(os);
        return os;
    }

    const Json *o_;
    bool is_pretty_print_;
    basic_output_format<char_type> format_;
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
                            const basic_output_format<typename Json::char_type>& format)
{
    return json_printable<Json>(val, false, format);
}

template<class Json>
json_printable<Json> pretty_print(const Json& val)
{
    return json_printable<Json>(val,true);
}

template<class Json>
json_printable<Json> pretty_print(const Json& val,
                                   const basic_output_format<typename Json::char_type>& format)
{
    return json_printable<Json>(val, true, format);
}

typedef basic_json<char,json_traits<char>,std::allocator<char>> json;
typedef basic_json<wchar_t,json_traits<wchar_t>,std::allocator<wchar_t>> wjson;

typedef basic_json_deserializer<json> json_deserializer;
typedef basic_json_deserializer<wjson> wjson_deserializer;

typedef basic_json<char,ojson_traits<char>,std::allocator<char>> ojson;
typedef basic_json<wchar_t,ojson_traits<wchar_t>,std::allocator<wchar_t>> wojson;

typedef basic_json_deserializer<ojson> ojson_deserializer;
typedef basic_json_deserializer<wojson> wojson_deserializer;
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
