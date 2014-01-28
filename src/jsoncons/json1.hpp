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
class json_object;

template <class C>
class json_array;

template <class C>
class basic_output_format;

template <class C>
std::basic_string<C> escape_string(const std::basic_string<C>& s, const basic_output_format<C>& format);

class json_base
{
public:
    enum value_type {empty_object_t,object_t,array_t,string_t,double_t,longlong_t,ulonglong_t,bool_t,null_t,custom_t};
    struct object_type {};
    struct array_type {};
    struct null_type {};
    struct custom_type {};
};

template <class C>
class basic_json : public json_base
{
public:
    static const basic_json<C> an_object;
    static const basic_json<C> an_array;
    static const basic_json<C> null;
    
    typedef typename json_object<C>::iterator object_iterator;
    typedef typename json_object<C>::const_iterator const_object_iterator;

    typedef typename json_array<C>::iterator array_iterator;
    typedef typename json_array<C>::const_iterator const_array_iterator;

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

#ifndef JSONCONS_NO_CXX11_RVALUE_REFERENCES
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
#endif
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
    basic_json(InputIterator first, InputIterator last);

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
        return is_type(T());
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
        return as_value<C,T>(*this);
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

#ifndef JSONCONS_NO_CXX11_RVALUE_REFERENCES

    basic_json(basic_json&& val);

    void set(std::basic_string<C>&& name, basic_json<C>&& value);

    void add(basic_json<C>&& value);

    void add(size_t index, basic_json<C>&& value);
#endif

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
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = as_value<C,T>(at(i)).get();
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

    template <class T>
    bool is_type(T) const
    {
        return false;
    }

    bool is_type(json_base::null_type) const
    {
        return type_ == null_t;
    }

    bool is_type(json_base::object_type) const
    {
        return type_ == object_t || type_ == empty_object_t;
    }

    bool is_type(json_base::array_type) const
    {
        return type_ == array_t;
    }

    bool is_type(bool) const
    {
        return type_ == bool_t;
    }

    bool is_type(double) const
    {
        return type_ == double_t;
    }

    bool is_type(float) const
    {
        return type_ == double_t && value_.double_value >= -std::numeric_limits<float>::max JSONCONS_NO_MACRO_EXPANSION() && value_.double_value <= std::numeric_limits<float>::max JSONCONS_NO_MACRO_EXPANSION();
    }

    bool is_type(std::basic_string<C>) const
    {
        return type_ == string_t;
    }

    bool is_type(short) const
    {
        switch (type_)
        {
        case longlong_t:
            return value_.longlong_value_ >= std::numeric_limits<short>::min JSONCONS_NO_MACRO_EXPANSION() && value_.longlong_value_ <= std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXPANSION();
        case ulonglong_t:
            return value_.ulonglong_value_ <= std::numeric_limits<short>::max JSONCONS_NO_MACRO_EXPANSION();
        default:
            return false;
        }
    }

    bool is_type(unsigned short) const
    {
        switch (type_)
        {
        case longlong_t:
            return value_.longlong_value_ >= 0 && value_.longlong_value_ <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXPANSION();
        case ulonglong_t:
            return value_.ulonglong_value_ <= std::numeric_limits<unsigned short>::max JSONCONS_NO_MACRO_EXPANSION();
        default:
            return false;
        }
    }

    bool is_type(int) const
    {
        switch (type_)
        {
        case longlong_t:
            return value_.longlong_value_ >= std::numeric_limits<int>::min JSONCONS_NO_MACRO_EXPANSION() && value_.longlong_value_ <= std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXPANSION();
        case ulonglong_t:
            return value_.ulonglong_value_ <= std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXPANSION();
        default:
            return false;
        }
    }

    bool is_type(unsigned int) const
    {
        switch (type_)
        {
        case longlong_t:
            return value_.longlong_value_ >= 0 && value_.longlong_value_ <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXPANSION();
        case ulonglong_t:
            return value_.ulonglong_value_ <= std::numeric_limits<unsigned int>::max JSONCONS_NO_MACRO_EXPANSION();
        default:
            return false;
        }
    }

    bool is_type(long) const
    {
        switch (type_)
        {
        case longlong_t:
            return value_.longlong_value_ >= std::numeric_limits<long>::min JSONCONS_NO_MACRO_EXPANSION() && value_.longlong_value_ <= std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXPANSION();
        case ulonglong_t:
            return value_.ulonglong_value_ <= std::numeric_limits<long>::max JSONCONS_NO_MACRO_EXPANSION();
        default:
            return false;
        }
    }

    bool is_type(unsigned long) const
    {
        switch (type_)
        {
        case longlong_t:
            return value_.longlong_value_ >= 0 && value_.longlong_value_ <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXPANSION();
        case ulonglong_t:
            return value_.ulonglong_value_ <= std::numeric_limits<unsigned long>::max JSONCONS_NO_MACRO_EXPANSION();
        default:
            return false;
        }
    }

    bool is_type(long long) const
    {
        switch (type_)
        {
        case longlong_t:
            return true;
        case ulonglong_t:
            return value_.ulonglong_value_ <= std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXPANSION();
        default:
            return false;
        }
    }

    bool is_type(unsigned long long) const
    {
        switch (type_)
        {
        case longlong_t:
            return value_.longlong_value_ >= 0;
        case ulonglong_t:
            return true;
        default:
            return false;
        }
    }

    bool is_type(json_base::custom_type) const
    {
        return type_ == custom_t;
    }

    template<typename C, typename T>
    class as_value
    {
    public:
        as_value (const basic_json<C>& value) : value_(value)
        {}

        T get () const
        {
            return static_cast<T>(*this);
        }

        operator object_type () const
        {
            return *this;
        }

        operator array_type () const
        {
            return *this;
        }
        
        operator std::basic_string<C> () const
        {
            return value_.as_string();
        }
        operator bool () const
        {
            return value_.as_bool();
        }
        operator char () const
        {
            return value_.as_char();
        }
        operator double () const
        {
            return value_.as_double();
        }
        operator int () const
        {
            return value_.as_int();
        }
        operator short () const
        {
            return static_cast<short>(value_.as_longlong());
        }
        operator unsigned short () const
        {
            return static_cast<unsigned short>(value_.as_ulonglong());
        }
        operator unsigned int () const
        {
            return value_.as_uint();
        }
        operator long () const
        {
            return value_.as_long();
        }
        operator unsigned long () const
        {
            return value_.as_ulong();
        }
        operator long long () const
        {
            return value_.as_longlong();
        }
        operator unsigned long long () const
        {
            return value_.as_ulonglong();
        }

    private:
        const basic_json<C>& value_;
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


}

#endif
