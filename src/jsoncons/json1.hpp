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
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_output_handler.hpp"
#include "jsoncons/output_format.hpp"

namespace jsoncons {

template <typename Char>
struct storage
{
};

template <typename Char,class T> inline
void serialize(basic_json_output_handler<Char>& os, const T& val)
{
    os.null_value();
}

template <typename Char>
class basic_custom_data
{
public:
    virtual ~basic_custom_data()
    {
    }

    virtual void to_stream(basic_json_output_handler<Char>& os) const = 0;

    virtual basic_custom_data<Char>* clone() const = 0;
};

template <typename Char, class T>
class custom_data_wrapper : public basic_custom_data<Char>
{
public:
    typedef custom_data_wrapper<Char,T>* Ptr;

    custom_data_wrapper(const T& value)
        : data1_(value)
    {
    }
    virtual basic_custom_data<Char>* clone() const
    {
        return new custom_data_wrapper<Char,T>(data1_);
    }

    virtual void to_stream(basic_json_output_handler<Char>& os) const
    {
        serialize(os,data1_);
    }

    T data1_;
};

template <typename Char, class Storage>
class basic_json;

template <typename Char,class Storage>
class json_object;

template <typename Char,class Storage>
class json_array;

template <typename Char>
class basic_output_format;

template <typename Char>
std::basic_string<Char> escape_string(const std::basic_string<Char>& s, const basic_output_format<Char>& format);

template <typename Char, typename Storage, typename T>
class value_adapter
{
public:
    bool is(const basic_json<Char,Storage>& val) const
    {
        return false;
    }
    T as(const basic_json<Char,Storage>& val) const;
    void assign(basic_json<Char,Storage>& self, const T val);
};

class json_base
{
public:
    enum value_type 
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
        custom_t
    };
};

template <typename Char, typename Storage>
class basic_json : public json_base
{
public:
    typedef Char char_type;
    typedef Storage allocator_type;

    typedef jsoncons::null_type null_type;
    class object;
    class array;
    struct custom_type {};

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
        member_type(const std::basic_string<Char>& nam, const basic_json<Char,Storage>& val)
            : name_(nam), value_(val)
        {
        }
        member_type(std::basic_string<Char>&& nam, basic_json<Char,Storage>&& val)
            : name_(nam), value_(val)
        {
        }

        const std::basic_string<Char>& name() const
        {
            return name_;
        }

        basic_json<Char,Storage>& value()
        {
            return value_;
        }

        const basic_json<Char,Storage>& value() const
        {
            return value_;
        }

        member_type& operator=(member_type rhs)
        {
            swap(rhs);
            return *this;
        }

        void swap(member_type& pair)
        {
            name_.swap(pair.name_);
            value_.swap(pair.value_);
        }

