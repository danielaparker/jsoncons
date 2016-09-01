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
#include "jsoncons/jsoncons.hpp"
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

#if !defined(JSONCONS_NO_DEPRECATED)

template <class CharT, class Allocator>
class serializable_any
{
public:
    typedef Allocator allocator_type;

    serializable_any(const Allocator& allocator = Allocator())
        : impl_(nullptr)
    {
        (void)allocator;
    }
    serializable_any(const serializable_any& val)
        : allocator_(std::allocator_traits<allocator_type>::select_on_container_copy_construction(val.get_allocator()))
    {
        impl_ = val.impl_ != nullptr ? val.impl_->clone(allocator_) : nullptr;
    }
    serializable_any(const serializable_any& val, const Allocator& allocator)
    {
        (void)allocator;
        impl_ = val.impl_ != nullptr ? val.impl_->clone(Allocator()) : nullptr;
    }

    serializable_any(serializable_any&& val)
        : impl_(std::move(val.impl_))
    {
        val.impl_ = nullptr;
    }
    serializable_any(serializable_any&& val, const Allocator& allocator)
        : impl_(std::move(val.impl_))
    {
        (void)allocator;
        val.impl_ = nullptr;
    }
    ~serializable_any()
    {
        if (impl_ != nullptr)
        {
            destroy_impl(allocator_,impl_);
        }
    }

    template<class T>
    explicit serializable_any(T val)
    {
        impl_ = create_impl<any_handle_impl<typename type_wrapper<T>::value_type>>(allocator_,val);
    }

    Allocator get_allocator() const
    {
        return allocator_;
    }

    template <class T>
    typename type_wrapper<T>::reference cast() 
    {
        if (typeid(*impl_) != typeid(any_handle_impl<typename type_wrapper<T>::value_type>))
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad serializable_any cast");
        }
        return static_cast<any_handle_impl<typename type_wrapper<T>::value_type>&>(*impl_).value_;
    }

    template <class T>
    typename type_wrapper<T>::const_reference cast() const
    {
        if (typeid(*impl_) != typeid(any_handle_impl<typename type_wrapper<T>::value_type>))
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad serializable_any cast");
        }
        return static_cast<any_handle_impl<typename type_wrapper<T>::value_type>&>(*impl_).value_;
    }

    serializable_any& operator=(serializable_any rhs)
    {
        std::swap(impl_,rhs.impl_);
        return *this;
    }

    void to_stream(basic_json_output_handler<CharT>& os) const 
    {
        impl_->to_stream(os);
    }

    class any_handle
    {
    public:
        virtual ~any_handle()
        {
        }

        virtual any_handle* clone(const Allocator& allocator) const = 0;

        virtual void to_stream(basic_json_output_handler<CharT>& os) const = 0;
    };

    template <class T>
    class any_handle_impl : public any_handle
    {
    public:
        any_handle_impl(T value, const Allocator& allocator = Allocator())
            : value_(value)
        {
            (void)allocator;
        }

        virtual any_handle* clone(const Allocator& allocator) const
        {
            return create_impl<any_handle_impl<T>>(allocator, value_);
        }

        virtual void to_stream(basic_json_output_handler<CharT>& os) const
        {
            serialize(os,value_);
        }

        T value_;
    };

    Allocator allocator_;
    any_handle* impl_;
};

template <class CharT,class T> inline
void serialize(basic_json_output_handler<CharT>& os, const T&)
{
    os.value(null_type());
}
#endif

enum class value_types : uint8_t 
{
    // Simple types
    empty_object_t,
    small_string_t,
    double_t,
    integer_t,
    uinteger_t,
    bool_t,
    null_t,
    // Non simple types
    string_t,
    object_t,
    array_t
#if !defined(JSONCONS_NO_DEPRECATED)
    ,
    any_t
#endif
};

inline
bool is_simple(value_types type)
{
    return type < value_types::string_t;
}

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

#if !defined(JSONCONS_NO_DEPRECATED)
    typedef serializable_any<char_type,Allocator> any;
