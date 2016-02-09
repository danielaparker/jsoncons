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
#include "json_structures.hpp"
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

template <class T, class Alloc>
T* create_instance(const Alloc& allocator)
{
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typename std::allocator_traits<Alloc>:: template rebind_alloc<T> alloc(allocator);
#else
    typename Alloc:: template rebind<T>::other alloc(allocator);
#endif
    T* storage = alloc.allocate(1);
    try
    {
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
        std::allocator_traits<Alloc>:: template rebind_traits<T>::construct(alloc, storage);
#else
        new(storage) T();
#endif
    }
    catch (...)
    {
        alloc.deallocate(storage,1);
        throw;
    }
    return storage;
}

template <class T, class Alloc, class Arg>
T* create_instance(const Alloc& allocator, Arg&& val)
{
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typename std::allocator_traits<Alloc>:: template rebind_alloc<T> alloc(allocator);
#else
    typename Alloc:: template rebind<T>::other alloc(allocator);
#endif
    T* storage = alloc.allocate(1);
    try
    {
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
        std::allocator_traits<Alloc>:: template rebind_traits<T>::construct(alloc, storage, std::forward<Arg>(val));
#else
        new(storage)T(std::forward<Arg>(val));
#endif
    }
    catch (...)
    {
        alloc.deallocate(storage,1);
        throw;
    }
    return storage;
}

template <class T, class Alloc, class Arg1, class Arg2>
T* create_instance(const Alloc& allocator, Arg1&& val1, Arg2&& val2)
{
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typename std::allocator_traits<Alloc>:: template rebind_alloc<T> alloc(allocator);
#else
    typename Alloc:: template rebind<T>::other alloc(allocator);
#endif
    T* storage = alloc.allocate(1);
    try
    {
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
        std::allocator_traits<Alloc>:: template rebind_traits<T>::construct(alloc, storage, std::forward<Arg1>(val1), std::forward<Arg2>(val2));
#else
    new(storage)T(std::forward<Arg1>(val1), std::forward<Arg2>(val2));
#endif
    }
    catch (...)
    {
        alloc.deallocate(storage,1);
        throw;
    }
    return storage;
}

template <class T, class Alloc, class Arg1, class Arg2, class Arg3>
T* create_instance(const Alloc& allocator, Arg1&& val1, Arg2&& val2, Arg3&& val3)
{
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typename std::allocator_traits<Alloc>:: template rebind_alloc<T> alloc(allocator);
#else
    typename Alloc:: template rebind<T>::other alloc(allocator);
#endif
    T* storage = alloc.allocate(1);
    try
    {
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
        std::allocator_traits<Alloc>:: template rebind_traits<T>::construct(alloc, storage, std::forward<Arg1>(val1), std::forward<Arg2>(val2), std::forward<Arg3>(val3));
#else
    new(storage)T(std::forward<Arg1>(val1), std::forward<Arg2>(val2), std::forward<Arg3>(val3));
#endif
    }
    catch (...)
    {
        alloc.deallocate(storage,1);
        throw;
    }
    return storage;
}

template <class T, class Alloc>
void destroy_instance(const Alloc& allocator, T* p)
{
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typename std::allocator_traits<Alloc>:: template rebind_alloc<T> alloc(allocator);
    std::allocator_traits<Alloc>:: template rebind_traits<T>::destroy(alloc, p);
#else
    typename Alloc:: template rebind<T>::other alloc(allocator);
    alloc.destroy(p);
#endif
    alloc.deallocate(p,1);
}

template <typename CharT, class Alloc>
class serializable_any
{
public:
    typedef Alloc allocator_type;

    serializable_any(const Alloc& allocator = Alloc())
        : impl_(nullptr)
    {
        (void)allocator;
    }
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    serializable_any(const serializable_any& val)
        : allocator_(std::allocator_traits<allocator_type>::select_on_container_copy_construction(val.get_allocator()))
    {
		impl_ = val.impl_ != nullptr ? val.impl_->clone(allocator_) : nullptr;
    }
#else
    serializable_any(const serializable_any& val)
        : allocator_(val.get_allocator())
    {
        impl_ = val.impl_ != nullptr ? val.impl_->clone(allocator_) : nullptr;
    }
#endif
    serializable_any(const serializable_any& val, const Alloc& allocator)
    {
        (void)allocator;
        impl_ = val.impl_ != nullptr ? val.impl_->clone(Alloc()) : nullptr;
    }

    serializable_any(serializable_any&& val)
        : impl_(std::move(val.impl_))
    {
        val.impl_ = nullptr;
    }
    serializable_any(serializable_any&& val, const Alloc& allocator)
        : impl_(std::move(val.impl_))
    {
        (void)allocator;
        val.impl_ = nullptr;
    }
    ~serializable_any()
    {
        if (impl_ != nullptr)
        {
            destroy_instance(allocator_,impl_);
        }
    }

    template<typename T>
    explicit serializable_any(T val)
    {
        impl_ = create_instance<any_handle_impl<typename type_wrapper<T>::value_type>>(allocator_,val);
    }

    Alloc get_allocator() const
    {
        return allocator_;
    }

    template <typename T>
    typename type_wrapper<T>::reference cast() 
    {
        if (typeid(*impl_) != typeid(any_handle_impl<typename type_wrapper<T>::value_type>))
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad serializable_any cast");
        }
        return static_cast<any_handle_impl<typename type_wrapper<T>::value_type>&>(*impl_).value_;
    }

    template <typename T>
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

        virtual any_handle* clone(const Alloc& allocator) const = 0;

        virtual void to_stream(basic_json_output_handler<CharT>& os) const = 0;
    };

    template <class T>
    class any_handle_impl : public any_handle
    {
    public:
        any_handle_impl(T value, const Alloc& allocator = Alloc())
            : value_(value)
        {
            (void)allocator;
        }

        virtual any_handle* clone(const Alloc& allocator) const
        {
            return create_instance<any_handle_impl<T>>(allocator, value_);
        }

        virtual void to_stream(basic_json_output_handler<CharT>& os) const
        {
            serialize(os,value_);
        }

        T value_;
    };

    Alloc allocator_;
    any_handle* impl_;
};

template <typename CharT>
struct string_data
{
	const CharT* c_str() const { return p_; }
	size_t length() const { return length_; }

	bool operator==(const string_data& rhs) const
	{
		return length() == rhs.length() ? std::char_traits<CharT>::compare(c_str(), rhs.c_str(), length()) == 0 : false;
	}

    string_data()
        : p_(nullptr), length_(0)
    {
    }

    CharT* p_;
	size_t length_;
private:
	string_data(const string_data&);
	string_data& operator=(const string_data&);
};

template <typename CharT>
struct string_dataA
{
	string_data<CharT> data;
	CharT c[1];
};

template <typename CharT, class Alloc>
string_data<CharT>* create_string_data(const Alloc& allocator)
{
    size_t length = 0;
    typedef typename std::aligned_storage<sizeof(string_dataA<CharT>), JSONCONS_ALIGNOF(string_dataA<CharT>)>::type storage_type;
    size_t mem_size = sizeof(storage_type) + length*sizeof(CharT);

#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typename std::allocator_traits<Alloc>:: template rebind_alloc<char> alloc(allocator);
#else
    typename Alloc:: template rebind<char>::other alloc(allocator);
#endif

	char* storage = alloc.allocate(mem_size);
    string_data<CharT>* ps = new(storage)string_data<CharT>();
    auto psa = reinterpret_cast<string_dataA<CharT>*>(storage); 

    ps->p_ = new(&psa->c)CharT[length + 1];
    ps->p_[length] = 0;
    ps->length_ = length;
    return ps;
}

