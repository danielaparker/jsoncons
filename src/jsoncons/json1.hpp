// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON1_HPP
#define JSONCONS_JSON1_HPP

#include <limits>
#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <memory>
#include <array>
#include <typeinfo>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_output_handler.hpp"
#include "jsoncons/output_format.hpp"

namespace jsoncons {

template <typename Char,class T> inline
void serialize(basic_json_output_handler<Char>& os, const T&)
{
    os.value(null_type());
}

template <typename Char, class Alloc>
class basic_json;

template <typename Char,class Alloc>
class json_object;

template <typename Char,class Alloc>
class json_array;

template <typename Char>
class basic_output_format;

template <typename Char>
std::basic_string<Char> escape_string(const std::basic_string<Char>& s, const basic_output_format<Char>& format);

template <typename Char>
class basic_parse_error_handler;

template <typename Char, typename Alloc, typename T>
class json_type_traits
{
public:
    static bool is(const basic_json<Char,Alloc>&)
    {
        return false;
    }
};

namespace value_types
{
    enum value_types_t 
    {
        empty_object_t,
        object_t,
        array_t,
        small_string_t,
        string_t,
        double_t,
        longlong_t,
        ulonglong_t,
        bool_t,
        null_t,
        any_t
    };
}

template <typename Char, typename Alloc = std::allocator<void>>
class basic_json
{
public:

    typedef Alloc allocator_type;

    typedef json_array<Char,Alloc> array;
    typedef json_object<Char,Alloc> object;

    typedef Char char_type;

    typedef jsoncons::null_type null_type;

    // Allocation
    static void* operator new(std::size_t) { return typename Alloc::template rebind<basic_json>::other().allocate(1); }
    static void operator delete(void* ptr) { return typename Alloc::template rebind<basic_json>::other().deallocate(static_cast<basic_json*>(ptr), 1); }
    static void* operator new( std::size_t s, void* p ) throw()
    {
        return ::operator new( s, p );
    }

    class any
    {
    public:
        any()
            : impl_(nullptr)
        {
        }
        any(const any& val)
        {
			impl_ = val.impl_ != nullptr ? val.impl_->clone() : nullptr;
        }
        any(any&& val)
            : impl_(val.impl_)
        {
            val.impl_ = nullptr;
        }
        ~any()
        {
            delete impl_;
        }

        template<typename T>
        explicit any(T val)
        {
            impl_ = new any_handle_impl<typename type_wrapper<T>::value_type>(val);
        }

        template <typename T>
        typename type_wrapper<T>::reference cast() 
        {
            if (typeid(*impl_) != typeid(any_handle_impl<typename type_wrapper<T>::value_type>))
            {
                JSONCONS_THROW_EXCEPTION("Bad any cast");
            }
            return static_cast<any_handle_impl<typename type_wrapper<T>::value_type>&>(*impl_).value_;
        }

        template <typename T>
        typename type_wrapper<T>::const_reference cast() const
        {
            if (typeid(*impl_) != typeid(any_handle_impl<typename type_wrapper<T>::value_type>))
            {
                JSONCONS_THROW_EXCEPTION("Bad any cast");
            }
            return static_cast<any_handle_impl<typename type_wrapper<T>::value_type>&>(*impl_).value_;
        }

        any& operator=(any rhs)
        {
            std::swap(impl_,rhs.impl_);
            return *this;
        }

        void to_stream(basic_json_output_handler<Char>& os) const 
        {
            impl_->to_stream(os);
        }

        class any_handle
        {
        public:
            virtual ~any_handle()
            {
            }

            virtual any_handle* clone() const = 0;

            virtual void to_stream(basic_json_output_handler<Char>& os) const = 0;
        };

        template <class T>
        class any_handle_impl : public any_handle
        {
        public:
            any_handle_impl(T value)
                : value_(value)
            {
            }

            virtual any_handle* clone() const
            {
                return new any_handle_impl<T>(value_);
            }

            virtual void to_stream(basic_json_output_handler<Char>& os) const
            {
                serialize(os,value_);
            }

            T value_;
        };

        any_handle* impl_;
    };

    struct variant
    {
        static const size_t small_string_capacity = (sizeof(long long)/sizeof(Char)) - 1;

        variant()
            : type_(value_types::empty_object_t)
        {
        }
		
        explicit variant(variant&& rhs)
        {
            type_ = rhs.type_;
            small_string_length_ = rhs.small_string_length_;
            value_ = rhs.value_;
            rhs.type_ = value_types::null_t;
        }

        explicit variant(const variant& var)
            : type_(var.type_)
        {
            switch (var.type_)
            {
            case value_types::null_t:
            case value_types::empty_object_t:
                break;
            case value_types::double_t:
                value_.float_value_ = var.value_.float_value_;
                break;
            case value_types::longlong_t:
                value_.si_value_ = var.value_.si_value_;
                break;
            case value_types::ulonglong_t:
                value_.ui_value_ = var.value_.ui_value_;
                break;
            case value_types::bool_t:
                value_.bool_value_ = var.value_.bool_value_;
                break;
            case value_types::small_string_t:
                small_string_length_ = var.small_string_length_;
                std::memcpy(value_.small_string_value_,var.value_.small_string_value_,var.small_string_length_*sizeof(Char));
                break;
            case value_types::string_t:
                value_.string_value_ = make_string_holder(var.value_.string_value_);
                break;
            case value_types::array_t:
                value_.array_ = new json_array<Char,Alloc>(*(var.value_.array_));
                break;
            case value_types::object_t:
                value_.object_ = new json_object<Char,Alloc>(*(var.value_.object_));
                break;
            case value_types::any_t:
                value_.any_value_ = new any(*(var.value_.any_value_));
                break;
            default:
                // throw
                break;
            }
        }

        variant(const json_object<Char,Alloc>& val)
            : type_(value_types::object_t)
        {
            value_.object_ = new json_object<Char,Alloc>(val);
        }

        variant(json_object<Char,Alloc>&& val)
            : type_(value_types::object_t)
        {
            value_.object_ = new json_object<Char,Alloc>(val);
        }

        variant(const json_array<Char,Alloc>& val)
            : type_(value_types::array_t)
        {
            value_.array_ = new json_array<Char,Alloc>(val);
        }