#endif

    typedef jsoncons::null_type null_type;

    typedef typename object::iterator object_iterator;
    typedef typename object::const_iterator const_object_iterator;
    typedef typename array::iterator array_iterator;
    typedef typename array::const_iterator const_array_iterator;
    typedef typename string_type::pointer string_iterator;
    typedef typename string_type::const_pointer const_string_iterator;

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
        friend class basic_json<CharT,JsonTraits,Allocator>;

        IteratorT begin()
        {
            return first_;
        }
        IteratorT end()
        {
            return last_;
        }
    };

    typedef range<object_iterator> object_range;
    typedef range<const_object_iterator> const_object_range;
    typedef range<array_iterator> array_range;
    typedef range<const_array_iterator> const_array_range;
    typedef range<string_iterator> string_range;
    typedef range<const_string_iterator> const_string_range;

    struct variant
    {
        struct string_data : public string_allocator
        {
            const char_type* c_str() const { return p_; }
            const char_type* data() const { return p_; }
            size_t length() const { return length_; }
            string_allocator get_allocator() const
            {
                return *this;
            }

            bool operator==(const string_data& rhs) const
            {
                return length() == rhs.length() ? std::char_traits<char_type>::compare(data(), rhs.data(), length()) == 0 : false;
            }

            string_data(const string_allocator& allocator)
                : string_allocator(allocator), p_(nullptr), length_(0)
            {
            }

            char_type* p_;
            size_t length_;
        private:
            string_data(const string_data&);
            string_data& operator=(const string_data&);
        };

        struct string_dataA
        {
            string_data data;
            char_type c[1];
        };
        typedef typename std::aligned_storage<sizeof(string_dataA), JSONCONS_ALIGNOF(string_dataA)>::type storage_type;

        static size_t aligned_size(size_t n)
        {
            return sizeof(storage_type) + n;
        }

        string_data* create_string_data(const char_type* s, size_t length, const string_allocator& allocator)
        {
            size_t mem_size = aligned_size(length*sizeof(char_type));

            typename std::allocator_traits<string_allocator>:: template rebind_alloc<char> alloc(allocator);

            char* storage = alloc.allocate(mem_size);
            string_data* ps = new(storage)string_data(allocator);
            auto psa = reinterpret_cast<string_dataA*>(storage); 

            ps->p_ = new(&psa->c)char_type[length + 1];
            memcpy(ps->p_, s, length*sizeof(char_type));
            ps->p_[length] = 0;
            ps->length_ = length;
            return ps;
        }

        void destroy_string_data(const string_allocator& allocator, string_data* p)
        {
            size_t mem_size = aligned_size(p->length_*sizeof(char_type));
            typename std::allocator_traits<string_allocator>:: template rebind_alloc<char> alloc(allocator);
            alloc.deallocate(reinterpret_cast<char*>(p),mem_size);
        }

        static const size_t small_string_capacity = (sizeof(int64_t)/sizeof(char_type)) - 1;

        variant()
            : type_(value_types::empty_object_t), length_or_precision_(0)
        {
        }

        variant(const Allocator& a)
            : type_(value_types::object_t), length_or_precision_(0)
        {
            value_.object_val_ = create_impl<object>(a, object_allocator(a));
        }

        explicit variant(variant&& var)
            : type_(value_types::null_t), length_or_precision_(0)
        {
            swap(var);
        }
        
        explicit variant(variant&& var, const Allocator& a)
            : type_(value_types::null_t), length_or_precision_(0)
        {
            swap(var);
        }

        explicit variant(const variant& var)
            : length_or_precision_(0)
        {
            init_variant(var);
        }
        explicit variant(const variant& var, const Allocator&)
            : type_(var.type_), length_or_precision_(0)
        {
            init_variant(var);
        }

        variant(const object & val)
            : type_(value_types::object_t), length_or_precision_(0)
        {
            value_.object_val_ = create_impl<object>(val.get_allocator(), val) ;
        }

        variant(const object & val, const Allocator& a)
            : type_(value_types::object_t), length_or_precision_(0)
        {
            value_.object_val_ = create_impl<object>(a, val, object_allocator(a)) ;
        }

        variant(object&& val)
            : type_(value_types::object_t), length_or_precision_(0)
        {
            value_.object_val_ = create_impl<object>(val.get_allocator(), std::move(val));
        }

        variant(object&& val, const Allocator& a)
            : type_(value_types::object_t), length_or_precision_(0)
        {
            value_.object_val_ = create_impl<object>(a, std::move(val), object_allocator(a));
        }

        variant(const array& val)
            : type_(value_types::array_t), length_or_precision_(0)
        {
            value_.array_val_ = create_impl<array>(val.get_allocator(), val);
        }

        variant(const array& val, const Allocator& a)
            : type_(value_types::array_t), length_or_precision_(0)
        {
            value_.array_val_ = create_impl<array>(a, val, array_allocator(a));
        }

        variant(array&& val)
            : type_(value_types::array_t), length_or_precision_(0)
        {
            value_.array_val_ = create_impl<array>(val.get_allocator(), std::move(val));
        }

        variant(array&& val, const Allocator& a)
            : type_(value_types::array_t), length_or_precision_(0)
        {
            value_.array_val_ = create_impl<array>(a, std::move(val), array_allocator(a));
        }

#if !defined(JSONCONS_NO_DEPRECATED)
        explicit variant(const any& val, const Allocator& a)
            : type_(value_types::any_t), length_or_precision_(0)
        {
            value_.any_val_ = create_impl<any>(a, val);
        }
#endif
        explicit variant(null_type)
            : type_(value_types::null_t), length_or_precision_(0)
        {
        }

        explicit variant(bool val)
            : type_(value_types::bool_t), length_or_precision_(0)
        {
            value_.bool_val_ = val;
        }

        explicit variant(double val, uint8_t precision)
            : type_(value_types::double_t), length_or_precision_(precision)
        {
            value_.double_val_ = val;
        }

        explicit variant(int64_t val)
            : type_(value_types::integer_t), length_or_precision_(0)
        {
            value_.integer_val_ = val;
        }

        explicit variant(uint64_t val)
            : type_(value_types::uinteger_t), length_or_precision_(0)
        {
            value_.uinteger_val_ = val;
        }

        explicit variant(const string_type& s, const Allocator& a)
            : length_or_precision_(0)
        {
            if (s.length() > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                //value_.string_val_ = create_impl<string_type>(a, s, string_allocator(a));
                value_.string_val_ = create_string_data(s.data(), s.length(), string_allocator(a));
            }
            else
            {
                type_ = value_types::small_string_t;
                length_or_precision_ = static_cast<uint8_t>(s.length());
                std::memcpy(value_.small_string_val_,s.data(),s.length()*sizeof(char_type));
                value_.small_string_val_[length_or_precision_] = 0;
            }
        }

        explicit variant(const char_type* s, const Allocator& a)
            : length_or_precision_(0)
        {
            size_t length = std::char_traits<char_type>::length(s);
            if (length > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                //value_.string_val_ = create_impl<string_type>(a, s, string_allocator(a));
                value_.string_val_ = create_string_data(s, length, string_allocator(a));
            }
            else
            {
                type_ = value_types::small_string_t;
                length_or_precision_ = static_cast<uint8_t>(length);
                std::memcpy(value_.small_string_val_,s,length*sizeof(char_type));
                value_.small_string_val_[length_or_precision_] = 0;
            }
        }

        explicit variant(const char_type* s, size_t length, const Allocator& a)
            : length_or_precision_(0)
        {
            if (length > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                //value_.string_val_ = create_impl<string_type>(a, s, length, string_allocator(a));
                value_.string_val_ = create_string_data(s, length, string_allocator(a));
            }
            else
            {
                type_ = value_types::small_string_t;
                length_or_precision_ = static_cast<uint8_t>(length);
                std::memcpy(value_.small_string_val_,s,length*sizeof(char_type));
                value_.small_string_val_[length_or_precision_] = 0;
            }
        }

        template<class InputIterator>
        variant(InputIterator first, InputIterator last, const Allocator& a)
            : type_(value_types::array_t), length_or_precision_(0)
        {
            value_.array_val_ = create_impl<array>(a, first, last, array_allocator(a));
        }

        void init_variant(const variant& var)
        {
            type_ = var.type_;
            switch (type_)
            {
            case value_types::null_t:
            case value_types::empty_object_t:
                break;
            case value_types::double_t:
                length_or_precision_ = 0;
                value_.double_val_ = var.value_.double_val_;
                break;
            case value_types::integer_t:
                value_.integer_val_ = var.value_.integer_val_;
                break;
            case value_types::uinteger_t:
                value_.uinteger_val_ = var.value_.uinteger_val_;
                break;
            case value_types::bool_t:
                value_.bool_val_ = var.value_.bool_val_;
                break;
            case value_types::small_string_t:
                length_or_precision_ = var.length_or_precision_;
                std::memcpy(value_.small_string_val_,var.value_.small_string_val_,var.length_or_precision_*sizeof(char_type));
                value_.small_string_val_[length_or_precision_] = 0;
                break;
            case value_types::string_t:
                //value_.string_val_ = create_impl<string_type>(var.value_.string_val_->get_allocator(), *(var.value_.string_val_), string_allocator(var.value_.string_val_->get_allocator()));
                value_.string_val_ = create_string_data(var.value_.string_val_->data(), var.value_.string_val_->length(), string_allocator(var.value_.string_val_->get_allocator()));
                break;
            case value_types::array_t:
                value_.array_val_ = create_impl<array>(var.value_.array_val_->get_allocator(), *(var.value_.array_val_), array_allocator(var.value_.array_val_->get_allocator()));
                break;
            case value_types::object_t:
                value_.object_val_ = create_impl<object>(var.value_.object_val_->get_allocator(), *(var.value_.object_val_), object_allocator(var.value_.object_val_->get_allocator()));
                break;

#if !defined(JSONCONS_NO_DEPRECATED)
            case value_types::any_t:
                value_.any_val_ = create_impl<any>(var.value_.any_val_->get_allocator(), *(var.value_.any_val_));
                break;
#endif
            default:
                break;
            }
        }

        ~variant()
        {
            destroy_variant();
        }

        void destroy_variant()
        {
            switch (type_)
            {
            case value_types::string_t:
                //destroy_impl(value_.string_val_->get_allocator(), value_.string_val_);
                destroy_string_data(value_.string_val_->get_allocator(), value_.string_val_);
                break;
            case value_types::array_t:
                destroy_impl(value_.array_val_->get_allocator(), value_.array_val_);
                break;
            case value_types::object_t:
                destroy_impl(value_.object_val_->get_allocator(), value_.object_val_);
                break;

#if !defined(JSONCONS_NO_DEPRECATED)
            case value_types::any_t:
                destroy_impl(value_.any_val_->get_allocator(), value_.any_val_);
                break;
#endif
            default:
                break; 
            }
        }

        variant& operator=(const variant& val)
        {
            if (this != &val)
            {
                if (is_simple(type_))
                {
                    if (is_simple(val.type_))
                    {
                        type_ = val.type_;
                        length_or_precision_ = val.length_or_precision_;
                        value_ = val.value_;
                    }
                    else
                    {
                        init_variant(val);
                    }
                }
                else
                {
                    destroy_variant();
                    init_variant(val);
                }
            }
            return *this;
        }

        variant& operator=(variant&& val)
        {
            if (this != &val)
            {
                val.swap(*this);
            }
            return *this;
        }

        void assign(const object & val)
        {
            destroy_variant();
            type_ = value_types::object_t;
            value_.object_val_ = create_impl<object>(val.get_allocator(), val, object_allocator(val.get_allocator()));
        }

        void assign(object && val)
        {
            switch (type_)
            {
            case value_types::object_t:
                value_.object_val_->swap(val);
                break;
            default:
                destroy_variant();
                type_ = value_types::object_t;
                value_.object_val_ = create_impl<object>(val.get_allocator(), std::move(val), object_allocator(val.get_allocator()));
                break;
            }
        }

        void assign(const array& val)
        {
            destroy_variant();
            type_ = value_types::array_t;
            value_.array_val_ = create_impl<array>(val.get_allocator(), val, array_allocator(val.get_allocator())) ;
        }

        void assign(array&& val)
        {
            switch (type_)
            {
            case value_types::array_t:
                value_.array_val_->swap(val);
                break;
            default:
                destroy_variant();
                type_ = value_types::array_t;
                value_.array_val_ = create_impl<array>(val.get_allocator(), std::move(val), array_allocator(val.get_allocator()));
                break;
            }
        }

        void assign(const string_type& s)
        {
            destroy_variant();
            if (s.length() > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                //value_.string_val_ = create_impl<string_type>(s.get_allocator(), s, string_allocator(s.get_allocator()));
                value_.string_val_ = create_string_data(s.data(), s.length(), string_allocator(s.get_allocator()));
            }
            else
            {
                type_ = value_types::small_string_t;
                length_or_precision_ = static_cast<uint8_t>(s.length());
                std::memcpy(value_.small_string_val_,s.data(),s.length()*sizeof(char_type));
                value_.small_string_val_[length_or_precision_] = 0;
            }
        }

        void assign_string(const char_type* s, size_t length, const Allocator& allocator = Allocator())
        {
            destroy_variant();
            if (length > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                //value_.string_val_ = create_impl<string_type>(allocator, s, length, string_allocator(allocator));
                value_.string_val_ = create_string_data(s, length, string_allocator(allocator));
            }
            else
            {
                type_ = value_types::small_string_t;
                length_or_precision_ = static_cast<uint8_t>(length);
                std::memcpy(value_.small_string_val_,s,length*sizeof(char_type));
                value_.small_string_val_[length_or_precision_] = 0;
            }
        }

        void assign(int64_t val)
        {
            destroy_variant();
            type_ = value_types::integer_t;
            value_.integer_val_ = val;
        }

        void assign(uint64_t val)
        {
            destroy_variant();
            type_ = value_types::uinteger_t;
            value_.uinteger_val_ = val;
        }

        void assign(double val, uint8_t precision = 0)
        {
            destroy_variant();
            type_ = value_types::double_t;
            length_or_precision_ = precision;
            value_.double_val_ = val;
        }

        void assign(bool val)
        {
            destroy_variant();
            type_ = value_types::bool_t;
            value_.bool_val_ = val;
        }

        void assign(null_type)
        {
            destroy_variant();
            type_ = value_types::null_t;
        }

#if !defined(JSONCONS_NO_DEPRECATED)
        void assign(const any& rhs)
        {
            destroy_variant();
            type_ = value_types::any_t;
            value_.any_val_ = create_impl<any>(rhs.get_allocator(), rhs);
        }
#endif
        bool operator!=(const variant& rhs) const
        {
            return !(*this == rhs);
        }

        bool operator==(const variant& rhs) const
        {
            if (is_number() & rhs.is_number())
            {
                switch (type_)
                {
                case value_types::integer_t:
                    switch (rhs.type_)
                    {
                    case value_types::integer_t:
                        return value_.integer_val_ == rhs.value_.integer_val_;
                    case value_types::uinteger_t:
                        return value_.integer_val_ >= 0 && (static_cast<uint64_t>(value_.integer_val_) == rhs.value_.uinteger_val_);
                    case value_types::double_t:
                        return static_cast<double>(value_.integer_val_) == rhs.value_.double_val_;
                    default:
                        break;
                    }
                    break;
                case value_types::uinteger_t:
                    switch (rhs.type_)
                    {
                    case value_types::integer_t:
                        return value_.uinteger_val_ == static_cast<uint64_t>(rhs.value_.integer_val_) && rhs.value_.integer_val_ >= 0;
                    case value_types::uinteger_t:
                        return value_.uinteger_val_ == rhs.value_.uinteger_val_;
                    case value_types::double_t:
                        return static_cast<double>(value_.uinteger_val_) == rhs.value_.double_val_;
                    default:
                        break;
                    }
                    break;
                case value_types::double_t:
                    switch (rhs.type_)
                    {
                    case value_types::integer_t:
                        return value_.double_val_ == static_cast<double>(rhs.value_.integer_val_);
                    case value_types::uinteger_t:
                        return value_.double_val_ == static_cast<double>(rhs.value_.uinteger_val_);
                    case value_types::double_t:
                        return value_.double_val_ == rhs.value_.double_val_;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
            }

            switch (type_)
            {
            case value_types::bool_t:
                return type_ == rhs.type_ && value_.bool_val_ == rhs.value_.bool_val_;
            case value_types::null_t:
                return type_ == rhs.type_;
            case value_types::empty_object_t:
                return type_ == rhs.type_ || (rhs.type_ == value_types::object_t && rhs.empty());
            case value_types::small_string_t:
                return type_ == rhs.type_ && length_or_precision_ == rhs.length_or_precision_ ? std::char_traits<char_type>::compare(value_.small_string_val_,rhs.value_.small_string_val_,length_or_precision_) == 0 : false;
            case value_types::string_t:
                return type_ == rhs.type_ &&  *(value_.string_val_) == *(rhs.value_.string_val_);
            case value_types::array_t:
                return type_ == rhs.type_ && *(value_.array_val_) == *(rhs.value_.array_val_);
                break;
            case value_types::object_t:
                return (type_ == rhs.type_ && *(value_.object_val_) == *(rhs.value_.object_val_)) || (rhs.type_ == value_types::empty_object_t && empty());
                break;

#if !defined(JSONCONS_NO_DEPRECATED)
            case value_types::any_t:
                return type_ == rhs.type_;
#endif
            default:
                // throw
                break;
            }
            return false;
        }

        bool is_null() const JSONCONS_NOEXCEPT
        {
            return type_ == value_types::null_t;
        }

        bool is_bool() const JSONCONS_NOEXCEPT
        {
            return type_ == value_types::bool_t;
        }

        bool empty() const JSONCONS_NOEXCEPT
        {
            switch (type_)
            {
            case value_types::small_string_t:
                return length_or_precision_ == 0;
            case value_types::string_t:
                return value_.string_val_->length() == 0;
            case value_types::array_t:
                return value_.array_val_->size() == 0;
            case value_types::empty_object_t:
                return true;
            case value_types::object_t:
                return value_.object_val_->size() == 0;
            default:
                return false;
            }
        }

        bool is_string() const JSONCONS_NOEXCEPT
        {
            return (type_ == value_types::string_t) | (type_ == value_types::small_string_t);
        }

        bool is_number() const JSONCONS_NOEXCEPT
        {
            return type_ == value_types::double_t || type_ == value_types::integer_t || type_ == value_types::uinteger_t;
        }

        void swap(variant& rhs)
        {
            using std::swap;
            if (this == &rhs)
            {
                // same object, do nothing
            }
            else
            {
                swap(type_, rhs.type_);
                swap(length_or_precision_, rhs.length_or_precision_);
                swap(value_, rhs.value_);
            }
        }

        value_types type_;
        uint8_t length_or_precision_;
        union
        {
            double double_val_;
            int64_t integer_val_;
            uint64_t uinteger_val_;
            bool bool_val_;
            object* object_val_;
            array* array_val_;
#if !defined(JSONCONS_NO_DEPRECATED)
            any* any_val_;
#endif
            string_data* string_val_;
            char_type small_string_val_[sizeof(int64_t)/sizeof(char_type)];
        } value_;
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

        value_types type() const
        {
            return evaluate().type();
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
#if !defined(JSONCONS_NO_DEPRECATED)

        any& any_value()
        {
            return evaluate().any_value();
        }

        const any& any_value() const
        {
            return evaluate().any_value();
        }
#endif
        bool as_bool() const JSONCONS_NOEXCEPT
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
#if !defined(JSONCONS_NO_DEPRECATED)

        template <class T>
        const T& any_cast() const
        {
            return evaluate().template any_cast<T>();
        }
        // Returns a const reference to the custom data associated with name

        template <class T>
        T& any_cast() 
        {
            return evaluate().template any_cast<T>();
        }
        // Returns a reference to the custom data associated with name
#endif

        template <class T>
        json_proxy& operator=(const T& val)
        {
            parent_.evaluate_with_default().set(name_, json_type(val));
            return *this;
        }

        json_proxy& operator=(const char_type* s)
        {
            parent_.evaluate_with_default().set(name_, json_type(s));
            return *this;
        }

        json_proxy& operator=(const basic_json& val)
        {
            parent_.evaluate_with_default().set(name_, val);
            return *this;
        }

        json_proxy& operator=(basic_json&& val)
        {
            parent_.evaluate_with_default().set(name_, std::move(val));
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

        void set(const string_type& name, const json_type& value)
        {
            evaluate().set(name,value);
        }

        void set(string_type&& name, const json_type& value)

        {
            evaluate().set(std::move(name),value);
        }

        void set(const string_type& name, json_type&& value)

        {
            evaluate().set(name,std::move(value));
        }

        void set(string_type&& name, json_type&& value)

        {
            evaluate().set(std::move(name),std::move(value));
        }

        object_iterator set(object_iterator hint, const string_type& name, const json_type& value)
        {
            return evaluate().set(hint, name,value);
        }

        object_iterator set(object_iterator hint, string_type&& name, const json_type& value)

        {
            return evaluate().set(hint, std::move(name),value);
        }

        object_iterator set(object_iterator hint, const string_type& name, json_type&& value)

        {
            return evaluate().set(hint, name,std::move(value));
        }

        object_iterator set(object_iterator hint, string_type&& name, json_type&& value)

        {
            return evaluate().set(hint, std::move(name),std::move(value));
        }

        template <class T>
        void add(const T& value)
        {
            evaluate_with_default().add(json_type(value));
        }

        void add(json_type&& value)
        {
            evaluate_with_default().add(std::move(value));
        }

        void add(const json_type& value)
        {
            evaluate_with_default().add(value);
        }

        template <class T>
        array_iterator add(const_array_iterator pos, const T& value)
        {
            return evaluate_with_default().add(pos, json_type(value));
        }

        array_iterator add(const_array_iterator pos, const json_type& value)
        {
            return evaluate_with_default().add(pos, value);
        }

        array_iterator add(const_array_iterator pos, json_type&& value)
        {
            return evaluate_with_default().add(pos, std::move(value));
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
            evaluate_with_default().add(index, std::move(value));
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
        return basic_json::array();
    }

    static basic_json make_array(std::initializer_list<json_type> init, const Allocator& allocator = Allocator())
    {
        return basic_json::array(std::move(init),allocator);
    }

    static basic_json make_array(size_t n, const array_allocator& allocator = array_allocator())
    {
        return basic_json::array(n,allocator);
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
        static json_type a_null = json_type(null_type());
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

    basic_json(json_type&& other)
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

    basic_json(array&& other)
        : var_(std::move(other))
    {
    }

    basic_json(const object& other)
        : var_(other)
    {
    }

    basic_json(object&& other)
        : var_(std::move(other))
    {
    }

    template <class ParentT>
    basic_json(const json_proxy<ParentT>& proxy, const Allocator& allocator = Allocator())
        : var_(proxy.evaluate().var_,allocator)
    {
    }

    template <class T>
    basic_json(const T& val)
        : var_(null_type())
    {
        json_type_traits<json_type,T>::assign(*this,val);
    }

    basic_json(const char_type* s, const Allocator& allocator = Allocator())
        : var_(s,allocator)
    {
    }

    basic_json(double val, uint8_t precision)
        : var_(val,precision)
    {
    }

    template <class T>
    basic_json(T val, const Allocator& allocator)
        : var_(allocator)
    {
        json_type_traits<json_type,T>::assign(*this,val);
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

    basic_json& operator=(json_type&& rhs)
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
        json_type_traits<json_type,T>::assign(*this,val);
        return *this;
    }

    json_type& operator=(const char_type* s)
    {
        size_t length = std::char_traits<char_type>::length(s);
        assign_string(s,length);
        return *this;
    }

    bool operator!=(const json_type& rhs) const;

    bool operator==(const json_type& rhs) const;

    size_t size() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return 0;
        case value_types::object_t:
            return var_.value_.object_val_->size();
        case value_types::array_t:
            return var_.value_.array_val_->size();
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
        switch (var_.type_)
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
        switch (var_.type_)
        {
        case value_types::small_string_t:
            handler.value(var_.value_.small_string_val_,var_.length_or_precision_);
            break;
        case value_types::string_t:
            handler.value(var_.value_.string_val_->data(),var_.value_.string_val_->length());
            break;
        case value_types::double_t:
            handler.value(var_.value_.double_val_, var_.length_or_precision_);
            break;
        case value_types::integer_t:
            handler.value(var_.value_.integer_val_);
            break;
        case value_types::uinteger_t:
            handler.value(var_.value_.uinteger_val_);
            break;
        case value_types::bool_t:
            handler.value(var_.value_.bool_val_);
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
                object* o = var_.value_.object_val_;
                for (const_object_iterator it = o->begin(); it != o->end(); ++it)
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
                array *o = var_.value_.array_val_;
                for (const_array_iterator it = o->begin(); it != o->end(); ++it)
                {
                    it->write_body(handler);
                }
                handler.end_array();
            }
            break;
#if !defined(JSONCONS_NO_DEPRECATED)
        case value_types::any_t:
            var_.value_.any_val_->to_stream(handler);
            break;
#endif
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
        return var_.is_null();
    }

    size_t count(const string_type& name) const
    {
        switch (var_.type_)
        {
        case value_types::object_t:
            {
                auto it = var_.value_.object_val_->find(name.data(),name.length());
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
        return var_.is_string();
    }


    bool is_bool() const JSONCONS_NOEXCEPT
    {
        return var_.is_bool();
    }

    bool is_object() const JSONCONS_NOEXCEPT
    {
        return var_.type_ == value_types::object_t || var_.type_ == value_types::empty_object_t;
    }

    bool is_array() const JSONCONS_NOEXCEPT
    {
        return var_.type_ == value_types::array_t;
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    bool is_any() const JSONCONS_NOEXCEPT
    {
        return var_.type_ == value_types::any_t;
    }
#endif 
    bool is_integer() const JSONCONS_NOEXCEPT
    {
        return var_.type_ == value_types::integer_t || (var_.type_ == value_types::uinteger_t && (as_uinteger() <= static_cast<unsigned long long>(std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP())));
    }

    bool is_uinteger() const JSONCONS_NOEXCEPT
    {
        return var_.type_ == value_types::uinteger_t || (var_.type_ == value_types::integer_t && as_integer() >= 0);
    }

    bool is_double() const JSONCONS_NOEXCEPT
    {
        return var_.type_ == value_types::double_t;
    }

    bool is_number() const JSONCONS_NOEXCEPT
    {
        return var_.is_number();
    }

    bool empty() const JSONCONS_NOEXCEPT
    {
        return var_.empty();
    }

    size_t capacity() const
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            return var_.value_.array_val_->capacity();
        case value_types::object_t:
            return var_.value_.object_val_->capacity();
        default:
            return 0;
        }
    }

    template<class U=Allocator,
         typename std::enable_if<std::is_default_constructible<U>::value
            >::type* = nullptr>
    void create_object_implicitly()
    {
        var_.type_ = value_types::object_t;
        var_.value_.object_val_ = create_impl<object>(Allocator(),object_allocator(Allocator()));
    }

    template<class U=Allocator,
         typename std::enable_if<!std::is_default_constructible<U>::value
            >::type* = nullptr>
    void create_object_implicitly() const
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Cannot create_impl object implicitly - allocator is not default constructible.");
    }

    void reserve(size_t n)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_val_->reserve(n);
            break;
        case value_types::empty_object_t:
        {
            create_object_implicitly();
            var_.value_.object_val_->reserve(n);
        }
        break;
        case value_types::object_t:
        {
            var_.value_.object_val_->reserve(n);
        }
            break;
        default:
            break;
        }
    }

    void resize(size_t n)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_val_->resize(n);
            break;
        default:
            break;
        }
    }

    template <class T>
    void resize(size_t n, T val)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_val_->resize(n, val);
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

    bool as_bool() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_)
        {
        case value_types::null_t:
        case value_types::empty_object_t:
            return false;
        case value_types::bool_t:
            return var_.value_.bool_val_;
        case value_types::double_t:
            return var_.value_.double_val_ != 0.0;
        case value_types::integer_t:
            return var_.value_.integer_val_ != 0;
        case value_types::uinteger_t:
            return var_.value_.uinteger_val_ != 0;
        case value_types::small_string_t:
            return var_.length_or_precision_ != 0;
        case value_types::string_t:
            return var_.value_.string_val_->length() != 0;
        case value_types::array_t:
            return var_.value_.array_val_->size() != 0;
        case value_types::object_t:
            return var_.value_.object_val_->size() != 0;
#if !defined(JSONCONS_NO_DEPRECATED)
        case value_types::any_t:
            return true;
#endif
        default:
            return false;
        }
    }

    int64_t as_integer() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<int64_t>(var_.value_.double_val_);
        case value_types::integer_t:
            return static_cast<int64_t>(var_.value_.integer_val_);
        case value_types::uinteger_t:
            return static_cast<int64_t>(var_.value_.uinteger_val_);
        case value_types::bool_t:
            return var_.value_.bool_val_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
        }
    }

    uint64_t as_uinteger() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<uint64_t>(var_.value_.double_val_);
        case value_types::integer_t:
            return static_cast<uint64_t>(var_.value_.integer_val_);
        case value_types::uinteger_t:
            return static_cast<uint64_t>(var_.value_.uinteger_val_);
        case value_types::bool_t:
            return var_.value_.bool_val_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
        }
    }

    double as_double() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return var_.value_.double_val_;
        case value_types::integer_t:
            return static_cast<double>(var_.value_.integer_val_);
        case value_types::uinteger_t:
            return static_cast<double>(var_.value_.uinteger_val_);
        case value_types::null_t:
            return std::numeric_limits<double>::quiet_NaN();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a double");
        }
    }

    string_type as_string() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_)
        {
        case value_types::small_string_t:
            return string_type(var_.value_.small_string_val_,var_.length_or_precision_);
        case value_types::string_t:
            return string_type(var_.value_.string_val_->data(),var_.value_.string_val_->length(),var_.value_.string_val_->get_allocator());
        default:
            return to_string();
        }
    }

    string_type as_string(const string_allocator& allocator) const JSONCONS_NOEXCEPT
    {
        switch (var_.type_)
        {
        case value_types::small_string_t:
            return string_type(var_.value_.small_string_val_,var_.length_or_precision_,allocator);
        case value_types::string_t:
            return string_type(var_.value_.string_val_->data(),var_.value_.string_val_->length(),allocator);
        default:
            return to_string(allocator);
        }
    }

    string_type as_string(const basic_output_format<char_type>& format) const 
    {
        switch (var_.type_)
        {
        case value_types::small_string_t:
            return string_type(var_.value_.small_string_val_,var_.length_or_precision_);
        case value_types::string_t:
            return string_type(var_.value_.string_val_->data(),var_.value_.string_val_->length(),var_.value_.string_val_->get_allocator());
        default:
            return to_string(format);
        }
    }

    string_type as_string(const basic_output_format<char_type>& format,
                          const string_allocator& allocator) const 
    {
        switch (var_.type_)
        {
        case value_types::small_string_t:
            return string_type(var_.value_.small_string_val_,var_.length_or_precision_,allocator);
        case value_types::string_t:
            return string_type(var_.value_.string_val_->data(),var_.value_.string_val_->length(),allocator);
        default:
            return to_string(format,allocator);
        }
    }

    const char_type* as_cstring() const
    {
        switch (var_.type_)
        {
        case value_types::small_string_t:
            return var_.value_.small_string_val_;
        case value_types::string_t:
            return var_.value_.string_val_->c_str();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a cstring");
        }
    }
