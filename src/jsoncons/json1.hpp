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
#include "jsoncons/json_exception.hpp"
#include "jsoncons/json_output_handler.hpp"
#include "jsoncons/output_format.hpp"

namespace jsoncons {

template <class C,class T> inline
void serialize(basic_json_output_handler<C>& os, const T& val)
{
    os.null_value();
}

template <class C>
class basic_custom_data
{
public:
    virtual ~basic_custom_data()
    {
    }

    virtual void to_stream(basic_json_output_handler<C>& os) const = 0;

    virtual basic_custom_data<C>* clone() const = 0;
};

template <class C, class T>
class custom_data_wrapper : public basic_custom_data<C>
{
public:
    typedef custom_data_wrapper<C,T>* Ptr;

    custom_data_wrapper(const T& value)
        : data1_(value)
    {
    }
    virtual basic_custom_data<C>* clone() const
    {
        return new custom_data_wrapper<C,T>(data1_);
    }

    virtual void to_stream(basic_json_output_handler<C>& os) const
    {
        jsoncons::serialize(os,data1_);
    }

    T data1_;
};

template <class C>
class basic_json;

template <class C>
class json_object;

template <class C>
class json_array;

template <class C>
class basic_output_format;

template <class C>
std::basic_string<C> escape_string(const std::basic_string<C>& s, const basic_output_format<C>& format);

template <typename C, typename T>
class value_adapter
{
public:
    bool is(const basic_json<C>& val) const
    {
        return false;
    }
    T as(const basic_json<C>& val) const;
};

class json_base
{
public:
    enum value_type {empty_object_t,object_t,array_t,string_t,double_t,longlong_t,ulonglong_t,bool_t,null_t,custom_t};
};

template <class C>
class basic_json : public json_base
{
public:
    class null_type;
    class object;
    class array;
    struct custom_type {};

    class name_value_pair
    {
    public:
        name_value_pair()
        {
        }
        name_value_pair(const name_value_pair& pair)
            : name_(pair.name_), value_(pair.value_)
        {
        }
        name_value_pair(name_value_pair&& pair)
            //: name_(std::move(pair.name_)), value_(std::move(pair.value_))
        {
            name_.swap(pair.name_);
            value_.swap(pair.value_);
        }
        name_value_pair(const std::basic_string<C>& nam, const basic_json<C>& val)
            : name_(nam), value_(val)
        {
        }
        name_value_pair(std::basic_string<C>&& nam, basic_json<C>&& val)
            : name_(nam), value_(val)
        {
        }

        const std::basic_string<C>& name() const
        {
            return name_;
        }

        basic_json<C>& value()
        {
            return value_;
        }

        const basic_json<C>& value() const
        {
            return value_;
        }

        name_value_pair& operator=(name_value_pair rhs)
        {
            swap(rhs);
            return *this;
        }

        void swap(name_value_pair& pair)
        {
            name_.swap(pair.name_);
            value_.swap(pair.value_);
        }

        std::basic_string<C> name_;
        basic_json<C> value_;
    };

    static const basic_json<C> an_object;
    static const basic_json<C> an_array;
    static const basic_json<C> null;
    
    typedef typename json_object<C>::iterator object_iterator;
    typedef typename json_object<C>::const_iterator const_object_iterator;

    typedef typename json_array<C>::iterator array_iterator;
    typedef typename json_array<C>::const_iterator const_array_iterator;

    operator null_type() const
    {
        JSONCONS_ASSERT(type_ == null_t);
        return null_type();
    }

    operator array() const
    {
        JSONCONS_ASSERT(type_ == array_t);
        return array(value_.array_->clone());
    }

    operator object() const
    {
        JSONCONS_ASSERT(type_ == object_t);
        return object(value_.object_->clone());
    }

    class const_val_proxy 
    {
    public:
        friend class basic_json<C>;

        size_t size() const
        {
            return val_.size();
        }

        value_type type() const
        {
            return val_.type();
        }

        bool has_member(const std::basic_string<C>& name) const
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

        bool is_custom() const
        {
            return val_.is_custom();
        }