        variant(json_array<Char,Alloc>&& val)
            : type_(value_types::array_t)
        {
            value_.array_ = new json_array<Char,Alloc>(val);
        }

        variant(value_types::value_types_t type, size_t size)
            : type_(type)
        {
            switch (type)
            {
            case value_types::null_t:
            case value_types::empty_object_t:
                break;
            case value_types::double_t:
                break;
            case value_types::longlong_t:
                break;
            case value_types::ulonglong_t:
                break;
            case value_types::bool_t:
                break;
            case value_types::small_string_t:
                small_string_length_ = 0;
                break;
            case value_types::string_t:
                value_.string_value_ = make_string_holder();
                break;
            case value_types::array_t:
                value_.array_ = new json_array<Char,Alloc>(size);
                break;
            case value_types::object_t:
                value_.object_ = new json_object<Char,Alloc>();
                break;
            case value_types::any_t:
                value_.any_value_ = new any();
                break;
            default:
                // throw
                break;
            }
        }

        explicit variant(const any& var)
            : type_(value_types::any_t)
        {
            value_.any_value_ = new any(var);
        }

        explicit variant(jsoncons::null_type)
            : type_(value_types::null_t)
        {
        }

        explicit variant(bool val)
            : type_(value_types::bool_t)
        {
            value_.bool_value_ = val;
        }

        explicit variant(double val)
            : type_(value_types::double_t)
        {
            value_.float_value_ = val;
        }

        explicit variant(long long val)
            : type_(value_types::longlong_t)
        {
            value_.si_value_ = val;
        }

        explicit variant(unsigned long long val)
            : type_(value_types::ulonglong_t)
        {
            value_.ui_value_ = val;
        }

        explicit variant(const std::basic_string<Char>& s)
        {
            if (s.length() > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                value_.string_value_ = make_string_holder(s);
            }
            else
            {
                type_ = value_types::small_string_t;
                small_string_length_ = (unsigned char)s.length();
                std::memcpy(value_.small_string_value_,s.c_str(),s.length()*sizeof(Char));
            }
        }

        explicit variant(const Char* s)
        {
            size_t length = std::char_traits<Char>::length(s);
            if (length > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                value_.string_value_ = make_string_holder(s);
            }
            else
            {
                type_ = value_types::small_string_t;
                small_string_length_ = (unsigned char)length;
                std::memcpy(value_.small_string_value_,s,length*sizeof(Char));
            }
        }

        explicit variant(const Char* s, size_t length)
        {
            if (length > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                value_.string_value_ = make_string_holder(s,length);
            }
            else
            {
                type_ = value_types::small_string_t;
                small_string_length_ = (unsigned char)length;
                std::memcpy(value_.small_string_value_,s,length*sizeof(Char));
            }
        }

        template<class InputIterator>
        variant(InputIterator first, InputIterator last)
            : type_(value_types::array_t)
        {
            value_.array_ = new json_array<Char, Alloc>(first, last);
        }

        ~variant()
        {
            switch (type_)
            {
            case value_types::string_t:
                delete_string_holder(value_.string_value_);
                break;
            case value_types::array_t:
                delete value_.array_;
                break;
            case value_types::object_t:
                delete value_.object_;
                break;
            case value_types::any_t:
                delete value_.any_value_;
                break;
            default:
                break; 
            }
        }

        void destroy()
        {
            switch (type_)
            {
            case value_types::string_t:
                delete_string_holder(value_.string_value_);
                break;
            case value_types::array_t:
                delete value_.array_;
                break;
            case value_types::object_t:
                delete value_.object_;
                break;
            case value_types::any_t:
                delete value_.any_value_;
                break;
            default:
                break; 
            }
        }