template <typename CharT, class Alloc>
string_data<CharT>* create_string_data(const CharT* s, size_t length, const Alloc& allocator)
{
    typedef typename std::aligned_storage<sizeof(string_dataA<CharT>), JSONCONS_ALIGNOF(string_dataA<CharT>)>::type storage_type;
    size_t mem_size = sizeof(storage_type) + length*sizeof(CharT);

#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typename std::allocator_traits<Alloc>:: template rebind_alloc<char> alloc(allocator);
#else
    typename Alloc:: template rebind<char>::other alloc(allocator);
#endif

    char* storage = alloc.allocate(mem_size);
    string_data<CharT>* ps = new(storage)string_data<CharT>();
    auto psa = reinterpret_cast<string_dataA<CharT>*>(storage); 

    ps->p_ = new(&psa->c)CharT[length + 1];
    memcpy(ps->p_, s, length*sizeof(CharT));
    ps->p_[length] = 0;
    ps->length_ = length;
    return ps;
}

template <typename CharT, class Alloc>
void destroy_string_data(const Alloc& allocator, string_data<CharT>* p)
{
    typedef typename std::aligned_storage<sizeof(string_dataA<CharT>), JSONCONS_ALIGNOF(string_dataA<CharT>)>::type storage_type;
    size_t mem_size = sizeof(storage_type) + p->length_*sizeof(CharT);
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typename std::allocator_traits<Alloc>:: template rebind_alloc<char> alloc(allocator);
#else
    typename Alloc:: template rebind<char>::other alloc(allocator);
#endif
    alloc.deallocate(reinterpret_cast<char*>(p),mem_size);
}

template <typename CharT,class T> inline
void serialize(basic_json_output_handler<CharT>& os, const T&)
{
    os.value(null_type());
}

template <typename StringT, class Alloc>
class basic_json;

template <typename CharT>
class basic_parse_error_handler;

namespace value_types
{
    enum value_types_t 
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
        array_t,
        any_t
    };
}

inline
bool is_simple(value_types::value_types_t type)
{
    return type < value_types::string_t;
}

template <typename StringT, typename Alloc = std::allocator<typename StringT::value_type>>
class basic_json
{
public:

    typedef Alloc allocator_type;

    typedef typename StringT::value_type char_type;

#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<char_type> name_allocator_type;
#else
    typedef typename allocator_type:: template rebind<char_type>::other name_allocator_type;
#endif
    typedef std::basic_string<char_type,std::char_traits<char_type>,name_allocator_type> string_type;
    typedef basic_json<StringT,Alloc> value_type;
    typedef name_value_pair<string_type,value_type> member_type;

#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typedef typename std::allocator_traits<Alloc>:: template rebind_alloc<basic_json<StringT,Alloc>> array_allocator_type;
#else
    typedef typename Alloc:: template rebind<value_type>::other array_allocator_type;
#endif

#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
    typedef typename std::allocator_traits<Alloc>:: template rebind_alloc<member_type> object_allocator_type;
#else
    typedef typename Alloc:: template rebind<member_type>::other object_allocator_type;
#endif

    typedef json_array<basic_json<StringT,Alloc>,array_allocator_type> array;
    typedef json_object<string_type,basic_json<StringT,Alloc>,object_allocator_type>  object;
    typedef serializable_any<char_type,Alloc> any;

    typedef jsoncons::null_type null_type;

    typedef typename object::iterator object_iterator;
    typedef typename object::const_iterator const_object_iterator;
    typedef typename array::iterator array_iterator;
    typedef typename array::const_iterator const_array_iterator;

    template <typename structure, bool is_const_iterator = true>
    class range 
    {
        typedef typename std::conditional<is_const_iterator, const structure&, structure&>::type structure_ref;
        typedef typename std::conditional<is_const_iterator, typename structure::const_iterator, typename structure::iterator>::type iterator;
        typedef typename structure::const_iterator const_iterator;
        structure_ref val_;

    public:
        range(structure_ref val)
            : val_(val)
        {
        }

    public:
        friend class basic_json<StringT, Alloc>;

        iterator begin()
        {
            return val_.begin();
        }
        iterator end()
        {
            return val_.end();
        }
    };

    typedef range<object,false> object_range;
    typedef range<object,true> const_object_range;
    typedef range<array,false> array_range;
    typedef range<array,true> const_array_range;

    allocator_type get_allocator() const
    {
        return var_;
    }

    struct variant : public Alloc
    {
        Alloc get_allocator() const
        {
            return *this;
        }

        static const size_t small_string_capacity = (sizeof(int64_t)/sizeof(char_type)) - 1;

        variant(const Alloc& a)
            : Alloc(a), type_(value_types::empty_object_t)
        {
        }

        explicit variant(variant&& var)
            : Alloc(var.get_allocator()), type_(value_types::null_t)
        {
            swap(var);
        }
		
        explicit variant(const Alloc& a, variant&& var)
            : Alloc(a), type_(value_types::null_t)
        {
            swap(var);
        }

#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
        explicit variant(const variant& var)
            : Alloc(std::allocator_traits<allocator_type>::select_on_container_copy_construction(var.get_allocator()))
        {
            init_variant(var);
        }
#else
        explicit variant(const variant& var)
            : Alloc(var.get_allocator())
        {
            init_variant(var);
        }
#endif
        explicit variant(const Alloc& a, const variant& var)
            : Alloc(a), type_(var.type_)
        {
            init_variant(var);
        }

        variant(const Alloc& a, const object & val)
            : Alloc(a), type_(value_types::object_t)
        {
            value_.object_value_ = create_instance<object>(get_allocator(), val, object_allocator_type(get_allocator())) ;
        }

        variant(const Alloc& a, object && val)
            : Alloc(a), type_(value_types::object_t)
        {
            value_.object_value_ = create_instance<object>(get_allocator(), std::move(val), object_allocator_type(get_allocator()));
        }

        variant(const Alloc& a, const array& val)
            : Alloc(a), type_(value_types::array_t)
        {
            value_.array_value_ = create_instance<array>(get_allocator(), val, array_allocator_type(*this));
        }

        variant(const Alloc& a, array&& val)
            : Alloc(a), type_(value_types::array_t)
        {
            value_.array_value_ = create_instance<array>(get_allocator(), std::move(val), array_allocator_type(*this));
        }

        explicit variant(const Alloc& a, const any& val)
            : Alloc(a), type_(value_types::any_t)
        {
            value_.any_value_ = create_instance<any>(get_allocator(), val);
        }

        explicit variant(const Alloc& a, jsoncons::null_type)
            : Alloc(a), type_(value_types::null_t)
        {
        }

        explicit variant(const Alloc& a, bool val)
            : Alloc(a), type_(value_types::bool_t)
        {
            value_.bool_value_ = val;
        }

        explicit variant(const Alloc& a, double val)
            : Alloc(a), type_(value_types::double_t)
        {
            value_.float_value_ = val;
        }

        explicit variant(const Alloc& a, int64_t val)
            : Alloc(a), type_(value_types::integer_t)
        {
            value_.integer_value_ = val;
        }

        explicit variant(const Alloc& a, uint64_t val)
            : Alloc(a), type_(value_types::uinteger_t)
        {
            value_.uinteger_value_ = val;
        }

        explicit variant(const Alloc& a, const string_type& s)
            : Alloc(a)
        {
            if (s.length() > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                value_.string_value_ = create_string_data(s.data(),s.length(),get_allocator());
            }
            else
            {
                type_ = value_types::small_string_t;
                small_string_length_ = (unsigned char)s.length();
                std::memcpy(value_.small_string_value_,s.data(),s.length()*sizeof(char_type));
                value_.small_string_value_[small_string_length_] = 0;
            }
        }