        std::basic_string<C> as_string() const
        {
            return val_.as_string();
        }

        std::basic_string<C> as_string(const basic_output_format<C>& format) const
        {
            return val_.as_string(format);
        }

        template<typename T>
        T as() const
        {
            return val_.template as<T>();
        }

        C as_char() const
        {
            return val_.as_char();
        }

        bool as_bool() const
        {
            return val_.as_bool();
        }

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
        std::vector<T> as_vector() const
        {
            return val_.as_vector<T>();
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

        bool operator==(const basic_json<C>& val) const
        {
            return val_ == val;
        }

        bool operator!=(const basic_json& val) const
        {
            return val_ != val;
        }

        const basic_json<C>& operator[](size_t i) const
        {
            return val_[i];
        }

        const basic_json<C>& operator[](const std::basic_string<C>& name) const
        {
            return val_.at(name);
        }

        const basic_json<C>& at(const std::basic_string<C>& name) const
        {
            return val_.at(name);
        }

        const basic_json<C>& get(const std::basic_string<C>& name) const
        {
            return val_.get(name);
        }

        const_val_proxy get(const std::basic_string<C>& name, const basic_json<C>& default_val) const
        {
            return val_.get(name,default_val);
        }

        std::basic_string<C> to_string() const
        {
            return val_.to_string();
        }

        std::basic_string<C> to_string(const basic_output_format<C>& format) const
        {
            return val_.to_string(format);
        }

        void to_stream(std::basic_ostream<C>& os) const
        {
            val_.to_stream(os);
        }

        void to_stream(std::basic_ostream<C>& os, const basic_output_format<C>& format) const
        {
            val_.to_stream(os,format);
        }

        void to_stream(std::basic_ostream<C>& os, const basic_output_format<C>& format, bool indenting) const
        {
            val_.to_stream(os,format,indenting);
        }

        friend std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, const const_val_proxy& o)
        {
            o.to_stream(os);
            return os;
        }

    private:
        const_val_proxy(); // nopop
        const_val_proxy& operator = (const const_val_proxy& other); // noop

        const_val_proxy(const basic_json<C>& val)
            : val_(val)
        {
        }

        const basic_json<C>& val_;
    };

    class object_key_proxy 
    {
    public:
        friend class basic_json<C>;

        size_t size() const
        {
            return val_.at(name_).size();
        }

        value_type type() const
        {
            return val_.at(name_).type();
        }

        bool has_member(const std::basic_string<C>& name) const
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

        void resize_array(size_t n, const basic_json<C>& val)
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
 
        bool is_custom() const
        {
            return val_.at(name_).is_custom();
        }

        std::basic_string<C> as_string() const
        {
            return val_.at(name_).as_string();
        }

        std::basic_string<C> as_string(const basic_output_format<C>& format) const
        {
            return val_.at(name_).as_string(format);
        }

        template<typename T>
        T as() const
        {
            return val_.at(name_).template as<T>();
        }

        C as_char() const
        {
            return val_.at(name_).as_char();
        }