        variant& operator=(const variant& val)
        {
            if (this != &val)
            {
                switch (type_)
                {
                case value_types::null_t:
                case value_types::bool_t:
                case value_types::empty_object_t:
                case value_types::small_string_t:
                case value_types::longlong_t:
                case value_types::ulonglong_t:
                case value_types::double_t:
                    switch (val.type_)
                    {
                    case value_types::null_t:
                    case value_types::bool_t:
                    case value_types::empty_object_t:
                    case value_types::small_string_t:
                    case value_types::longlong_t:
                    case value_types::ulonglong_t:
                    case value_types::double_t:
                        type_ = val.type_;
                        small_string_length_ = val.small_string_length_;
                        value_ = val.value_;
                        break;
                    default:
                        variant temp(val);
                        temp.swap(*this);
                        break;
                    }
                    break;
                default:
                    {
                        variant temp(val);
                        temp.swap(*this);
                    }
                    break;
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

        void assign(const json_object<Char,Alloc>& val)
        {
			destroy();
			type_ = value_types::object_t;
			value_.object_ = new json_object<Char, Alloc>(val);
		}

        void assign(json_object<Char,Alloc>&& val)
        {
			switch (type_)
			{
			case value_types::object_t:
				value_.object_->swap(val);
				break;
			default:
				destroy();
				type_ = value_types::object_t;
				value_.object_ = new json_object<Char, Alloc>(val);
				break;
			}
		}

        void assign(const json_array<Char,Alloc>& val)
        {
            destroy();
            type_ = value_types::array_t;
            value_.array_ = new json_array<Char,Alloc>(val);
        }

        void assign(json_array<Char,Alloc>&& val)
        {
			switch (type_)
			{
			case value_types::array_t:
				value_.array_->swap(val);
				break;
			default:
				destroy();
				type_ = value_types::array_t;
				value_.array_ = new json_array<Char, Alloc>(val);
				break;
			}
		}

        void assign(const std::basic_string<Char>& s)
        {
            switch (type_)
            {
            case value_types::null_t:
            case value_types::bool_t:
            case value_types::empty_object_t:
            case value_types::small_string_t:
            case value_types::longlong_t:
            case value_types::ulonglong_t:
            case value_types::double_t:
                if (s.length() > variant::small_string_capacity)
                {
                    type_ = value_types::string_t;
                    value_.string_value_ = make_string_holder(s);
                }
                else
                {
                    type_ = value_types::small_string_t;
                    small_string_length_ = (unsigned char)s.length();
                    std::memcpy(value_.small_string_value_,s.c_str(),s.length()*sizeof(Char));
                }
                break;
            default:
                variant(s).swap(*this);
                break;
            }
        }

        void assign_string(const Char* s, size_t length)
        {
            switch (type_)
            {
            case value_types::null_t:
            case value_types::bool_t:
            case value_types::empty_object_t:
            case value_types::small_string_t:
            case value_types::longlong_t:
            case value_types::ulonglong_t:
            case value_types::double_t:
				{
					if (length > variant::small_string_capacity)
					{
						type_ = value_types::string_t;
						value_.string_value_ = make_string_holder(s,length);
					}
					else
					{
						type_ = value_types::small_string_t;
						small_string_length_ = (unsigned char)length;
						std::memcpy(value_.small_string_value_,s,length*sizeof(Char));
					}
				}
                break;
            default:
                variant(s,length).swap(*this);
                break;
            }
        }

        void assign(long long val)
        {
            switch (type_)
            {
            case value_types::null_t:
            case value_types::bool_t:
            case value_types::empty_object_t:
            case value_types::small_string_t:
            case value_types::longlong_t:
            case value_types::ulonglong_t:
            case value_types::double_t:
                type_ = value_types::longlong_t;
                value_.si_value_ = val;
                break;
            default:
                variant(val).swap(*this);
                break;
            }
        }

        void assign(unsigned long long val)
        {
            switch (type_)
            {
            case value_types::null_t:
            case value_types::bool_t:
            case value_types::empty_object_t:
            case value_types::small_string_t:
            case value_types::longlong_t:
            case value_types::ulonglong_t:
            case value_types::double_t:
                type_ = value_types::ulonglong_t;
                value_.ui_value_ = val;
                break;
            default:
                variant(val).swap(*this);
                break;
            }
        }

        void assign(double val)
        {
            switch (type_)
            {
            case value_types::null_t:
            case value_types::bool_t:
            case value_types::empty_object_t:
            case value_types::small_string_t:
            case value_types::longlong_t:
            case value_types::ulonglong_t:
            case value_types::double_t:
                type_ = value_types::double_t;
                value_.float_value_ = val;
                break;
            default:
                variant(val).swap(*this);
                break;
            }
        }

        void assign(bool val)
        {
            switch (type_)
            {
            case value_types::null_t:
            case value_types::bool_t:
            case value_types::empty_object_t:
            case value_types::small_string_t:
            case value_types::longlong_t:
            case value_types::ulonglong_t:
            case value_types::double_t:
                type_ = value_types::bool_t;
                value_.bool_value_ = val;
                break;
            default:
                variant(val).swap(*this);
                break;
            }
        }

        void assign(null_type)
        {
            switch (type_)
            {
            case value_types::null_t:
            case value_types::bool_t:
            case value_types::empty_object_t:
            case value_types::small_string_t:
            case value_types::longlong_t:
            case value_types::ulonglong_t:
            case value_types::double_t:
                type_ = value_types::null_t;
                break;
            default:
                variant(null_type()).swap(*this);
                break;
            }
        }

        void assign(const any& rhs)
        {
            switch (type_)
            {
            case value_types::null_t:
            case value_types::bool_t:
            case value_types::empty_object_t:
            case value_types::small_string_t:
            case value_types::longlong_t:
            case value_types::ulonglong_t:
            case value_types::double_t:
                type_ = value_types::any_t;
                value_.any_value_ = new any(rhs);
                break;
            default:
                variant(rhs).swap(*this);
                break;
            }
        }

        bool operator!=(const variant& rhs) const
        {
            return !(*this == rhs);
        }

        bool operator==(const variant& rhs) const
        {
            if (is_numeric() & rhs.is_numeric())
            {
                switch (type_)
                {
                case value_types::longlong_t:
                    switch (rhs.type_)
                    {
                    case value_types::longlong_t:
                        return value_.si_value_ == rhs.value_.si_value_;
                    case value_types::ulonglong_t:
                        return value_.si_value_ == rhs.value_.ui_value_;
                    case value_types::double_t:
                        return value_.si_value_ == rhs.value_.float_value_;
                    }
                    break;
                case value_types::ulonglong_t:
                    switch (rhs.type_)
                    {
                    case value_types::longlong_t:
                        return value_.ui_value_ == rhs.value_.si_value_;
                    case value_types::ulonglong_t:
                        return value_.ui_value_ == rhs.value_.ui_value_;
                    case value_types::double_t:
                        return value_.ui_value_ == rhs.value_.float_value_;
                    }
                    break;
                case value_types::double_t:
                    switch (rhs.type_)
                    {
                    case value_types::longlong_t:
                        return value_.float_value_ == rhs.value_.si_value_;
                    case value_types::ulonglong_t:
                        return value_.float_value_ == rhs.value_.ui_value_;
                    case value_types::double_t:
                        return value_.float_value_ == rhs.value_.float_value_;
                    }
                    break;
                }
            }

            if (rhs.type_ != type_)
            {
                return false;
            }
            switch (type_)
            {
            case value_types::bool_t:
                return value_.bool_value_ == rhs.value_.bool_value_;
            case value_types::null_t:
            case value_types::empty_object_t:
                return true;
            case value_types::small_string_t:
                return small_string_length_ == rhs.small_string_length_ ? std::char_traits<Char>::compare(value_.small_string_value_,rhs.value_.small_string_value_,small_string_length_) == 0 : false;
            case value_types::string_t:
                return *(value_.string_value_) == *(rhs.value_.string_value_);
            case value_types::array_t:
                return *(value_.array_) == *(rhs.value_.array_);
                break;
            case value_types::object_t:
                return *(value_.object_) == *(rhs.value_.object_);
                break;
            case value_types::any_t:
                break;
            default:
                // throw
                break;
            }
            return false;
        }

        bool is_null() const
        {
            return type_ == value_types::null_t;
        }

        bool is_bool() const
        {
            return type_ == value_types::bool_t;
        }

        bool is_empty() const
        {
            switch (type_)
            {
            case value_types::small_string_t:
                return small_string_length_ == 0;
            case value_types::string_t:
                return value_.string_value_->length() == 0;
            case value_types::array_t:
                return value_.array_->size() == 0;
            case value_types::empty_object_t:
                return true;
            case value_types::object_t:
                return value_.object_->size() == 0;
            default:
                return false;
            }
        }

        bool is_string() const
        {
            return (type_ == value_types::string_t) | (type_ == value_types::small_string_t);
        }

        bool is_numeric() const
        {
            return type_ == value_types::double_t || type_ == value_types::longlong_t || type_ == value_types::ulonglong_t;
        }

        void swap(variant& var)
        {
            using std::swap;

            swap(type_,var.type_);
            swap(small_string_length_,var.small_string_length_);
            swap(value_,var.value_);
        }

        value_types::value_types_t type_;
        unsigned char small_string_length_;
        union
        {
            double float_value_;
            long long si_value_;
            unsigned long long ui_value_;
            bool bool_value_;
            json_object<Char,Alloc>* object_;
            json_array<Char,Alloc>* array_;
            any* any_value_;
            std::basic_string<Char>* string_value_;
            Char small_string_value_[sizeof(long long)/sizeof(Char)];
        } value_;

        static void delete_string_holder(std::basic_string<Char>* other)
        {
            delete other;
        }

        static std::basic_string<Char>* make_string_holder(const std::basic_string<Char>* other)
        {
            return new std::basic_string<Char>(*other);
        }

        static std::basic_string<Char>* make_string_holder(const std::basic_string<Char>& s)
        {
            return new std::basic_string<Char>(s);
        }

        static std::basic_string<Char>* make_string_holder(const Char* p)
        {
            return make_string_holder(p,std::char_traits<Char>::length(p));
        }

        static std::basic_string<Char>* make_string_holder(const Char* p, size_t length)
        {
            return new std::basic_string<Char>(p,length);
        }

        static std::basic_string<Char>* make_string_holder()
        {
          return new std::basic_string<Char>();
        }

        static std::basic_string<Char>* make_string_holder(Char c)
        {
            return make_string_holder(&c,1);
        }

    };

    // Deprecated
    typedef any json_any_type;

    class member_type
    {
    public:
        member_type()
        {
        }
        member_type(const member_type& pair)
            : name_(pair.name_), value_(pair.value_)
        {
        }
        member_type(member_type&& pair)
            //: name_(std::move(pair.name_)), value_(std::move(pair.value_))
        {
            name_.swap(pair.name_);
            value_.swap(pair.value_);
        }
        member_type(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& value)
            : name_(name), value_(value)
        {
        }
        member_type(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& value)
            : name_(name), value_(value)
        {
        }

        const std::basic_string<Char>& name() const
        {
            return name_;
        }

        basic_json<Char,Alloc>& value()
        {
            return value_;
        }

        const basic_json<Char,Alloc>& value() const
        {
            return value_;
        }

        void swap(member_type& pair)
        {
            name_.swap(pair.name_);
            value_.swap(pair.value_);
        }
    private:
        // Not to be implemented
        void operator=(member_type const &); 

        std::basic_string<Char> name_;
        basic_json<Char,Alloc> value_;
    };

    typedef member_type name_value_pair;

    // Deprecated static data members
    static const basic_json<Char,Alloc> an_object;
    static const basic_json<Char,Alloc> an_array;
    static const basic_json<Char,Alloc> null;

    typedef typename json_object<Char,Alloc>::iterator object_iterator;
    typedef typename json_object<Char,Alloc>::const_iterator const_object_iterator;

    typedef typename json_array<Char,Alloc>::iterator array_iterator;
    typedef typename json_array<Char,Alloc>::const_iterator const_array_iterator;

    class const_val_proxy 
    {
    public:
        friend class basic_json<Char,Alloc>;

        const_object_iterator begin_members() const
        {
            return val_.begin_members();
        }

        const_object_iterator end_members() const
        {
            return val_.end_members();
        }

        const_array_iterator begin_elements() const
        {
            return val_.begin_elements();
        }

        const_array_iterator end_elements() const
        {
            return val_.end_elements();
        }

        size_t size() const
        {
            return val_.size();
        }

        value_types::value_types_t type() const
        {
            return val_.type();
        }

        bool has_member(const std::basic_string<Char>& name) const
        {
            return val_.has_member(name);
        }

        template<typename T>
        bool is() const
        {
            return val_.template is<T>();
        }

        bool is_null() const
        {
            return val_.is_null();
        }

        bool is_empty() const
        {
            return val_.is_empty();
        }

        size_t capacity() const
        {
            return val_.capacity();
        }

        bool is_string() const
        {
            return val_.is_string();
        }

        bool is_number() const
        {
            return val_.is_number();
        }

        bool is_numeric() const
        {
            return val_.is_numeric();
        }

        bool is_bool() const
        {
            return val_.is_bool();
        }

        bool is_object() const
        {
            return val_.is_object();
        }

        bool is_array() const
        {
            return val_.is_array();
        }

        bool is_any() const
        {
            return val_.is_any();
        }

        bool is_longlong() const
        {
            return val_.is_longlong();
        }

        bool is_ulonglong() const
        {
            return val_.is_ulonglong();
        }

        bool is_double() const
        {
            return val_.is_double();
        }

        // Deprecated

        std::basic_string<Char> as_string() const
        {
            return val_.as_string();
        }

        std::basic_string<Char> as_string(const basic_output_format<Char>& format) const
        {
            return val_.as_string(format);
        }

        template<typename T>
        T as() const
        {
            return val_.template as<T>();
        }

        any& any_value()
        {
            return val_.any_value();
        }

        const any& any_value() const
        {
            return val_.any_value();
        }

        bool as_bool() const
        {
            return val_.as_bool();
        }

        template <class T>
        std::vector<T> as_vector() const
        {
            return val_.as_vector<T>();
        }

        // Deprecated

        double as_double() const
        {
            return val_.as_double();
        }

        int as_int() const
        {
            return val_.as_int();
        }

        unsigned int as_uint() const
        {
            return val_.as_uint();
        }

        long as_long() const
        {
            return val_.as_long();
        }

        unsigned long as_ulong() const
        {
            return val_.as_ulong();
        }

        long long as_longlong() const
        {
            return val_.as_longlong();
        }

        unsigned long long as_ulonglong() const
        {
            return val_.as_ulonglong();
        }

        template <class T>
        const T& any_cast() const
        {
            return val_.any_cast<T>();
        }

        operator basic_json() const
        {
            return val_;
        }

        bool operator==(const basic_json<Char,Alloc>& val) const
        {
            return val_ == val;
        }

        bool operator!=(const basic_json& val) const
        {
            return val_ != val;
        }

        const basic_json<Char,Alloc>& operator[](size_t i) const
        {
            return val_[i];
        }

        const basic_json<Char,Alloc>& operator[](const std::basic_string<Char>& name) const
        {
            return val_.at(name);
        }

        const basic_json<Char,Alloc>& at(const std::basic_string<Char>& name) const
        {
            return val_.at(name);
        }

        const basic_json<Char,Alloc>& get(const std::basic_string<Char>& name) const
        {
            return val_.get(name);
        }

        template <typename T>
        const_val_proxy get(const std::basic_string<Char>& name, T default_val) const
        {
            return val_.get(name,default_val);
        }

        std::basic_string<Char> to_string() const
        {
            return val_.to_string();
        }

        std::basic_string<Char> to_string(const basic_output_format<Char>& format) const
        {
            return val_.to_string(format);
        }

        void to_stream(std::basic_ostream<Char>& os) const
        {
            val_.to_stream(os);
        }

        void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format) const
        {
            val_.to_stream(os,format);
        }

        void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format, bool indenting) const
        {
            val_.to_stream(os,format,indenting);
        }

        friend std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const const_val_proxy& o)
        {
            o.to_stream(os);
            return os;
        }