#if !defined(JSONCONS_NO_DEPRECATED)

    any& any_value();

    const any& any_value() const;
#endif
    json_type& at(const string_type& name)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case value_types::object_t:
            {
                auto it = var_.value_.object_val_->find(name.data(),name.length());
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
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case value_types::object_t:
            {
                auto it = var_.value_.object_val_->find(name.data(),name.length());
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
        switch (var_.type_)
        {
        case value_types::array_t:
            if (i >= var_.value_.array_val_->size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return var_.value_.array_val_->operator[](i);
        case value_types::object_t:
            return var_.value_.object_val_->at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    const json_type& at(size_t i) const
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            if (i >= var_.value_.array_val_->size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return var_.value_.array_val_->operator[](i);
        case value_types::object_t:
            return var_.value_.object_val_->at(i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    object_iterator find(const string_type& name)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.value_.object_val_->find(name.data(),name.length());
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const_object_iterator find(const string_type& name) const
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.value_.object_val_->find(name.data(),name.length());
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    object_iterator find(const char_type* name)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.value_.object_val_->find(name, std::char_traits<char_type>::length(name));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const_object_iterator find(const char_type* name) const
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.value_.object_val_->find(name, std::char_traits<char_type>::length(name));
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    object_iterator find(const char_type* name, size_t length)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.value_.object_val_->find(name, length);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    const_object_iterator find(const char_type* name, size_t length) const
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return members().end();
        case value_types::object_t:
            return var_.value_.object_val_->find(name, length);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    template<class T>
    json_type get(const string_type& name, T&& default_val) const
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            {
                return json_type(std::forward<T>(default_val));
            }
        case value_types::object_t:
            {
                const_object_iterator it = var_.value_.object_val_->find(name.data(),name.length());
                if (it != members().end())
                {
                    return it->value();
                }
                else
                {
                    return json_type(std::forward<T>(default_val));
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
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            {
                return default_val;
            }
        case value_types::object_t:
            {
                const_object_iterator it = var_.value_.object_val_->find(name.data(),name.length());
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
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            {
                return default_val;
            }
        case value_types::object_t:
            {
                const_object_iterator it = var_.value_.object_val_->find(name.data(),name.length());
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
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_val_->shrink_to_fit();
            break;
        case value_types::object_t:
            var_.value_.object_val_->shrink_to_fit();
            break;
        default:
            break;
        }
    }

    void clear()
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_val_->clear();
            break;
        case value_types::object_t:
            var_.value_.object_val_->clear();
            break;
        default:
            break;
        }
    }

    void erase(object_iterator first, object_iterator last)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            break;
        case value_types::object_t:
            var_.value_.object_val_->erase(first, last);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    void erase(array_iterator first, array_iterator last)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_val_->erase(first, last);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
            break;
        }
    }

    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void erase(const string_type& name)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            break;
        case value_types::object_t:
            var_.value_.object_val_->erase(name.data(),name.length());
            break;
        default:
            JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            break;
        }
    }

    template <class T>
    void set(const string_type& name, const T& value)
    {
        set(name,json_type(value));
    }

    template <class T>
    void set(string_type&& name, const T& value)
    {
        set(name,json_type(value));
    }

    void set(const string_type& name, const json_type& value)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            var_.value_.object_val_->set(name, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    void set(string_type&& name, const json_type& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            var_.value_.object_val_->set(std::move(name),value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    void set(const string_type& name, json_type&& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            var_.value_.object_val_->set(name,std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    void set(string_type&& name, json_type&& value)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            var_.value_.object_val_->set(std::move(name),std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    template <class T>
    object_iterator set(object_iterator hint, const string_type& name, const T& value)
    {
        return set(hint,name,json_type(value));
    }

    template <class T>
    object_iterator set(object_iterator hint, string_type&& name, const T& value)
    {
        return set(hint,name,json_type(value));
    }

    object_iterator set(object_iterator hint, const string_type& name, const json_type& value)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            return var_.value_.object_val_->set(hint, name, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    object_iterator set(object_iterator hint, string_type&& name, const json_type& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            return var_.value_.object_val_->set(hint, std::move(name),value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    object_iterator set(object_iterator hint, const string_type& name, json_type&& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            return var_.value_.object_val_->set(hint, name,std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    } 

    object_iterator set(object_iterator hint, string_type&& name, json_type&& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            return var_.value_.object_val_->set(hint, std::move(name),std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    template <class T>
    void add(const T& value)
    {
        add(json_type(value));
    }

    void add(const json_type& value)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_val_->push_back(value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    void add(json_type&& value){
        switch (var_.type_){
        case value_types::array_t:
            var_.value_.array_val_->push_back(std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    template <class T>
    array_iterator add(const_array_iterator pos, const T& value)
    {
        return add(pos,json_type(value));
    }

    array_iterator add(const_array_iterator pos, const json_type& value)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            return var_.value_.array_val_->add(pos, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    array_iterator add(const_array_iterator pos, json_type&& value){
        switch (var_.type_){
        case value_types::array_t:
            return var_.value_.array_val_->add(pos, std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    value_types type() const
    {
        return var_.type_;
    }

    uint8_t length_or_precision() const
    {
        return var_.length_or_precision_;
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
#if !defined(JSONCONS_NO_DEPRECATED)

    void assign_any(const typename json_type::any& rhs)
    {
        var_.assign(rhs);
    }
#endif
    void assign_string(const string_type& rhs)
    {
        var_.assign(rhs);
    }

    void assign_string(const char_type* rhs, size_t length)
    {
        var_.assign_string(rhs,length);
    }

    void assign_bool(bool rhs)
    {
        var_.assign(rhs);
    }

    void assign_object(const object & rhs)
    {
        var_.assign(rhs);
    }

    void assign_array(const array& rhs)
    {
        var_.assign(rhs);
    }

    void assign_null()
    {
        var_.assign(null_type());
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    template <class T>
    const T& any_cast() const
    {
        if (var_.type_ != value_types::any_t)
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad any cast");
        }
        return var_.value_.any_val_->template cast<T>();
    }
    template <class T>
    T& any_cast() 
    {
        if (var_.type_ != value_types::any_t)
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad any cast");
        }
        return var_.value_.any_val_->template cast<T>();
    }
#endif 
    void assign_integer(int64_t rhs)
    {
        var_.assign(rhs);
    }

    void assign_uinteger(uint64_t rhs)
    {
        var_.assign(rhs);
    }

    void assign_double(double rhs, uint8_t precision = 0)
    {
        var_.assign(rhs,precision);
    }

    static basic_json make_2d_array(size_t m, size_t n);

    template <class T>
    static basic_json make_2d_array(size_t m, size_t n, T val);

    static basic_json make_3d_array(size_t m, size_t n, size_t k);

    template <class T>
    static basic_json make_3d_array(size_t m, size_t n, size_t k, T val);

#if !defined(JSONCONS_NO_DEPRECATED)
    typedef any json_any_type;

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

        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return a_null;
        case value_types::object_t:
            {
                const_object_iterator it = var_.value_.object_val_->find(name.data(),name.length());
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
        return var_.type_ == value_types::integer_t;
    }

    bool is_ulonglong() const JSONCONS_NOEXCEPT
    {
        return var_.type_ == value_types::uinteger_t;
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
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<int>(var_.value_.double_val_);
        case value_types::integer_t:
            return static_cast<int>(var_.value_.integer_val_);
        case value_types::uinteger_t:
            return static_cast<int>(var_.value_.uinteger_val_);
        case value_types::bool_t:
            return var_.value_.bool_val_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an int");
        }
    }

    unsigned int as_uint() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<unsigned int>(var_.value_.double_val_);
        case value_types::integer_t:
            return static_cast<unsigned int>(var_.value_.integer_val_);
        case value_types::uinteger_t:
            return static_cast<unsigned int>(var_.value_.uinteger_val_);
        case value_types::bool_t:
            return var_.value_.bool_val_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned int");
        }
    }

    long as_long() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<long>(var_.value_.double_val_);
        case value_types::integer_t:
            return static_cast<long>(var_.value_.integer_val_);
        case value_types::uinteger_t:
            return static_cast<long>(var_.value_.uinteger_val_);
        case value_types::bool_t:
            return var_.value_.bool_val_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a long");
        }
    }

    unsigned long as_ulong() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<unsigned long>(var_.value_.double_val_);
        case value_types::integer_t:
            return static_cast<unsigned long>(var_.value_.integer_val_);
        case value_types::uinteger_t:
            return static_cast<unsigned long>(var_.value_.uinteger_val_);
        case value_types::bool_t:
            return var_.value_.bool_val_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned long");
        }
    }

    void add(size_t index, const json_type& value)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_val_->add(index, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    void add(size_t index, json_type&& value){
        switch (var_.type_){
        case value_types::array_t:
            var_.value_.array_val_->add(index, std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    bool has_member(const string_type& name) const
    {
        switch (var_.type_)
        {
        case value_types::object_t:
            {
                const_object_iterator it = var_.value_.object_val_->find(name.data(),name.length());
                return it != members().end();
            }
            break;
        default:
            return false;
        }
    }

    void remove_range(size_t from_index, size_t to_index)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_val_->remove_range(from_index, to_index);
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

    void assign_longlong(long long rhs)
    {
        var_.assign(rhs);
    }
    void assign_ulonglong(unsigned long long rhs)
    {
        var_.assign(rhs);
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
        switch (var_.type_)
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
        switch (var_.type_)
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
        switch (var_.type_)
        {
        case value_types::array_t:
            return array_range(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    const_array_range elements() const
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            return const_array_range(array_value().begin(),array_value().end());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    array& array_value() 
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            return *(var_.value_.array_val_);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad array cast");
            break;
        }
    }

    const array& array_value() const
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            return *(var_.value_.array_val_);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad array cast");
            break;
        }
    }

    object& object_value()
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            create_object_implicitly();
        case value_types::object_t:
            return *(var_.value_.object_val_);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad object cast");
            break;
        }
    }

    const object& object_value() const
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            const_cast<json_type*>(this)->create_object_implicitly(); // HERE
        case value_types::object_t:
            return *(var_.value_.object_val_);
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
#if !defined(JSONCONS_NO_DEPRECATED)

template<class CharT,class JsonTraits,class Allocator>
typename basic_json<CharT,JsonTraits,Allocator>::any& basic_json<CharT,JsonTraits,Allocator>::any_value()
{
    switch (var_.type_)
    {
    case value_types::any_t:
        {
            return *var_.value_.any_val_;
        }
    default:
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an any value");
    }
}

template<class CharT,class JsonTraits,class Allocator>
const typename basic_json<CharT,JsonTraits,Allocator>::any& basic_json<CharT,JsonTraits,Allocator>::any_value() const
{
    switch (var_.type_)
    {
    case value_types::any_t:
        {
            return *var_.value_.any_val_;
        }
    default:
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an any value");
    }
}
#endif
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