        bool as_bool() const
        {
            return val_.at(name_).as_bool();
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
        std::vector<T> as_vector() const
        {
            return val_.at(name_).template as_vector<T>();
        }

        template <class T>
        const T& custom_data() const
        {
            return val_.at(name_).template custom_data<T>();
        }
        // Returns a const reference to the custom data associated with name

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

        basic_json<C>& operator[](size_t i)
        {
            return val_.at(name_)[i];
        }

        const basic_json<C>& operator[](size_t i) const
        {
            return val_.at(name_)[i];
        }

        object_key_proxy operator[](const std::basic_string<C>& name)
        {
            return object_key_proxy(val_.at(name_),name);
        }

        const basic_json<C>& operator[](const std::basic_string<C>& name) const
        {
            return val_.at(name_).at(name);
        }

        basic_json<C>& at(const std::basic_string<C>& name)
        {
            return val_.at(name_).at(name);
        }

        const basic_json<C>& at(const std::basic_string<C>& name) const
        {
            return val_.at(name_).at(name);
        }

        const basic_json<C>& get(const std::basic_string<C>& name) const
        {
            return val_.at(name_).get(name);
        }

        const_val_proxy get(const std::basic_string<C>& name, const basic_json<C>& default_val) const
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

        void remove_member(const std::basic_string<C>& name)
        {
            val_.at(name_).remove_member(name);
        }
        // Remove a member from an object 

        void set(const std::basic_string<C>& name, const basic_json<C>& value)
        {
            val_.at(name_).set(name,value);
        }

        void set(std::basic_string<C>&& name, basic_json<C>&& value)

        {
            val_.at(name_).set(name,value);
        }

        void add(basic_json<C>&& value)
        {
            val_.at(name_).add(value);
        }

        void add(size_t index, basic_json<C>&& value)
        {
            val_.at(name_).add(index, value);
        }

        template <class T>
        void set_custom_data(const std::basic_string<C>& name, const T& value)
        {
            val_.at(name_).set_custom_data(name,value);
        }

        void add(const basic_json<C>& value)
        {
            val_.at(name_).add(value);
        }

        void add(size_t index, const basic_json<C>& value)
        {
            val_.at(name_).add(index, value);
        }

        template <class T>
        void add_custom_data(const T& value)
        {
            val_.at(name_).add_custom_data(value);
        }

        template <class T>
        void add_custom_data(size_t index, const T& value)
        {
            val_.at(name_).add_custom_data(index, value);
        }

        std::basic_string<C> to_string() const
        {
            return val_.at(name_).to_string();
        }

        std::basic_string<C> to_string(const basic_output_format<C>& format) const
        {
            return val_.at(name_).to_string(format);
        }

        void to_stream(std::basic_ostream<C>& os) const
        {
            val_.at(name_).to_stream(os);
        }

        void to_stream(std::basic_ostream<C>& os, const basic_output_format<C>& format) const
        {
            val_.at(name_).to_stream(os,format);
        }

        void to_stream(std::basic_ostream<C>& os, const basic_output_format<C>& format, bool indenting) const
        {
            val_.at(name_).to_stream(os,format,indenting);
        }

        void swap(basic_json<C>& val)
        {
            val_.swap(val);
        }

        friend std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, const object_key_proxy& o)
        {
            o.to_stream(os);
            return os;
        }

    private:
        object_key_proxy(); // nopop
        object_key_proxy& operator = (const object_key_proxy& other); // noop

        object_key_proxy(basic_json<C>& var, 
              const std::basic_string<C>& name)
            : val_(var), name_(name)
        {
        }

        basic_json<C>& val_;

        const std::basic_string<C>& name_;
    };

    static basic_json parse(std::basic_istream<C>& is);

    static basic_json parse_string(const std::basic_string<C>& s);

    static basic_json parse_file(const std::string& s);