    private:
        const_val_proxy(); // no op
        const_val_proxy& operator = (const const_val_proxy& other); // noop

        const_val_proxy(const basic_json<Char,Alloc>& val)
            : val_(val)
        {
        }

        template <typename T>
        const_val_proxy(T val)
            : val2_(val), val_(val2_)
        {
        }

        basic_json<Char,Alloc> val2_;
        const basic_json<Char,Alloc>& val_;
    };

    class object_key_proxy 
    {
    public:
        friend class basic_json<Char,Alloc>;

        object_iterator begin_members()
        {
            return val_.at(name_).begin_members();
        }

        const_object_iterator begin_members() const
        {
            return val_.at(name_).begin_members();
        }

        object_iterator end_members()
        {
            return val_.at(name_).end_members();
        }

        const_object_iterator end_members() const
        {
            return val_.at(name_).end_members();
        }

        array_iterator begin_elements()
        {
            return val_.at(name_).begin_elements();
        }

        const_array_iterator begin_elements() const
        {
            return val_.at(name_).begin_elements();
        }

        array_iterator end_elements()
        {
            return val_.at(name_).end_elements();
        }

        const_array_iterator end_elements() const
        {
            return val_.at(name_).end_elements();
        }

        size_t size() const
        {
            return val_.at(name_).size();
        }

