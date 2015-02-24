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
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_output_handler.hpp"
#include "jsoncons/output_format.hpp"

namespace jsoncons {

template <typename Char,class T> inline
void serialize(basic_json_output_handler<Char>& os, const T&)
{
    os.do_null_value();
}

template <typename Char>
class json_any_impl
{
public:
    virtual ~json_any_impl()
    {
    }

    virtual void* data() = 0;

    virtual const void* data() const = 0;

    virtual void to_stream(basic_json_output_handler<Char>& os) const = 0;

    virtual json_any_impl<Char>* clone() const = 0;
};

template <typename Char, class T>
class typed_json_any : public json_any_impl<Char>
{
public:
    typedef typed_json_any<Char,T>* Ptr;

    typed_json_any(T value)
        : data_(value)
    {
    }

    virtual void* data() 
    {
        return &data_;
    }

    virtual const void* data() const  
    {
        return &data_;
    }

    virtual json_any_impl<Char>* clone() const
    {
        return new typed_json_any<Char,T>(data_);
    }

    virtual void to_stream(basic_json_output_handler<Char>& os) const
    {
        serialize(os,data_);
    }

    T data_;
};

template <typename Char, class Alloc>
class basic_json;

template <typename Char,class Alloc>
class json_object_impl;

template <typename Char,class Alloc>
class json_array_impl;

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
    bool is(const basic_json<Char,Alloc>&) const
    {
        return false;
    }
    T as(const basic_json<Char,Alloc>& val) const;
    void assign(basic_json<Char,Alloc>& self, const T val);
};

namespace value_type
{
    enum value_type_t 
    {
        empty_object_t,
        object_t,
        array_t,
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
            : content_(nullptr)
        {
        }
        any(const any& val)
            : content_(nullptr)
        {
			content_ = val.content_->clone();
        }
        any(any&& val)
            : content_(val.content_)
        {
            val.content_ = nullptr;
        }

        template<typename T>
        explicit any(T val, typename std::enable_if<!std::is_same<any, typename std::decay<T>::type>::value,int>::type* = 0)
        {
    		content_ = new typed_json_any<Char,T>(val);
        }
    	~any()
    	{
    		delete content_;
    	}
        template <typename T>
        const T& cast() const
        {
            const T* p = (const T*)content_->data();
            return *p;
        }
        template <typename T>
        T& cast() 
        {
            T* p = (T*)content_->data();
            return *p;
        }

        any& operator=(any rhs)
        {
            std::swap(content_,rhs.content_);
            return *this;
        }

        void to_stream(basic_json_output_handler<Char>& os) const 
        {
            content_->to_stream(os);
        }

    	json_any_impl<Char>* content_;
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

    static const basic_json<Char,Alloc> an_object;
    static const basic_json<Char,Alloc> an_array;
    static const basic_json<Char,Alloc> null;
    
    typedef typename json_object_impl<Char,Alloc>::iterator object_iterator;
    typedef typename json_object_impl<Char,Alloc>::const_iterator const_object_iterator;

    typedef typename json_array_impl<Char,Alloc>::iterator array_iterator;
    typedef typename json_array_impl<Char,Alloc>::const_iterator const_array_iterator;

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

        value_type::value_type_t type() const
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

        bool is_custom() const
        {
            return val_.is_custom();
        }

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