    template<int size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array()
    {
        return build_array<C,size>()();
    }
    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n)
    {
        return build_array<C,size>()(n);
    }
    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n, const basic_json<C>& val)
    {
        return build_array<C,size>()(n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n)
    {
        return build_array<C,size>()(m, n);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n, const basic_json<C>& val)
    {
        return build_array<C,size>()(m, n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k)
    {
        return build_array<C,size>()(m, n, k);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k, const basic_json<C>& val)
    {
        return build_array<C,size>()(m, n, k, val);
    }

    static basic_json make_array();

    static basic_json make_array(size_t n);

    static basic_json make_array(size_t n, const basic_json<C>& val);

    static basic_json make_2d_array(size_t m, size_t n);

    static basic_json make_2d_array(size_t m, size_t n, const basic_json<C>& val);

    static basic_json make_3d_array(size_t m, size_t n, size_t k);

    static basic_json make_3d_array(size_t m, size_t n, size_t k, const basic_json<C>& val);

    explicit basic_json();

    basic_json(const basic_json& val);

    basic_json(double val);

    basic_json(int val);

    basic_json(unsigned int val);

    basic_json(long val);

    basic_json(unsigned long val);

    basic_json(long long val);

    basic_json(unsigned long long val);

    basic_json(const C* val);

    basic_json(const std::basic_string<C>& val);

    basic_json(bool val);

    template <class InputIterator>
    basic_json(InputIterator name, InputIterator last);

    explicit basic_json(json_object<C>* var);

    explicit basic_json(json_array<C>* var);

    explicit basic_json(basic_custom_data<C>* var);

    ~basic_json();

    object_iterator begin_members();

    const_object_iterator begin_members() const;

    object_iterator end_members();

    const_object_iterator end_members() const;

    array_iterator begin_elements();

    const_array_iterator begin_elements() const;

    array_iterator end_elements();

    const_array_iterator end_elements() const;

    basic_json& operator=(const char* rhs);

    basic_json& operator=(const std::basic_string<C>& rhs);

    basic_json& operator=(bool rhs);

    basic_json& operator=(int rhs);

    basic_json& operator=(unsigned int rhs);

    basic_json& operator=(long rhs);

    basic_json& operator=(unsigned long rhs);

    basic_json& operator=(long long rhs);

    basic_json& operator=(unsigned long long rhs);

    basic_json& operator=(double rhs);

    basic_json& operator=(basic_json<C> rhs);

    bool operator!=(const basic_json<C>& rhs) const;

    bool operator==(const basic_json<C>& rhs) const;

    size_t size() const; 

    basic_json<C>& operator[](size_t i);

    const basic_json<C>& operator[](size_t i) const;

    object_key_proxy operator[](const std::basic_string<C>& name);

    const basic_json<C>& operator[](const std::basic_string<C>& name) const;

    std::basic_string<C> to_string() const;

    std::basic_string<C> to_string(const basic_output_format<C>& format) const;

    void to_stream(std::basic_ostream<C>& os) const;

    void to_stream(std::basic_ostream<C>& os, const basic_output_format<C>& format) const;

    void to_stream(std::basic_ostream<C>& os, const basic_output_format<C>& format, bool indenting) const;

    bool is_null() const
    {
        return type_ == null_t;
    }

    bool has_member(const std::basic_string<C>& name) const;

    template<typename T>
    bool is() const
    {
        value_adapter<C,T> adapter;
        return adapter.is(*this);
    }

    bool is_string() const
    {
        return type_ == string_t;
    }

    bool is_numeric() const
    {
        return type_ == double_t || type_ == longlong_t || type_ == ulonglong_t;
    }

    bool is_longlong() const
    {
        return type_ == longlong_t;
    }

    bool is_ulonglong() const
    {
        return type_ == ulonglong_t;
    }

    bool is_double() const
    {
        return type_ == double_t;
    }

    bool is_bool() const
    {
        return type_ == bool_t;
    }

    bool is_object() const
    {
        return type_ == object_t || type_ == empty_object_t;
    }

    bool is_array() const
    {
        return type_ == array_t;
    }

    bool is_custom() const
    {
        return type_ == custom_t;
    }

    bool is_empty() const;

    size_t capacity() const;

    void reserve(size_t n);

    void resize_array(size_t n);

    void resize_array(size_t n, const basic_json<C>& val);

    template<typename T>
    T as() const
    {
        value_adapter<C,T> adapter;
        return adapter.as(*this);
    }

    bool as_bool() const;

    double as_double() const;

    int as_int() const;

    unsigned int as_uint() const;

    long as_long() const;

    unsigned long as_ulong() const;

    long long as_longlong() const;

    unsigned long long as_ulonglong() const;

    template <class T>
    const T& custom_data() const;
    // Returns a const reference to the custom data associated with name

    template <class T>
    T& custom_data();
    // Returns a reference to the custom data associated with name

    std::basic_string<C> as_string() const;

    std::basic_string<C> as_string(const basic_output_format<C>& format) const;

    C as_char() const;

    basic_json<C>& at(const std::basic_string<C>& name);
    const basic_json<C>& at(const std::basic_string<C>& name) const;

    basic_json<C>& at(size_t i);
    const basic_json<C>& at(size_t i) const;

    const basic_json<C>& get(const std::basic_string<C>& name) const;

    const_val_proxy get(const std::basic_string<C>& name, const basic_json<C>& default_val) const;

    // Modifiers

    void clear();
    // Remove all elements from an array or object

    void remove_range(size_t from_index, size_t to_index);
    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void remove_member(const std::basic_string<C>& name);
    // Removes a member from an object value

    void set(const std::basic_string<C>& name, const basic_json<C>& value);

    basic_json(basic_json&& val);

    void set(std::basic_string<C>&& name, basic_json<C>&& value);

    void add(basic_json<C>&& value);

    void add(size_t index, basic_json<C>&& value);

    template <class T>
    void set_custom_data(const std::basic_string<C>& name, const T& value);

    void add(const basic_json<C>& value);

    void add(size_t index, const basic_json<C>& value);

    template <class T>
    void add_custom_data(const T& value);

    template <class T>
    void add_custom_data(size_t index, const T& value);

    value_type type() const
    {
        return type_;
    }

    void to_stream(basic_json_output_handler<C>& handler) const;

    void swap(basic_json<C>& b)
    {
        using std::swap;

        swap(type_,b.type_);
        swap(value_,b.value_);
    }

    template <class T>
    std::vector<T> as_vector() const
    {
        std::vector<T> v(size());
        value_adapter<C,T> adapter;
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = adapter.as(at(i));
        }
        return v;
    }

    friend void swap(basic_json<C>& a, basic_json<C>& b)
    {
        a.swap(b);
    }

//  Deprecated
    bool is_number() const
    {
        return is_numeric();
    }
private:
	basic_json(value_type t);

    template<typename Char, size_t size>
    class build_array
    {};
    template<typename Char>
    class build_array<Char,1>
    {
    public:
        basic_json<Char> operator() ()
        {
            return basic_json<Char>::make_array();
        }
        basic_json<Char> operator() (size_t n)
        {
            return basic_json<Char>::make_array(n);
        }
        basic_json<Char> operator() (size_t n, const basic_json<Char>& val)
        {
            return basic_json<Char>::make_array(n, val);
        }
    };
    template<typename Char>
    class build_array<Char,2>
    {
    public:
        basic_json<Char> operator() (size_t m, size_t n)
        {
            return basic_json<Char>::make_2d_array(m, n);
        }
        basic_json<Char> operator() (size_t m, size_t n, const basic_json<Char>& val)
        {
            return basic_json<Char>::make_2d_array(m, n, val);
        }
    };
    template<typename Char>
    class build_array<Char,3>
    {
    public:
        basic_json<Char> operator() (size_t m, size_t n, size_t k)
        {
            return basic_json<Char>::make_3d_array (m, n, k);
        }
        basic_json<Char> operator() (size_t m, size_t n, size_t k, const basic_json<Char>& val)
        {
            return basic_json<Char>::make_3d_array (m, n, k, val);
        }
    };

	value_type type_;
    union
    {
        double double_value_;
        long long longlong_value_;
        unsigned long long ulonglong_value_;
        bool bool_value_;
        json_object<C>* object_;
        json_array<C>* array_;
        std::basic_string<C>* string_value_;
        basic_custom_data<C>* userdata_;
    } value_;
};