        value_types::value_types_t type() const
        {
            return val_.at(name_).type();
        }

        bool has_member(const std::basic_string<Char>& name) const
        {
            return val_.at(name_).has_member(name);
        }

        bool is_null() const
        {
            return val_.at(name_).is_null();
        }

        bool is_empty() const
        {
            return val_.at(name_).is_empty();
        }

        size_t capacity() const
        {
            return val_.at(name_).capacity();
        }

        void reserve(size_t n)
        {
            val_.at(name_).reserve(n);
        }

        void resize(size_t n)
        {
            val_.at(name_).resize(n);
        }

        template <typename T>
        void resize(size_t n, T val)
        {
            val_.at(name_).resize(n,val);
        }

        void resize_array(size_t n)
        {
            val_.at(name_).resize_array(n);
        }

        template <typename T>
        void resize_array(size_t n, T val)
        {
            val_.at(name_).resize_array(n,val);
        }

        template<typename T>
        bool is() const
        {
            return val_.at(name_).template is<T>();
        }

        bool is_string() const
        {
            return val_.at(name_).is_string();
        }

        bool is_number() const
        {
            return val_.at(name_).is_number();
        }

        bool is_numeric() const
        {
            return val_.at(name_).is_numeric();
        }

        bool is_bool() const
        {
            return val_.at(name_).is_bool();
        }

        bool is_object() const
        {
            return val_.at(name_).is_object();
        }

        bool is_array() const
        {
            return val_.at(name_).is_array();
        }
 
        bool is_any() const
        {
            return val_.at(name_).is_any();
        }

        bool is_longlong() const
        {
            return val_.at(name_).is_longlong();
        }

        bool is_ulonglong() const
        {
            return val_.at(name_).is_ulonglong();
        }

        bool is_double() const
        {
            return val_.at(name_).is_double();
        }

        std::basic_string<Char> as_string() const
        {
            return val_.at(name_).as_string();
        }

        std::basic_string<Char> as_string(const basic_output_format<Char>& format) const
        {
            return val_.at(name_).as_string(format);
        }

        template<typename T>
        T as() const
        {
            return val_.at(name_).template as<T>();
        }

        any& any_value()
        {
            return val_.at(name_).any_value();
        }

        const any& any_value() const
        {
            return val_.at(name_).any_value();
        }

        bool as_bool() const
        {
            return val_.at(name_).as_bool();
        }

        template <class T>
        std::vector<T> as_vector() const
        {
            return val_.at(name_).template as_vector<T>();
        }

        double as_double() const
        {
            return val_.at(name_).as_double();
        }

        int as_int() const
        {
            return val_.at(name_).as_int();
        }

        unsigned int as_uint() const
        {
            return val_.at(name_).as_uint();
        }

        long as_long() const
        {
            return val_.at(name_).as_long();
        }

        unsigned long as_ulong() const
        {
            return val_.at(name_).as_ulong();
        }

        long long as_longlong() const
        {
            return val_.at(name_).as_longlong();
        }

        unsigned long long as_ulonglong() const
        {
            return val_.at(name_).as_ulonglong();
        }

        template <class T>
        const T& any_cast() const
        {
            return val_.at(name_).template any_cast<T>();
        }
        // Returns a const reference to the custom data associated with name