        Char as_char() const
        {
            return val_.as_char();
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
        const T& custom_data() const
        {
            return val_.custom_data<T>();
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

        value_type::value_type_t type() const
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

        // Deprecated

        bool is_custom() const
        {
            return val_.at(name_).is_custom();
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

        Char as_char() const
        {
            return val_.at(name_).as_char();
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

        // Deprecated

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
        const T& custom_data() const
        {
            return val_.at(name_).template custom_data<T>();
        }
        // Returns a const reference to the custom data associated with name

        // Deprecated
        template <class T>
        T& custom_data() 
        {
            return val_.at(name_).template custom_data<T>();
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
            return val_ == val;
        }

        bool operator!=(const basic_json& val) const
        {
            return val_ != val;
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

        template <typename T>
        void set(const std::basic_string<Char>& name, T value)
        {
            val_.at(name_).set(name,value);
        }

        void set(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& value)
        {
            val_.at(name_).set(name,value);
        }

        void set(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& value)

        {
            val_.at(name_).set(name,value);
        }

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

        // Deprecated
        template <class T>
        void set_custom_data(const std::basic_string<Char>& name, T value)
        {
            val_.at(name_).set_custom_data(name,value);
        }

        // Deprecated
        template <class T>
        void add_custom_data(T value)
        {
            val_.at(name_).add_custom_data(value);
        }

        // Deprecated
        template <class T>
        void add_custom_data(size_t index, T value)
        {
            val_.at(name_).add_custom_data(index, value);
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

    static basic_json<Char,Alloc> make_float(double val)
    {
        basic_json<Char,Alloc> temp(value_type::double_t);
        temp.value_.float_value_ = val;
        return temp;
    }

    static basic_json<Char,Alloc> make_integer(long long val)
    {
        basic_json<Char,Alloc> temp(value_type::longlong_t);
        temp.value_.si_value_ = val;
        return temp;
    }

    static basic_json<Char,Alloc> make_unsigned(unsigned long long val)
    {
        basic_json<Char,Alloc> temp(value_type::ulonglong_t);
        temp.value_.ui_value_ = val;
        return temp;
    }

    static basic_json make_array()
    {
        return basic_json<Char, Alloc>(new json_array_impl<Char, Alloc>());
    }

    static basic_json make_array(size_t n)
    {
        return basic_json<Char, Alloc>(new json_array_impl<Char, Alloc>(n));
    }

    template <typename T>
    static basic_json make_array(size_t n, T val)
    {
        basic_json<Char, Alloc> v;
        v = val;
        return basic_json<Char, Alloc>(new json_array_impl<Char, Alloc>(n, v));
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

    explicit basic_json();

    basic_json(const basic_json& val);

    basic_json(basic_json&& val);

    explicit basic_json(any val);

    explicit basic_json(jsoncons::null_type);

    explicit basic_json(Char c);

    explicit basic_json(double val);

    explicit basic_json(int val);

    explicit basic_json(unsigned int val);

    explicit basic_json(long val);

    explicit basic_json(unsigned long val);

    explicit basic_json(long long val);

    explicit basic_json(unsigned long long val);

    explicit basic_json(const Char* val);

    explicit basic_json(const Char* val, size_t length);

    explicit basic_json(const std::basic_string<Char>& val);

    explicit basic_json(bool val);

    template <class InputIterator>
    basic_json(InputIterator name, InputIterator last);

    explicit basic_json(json_object_impl<Char,Alloc>* var);

    explicit basic_json(json_array_impl<Char,Alloc>* var);

    ~basic_json();

    object_iterator begin_members();

    const_object_iterator begin_members() const;

    object_iterator end_members();

    const_object_iterator end_members() const;

    array_iterator begin_elements();

    const_array_iterator begin_elements() const;

    array_iterator end_elements();

    const_array_iterator end_elements() const;

    template <class T>
    basic_json& operator=(T val);

    basic_json& operator=(basic_json<Char,Alloc> rhs);

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
        return type_ == value_type::null_t;
    }

    bool has_member(const std::basic_string<Char>& name) const;

    template<typename T>
    bool is() const
    {
        json_type_traits<Char,Alloc,T> adapter;
        return adapter.is(*this);
    }

    bool is_string() const
    {
        return type_ == value_type::string_t;
    }

    bool is_numeric() const
    {
        return type_ == value_type::double_t || type_ == value_type::longlong_t || type_ == value_type::ulonglong_t;
    }

    bool is_bool() const
    {
        return type_ == value_type::bool_t;
    }

    bool is_object() const
    {
        return type_ == value_type::object_t || type_ == value_type::empty_object_t;
    }

    bool is_array() const
    {
        return type_ == value_type::array_t;
    }

    bool is_any() const
    {
        return type_ == value_type::any_t;
    }

    bool is_longlong() const
    {
        return type_ == value_type::longlong_t;
    }

    bool is_ulonglong() const
    {
        return type_ == value_type::ulonglong_t;
    }

    bool is_double() const
    {
        return type_ == value_type::double_t;
    }

    // Deprecated

    bool is_custom() const
    {
        return type_ == value_type::any_t;
    }

    bool is_empty() const;

    size_t capacity() const;

    void reserve(size_t n);

    void resize_array(size_t n);

    template <typename T>
    void resize_array(size_t n, T val);

    template<typename T>
    T as() const
    {
        json_type_traits<Char,Alloc,T> adapter;
        return adapter.as(*this);
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

    template <class T>
    const T& custom_data() const;
    // Returns a const reference to the custom data associated with name

    // Deprecated
    template <class T>
    T& custom_data();
    // Returns a reference to the custom data associated with name

    std::basic_string<Char> as_string() const;

    std::basic_string<Char> as_string(const basic_output_format<Char>& format) const;

    Char as_char() const;

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

    template <typename T>
    void set(const std::basic_string<Char>& name, T value)
    {
        switch (type_)
        {
        case value_type::empty_object_t:
            type_ = value_type::object_t;
            value_.object_ = new json_object_impl<Char,Alloc>();
        case value_type::object_t:
            {
                json_type_traits<Char,Alloc,T> adapter;
                basic_json<Char,Alloc> o;
                adapter.assign(o,value);
                value_.object_->set(name,o);
            }
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION_1("Attempting to set %s on a value that is not an object",name);
            }
        }

    }

    void set(const std::basic_string<Char>& name, const basic_json<Char,Alloc>& value);

    void set(std::basic_string<Char>&& name, basic_json<Char,Alloc>&& value);

    template <typename T>
    void add(T val)
    {
        switch (type_)
        {
        case value_type::array_t:
            {
                json_type_traits<Char,Alloc,T> adapter;
                basic_json<Char,Alloc> a;
                adapter.assign(a,val);
                value_.array_->push_back(std::move(a));
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
        switch (type_)
        {
        case value_type::array_t:
            {
                json_type_traits<Char,Alloc,T> adapter;
                basic_json<Char,Alloc> a;
                adapter.assign(a,val);
                value_.array_->add(index, std::move(a));
            }
            break;
        default:
            {
                JSONCONS_THROW_EXCEPTION("Attempting to insert into a value that is not an array");
            }
        }
    }

    void add(basic_json<Char,Alloc>&& value);

    void add(size_t index, basic_json<Char,Alloc>&& value);

    void add(const basic_json<Char,Alloc>& value);

    void add(size_t index, const basic_json<Char,Alloc>& value);

    value_type::value_type_t type() const
    {
        return type_;
    }

    void to_stream(basic_json_output_handler<Char>& handler) const;

    void swap(basic_json<Char,Alloc>& b)
    {
        using std::swap;

        swap(type_,b.type_);
        swap(value_,b.value_);
    }

    template <class T>
    std::vector<T> as_vector() const
    {
        std::vector<T> v(size());
        json_type_traits<Char,Alloc,T> adapter;
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = adapter.as(at(i));
        }
        return v;
    }

    friend void swap(basic_json<Char,Alloc>& a, basic_json<Char,Alloc>& b)
    {
        a.swap(b);
    }

    void assign_any(const any& rhs);
    void assign_string(const std::basic_string<Char>& rhs);
    void assign_integer(long long rhs);
    void assign_unsigned(unsigned long long rhs);
    void assign_float(double rhs);
    void assign_bool(bool rhs);
    void assign_null();

    template <typename T>
    const T& any_cast() const
    {
        JSONCONS_ASSERT(type_ == value_type::any_t);
        const T* p = (const T*)value_.any_value_->content_->data();
        return *p;
    }
    template <typename T>
    T& any_cast() 
    {
        JSONCONS_ASSERT(type_ == value_type::any_t);
        T* p = (T*)value_.any_value_->content_->data();
        return *p;
    }


//  Deprecated
    void assign_double(double rhs);
    void assign_longlong(long long rhs);
    void assign_ulonglong(unsigned long long rhs);

    template <class T>
    void set_custom_data(const std::basic_string<Char>& name, T value);

    template <class T>
    void add_custom_data(T value);

    template <class T>
    void add_custom_data(size_t index, T value);

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

    struct string_data
    {
        size_t length;
        Char* p;
    };

private:
	basic_json(value_type::value_type_t t);

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

    string_data get_string_data() 
    {
        //return value_.string_wrapper_->s_;
        return value_.string_value_;
    }

    static void delete_string_env(const string_data* other)
    {
        //other->~string_data();
        ::operator delete((void*)other);
    }

    static string_data* create_string_env(const string_data* other)
    {
        size_t size = sizeof(string_data) + (other->length+1)*sizeof(Char);
        char* buffer = (char*)::operator new(size);
        string_data* env = new(buffer)string_data;
        env->length = other->length;
        env->p = new(buffer+sizeof(string_data))Char[other->length+1];
		memcpy(env->p,other->p,other->length*sizeof(Char));
		env->p[env->length] = 0;
        return env;
    }

    static string_data* create_string_env(const std::basic_string<Char>& s)
    {
        size_t size = sizeof(string_data) + (s.length()+1)*sizeof(Char);
        char* buffer = (char*)::operator new(size);
        string_data* env = new(buffer)string_data;
        env->length = s.length();
        env->p = new(buffer+sizeof(string_data))Char[s.length()+1];
        memcpy(env->p,s.c_str(),s.length()*sizeof(Char));
        env->p[env->length] = 0;
        return env;
    }

    static string_data* create_string_env(const Char* p)
    {
        return create_string_env(p,std::char_traits<Char>::length(p));
    }

    static string_data* create_string_env(const Char* p, size_t length)
    {
        size_t size = sizeof(string_data) + (length+1)*sizeof(Char);
        char* buffer = (char*)::operator new(size);
        string_data* env = new(buffer)string_data;
        env->length = length;
        env->p = new(buffer+sizeof(string_data))Char[length+1];
        memcpy(env->p,p,length*sizeof(Char));
        env->p[env->length] = 0;
        return env;
    }

    static string_data* create_string_env()
    {
        size_t size = sizeof(string_data) + sizeof(Char);
        char* buffer = (char*)::operator new(size);
        string_data* env = new(buffer)string_data;
        env->length = 0;
        env->p = new(buffer+sizeof(string_data))Char[1];
        env->p[0] = 0;
        return env;
    }

    static string_data* create_string_env(Char c)
    {
        return create_string_env(&c,1);
    }

    value_type::value_type_t type_;
    union
    {
        double float_value_;
        long long si_value_;
        unsigned long long ui_value_;
        bool bool_value_;
        json_object_impl<Char,Alloc>* object_;
        json_array_impl<Char,Alloc>* array_;
        any* any_value_;
        string_data* string_value_;
    } value_;
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