template <class C>
void swap(typename basic_json<C>::name_value_pair& a, typename basic_json<C>::name_value_pair& b)
{
    a.swap(b);
}

template <typename C>
class value_adapter<C,std::basic_string<C>>
{
public:
    bool is(const basic_json<C>& val) const
    {
        return val.is_string();
    }
    std::basic_string<C> as(const basic_json<C>& val) const
    {
        return val.as_string();
    }
};

template <typename C>
class value_adapter<C,typename basic_json<C>::object>
{
public:
    bool is(const basic_json<C>& val) const
    {
        return val.is_object();
    }
    typename basic_json<C>::object as(const basic_json<C>& val) const
    {
        return val;
    }
};

template <typename C>
class value_adapter<C,typename basic_json<C>::array>
{
public:
    bool is(const basic_json<C>& val) const
    {
        return val.is_array();
    }
    typename basic_json<C>::array as(const basic_json<C>& val) const
    {
        return val;
    }
};

template <typename C>
class value_adapter<C,typename basic_json<C>::null_type>
{
public:
    bool is(const basic_json<C>& val) const
    {
        return val.is_null();
    }
    typename basic_json<C>::null_type as(const basic_json<C>& val) const
    {
        JSONCONS_ASSERT(val.is_null());
        return basic_json<C>::null_type();
    }
};