        template <class T>
        T& any_cast() 
        {
            return val_.at(name_).template any_cast<T>();
        }
        // Returns a reference to the custom data associated with name

        operator basic_json&()
        {
            return val_.at(name_);
        }

        operator const basic_json&() const
        {
            return val_.at(name_);
        }

        template <typename T>
        object_key_proxy& operator=(T val)
        {
            val_.set(name_, val);
            return *this;
        }

        object_key_proxy& operator=(const basic_json& val)
        {
            val_.set(name_, val);
            return *this;
        }

        bool operator==(const basic_json& val) const
        {
            return val_.at(name_) == val;
        }

        bool operator!=(const basic_json& val) const
        {
            return val_.at(name_) != val;
        }

        basic_json<Char,Alloc>& operator[](size_t i)
        {
            return val_.at(name_)[i];
        }

        const basic_json<Char,Alloc>& operator[](size_t i) const
        {
            return val_.at(name_)[i];
        }

        object_key_proxy operator[](const std::basic_string<Char>& name)
        {
            return object_key_proxy(val_.at(name_),name);
        }

        const basic_json<Char,Alloc>& operator[](const std::basic_string<Char>& name) const
        {
            return val_.at(name_).at(name);
        }

        basic_json<Char,Alloc>& at(const std::basic_string<Char>& name)
        {
            return val_.at(name_).at(name);
        }

        const basic_json<Char,Alloc>& at(const std::basic_string<Char>& name) const
        {
            return val_.at(name_).at(name);
        }

        const basic_json<Char,Alloc>& get(const std::basic_string<Char>& name) const
        {
            return val_.at(name_).get(name);
        }

        template <typename T>
        const_val_proxy get(const std::basic_string<Char>& name, T default_val) const
        {
            return val_.at(name_).get(name,default_val);
        }

        void clear()
        {
            val_.at(name_).clear();
        }
        // Remove all elements from an array or object

        void remove_range(size_t from_index, size_t to_index)
        {
            val_.at(name_).remove_range(from_index, to_index);
        }
        // Remove a range of elements from an array 

        void remove_member(const std::basic_string<Char>& name)
        {
            val_.at(name_).remove_member(name);
        }
        // Remove a member from an object 
/*
        template <typename T>
        void set(const std::basic_string<Char>& name, T value)
        {
            val_.at(name_).set(name,value);
        }
*/
        void set(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& value)
        {
            val_.at(name_).set(name,value);
        }

        void set(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& value)

        {
            val_.at(name_).set(name,value);
        }
/*
        template <typename T>
        void add(T value)
        {
            val_.at(name_).add(value);
        }

        template <typename T>
        void add(size_t index, T value)
        {
            val_.at(name_).add(index, value);
        }
*/
        void add(basic_json<Char,Alloc>&& value)
        {
            val_.at(name_).add(value);
        }

        void add(size_t index, basic_json<Char,Alloc>&& value)
        {
            val_.at(name_).add(index, value);
        }

        void add(const basic_json<Char,Alloc>& value)
        {
            val_.at(name_).add(value);
        }

        void add(size_t index, const basic_json<Char,Alloc>& value)
        {
            val_.at(name_).add(index, value);
        }

        std::basic_string<Char> to_string() const
        {
            return val_.at(name_).to_string();
        }

        std::basic_string<Char> to_string(const basic_output_format<Char>& format) const
        {
            return val_.at(name_).to_string(format);
        }

        void to_stream(std::basic_ostream<Char>& os) const
        {
            val_.at(name_).to_stream(os);
        }

        void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format) const
        {
            val_.at(name_).to_stream(os,format);
        }

        void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format, bool indenting) const
        {
            val_.at(name_).to_stream(os,format,indenting);
        }

        void swap(basic_json<Char,Alloc>& val)
        {
            val_.swap(val);
        }