        explicit variant(const Alloc& a, const char_type* s)
            : Alloc(a)
        {
            size_t length = std::char_traits<char_type>::length(s);
            if (length > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                value_.string_value_ = create_string_data(s,std::char_traits<char_type>::length(s),get_allocator());
            }
            else
            {
                type_ = value_types::small_string_t;
                small_string_length_ = (unsigned char)length;
                std::memcpy(value_.small_string_value_,s,length*sizeof(char_type));
                value_.small_string_value_[small_string_length_] = 0;
            }
        }

        explicit variant(const Alloc& a, const char_type* s, size_t length)
            : Alloc(a)
        {
            if (length > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                value_.string_value_ = create_string_data(s,length,get_allocator());
            }
            else
            {
                type_ = value_types::small_string_t;
                small_string_length_ = (unsigned char)length;
                std::memcpy(value_.small_string_value_,s,length*sizeof(char_type));
                value_.small_string_value_[small_string_length_] = 0;
            }
        }

        template<class InputIterator>
        variant(const Alloc& a, InputIterator first, InputIterator last)
            : Alloc(a), type_(value_types::array_t)
        {
            value_.array_value_ = create_instance<array>(get_allocator(), first, last, array_allocator_type(*this));
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
                value_.float_value_ = var.value_.float_value_;
                break;
            case value_types::integer_t:
                value_.integer_value_ = var.value_.integer_value_;
                break;
            case value_types::uinteger_t:
                value_.uinteger_value_ = var.value_.uinteger_value_;
                break;
            case value_types::bool_t:
                value_.bool_value_ = var.value_.bool_value_;
                break;
            case value_types::small_string_t:
                small_string_length_ = var.small_string_length_;
                std::memcpy(value_.small_string_value_,var.value_.small_string_value_,var.small_string_length_*sizeof(char_type));
                value_.small_string_value_[small_string_length_] = 0;
                break;
            case value_types::string_t:
                value_.string_value_ = create_string_data(var.value_.string_value_->c_str(),var.value_.string_value_->length(),get_allocator());
                break;
            case value_types::array_t:
                value_.array_value_ = create_instance<array>(get_allocator(), *(var.value_.array_value_), array_allocator_type(*this));
                break;
            case value_types::object_t:
                value_.object_value_ = create_instance<object>(get_allocator(), *(var.value_.object_value_), object_allocator_type(*this));
                break;
            case value_types::any_t:
                value_.any_value_ = create_instance<any>(get_allocator(), *(var.value_.any_value_));
                break;
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
                destroy_string_data(get_allocator(), value_.string_value_);
                break;
            case value_types::array_t:
                destroy_instance(get_allocator(), value_.array_value_);
                break;
            case value_types::object_t:
                destroy_instance(get_allocator(), value_.object_value_);
                break;
            case value_types::any_t:
                destroy_instance(get_allocator(), value_.any_value_);
                break;
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
                        small_string_length_ = val.small_string_length_;
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
			value_.object_value_ = create_instance<object>(get_allocator(), val, object_allocator_type(*this));
		}

        void assign(object && val)
        {
			switch (type_)
			{
			case value_types::object_t:
				value_.object_value_->swap(val);
				break;
			default:
				destroy_variant();
				type_ = value_types::object_t;
				value_.object_value_ = create_instance<object>(get_allocator(), std::move(val), object_allocator_type(*this));
				break;
			}
		}

        void assign(const array& val)
        {
            destroy_variant();
            type_ = value_types::array_t;
            value_.array_value_ = create_instance<array>(get_allocator(), val, array_allocator_type(*this)) ;
        }

        void assign(array&& val)
        {
			switch (type_)
			{
			case value_types::array_t:
				value_.array_value_->swap(val);
				break;
			default:
				destroy_variant();
				type_ = value_types::array_t;
				value_.array_value_ = create_instance<array>(get_allocator(), std::move(val), array_allocator_type(*this));
				break;
			}
		}

        void assign(const string_type& s)
        {
            destroy_variant();
            if (s.length() > variant::small_string_capacity)
            {
                type_ = value_types::string_t;
                value_.string_value_ = create_string_data(s.data(),s.length(),get_allocator());
            }
            else
            {
                type_ = value_types::small_string_t;
                small_string_length_ = (unsigned char)s.length();
                std::memcpy(value_.small_string_value_,s.data(),s.length()*sizeof(char_type));
                value_.small_string_value_[small_string_length_] = 0;
            }
        }

        void assign_string(const char_type* s, size_t length)
        {
            destroy_variant();
			if (length > variant::small_string_capacity)
			{
				type_ = value_types::string_t;
				value_.string_value_ = create_string_data(s,length,get_allocator());
			}
			else
			{
				type_ = value_types::small_string_t;
				small_string_length_ = (unsigned char)length;
				std::memcpy(value_.small_string_value_,s,length*sizeof(char_type));
                value_.small_string_value_[small_string_length_] = 0;
			}
        }

        void assign(int64_t val)
        {
            destroy_variant();
            type_ = value_types::integer_t;
            value_.integer_value_ = val;
        }

        void assign(uint64_t val)
        {
            destroy_variant();
            type_ = value_types::uinteger_t;
            value_.uinteger_value_ = val;
        }

        void assign(double val)
        {
            destroy_variant();
            type_ = value_types::double_t;
            value_.float_value_ = val;
        }

        void assign(bool val)
        {
            destroy_variant();
            type_ = value_types::bool_t;
            value_.bool_value_ = val;
        }

        void assign(null_type)
        {
            destroy_variant();
            type_ = value_types::null_t;
        }