template <typename C>
class value_adapter<C,bool>
{
public:
    bool is(const basic_json<C>& val) const
    {
        return val.is_bool();
    }
    bool as(const basic_json<C>& val) const
    {
        return val.as_bool();
    }
};

template <typename C>
class value_adapter<C,int>
{
public:
    bool is(const basic_json<C>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= std::numeric_limits<int>::min JSONCONS_NO_MACRO_EXP() && val.as_longlong() <= std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    int as(const basic_json<C>& val) const
    {
        return val.as_int();
    }
};

template <typename C>
class value_adapter<C,unsigned int>
{
public:
    bool is(const basic_json<C>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && val.as_longlong() <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    unsigned int as(const basic_json<C>& val) const
    {
        return val.as_uint();
    }
};

template <typename C>
class value_adapter<C,short>
{
public:
    bool is(const basic_json<C>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= std::numeric_limits<short>::min JSONCONS_NO_MACRO_EXP() && val.as_longlong() <= std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    short as(const basic_json<C>& val) const
    {
        return (short)val.as_int();
    }
};

template <typename C>
class value_adapter<C,unsigned short>
{
public:
    bool is(const basic_json<C>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && val.as_longlong() <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    unsigned short as(const basic_json<C>& val) const
    {
        return (unsigned short)val.as_uint();
    }
};

template <typename C>
class value_adapter<C,long>
{
public:
    bool is(const basic_json<C>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= std::numeric_limits<long>::min JSONCONS_NO_MACRO_EXP() && val.as_longlong() <= std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    long as(const basic_json<C>& val) const
    {
        return val.as_long();
    }
};

template <typename C>
class value_adapter<C,unsigned long>
{
public:
    bool is(const basic_json<C>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0 && val.as_longlong() <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP();
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    unsigned long as(const basic_json<C>& val) const
    {
        return val.as_ulong();
    }
};

template <typename C>
class value_adapter<C,char>
{
public:
    char as(const basic_json<C>& val) const
    {
        return val.as_char();
    }
};

template <typename C>
class value_adapter<C,double>
{
public:
    bool is(const basic_json<C>& val) const
    {
        return val.is_double();
    }

    double as(const basic_json<C>& val) const
    {
        return val.as_double();
    }
};

template <typename C>
class value_adapter<C,long long>
{
public:
    bool is(const basic_json<C>& val) const
    {
        if (val.is_longlong())
        {
            return true;
        }
        else if (val.is_ulonglong())
        {
            return val.as_ulonglong() <= std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP();
        }
        else
        {
            return false;
        }
    }
    long long as(const basic_json<C>& val) const
    {
        return val.as_longlong();
    }
};

template <typename C>
class value_adapter<C,unsigned long long>
{
public:
    bool is(const basic_json<C>& val) const
    {
        if (val.is_longlong())
        {
            return val.as_longlong() >= 0;
        }
        else if (val.is_ulonglong())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    unsigned long long as(const basic_json<C>& val) const
    {
        return val.as_ulonglong();
    }
};

template <typename C>
class basic_json<C>::null_type : public basic_json<C> 
{
private:
    null_type()
        : basic_json<C>(null_t)
    {
    }
};

template <typename C>
class basic_json<C>::object : public basic_json<C>
{
public:
    object(json_object<C>* impl)
        : basic_json<C>(impl)
    {
    }
//private:
    object()
        : basic_json<C>(object_t)
    {
    }
};

template <typename C>
class basic_json<C>::array : public basic_json<C>
{
public:
    array(size_t n)
        : basic_json<C>(array_t)
    {
        value_.array_->resize(n);
    }
    array(size_t n, const basic_json<C>& val)
        : basic_json<C>(array_t)
    {
        value_.array_->resize(n,val);
    }
    array(json_array<C>* impl)
        : basic_json<C>(impl)
    {
    }
private:
    array()
        : basic_json<C>(array_t)
    {
    }
};

}

#endif