        friend std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const object_key_proxy& o)
        {
            o.to_stream(os);
            return os;
        }

    private:
        object_key_proxy(); // nopop
        object_key_proxy& operator = (const object_key_proxy& other); // noop

        object_key_proxy(basic_json<Char,Alloc>& var, 
              const std::basic_string<Char>& name)
            : val_(var), name_(name)
        {
        }

        basic_json<Char,Alloc>& val_;

        const std::basic_string<Char>& name_;
    };

    static basic_json parse(std::basic_istream<Char>& is);

    static basic_json parse(std::basic_istream<Char>& is, basic_parse_error_handler<Char>& err_handler);

    static basic_json parse_string(const std::basic_string<Char>& s);

    static basic_json parse_string(const std::basic_string<Char>& s, basic_parse_error_handler<Char>& err_handler);

    static basic_json parse_file(const std::string& s);

    static basic_json parse_file(const std::string& s, basic_parse_error_handler<Char>& err_handler);

    static basic_json<Char,Alloc> make_array()
    {
        return typename basic_json<Char,Alloc>::array();
    }

    static basic_json<Char,Alloc> make_array(size_t n)
    {
        basic_json<Char,Alloc> val = make_array();
        val.resize(n);
        return val;
    }

    template <typename T>
    static basic_json<Char,Alloc> make_array(size_t n, T val)
    {
        basic_json<Char,Alloc> a = make_array();
        a.resize(n,val);
        return a;
    }

    template<int size>
    static typename std::enable_if<size==1,basic_json>::type make_array()
    {
        return build_array<Char,Alloc,size>()();
    }

    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_array(size_t n)
    {
        return build_array<Char,Alloc,size>()(n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==1,basic_json>::type make_array(size_t n, T val)
    {
        return build_array<Char,Alloc,size>()(n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_array(size_t m, size_t n)
    {
        return build_array<Char,Alloc,size>()(m, n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==2,basic_json>::type make_array(size_t m, size_t n, T val)
    {
        return build_array<Char,Alloc,size>()(m, n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_array(size_t m, size_t n, size_t k)
    {
        return build_array<Char,Alloc,size>()(m, n, k);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==3,basic_json>::type make_array(size_t m, size_t n, size_t k, T val)
    {
        return build_array<Char,Alloc,size>()(m, n, k, val);
    }

    basic_json()
    {
    }

    basic_json(const basic_json<Char, Alloc>& val)
        : var_(val.var_)
    {
    }

    basic_json(basic_json<Char,Alloc>&& other)
        : var_(std::move(other.var_))
    {
    }

    basic_json(const json_array<Char, Alloc>& val)
        : var_(val)
    {
    }

    basic_json(json_array<Char,Alloc>&& other)
        : var_(other)
    {
    }

    basic_json(json_object<Char,Alloc>&& other)
        : var_(other)
    {
    }

    template <typename T>
    basic_json(T val)
        : var_()
    {
        json_type_traits<Char,Alloc,T>::assign(*this,val);
    }

    basic_json(const Char *s, size_t length)
        : var_(s, length)
    {
    }

    basic_json(value_types::value_types_t type, size_t size)
        : var_(type,size)
    {
    }

    template<class InputIterator>
    basic_json(InputIterator first, InputIterator last)
        : var_(first,last)
    {
    }

    ~basic_json()
    {
    }

    object_iterator begin_members();

    const_object_iterator begin_members() const;

    object_iterator end_members();

    const_object_iterator end_members() const;

    array_iterator begin_elements();

    const_array_iterator begin_elements() const;

    array_iterator end_elements();

    const_array_iterator end_elements() const;

    basic_json& operator=(const basic_json<Char,Alloc>& rhs)
    {
        var_ = rhs.var_;
        return *this;
    }

    basic_json& operator=(basic_json<Char,Alloc>&& rhs)
    {
        var_ = std::move(rhs.var_);
        return *this;
    }

    template <class T>
    basic_json<Char, Alloc>& operator=(T val)
    {
        json_type_traits<Char,Alloc,T>::assign(*this,val);
        return *this;
    }

    bool operator!=(const basic_json<Char,Alloc>& rhs) const;

    bool operator==(const basic_json<Char,Alloc>& rhs) const;

    size_t size() const; 

    basic_json<Char,Alloc>& operator[](size_t i);

    const basic_json<Char,Alloc>& operator[](size_t i) const;

    object_key_proxy operator[](const std::basic_string<Char>& name);

    const basic_json<Char,Alloc>& operator[](const std::basic_string<Char>& name) const;

    std::basic_string<Char> to_string() const;

    std::basic_string<Char> to_string(const basic_output_format<Char>& format) const;

    void to_stream(std::basic_ostream<Char>& os) const;

    void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format) const;

    void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format, bool indenting) const;

    bool is_null() const
    {
        return var_.is_null();
    }

    bool has_member(const std::basic_string<Char>& name) const;

    template<typename T>
    bool is() const
    {
        return json_type_traits<Char,Alloc,T>::is(*this);
    }

    bool is_string() const
    {
        return var_.is_string();
    }

    bool is_numeric() const
    {
        return var_.type_ == value_types::double_t || var_.type_ == value_types::longlong_t || var_.type_ == value_types::ulonglong_t;
    }

    bool is_bool() const
    {
        return var_.is_bool();
    }

    bool is_object() const
    {
        return var_.type_ == value_types::object_t || var_.type_ == value_types::empty_object_t;
    }

    bool is_array() const
    {
        return var_.type_ == value_types::array_t;
    }

    bool is_any() const
    {
        return var_.type_ == value_types::any_t;
    }

    bool is_longlong() const
    {
        return var_.type_ == value_types::longlong_t;
    }

    bool is_ulonglong() const
    {
        return var_.type_ == value_types::ulonglong_t;
    }

    bool is_double() const
    {
        return var_.type_ == value_types::double_t;
    }

    bool is_empty() const;

    size_t capacity() const;

    void reserve(size_t n);

    void resize(size_t n);

    void resize_array(size_t n)
    {
        resize(n);
    }

    template <typename T>
    void resize(size_t n, T val);

    template <typename T>
    void resize_array(size_t n, T val)
    {
        resize(n,val);
    }

    template<typename T>
    T as() const
    {
        return json_type_traits<Char,Alloc,T>::as(*this);
    }

    bool as_bool() const;

    long long as_longlong() const;

    unsigned long long as_ulonglong() const;

    double as_double() const;

    // Deprecated

    int as_int() const;

    unsigned int as_uint() const;

    long as_long() const;

    unsigned long as_ulong() const;

    std::basic_string<Char> as_string() const;

    std::basic_string<Char> as_string(const basic_output_format<Char>& format) const;

    const Char* as_cstring() const;

    any& any_value();

    const any& any_value() const;

    basic_json<Char,Alloc>& at(const std::basic_string<Char>& name);
    const basic_json<Char,Alloc>& at(const std::basic_string<Char>& name) const;

    basic_json<Char,Alloc>& at(size_t i);
    const basic_json<Char,Alloc>& at(size_t i) const;

    const basic_json<Char,Alloc>& get(const std::basic_string<Char>& name) const;

    template <typename T>
    const_val_proxy get(const std::basic_string<Char>& name, T default_val) const;

    // Modifiers

    void clear();
    // Remove all elements from an array or object

    void remove_range(size_t from_index, size_t to_index);
    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void remove_member(const std::basic_string<Char>& name);
    // Removes a member from an object value
/*
    template <typename T>
    void set(const std::basic_string<Char>& name, T value)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_ = new json_object<Char,Alloc>();
        case value_types::object_t:
            {
                basic_json<Char,Alloc> o;
                json_type_traits<Char,Alloc,T>::assign(o,value);
                var_.value_.object_->set(name,o);
            }
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
            }
        }

    }
*/
    void set(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& value);

    void set(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& value);
/*
    template <typename T>
    void add(T val)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            {
                basic_json<Char,Alloc> a;
                json_type_traits<Char,Alloc,T>::assign(a,val);
                var_.value_.array_->push_back(std::move(a));
            }
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
            }
        }
    }

    template <typename T>
    void add(size_t index, T val)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            {
                basic_json<Char,Alloc> a;
                json_type_traits<Char,Alloc,T>::assign(a,val);
                var_.value_.array_->add(index, std::move(a));
            }
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
            }
        }
    }
*/
    void add(basic_json<Char,Alloc>&& value);

    void add(size_t index, basic_json<Char,Alloc>&& value);

    void add(const basic_json<Char,Alloc>& value);

    void add(size_t index, const basic_json<Char,Alloc>& value);

    value_types::value_types_t type() const
    {
        return var_.type_;
    }

    void to_stream(basic_json_output_handler<Char>& handler) const;

    void swap(basic_json<Char,Alloc>& b)
    {
        var_.swap(b.var_);
    }

    template <class T>
    std::vector<T> as_vector() const
    {
        std::vector<T> v(size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = json_type_traits<Char,Alloc,T>::as(at(i));
        }
        return v;
    }

    friend void swap(basic_json<Char,Alloc>& a, basic_json<Char,Alloc>& b)
    {
        a.swap(b);
    }

    void assign_any(const typename basic_json<Char,Alloc>::any& rhs)
    {
        var_.assign(rhs);
    }

    void assign_string(const std::basic_string<Char>& rhs)
    {
        var_.assign(rhs);
    }

    void assign_string(const Char* rhs, size_t length)
    {
        var_.assign_string(rhs,length);
    }

    void assign_bool(bool rhs)
    {
        var_.assign(rhs);
    }

    void assign_object(const json_object<Char,Alloc>& rhs)
    {
        var_.assign(rhs);
    }

    void assign_array(const json_array<Char,Alloc>& rhs)
    {
        var_.assign(rhs);
    }

    void assign_null()
    {
        var_.assign(null_type());
    }

    template <typename T>
    const T& any_cast() const
    {
        if (var_.type_ != value_types::any_t)
        {
            JSONCONS_THROW_EXCEPTION("Bad any cast");
        }
        return var_.value_.any_value_->template cast<T>();
    }
    template <typename T>
    T& any_cast() 
    {
        if (var_.type_ != value_types::any_t)
        {
            JSONCONS_THROW_EXCEPTION("Bad any cast");
        }
        return var_.value_.any_value_->template cast<T>();
    }

    void assign_double(double rhs)
    {
        var_.assign(rhs);
    }
    void assign_longlong(long long rhs)
    {
        var_.assign(rhs);
    }
    void assign_ulonglong(unsigned long long rhs)
    {
        var_.assign(rhs);
    }

    static basic_json make_2d_array(size_t m, size_t n);

    template <typename T>
    static basic_json make_2d_array(size_t m, size_t n, T val);

    static basic_json make_3d_array(size_t m, size_t n, size_t k);

    template <typename T>
    static basic_json make_3d_array(size_t m, size_t n, size_t k, T val);

    template<int size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array()
    {
        return build_array<Char,Alloc,size>()();
    }
    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n)
    {
        return build_array<Char,Alloc,size>()(n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n, T val)
    {
        return build_array<Char,Alloc,size>()(n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n)
    {
        return build_array<Char,Alloc,size>()(m, n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n, T val)
    {
        return build_array<Char,Alloc,size>()(m, n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k)
    {
        return build_array<Char,Alloc,size>()(m, n, k);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k, T val)
    {
        return build_array<Char,Alloc,size>()(m, n, k, val);
    }

    bool is_number() const
    {
        return is_numeric();
    }

    array& array_value() 
    {
		switch (var_.type_)
		{
        case value_types::array_t:
            return *(var_.value_.array_);
        default:
            JSONCONS_THROW_EXCEPTION("Bad array cast");
            break;
        }
    }

    const array& array_value() const
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            return *(var_.value_.array_);
        default:
            JSONCONS_THROW_EXCEPTION("Bad array cast");
            break;
        }
    }

    object& object_value()
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_ = new json_object<Char,Alloc>();
            return *(var_.value_.object_);
        case value_types::object_t:
            return *(var_.value_.object_);
        default:
            JSONCONS_THROW_EXCEPTION("Bad object cast");
            break;
        }
    }

    const object& object_value() const
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return cobject().object_value();
        case value_types::object_t:
            return *(var_.value_.object_);
        default:
            JSONCONS_THROW_EXCEPTION("Bad object cast");
            break;
        }
    }

private:
    const basic_json<Char,Alloc>& cobject() const
    {
        static const basic_json<Char, Alloc> c = basic_json<Char, Alloc>(value_types::object_t,0);
        return c;
    }

    template<typename Char2, typename Allocator2, size_t size>
    class build_array
    {};
    template<typename Char2, typename Allocator2>
    class build_array<Char2,Allocator2,1>
    {
    public:
        basic_json<Char2,Allocator2> operator() ()
        {
            return basic_json<Char2,Allocator2>::make_array();
        }
        basic_json<Char2,Allocator2> operator() (size_t n)
        {
            return basic_json<Char2,Allocator2>::make_array(n);
        }
        template <typename T>
        basic_json<Char2,Allocator2> operator() (size_t n, T val)
        {
            return basic_json<Char2,Allocator2>::make_array(n, val);
        }
    };
    template<typename Char2, typename Allocator2>
    class build_array<Char2,Allocator2,2>
    {
    public:
        basic_json<Char2,Allocator2> operator() (size_t m, size_t n)
        {
            return basic_json<Char2,Allocator2>::make_2d_array(m, n);
        }
        template <typename T>
        basic_json<Char2,Allocator2> operator() (size_t m, size_t n, T val)
        {
            return basic_json<Char2,Allocator2>::make_2d_array(m, n, val);
        }
    };
    template<typename Char2, typename Allocator2>
    class build_array<Char2,Allocator2,3>
    {
    public:
        basic_json<Char2,Allocator2> operator() (size_t m, size_t n, size_t k)
        {
            return basic_json<Char2,Allocator2>::make_3d_array (m, n, k);
        }
        template <typename T>
        basic_json<Char2,Allocator2> operator() (size_t m, size_t n, size_t k, T val)
        {
            return basic_json<Char2,Allocator2>::make_3d_array (m, n, k, val);
        }
    };

private:
	variant var_;
};

template <typename Char, typename Alloc>
void swap(typename basic_json<Char,Alloc>::member_type& a, typename basic_json<Char,Alloc>::member_type& b)
{
    a.swap(b);
}

typedef basic_json<char,std::allocator<void>> json;
typedef basic_json<wchar_t,std::allocator<void>> wjson;

}

#endif