        std::basic_string<Char> name_;
        basic_json<Char,Storage> value_;
    };

    typedef member_type name_value_pair;

    static const basic_json<Char,Storage> an_object;
    static const basic_json<Char,Storage> an_array;
    static const basic_json<Char,Storage> null;
    
    typedef typename json_object<Char,Storage>::iterator object_iterator;
    typedef typename json_object<Char,Storage>::const_iterator const_object_iterator;

    typedef typename json_array<Char,Storage>::iterator array_iterator;
    typedef typename json_array<Char,Storage>::const_iterator const_array_iterator;

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
        friend class basic_json<Char,Storage>;

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

        value_type type() const
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

        bool operator==(const basic_json<Char,Storage>& val) const
        {
            return val_ == val;
        }

        bool operator!=(const basic_json& val) const
        {
            return val_ != val;
        }

        const basic_json<Char,Storage>& operator[](size_t i) const
        {
            return val_[i];
        }

        const basic_json<Char,Storage>& operator[](const std::basic_string<Char>& name) const
        {
            return val_.at(name);
        }

        const basic_json<Char,Storage>& at(const std::basic_string<Char>& name) const
        {
            return val_.at(name);
        }

        const basic_json<Char,Storage>& get(const std::basic_string<Char>& name) const
        {
            return val_.get(name);
        }

        const_val_proxy get(const std::basic_string<Char>& name, const basic_json<Char,Storage>& default_val) const
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

        const_val_proxy(const basic_json<Char,Storage>& val)
            : val_(val)
        {
        }

        const basic_json<Char,Storage>& val_;
    };

    class object_key_proxy 
    {
    public:
        friend class basic_json<Char,Storage>;

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

        value_type type() const
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

        void resize_array(size_t n, const basic_json<Char,Storage>& val)
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

        basic_json<Char,Storage>& operator[](size_t i)
        {
            return val_.at(name_)[i];
        }

        const basic_json<Char,Storage>& operator[](size_t i) const
        {
            return val_.at(name_)[i];
        }

        object_key_proxy operator[](const std::basic_string<Char>& name)
        {
            return object_key_proxy(val_.at(name_),name);
        }

        const basic_json<Char,Storage>& operator[](const std::basic_string<Char>& name) const
        {
            return val_.at(name_).at(name);
        }

        basic_json<Char,Storage>& at(const std::basic_string<Char>& name)
        {
            return val_.at(name_).at(name);
        }

        const basic_json<Char,Storage>& at(const std::basic_string<Char>& name) const
        {
            return val_.at(name_).at(name);
        }

        const basic_json<Char,Storage>& get(const std::basic_string<Char>& name) const
        {
            return val_.at(name_).get(name);
        }

        const_val_proxy get(const std::basic_string<Char>& name, const basic_json<Char,Storage>& default_val) const
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

        void set(const std::basic_string<Char>& name, const basic_json<Char,Storage>& value)
        {
            val_.at(name_).set(name,value);
        }

        void set(std::basic_string<Char>&& name, basic_json<Char,Storage>&& value)

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

        void add(basic_json<Char,Storage>&& value)
        {
            val_.at(name_).add(value);
        }

        void add(size_t index, basic_json<Char,Storage>&& value)
        {
            val_.at(name_).add(index, value);
        }

        void add(const basic_json<Char,Storage>& value)
        {
            val_.at(name_).add(value);
        }

        void add(size_t index, const basic_json<Char,Storage>& value)
        {
            val_.at(name_).add(index, value);
        }

        template <class T>
        void set_custom_data(const std::basic_string<Char>& name, const T& value)
        {
            val_.at(name_).set_custom_data(name,value);
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

        void swap(basic_json<Char,Storage>& val)
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

        object_key_proxy(basic_json<Char,Storage>& var, 
              const std::basic_string<Char>& name)
            : val_(var), name_(name)
        {
        }

        basic_json<Char,Storage>& val_;

        const std::basic_string<Char>& name_;
    };

    static basic_json parse(std::basic_istream<Char>& is);

    static basic_json parse_string(const std::basic_string<Char>& s);

    static basic_json parse_file(const std::string& s);

    template<int size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array()
    {
        return build_array<Char,Storage,size>()();
    }
    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n)
    {
        return build_array<Char,Storage,size>()(n);
    }
    template<size_t size>
    static typename std::enable_if<size==1,basic_json>::type make_multi_array(size_t n, const basic_json<Char,Storage>& val)
    {
        return build_array<Char,Storage,size>()(n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n)
    {
        return build_array<Char,Storage,size>()(m, n);
    }
    template<size_t size>
    static typename std::enable_if<size==2,basic_json>::type make_multi_array(size_t m, size_t n, const basic_json<Char,Storage>& val)
    {
        return build_array<Char,Storage,size>()(m, n, val);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k)
    {
        return build_array<Char,Storage,size>()(m, n, k);
    }
    template<size_t size>
    static typename std::enable_if<size==3,basic_json>::type make_multi_array(size_t m, size_t n, size_t k, const basic_json<Char,Storage>& val)
    {
        return build_array<Char,Storage,size>()(m, n, k, val);
    }

    explicit basic_json();

    basic_json(const basic_json& val);

    basic_json(jsoncons::null_type);

    basic_json(Char c);

    basic_json(double val);

    basic_json(int val);

    basic_json(unsigned int val);

    basic_json(long val);

    basic_json(unsigned long val);

    basic_json(long long val);

    basic_json(unsigned long long val);

    basic_json(const Char* val);

    basic_json(const std::basic_string<Char>& val);

    basic_json(bool val);

    template <class InputIterator>
    basic_json(InputIterator name, InputIterator last);

    explicit basic_json(json_object<Char,Storage>* var);

    explicit basic_json(json_array<Char,Storage>* var);

    explicit basic_json(basic_custom_data<Char>* var);

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
    basic_json& operator=(T val)
    {
        value_adapter<Char,Storage,T> adapter;
        adapter.assign(*this,val);
        return *this;
    }

    basic_json& operator=(basic_json<Char,Storage> rhs);

    bool operator!=(const basic_json<Char,Storage>& rhs) const;

    bool operator==(const basic_json<Char,Storage>& rhs) const;

    size_t size() const; 

    basic_json<Char,Storage>& operator[](size_t i);

    const basic_json<Char,Storage>& operator[](size_t i) const;

    object_key_proxy operator[](const std::basic_string<Char>& name);

    const basic_json<Char,Storage>& operator[](const std::basic_string<Char>& name) const;

    std::basic_string<Char> to_string() const;

    std::basic_string<Char> to_string(const basic_output_format<Char>& format) const;

    void to_stream(std::basic_ostream<Char>& os) const;

    void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format) const;

    void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format, bool indenting) const;

    bool is_null() const
    {
        return type_ == null_t;
    }

    bool has_member(const std::basic_string<Char>& name) const;

    template<typename T>
    bool is() const
    {
        value_adapter<Char,Storage,T> adapter;
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

    void resize_array(size_t n, const basic_json<Char,Storage>& val);

    template<typename T>
    T as() const
    {
        value_adapter<Char,Storage,T> adapter;
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

    std::basic_string<Char> as_string() const;

    std::basic_string<Char> as_string(const basic_output_format<Char>& format) const;

    Char as_char() const;

    basic_json<Char,Storage>& at(const std::basic_string<Char>& name);
    const basic_json<Char,Storage>& at(const std::basic_string<Char>& name) const;

    basic_json<Char,Storage>& at(size_t i);
    const basic_json<Char,Storage>& at(size_t i) const;

    const basic_json<Char,Storage>& get(const std::basic_string<Char>& name) const;

    const_val_proxy get(const std::basic_string<Char>& name, const basic_json<Char,Storage>& default_val) const;

    // Modifiers

    void clear();
    // Remove all elements from an array or object

    void remove_range(size_t from_index, size_t to_index);
    // Removes all elements from an array value whose index is between from_index, inclusive, and to_index, exclusive.

    void remove_member(const std::basic_string<Char>& name);
    // Removes a member from an object value

    basic_json(basic_json&& val);

    template <typename T>
    void set(const std::basic_string<Char>& name, T value)
    {
        switch (type_)
        {
        case empty_object_t:
            type_ = object_t;
            value_.object_ = new json_object<Char,Storage>();
        case object_t:
            {
                value_adapter<Char,Storage,T> adapter;
                basic_json<Char,Storage> o;
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

    void set(const std::basic_string<Char>& name, const basic_json<Char,Storage>& value);

    void set(std::basic_string<Char>&& name, basic_json<Char,Storage>&& value);

    template <typename T>
    void add(T val)
    {
        switch (type_)
        {
        case array_t:
            {
                value_adapter<Char,Storage,T> adapter;
                basic_json<Char,Storage> a;
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
        case array_t:
            {
                value_adapter<Char,Storage,T> adapter;
                basic_json<Char,Storage> a;
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

    void add(basic_json<Char,Storage>&& value);

    void add(size_t index, basic_json<Char,Storage>&& value);

    template <class T>
    void set_custom_data(const std::basic_string<Char>& name, const T& value);

    void add(const basic_json<Char,Storage>& value);

    void add(size_t index, const basic_json<Char,Storage>& value);

    template <class T>
    void add_custom_data(const T& value);

    template <class T>
    void add_custom_data(size_t index, const T& value);

    value_type type() const
    {
        return type_;
    }

    void to_stream(basic_json_output_handler<Char>& handler) const;

    void swap(basic_json<Char,Storage>& b)
    {
        using std::swap;

        swap(type_,b.type_);
        swap(value_,b.value_);
    }

    template <class T>
    std::vector<T> as_vector() const
    {
        std::vector<T> v(size());
        value_adapter<Char,Storage,T> adapter;
        for (size_t i = 0; i < v.size(); ++i)
        {
            v[i] = adapter.as(at(i));
        }
        return v;
    }

    friend void swap(basic_json<Char,Storage>& a, basic_json<Char,Storage>& b)
    {
        a.swap(b);
    }

    void assign_string(const std::basic_string<Char>& rhs);
    void assign_double(double rhs);
    void assign_longlong(long long rhs);
    void assign_ulonglong(unsigned long long rhs);
    void assign_bool(bool rhs);
    void assign_null();

//  Deprecated

    static basic_json make_array();
    static basic_json make_array(size_t n);
    static basic_json make_array(size_t n, const basic_json<Char,Storage>& val);

    static basic_json make_2d_array(size_t m, size_t n);

    static basic_json make_2d_array(size_t m, size_t n, const basic_json<Char,Storage>& val);

    static basic_json make_3d_array(size_t m, size_t n, size_t k);

    static basic_json make_3d_array(size_t m, size_t n, size_t k, const basic_json<Char,Storage>& val);

    bool is_number() const
    {
        return is_numeric();
    }
private:
	basic_json(value_type t);

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
        basic_json<Char2,Allocator2> operator() (size_t n, const basic_json<Char2,Allocator2>& val)
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
        basic_json<Char2,Allocator2> operator() (size_t m, size_t n, const basic_json<Char2,Allocator2>& val)
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
        basic_json<Char2,Allocator2> operator() (size_t m, size_t n, size_t k, const basic_json<Char2,Allocator2>& val)
        {
            return basic_json<Char2,Allocator2>::make_3d_array (m, n, k, val);
        }
    };

	value_type type_;
    union
    {
        double value_double_;
        long long longlong_value_;
        unsigned long long ulonglong_value_;
        bool bool_value_;
        json_object<Char,Storage>* object_;
        json_array<Char,Storage>* array_;
        std::basic_string<Char>* value_string_;
        basic_custom_data<Char>* userdata_;
    } value_;
};

template <typename Char, typename Storage>
void swap(typename basic_json<Char,Storage>::member_type& a, typename basic_json<Char,Storage>::member_type& b)
{
    a.swap(b);
}

typedef basic_json<char,storage<char>> json;
typedef basic_json<wchar_t,storage<wchar_t>> wjson;

}

#endif