        void assign(const any& rhs)
        {
            destroy_variant();
            type_ = value_types::any_t;
            value_.any_value_ = create_instance<any>(get_allocator(), rhs);
        }

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
                        return value_.integer_value_ == rhs.value_.integer_value_;
                    case value_types::uinteger_t:
                        return value_.integer_value_ == rhs.value_.uinteger_value_;
                    case value_types::double_t:
                        return value_.integer_value_ == rhs.value_.float_value_;
                    default:
                        break;
                    }
                    break;
                case value_types::uinteger_t:
                    switch (rhs.type_)
                    {
                    case value_types::integer_t:
                        return value_.uinteger_value_ == rhs.value_.integer_value_;
                    case value_types::uinteger_t:
                        return value_.uinteger_value_ == rhs.value_.uinteger_value_;
                    case value_types::double_t:
                        return value_.uinteger_value_ == rhs.value_.float_value_;
                    default:
                        break;
                    }
                    break;
                case value_types::double_t:
                    switch (rhs.type_)
                    {
                    case value_types::integer_t:
                        return value_.float_value_ == rhs.value_.integer_value_;
                    case value_types::uinteger_t:
                        return value_.float_value_ == rhs.value_.uinteger_value_;
                    case value_types::double_t:
                        return value_.float_value_ == rhs.value_.float_value_;
                    default:
                        break;
                    }
                    break;
                default:
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
                return small_string_length_ == rhs.small_string_length_ ? std::char_traits<char_type>::compare(value_.small_string_value_,rhs.value_.small_string_value_,small_string_length_) == 0 : false;
            case value_types::string_t:
                return *(value_.string_value_) == *(rhs.value_.string_value_);
            case value_types::array_t:
                return *(value_.array_value_) == *(rhs.value_.array_value_);
                break;
            case value_types::object_t:
                return *(value_.object_value_) == *(rhs.value_.object_value_);
                break;
            case value_types::any_t:
                break;
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
                return small_string_length_ == 0;
            case value_types::string_t:
                return value_.string_value_->length() == 0;
            case value_types::array_t:
                return value_.array_value_->size() == 0;
            case value_types::empty_object_t:
                return true;
            case value_types::object_t:
                return value_.object_value_->size() == 0;
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
            else if (get_allocator() == rhs.get_allocator())
            {
                // same allocator, swap contents
                swap(type_,rhs.type_);
                swap(small_string_length_, rhs.small_string_length_);
                swap(value_, rhs.value_);
            }
#if !defined(JSONCONS_NO_CXX11_ALLOCATOR)
            else if (std::allocator_traits<allocator_type>::propagate_on_container_swap::value)
            {
                // swap allocators and contents
                swap(static_cast<Alloc&>(*this),static_cast<Alloc&>(rhs));
                swap(type_, rhs.type_);
                swap(small_string_length_, rhs.small_string_length_);
                swap(value_, rhs.value_);
            }
#endif
            else
            {
                // Undefined behaviour
                std::terminate();
            }
        }

        value_types::value_types_t type_;
        unsigned char small_string_length_;
        union
        {
            double float_value_;
            int64_t integer_value_;
            uint64_t uinteger_value_;
            bool bool_value_;
            object* object_value_;
            array* array_value_;
            any* any_value_;
            string_data<char_type>* string_value_;
            char_type small_string_value_[sizeof(int64_t)/sizeof(char_type)];
        } value_;
    };

    template <class ParentT>
    class json_proxy 
    {
    private:
        typedef json_proxy<ParentT> proxy_type;

        ParentT& parent_;
        const string_type& name_;

        json_proxy(); // noop
        json_proxy& operator = (const json_proxy& other); // noop

        json_proxy(ParentT& parent, const string_type& name)
            : parent_(parent), name_(name)
        {
        }

        basic_json<StringT,Alloc>& evaluate() 
        {
            return parent_.evaluate(name_);
        }

        const basic_json<StringT,Alloc>& evaluate() const
        {
            return parent_.evaluate(name_);
        }

        basic_json<StringT,Alloc>& evaluate_with_default()
        {
            basic_json<StringT,Alloc>& val = parent_.evaluate_with_default();
            auto it = val.find(name_);
            if (it == val.members().end())
            {
                it = val.set(val.members().begin(),name_,basic_json<StringT,Alloc>());
            }
            return it->value();
        }

        basic_json<StringT,Alloc>& evaluate(size_t index)
        {
            return parent_.evaluate(name_).at(index);
        }

        const basic_json<StringT,Alloc>& evaluate(size_t index) const
        {
            return parent_.evaluate(name_).at(index);
        }

        basic_json<StringT,Alloc>& evaluate(const string_type& index)
        {
            return parent_.evaluate(name_).at(index);
        }

        const basic_json<StringT,Alloc>& evaluate(const string_type& index) const
        {
            return parent_.evaluate(name_).at(index);
        }
    public:

        friend class basic_json<StringT,Alloc>;

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

        value_types::value_types_t type() const
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

        template <typename T>
        void resize(size_t n, T val)
        {
            evaluate().resize(n,val);
        }

        template<typename T>
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
 
        bool is_any() const JSONCONS_NOEXCEPT
        {
            return evaluate().is_any();
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

        string_type as_string() const JSONCONS_NOEXCEPT
        {
            return evaluate().as_string();
        }

        string_type as_string(const basic_output_format<char_type>& format) const
        {
            return evaluate().as_string(format);
        }

        template<typename T>
        T as() const
        {
            return evaluate().template as<T>();
        }

        any& any_value()
        {
            return evaluate().any_value();
        }

        const any& any_value() const
        {
            return evaluate().any_value();
        }

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

        operator basic_json&()
        {
            return evaluate();
        }

        operator const basic_json&() const
        {
            return evaluate();
        }

        template <typename T>
        json_proxy& operator=(T val)
        {
            parent_.evaluate_with_default().set(name_, val);
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

        basic_json<StringT,Alloc>& operator[](size_t i)
        {
            return evaluate_with_default().at(i);
        }

        const basic_json<StringT,Alloc>& operator[](size_t i) const
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

        basic_json<StringT,Alloc>& at(const string_type& name)
        {
            return evaluate().at(name);
        }

        const basic_json<StringT,Alloc>& at(const string_type& name) const
        {
            return evaluate().at(name);
        }

        const basic_json<StringT,Alloc>& at(size_t index)
        {
            return evaluate().at(index);
        }

        const basic_json<StringT,Alloc>& at(size_t index) const
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

        template <typename T>
        basic_json<StringT,Alloc> get(const string_type& name, T&& default_val) const
        {
            return evaluate().get(name,std::forward<T>(default_val));
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

        void set(const string_type& name, const basic_json<StringT,Alloc>& value)
        {
            evaluate().set(name,value);
        }

        void set(string_type&& name, const basic_json<StringT,Alloc>& value)

        {
            evaluate().set(std::move(name),value);
        }

        void set(const string_type& name, basic_json<StringT,Alloc>&& value)

        {
            evaluate().set(name,std::move(value));
        }

        void set(string_type&& name, basic_json<StringT,Alloc>&& value)

        {
            evaluate().set(std::move(name),std::move(value));
        }

        object_iterator set(object_iterator hint, const string_type& name, const basic_json<StringT,Alloc>& value)
        {
            return evaluate().set(hint, name,value);
        }

        object_iterator set(object_iterator hint, string_type&& name, const basic_json<StringT,Alloc>& value)

        {
            return evaluate().set(hint, std::move(name),value);
        }

        object_iterator set(object_iterator hint, const string_type& name, basic_json<StringT,Alloc>&& value)

        {
            return evaluate().set(hint, name,std::move(value));
        }

        object_iterator set(object_iterator hint, string_type&& name, basic_json<StringT,Alloc>&& value)

        {
            return evaluate().set(hint, std::move(name),std::move(value));
        }

        void add(basic_json<StringT,Alloc>&& value)
        {
            evaluate_with_default().add(std::move(value));
        }

        void add(const basic_json<StringT,Alloc>& value)
        {
            evaluate_with_default().add(value);
        }

        array_iterator add(const_array_iterator pos, const basic_json<StringT,Alloc>& value)
        {
            return evaluate_with_default().add(pos, value);
        }

        array_iterator add(const_array_iterator pos, basic_json<StringT,Alloc>&& value)
        {
            return evaluate_with_default().add(pos, std::move(value));
        }

        string_type to_string() const JSONCONS_NOEXCEPT
        {
            return evaluate().to_string();
        }

        string_type to_string(const basic_output_format<char_type>& format) const
        {
            return evaluate().to_string(format);
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

        void swap(basic_json<StringT,Alloc>& val)
        {
            evaluate_with_default().swap(val);
        }

        friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_proxy& o)
        {
            o.to_stream(os);
            return os;
        }

#if !defined(JSONCONS_NO_DEPRECATED)

        void resize_array(size_t n)
        {
            evaluate().resize_array(n);
        }

        template <typename T>
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

        const basic_json<StringT,Alloc>& get(const string_type& name) const
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

        void add(size_t index, const basic_json<StringT,Alloc>& value)
        {
            evaluate_with_default().add(index, value);
        }

        void add(size_t index, basic_json<StringT,Alloc>&& value)
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
        basic_json_deserializer<basic_json<StringT, Alloc>> handler;
        basic_json_parser<char_type> parser(handler);
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

    static basic_json parse(const string_type& s, basic_parse_error_handler<char_type>& err_handler)
    {
        basic_json_deserializer<basic_json<StringT, Alloc>> handler;
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

    static basic_json parse_file(const std::string& s);

    static basic_json parse_file(const std::string& s, basic_parse_error_handler<char_type>& err_handler);

    static basic_json<StringT,Alloc> make_array()
    {
        return typename basic_json<StringT,Alloc>::array();
    }

    static basic_json<StringT,Alloc> make_array(size_t n)
    {
        basic_json<StringT,Alloc> val = make_array();
        val.resize(n);
        return val;
    }

    template <typename U>
    static basic_json<StringT,Alloc> make_array(size_t n, U val)
    {
        basic_json<StringT,Alloc> a = make_array();
        a.resize(n,val);
        return a;
    }

    template<int size>
    static typename std::enable_if<size==1,basic_json>::type make_array()
    {
        return build_array<StringT,Alloc,size>()();
    }

    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_array(size_t n)
    {
        return build_array<StringT,Alloc,size>()(n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==1,basic_json>::type make_array(size_t n, T val)
    {
        return build_array<StringT,Alloc,size>()(n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_array(size_t m, size_t n)
    {
        return build_array<StringT,Alloc,size>()(m, n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==2,basic_json>::type make_array(size_t m, size_t n, T val)
    {
        return build_array<StringT,Alloc,size>()(m, n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_array(size_t m, size_t n, size_t k)
    {
        return build_array<StringT,Alloc,size>()(m, n, k);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==3,basic_json>::type make_array(size_t m, size_t n, size_t k, T val)
    {
        return build_array<StringT,Alloc,size>()(m, n, k, val);
    }

    variant var_;

    basic_json(const Alloc& allocator = Alloc()) 
        : var_(allocator)
    {
    }

    basic_json(const basic_json<StringT, Alloc>& val)
        : var_(val.var_)
    {
    }

    basic_json(const basic_json<StringT, Alloc>& val, const Alloc& allocator)
        : var_(allocator, val.var_)
    {
    }

    basic_json(basic_json<StringT,Alloc>&& other)
        : var_(std::move(other.var_))
    {
    }

    basic_json(basic_json<StringT,Alloc>&& other, const Alloc& allocator)
        : var_(allocator, std::move(other.var_))
    {
    }

    basic_json(const array& val, const Alloc& allocator = Alloc())
        : var_(allocator, val)
    {
    }

    basic_json(array&& other, const Alloc& allocator = Alloc())
        : var_(allocator, std::move(other))
    {
    }

    basic_json(object && other, const Alloc& allocator = Alloc())
        : var_(allocator, std::move(other))
    {
    }

    template <class ParentT>
    basic_json(const json_proxy<ParentT>& proxy, const Alloc& allocator = Alloc())
        : var_(allocator, proxy.evaluate().var_)
    {
    }

    template <typename T>
    basic_json(T val, const Alloc& allocator = Alloc())
        : var_(allocator)
    {
        json_type_traits<value_type,T>::assign(*this,val);
    }

    basic_json(const char_type *s, size_t length, const Alloc& allocator = Alloc())
        : var_(allocator, s, length)
    {
    }
    template<class InputIterator>
    basic_json(InputIterator first, InputIterator last, const Alloc& allocator = Alloc())
        : var_(allocator,first,last)
    {
    }

    ~basic_json()
    {
    }

    basic_json& operator=(const basic_json<StringT,Alloc>& rhs)
    {
        var_ = rhs.var_;
        return *this;
    }

    basic_json& operator=(basic_json<StringT,Alloc>&& rhs)
    {
        if (this != &rhs)
        {
            var_ = std::move(rhs.var_);
        }
        return *this;
    }

    template <class T>
    basic_json<StringT, Alloc>& operator=(T val)
    {
        json_type_traits<value_type,T>::assign(*this,val);
        return *this;
    }

    bool operator!=(const basic_json<StringT,Alloc>& rhs) const;

    bool operator==(const basic_json<StringT,Alloc>& rhs) const;

    size_t size() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return 0;
        case value_types::object_t:
            return var_.value_.object_value_->size();
        case value_types::array_t:
            return var_.value_.array_value_->size();
        default:
            return 0;
        }
    }

    basic_json<StringT,Alloc>& operator[](size_t i)
    {
        return at(i);
    }

    const basic_json<StringT,Alloc>& operator[](size_t i) const
    {
        return at(i);
    }

    json_proxy<basic_json<StringT, Alloc>> operator[](const string_type& name)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
        case value_types::object_t:
            return json_proxy<basic_json<StringT,Alloc>>(*this, name);
            break;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
            break;
        }
    }

    const basic_json<StringT,Alloc>& operator[](const string_type& name) const
    {
        return at(name);
    }

    string_type to_string() const JSONCONS_NOEXCEPT
    {
        std::basic_ostringstream<char_type> os;
        {
            basic_json_serializer<char_type> serializer(os);
            to_stream(serializer);
        }
        return os.str();
    }

    string_type to_string(const basic_output_format<char_type>& format) const
    {
        std::basic_ostringstream<char_type> os;
        {
            basic_json_serializer<char_type> serializer(os, format);
            to_stream(serializer);
        }
        return os.str();
    }

    void to_stream(basic_json_output_handler<char_type>& handler) const
    {
        switch (var_.type_)
        {
        case value_types::small_string_t:
            handler.value(var_.value_.small_string_value_,var_.small_string_length_);
            break;
        case value_types::string_t:
            handler.value(var_.value_.string_value_->c_str(),var_.value_.string_value_->length());
            break;
        case value_types::double_t:
            handler.value(var_.value_.float_value_);
            break;
        case value_types::integer_t:
            handler.value(var_.value_.integer_value_);
            break;
        case value_types::uinteger_t:
            handler.value(var_.value_.uinteger_value_);
            break;
        case value_types::bool_t:
            handler.value(var_.value_.bool_value_);
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
                object* o = var_.value_.object_value_;
                for (const_object_iterator it = o->begin(); it != o->end(); ++it)
                {
                    handler.name((it->name()).data(),it->name().length());
                    it->value().to_stream(handler);
                }
                handler.end_object();
            }
            break;
        case value_types::array_t:
            {
                handler.begin_array();
                array *o = var_.value_.array_value_;
                for (const_array_iterator it = o->begin(); it != o->end(); ++it)
                {
                    it->to_stream(handler);
                }
                handler.end_array();
            }
            break;
        case value_types::any_t:
            var_.value_.any_value_->to_stream(handler);
            break;
        default:
            break;
        }
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
                auto it = var_.value_.object_value_->find(name);
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

    template<typename T>
    bool is() const
    {
        return json_type_traits<value_type,T>::is(*this);
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

    bool is_any() const JSONCONS_NOEXCEPT
    {
        return var_.type_ == value_types::any_t;
    }

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
            return var_.value_.array_value_->capacity();
        case value_types::object_t:
            return var_.value_.object_value_->capacity();
        default:
            return 0;
        }
    }

    void reserve(size_t n)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_value_->reserve(n);
            break;
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
        case value_types::object_t:
            var_.value_.object_value_->reserve(n);
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
            var_.value_.array_value_->resize(n);
            break;
        }
    }

    template <typename T>
    void resize(size_t n, T val)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_value_->resize(n, val);
            break;
        }
    }

    template<typename T>
    T as() const
    {
        return json_type_traits<value_type,T>::as(*this);
    }

    bool as_bool() const JSONCONS_NOEXCEPT
    {
        switch (var_.type_)
        {
        case value_types::null_t:
        case value_types::empty_object_t:
            return false;
        case value_types::bool_t:
            return var_.value_.bool_value_;
        case value_types::double_t:
            return var_.value_.float_value_ != 0.0;
        case value_types::integer_t:
            return var_.value_.integer_value_ != 0;
        case value_types::uinteger_t:
            return var_.value_.uinteger_value_ != 0;
        case value_types::small_string_t:
            return var_.small_string_length_ != 0;
        case value_types::string_t:
            return var_.value_.string_value_->length() != 0;
        case value_types::array_t:
            return var_.value_.array_value_->size() != 0;
        case value_types::object_t:
            return var_.value_.object_value_->size() != 0;
        case value_types::any_t:
            return true;
        default:
            return false;
        }
    }

    int64_t as_integer() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<int64_t>(var_.value_.float_value_);
        case value_types::integer_t:
            return static_cast<int64_t>(var_.value_.integer_value_);
        case value_types::uinteger_t:
            return static_cast<int64_t>(var_.value_.uinteger_value_);
        case value_types::bool_t:
            return var_.value_.bool_value_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an integer");
        }
    }

    uint64_t as_uinteger() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<uint64_t>(var_.value_.float_value_);
        case value_types::integer_t:
            return static_cast<uint64_t>(var_.value_.integer_value_);
        case value_types::uinteger_t:
            return static_cast<uint64_t>(var_.value_.uinteger_value_);
        case value_types::bool_t:
            return var_.value_.bool_value_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned integer");
        }
    }

    double as_double() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return var_.value_.float_value_;
        case value_types::integer_t:
            return static_cast<double>(var_.value_.integer_value_);
        case value_types::uinteger_t:
            return static_cast<double>(var_.value_.uinteger_value_);
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
            return string_type(var_.value_.small_string_value_,var_.small_string_length_);
        case value_types::string_t:
            return string_type(var_.value_.string_value_->c_str(),var_.value_.string_value_->length());
        default:
            return to_string();
        }
    }

    string_type as_string(const basic_output_format<char_type>& format) const 
    {
        switch (var_.type_)
        {
        case value_types::small_string_t:
            return string_type(var_.value_.small_string_value_,var_.small_string_length_);
        case value_types::string_t:
            return string_type(var_.value_.string_value_->c_str(),var_.value_.string_value_->length());
        default:
            return to_string(format);
        }
    }

    const char_type* as_cstring() const
    {
        switch (var_.type_)
        {
        case value_types::small_string_t:
            return var_.value_.small_string_value_;
        case value_types::string_t:
            return var_.value_.string_value_->c_str();
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a cstring");
        }
    }

    any& any_value();

    const any& any_value() const;

    basic_json<StringT, Alloc>& at(const string_type& name)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case value_types::object_t:
            {
                auto it = var_.value_.object_value_->find(name);
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

    basic_json<StringT, Alloc>& evaluate() 
    {
        return *this;
    }

    basic_json<StringT, Alloc>& evaluate_with_default() 
    {
        return *this;
    }

    const basic_json<StringT, Alloc>& evaluate() const
    {
        return *this;
    }

    basic_json<StringT, Alloc>& evaluate(size_t i) 
    {
        return at(i);
    }

    const basic_json<StringT, Alloc>& evaluate(size_t i) const
    {
        return at(i);
    }

    basic_json<StringT, Alloc>& evaluate(const string_type& name) 
    {
        return at(name);
    }

    const basic_json<StringT, Alloc>& evaluate(const string_type& name) const
    {
        return at(name);
    }

    const basic_json<StringT, Alloc>& at(const string_type& name) const
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            JSONCONS_THROW_EXCEPTION_1(std::out_of_range,"%s not found", name);
        case value_types::object_t:
            {
                auto it = var_.value_.object_value_->find(name);
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

    basic_json<StringT, Alloc>& at(size_t i)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            if (i >= var_.value_.array_value_->size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return var_.value_.array_value_->operator[](i);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Index on non-array value not supported");
        }
    }

    const basic_json<StringT, Alloc>& at(size_t i) const
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            if (i >= var_.value_.array_value_->size())
            {
                JSONCONS_THROW_EXCEPTION(std::out_of_range,"Invalid array subscript");
            }
            return var_.value_.array_value_->operator[](i);
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
            return var_.value_.object_value_->find(name);
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
            return var_.value_.object_value_->find(name);
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
            return var_.value_.object_value_->find(name);
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
            return var_.value_.object_value_->find(name);
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to get %s from a value that is not an object", name);
            }
        }
    }

    template<typename T>
    basic_json<StringT, Alloc> get(const string_type& name, T&& default_val) const
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            {
                return basic_json<StringT,Alloc>(std::forward<T>(default_val));
            }
        case value_types::object_t:
            {
                const_object_iterator it = var_.value_.object_value_->find(name);
                if (it != members().end())
                {
                    return it->value();
                }
                else
                {
                    return basic_json<StringT,Alloc>(std::forward<T>(default_val));
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
            var_.value_.array_value_->shrink_to_fit();
            break;
        case value_types::object_t:
            var_.value_.object_value_->shrink_to_fit();
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
            var_.value_.array_value_->clear();
            break;
        case value_types::object_t:
            var_.value_.object_value_->clear();
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
            var_.value_.object_value_->erase(first, last);
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
            var_.value_.array_value_->erase(first, last);
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
        case value_types::object_t:
            var_.value_.object_value_->erase(name);
            break;
        default:
            JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            break;
        }
    }

    void set(const string_type& name, const basic_json<StringT, Alloc>& value)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
        case value_types::object_t:
            var_.value_.object_value_->set(name, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    void set(string_type&& name, const basic_json<StringT, Alloc>& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
        case value_types::object_t:
            var_.value_.object_value_->set(std::move(name),value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    void set(const string_type& name, basic_json<StringT, Alloc>&& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
        case value_types::object_t:
            var_.value_.object_value_->set(name,std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    void set(string_type&& name, basic_json<StringT, Alloc>&& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
        case value_types::object_t:
            var_.value_.object_value_->set(std::move(name),std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    object_iterator set(object_iterator hint, const string_type& name, const basic_json<StringT, Alloc>& value)
    {
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
            var_.value_.object_value_->set(name, value);
            return members().begin();
            break;
        case value_types::object_t:
            return var_.value_.object_value_->set(hint, name, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object", name);
            }
        }
    }

    object_iterator set(object_iterator hint, string_type&& name, const basic_json<StringT, Alloc>& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
            var_.value_.object_value_->set(std::move(name), value);
            return members().begin();
            break;
        case value_types::object_t:
            return var_.value_.object_value_->set(hint, std::move(name),value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    object_iterator set(object_iterator hint, const string_type& name, basic_json<StringT, Alloc>&& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
            var_.value_.object_value_->set(name, std::move(value));
            return members().begin();
            break;
        case value_types::object_t:
            return var_.value_.object_value_->set(hint, name,std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    } 

    object_iterator set(object_iterator hint, string_type&& name, basic_json<StringT, Alloc>&& value){
        switch (var_.type_){
        case value_types::empty_object_t:
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
            var_.value_.object_value_->set(std::move(name), std::move(value));
            return members().begin();
            break;
        case value_types::object_t:
            return var_.value_.object_value_->set(hint, std::move(name),std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Attempting to set %s on a value that is not an object",name);
            }
        }
    }

    void add(const basic_json<StringT, Alloc>& value)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_value_->push_back(value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    void add(basic_json<StringT, Alloc>&& value){
        switch (var_.type_){
        case value_types::array_t:
            var_.value_.array_value_->push_back(std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    array_iterator add(const_array_iterator pos, const basic_json<StringT, Alloc>& value)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            return var_.value_.array_value_->add(pos, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    array_iterator add(const_array_iterator pos, basic_json<StringT, Alloc>&& value){
        switch (var_.type_){
        case value_types::array_t:
            return var_.value_.array_value_->add(pos, std::move(value));
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    value_types::value_types_t type() const
    {
        return var_.type_;
    }

    void swap(basic_json<StringT,Alloc>& b)
    {
        var_.swap(b.var_);
    }

    template <class T>
    std::vector<T> as_vector() const
    {
        std::vector<T> v(size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = json_type_traits<value_type,T>::as(at(i));
        }
        return v;
    }

    friend void swap(basic_json<StringT,Alloc>& a, basic_json<StringT,Alloc>& b)
    {
        a.swap(b);
    }

    void assign_any(const typename basic_json<StringT,Alloc>::any& rhs)
    {
        var_.assign(rhs);
    }

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

    template <typename T>
    const T& any_cast() const
    {
        if (var_.type_ != value_types::any_t)
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad any cast");
        }
        return var_.value_.any_value_->template cast<T>();
    }
    template <typename T>
    T& any_cast() 
    {
        if (var_.type_ != value_types::any_t)
        {
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad any cast");
        }
        return var_.value_.any_value_->template cast<T>();
    }

    void assign_integer(int64_t rhs)
    {
        var_.assign(rhs);
    }

    void assign_uinteger(uint64_t rhs)
    {
        var_.assign(rhs);
    }

    void assign_double(double rhs)
    {
        var_.assign(rhs);
    }

    static basic_json make_2d_array(size_t m, size_t n);

    template <typename T>
    static basic_json make_2d_array(size_t m, size_t n, T val);

    static basic_json make_3d_array(size_t m, size_t n, size_t k);

    template <typename T>
    static basic_json make_3d_array(size_t m, size_t n, size_t k, T val);

#if !defined(JSONCONS_NO_DEPRECATED)
    typedef any json_any_type;
    typedef member_type name_value_pair;

    static const basic_json<StringT,Alloc> an_object;
    static const basic_json<StringT,Alloc> an_array;
    static const basic_json<StringT,Alloc> null;

    static basic_json parse(std::basic_istream<char_type>& is)
    {
        parse_stream(is);
    }
    static basic_json parse(std::basic_istream<char_type>& is, basic_parse_error_handler<char_type>& err_handler)
    {
        parse_stream(is,err_handler);
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

    template <typename T>
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

    const basic_json<StringT,Alloc>& get(const string_type& name) const
    {
        static const basic_json<StringT, Alloc> a_null = null_type();

        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return a_null;
        case value_types::object_t:
            {
                const_object_iterator it = var_.value_.object_value_->find(name);
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
            return static_cast<int>(var_.value_.float_value_);
        case value_types::integer_t:
            return static_cast<int>(var_.value_.integer_value_);
        case value_types::uinteger_t:
            return static_cast<int>(var_.value_.uinteger_value_);
        case value_types::bool_t:
            return var_.value_.bool_value_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an int");
        }
    }

    unsigned int as_uint() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<unsigned int>(var_.value_.float_value_);
        case value_types::integer_t:
            return static_cast<unsigned int>(var_.value_.integer_value_);
        case value_types::uinteger_t:
            return static_cast<unsigned int>(var_.value_.uinteger_value_);
        case value_types::bool_t:
            return var_.value_.bool_value_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned int");
        }
    }

    long as_long() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<long>(var_.value_.float_value_);
        case value_types::integer_t:
            return static_cast<long>(var_.value_.integer_value_);
        case value_types::uinteger_t:
            return static_cast<long>(var_.value_.uinteger_value_);
        case value_types::bool_t:
            return var_.value_.bool_value_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not a long");
        }
    }

    unsigned long as_ulong() const
    {
        switch (var_.type_)
        {
        case value_types::double_t:
            return static_cast<unsigned long>(var_.value_.float_value_);
        case value_types::integer_t:
            return static_cast<unsigned long>(var_.value_.integer_value_);
        case value_types::uinteger_t:
            return static_cast<unsigned long>(var_.value_.uinteger_value_);
        case value_types::bool_t:
            return var_.value_.bool_value_ ? 1 : 0;
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an unsigned long");
        }
    }

    void add(size_t index, const basic_json<StringT, Alloc>& value)
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            var_.value_.array_value_->add(index, value);
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Attempting to insert into a value that is not an array");
            }
        }
    }

    void add(size_t index, basic_json<StringT, Alloc>&& value){
        switch (var_.type_){
        case value_types::array_t:
            var_.value_.array_value_->add(index, std::move(value));
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
                const_object_iterator it = var_.value_.object_value_->find(name);
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
            var_.value_.array_value_->remove_range(from_index, to_index);
            break;
        default:
            break;
        }
    }
    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void remove(const string_type& name)
    {
        erase(name);
    }
    void remove_member(const string_type& name)
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
        return build_array<StringT,Alloc,size>()();
    }
    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n)
    {
        return build_array<StringT,Alloc,size>()(n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n, T val)
    {
        return build_array<StringT,Alloc,size>()(n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n)
    {
        return build_array<StringT,Alloc,size>()(m, n);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n, T val)
    {
        return build_array<StringT,Alloc,size>()(m, n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k)
    {
        return build_array<StringT,Alloc,size>()(m, n, k);
    }
    template<size_t size,typename T>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k, T val)
    {
        return build_array<StringT,Alloc,size>()(m, n, k, val);
    }
#endif

    object_range members()
    {
        static object empty;
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return object_range(empty);
        case value_types::object_t:
            return object_range(object_value());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    const_object_range members() const
    {
        static const object empty;
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return const_object_range(empty);
        case value_types::object_t:
            return const_object_range(object_value());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an object");
        }
    }

    array_range elements()
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            return array_range(array_value());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    const_array_range elements() const
    {
        switch (var_.type_)
        {
        case value_types::array_t:
            return const_array_range(array_value());
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an array");
        }
    }

    array& array_value() 
    {
		switch (var_.type_)
		{
        case value_types::array_t:
            return *(var_.value_.array_value_);
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
            return *(var_.value_.array_value_);
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
            var_.type_ = value_types::object_t;
            var_.value_.object_value_ = create_instance<object>(var_, object_allocator_type(var_));
            return *(var_.value_.object_value_);
        case value_types::object_t:
            return *(var_.value_.object_value_);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad object cast");
            break;
        }
    }

    const object& object_value() const
    {
        static const basic_json<StringT, Alloc> cobject = basic_json<StringT, Alloc>::object();
        switch (var_.type_)
        {
        case value_types::empty_object_t:
            return cobject.object_value();
        case value_types::object_t:
            return *(var_.value_.object_value_);
        default:
            JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad object cast");
            break;
        }
    }

private:

    template<typename StringT2, typename Allocator2, size_t size>
    class build_array
    {};
    template<typename StringT2, typename Allocator2>
    class build_array<StringT2,Allocator2,1>
    {
    public:
        basic_json<StringT2,Allocator2> operator() ()
        {
            return basic_json<StringT2,Allocator2>::make_array();
        }
        basic_json<StringT2,Allocator2> operator() (size_t n)
        {
            return basic_json<StringT2,Allocator2>::make_array(n);
        }
        template <typename T>
        basic_json<StringT2,Allocator2> operator() (size_t n, T val)
        {
            return basic_json<StringT2,Allocator2>::make_array(n, val);
        }
    };
    template<typename StringT2, typename Allocator2>
    class build_array<StringT2,Allocator2,2>
    {
    public:
        basic_json<StringT2,Allocator2> operator() (size_t m, size_t n)
        {
            return basic_json<StringT2,Allocator2>::make_2d_array(m, n);
        }
        template <typename T>
        basic_json<StringT2,Allocator2> operator() (size_t m, size_t n, T val)
        {
            return basic_json<StringT2,Allocator2>::make_2d_array(m, n, val);
        }
    };
    template<typename StringT2, typename Allocator2>
    class build_array<StringT2,Allocator2,3>
    {
    public:
        basic_json<StringT2,Allocator2> operator() (size_t m, size_t n, size_t k)
        {
            return basic_json<StringT2,Allocator2>::make_3d_array (m, n, k);
        }
        template <typename T>
        basic_json<StringT2,Allocator2> operator() (size_t m, size_t n, size_t k, T val)
        {
            return basic_json<StringT2,Allocator2>::make_3d_array (m, n, k, val);
        }
    };

    friend std::basic_ostream<typename StringT::value_type>& operator<<(std::basic_ostream<typename StringT::value_type>& os, const basic_json<StringT, Alloc>& o)
    {
        o.to_stream(os);
        return os;
    }

    friend std::basic_istream<typename StringT::value_type>& operator<<(std::basic_istream<typename StringT::value_type>& is, basic_json<StringT, Alloc>& o)
    {
        basic_json_deserializer<basic_json<StringT, Alloc>> handler;
        basic_json_reader<typename StringT::value_type> reader(is, handler);
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

template <class JsonT>
void swap(typename JsonT::member_type& a, typename JsonT::member_type& b)
{
    a.swap(b);
}

template<typename StringT, typename Alloc>
bool basic_json<StringT, Alloc>::operator!=(const basic_json<StringT, Alloc>& rhs) const
{
    return !(*this == rhs);
}

template<typename StringT, typename Alloc>
bool basic_json<StringT, Alloc>::operator==(const basic_json<StringT, Alloc>& rhs) const
{
    return var_ == rhs.var_;
}

#if !defined(JSONCONS_NO_DEPRECATED)
template<typename StringT, typename Alloc>
const basic_json<StringT, Alloc> basic_json<StringT, Alloc>::an_object = basic_json<StringT, Alloc>::object();
template<typename StringT, typename Alloc>
const basic_json<StringT, Alloc> basic_json<StringT, Alloc>::an_array = basic_json<StringT, Alloc>::array();        
template<typename StringT, typename Alloc>
const basic_json<StringT, Alloc> basic_json<StringT, Alloc>::null = basic_json<StringT, Alloc>(jsoncons::null_type());
#endif

template<typename StringT, typename Alloc>
basic_json<StringT, Alloc> basic_json<StringT, Alloc>::make_2d_array(size_t m, size_t n)
{
    basic_json<StringT, Alloc> a = basic_json<StringT, Alloc>::array();
    a.resize(m);
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<StringT, Alloc>::make_array(n);
    }
    return a;
}

template<typename StringT, typename Alloc>
template<typename T>
basic_json<StringT, Alloc> basic_json<StringT, Alloc>::make_2d_array(size_t m, size_t n, T val)
{
    basic_json<StringT, Alloc> v;
    v = val;
    basic_json<StringT, Alloc> a = make_array(m);
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<StringT, Alloc>::make_array(n, v);
    }
    return a;
}

template<typename StringT, typename Alloc>
basic_json<StringT, Alloc> basic_json<StringT, Alloc>::make_3d_array(size_t m, size_t n, size_t k)
{
    basic_json<StringT, Alloc> a = basic_json<StringT, Alloc>::array();
    a.resize(m);
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<StringT, Alloc>::make_2d_array(n, k);
    }
    return a;
}

template<typename StringT, typename Alloc>
template<typename T>
basic_json<StringT, Alloc> basic_json<StringT, Alloc>::make_3d_array(size_t m, size_t n, size_t k, T val)
{
    basic_json<StringT, Alloc> v;
    v = val;
    basic_json<StringT, Alloc> a = make_array(m);
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = basic_json<StringT, Alloc>::make_2d_array(n, k, v);
    }
    return a;
}

template<typename StringT, typename Alloc>
basic_json<StringT, Alloc> basic_json<StringT, Alloc>::parse_stream(std::basic_istream<char_type>& is)
{
    basic_json_deserializer<basic_json<StringT, Alloc>> handler;
    basic_json_reader<char_type> reader(is, handler);
    reader.read_next();
    reader.check_done();
    if (!handler.is_valid())
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
    }
    return handler.get_result();
}

template<typename StringT, typename Alloc>
basic_json<StringT, Alloc> basic_json<StringT, Alloc>::parse_stream(std::basic_istream<char_type>& is, 
                                                              basic_parse_error_handler<char_type>& err_handler)
{
    basic_json_deserializer<basic_json<StringT, Alloc>> handler;
    basic_json_reader<char_type> reader(is, handler, err_handler);
    reader.read_next();
    reader.check_done();
    if (!handler.is_valid())
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
    }
    return handler.get_result();
}

template<typename StringT, typename Alloc>
basic_json<StringT, Alloc> basic_json<StringT, Alloc>::parse_file(const std::string& filename)
{
    FILE* fp;

#if defined(JSONCONS_HAS_FOPEN_S)
    errno_t err = fopen_s(&fp, filename.c_str(), "rb");
    if (err != 0) 
    {
        JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Cannot open file %s", filename);
    }
#else
    fp = std::fopen(filename.c_str(), "rb");
    if (fp == nullptr)
    {
        JSONCONS_THROW_EXCEPTION_1(std::runtime_error,"Cannot open file %s", filename);
    }
#endif
    basic_json_deserializer<basic_json<StringT, Alloc>> handler;
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

            basic_json_parser<char_type> parser(handler);
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

template<typename StringT, typename Alloc>
basic_json<StringT, Alloc> basic_json<StringT, Alloc>::parse_file(const std::string& filename, 
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

    basic_json_deserializer<basic_json<StringT, Alloc>> handler;
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

template<typename StringT, typename Alloc>
typename basic_json<StringT, Alloc>::any& basic_json<StringT, Alloc>::any_value()
{
    switch (var_.type_)
    {
    case value_types::any_t:
        {
			return *var_.value_.any_value_;
        }
    default:
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an any value");
    }
}

template<typename StringT, typename Alloc>
const typename basic_json<StringT, Alloc>::any& basic_json<StringT, Alloc>::any_value() const
{
    switch (var_.type_)
    {
    case value_types::any_t:
        {
			return *var_.value_.any_value_;
        }
    default:
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Not an any value");
    }
}

template <typename JsonT>
std::basic_istream<typename JsonT::char_type>& operator>>(std::basic_istream<typename JsonT::char_type>& is, JsonT& o)
{
    basic_json_deserializer<JsonT> handler;
    basic_json_reader<typename JsonT::char_type> reader(is, handler);
    reader.read_next();
    reader.check_done();
    if (!handler.is_valid())
    {
        JSONCONS_THROW_EXCEPTION(std::runtime_error,"Failed to parse json stream");
    }
    o = handler.get_result();
    return is;
}

template<typename JsonT>
class json_printable
{
public:
    typedef typename JsonT::char_type char_type;

    json_printable(const JsonT& o,
                   bool is_pretty_print)
       : o_(&o), is_pretty_print_(is_pretty_print)
    {
    }

    json_printable(const JsonT& o,
                   bool is_pretty_print,
                   const basic_output_format<char_type>& format)
       : o_(&o), is_pretty_print_(is_pretty_print), format_(format)
    {
        ;
    }

    void to_stream(std::basic_ostream<char_type>& os) const
    {
        o_->to_stream(os, format_, is_pretty_print_);
    }

    friend std::basic_ostream<char_type>& operator<<(std::basic_ostream<char_type>& os, const json_printable<JsonT>& o)
    {
        o.to_stream(os);
        return os;
    }

    const JsonT *o_;
    bool is_pretty_print_;
    basic_output_format<char_type> format_;
private:
    json_printable();
};

template<typename JsonT>
json_printable<JsonT> print(const JsonT& val)
{
    return json_printable<JsonT>(val,false);
}

template<class JsonT>
json_printable<JsonT> print(const JsonT& val,
                            const basic_output_format<typename JsonT::char_type>& format)
{
    return json_printable<JsonT>(val, false, format);
}

template<class JsonT>
json_printable<JsonT> pretty_print(const JsonT& val)
{
    return json_printable<JsonT>(val,true);
}

template<typename JsonT>
json_printable<JsonT> pretty_print(const JsonT& val,
                                   const basic_output_format<typename JsonT::char_type>& format)
{
    return json_printable<JsonT>(val, true, format);
}

typedef basic_json<std::string,std::allocator<char>> json;
typedef basic_json<std::wstring,std::allocator<wchar_t>> wjson;

typedef basic_json_deserializer<json> json_deserializer;
typedef basic_json_deserializer<wjson> wjson_deserializer;

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif
