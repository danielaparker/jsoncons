// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON1_HPP
#define JSONCONS_JSON1_HPP

#include <string>
#include <vector>
#include <exception>
#include <cstdlib>
#include <cstring>
#include "jsoncons/json_exception.hpp"
#include "jsoncons/json_out_stream.hpp"
#include "jsoncons/output_format.hpp"

namespace jsoncons {

template <class Char>
class basic_custom_data
{
public:
    virtual ~basic_custom_data()
    {
    }

    virtual void serialize(basic_json_out_stream<Char>& os) const = 0;

    virtual basic_custom_data<Char>* clone() const = 0;
};

typedef basic_custom_data<char> custom_data;

template <class Char,class T>
void serialize(basic_json_out_stream<Char>& os, 
               const T& val)
{
    os.null_value();
}

template <class Char, class T>
class custom_data_wrapper : public basic_custom_data<Char>
{
public:
    custom_data_wrapper(const T& value)
        : data_(value)
    {
    }
    virtual basic_custom_data<Char>* clone() const
    {
        return new custom_data_wrapper<Char,T>(data_);
    }

    virtual void serialize(basic_json_out_stream<Char>& os) const
    {
        jsoncons::serialize(os,data_);
    }

    T data_;
};

template <class Char>
class json_object;

template <class Char>
class json_array;

template <class Char>
class json_string;

template <class Char>
class basic_output_format;

template <class Char>
std::basic_string<Char> escape_string(const std::basic_string<Char>& s, const basic_output_format<Char>& format);

template <class Char>
class basic_json
{
public:
    enum value_type {object_t,array_t,string_t,double_t,longlong_t,ulonglong_t,bool_t,null_t,custom_t};

    static const basic_json<Char> an_object;
    static const basic_json<Char> an_array;
    static const basic_json<Char> null;

    typedef typename json_object<Char>::iterator object_iterator;
    typedef typename json_object<Char>::const_iterator const_object_iterator;

    typedef typename json_array<Char>::iterator array_iterator;
    typedef typename json_array<Char>::const_iterator const_array_iterator;

    class proxy 
    {
    public:
        friend class basic_json<Char>;

        size_t size() const
        {
            return val_.get(name_).size();
        }

        bool is_null() const
        {
            return val_.get(name_).is_null();
        }

        bool has_member(const std::basic_string<Char>& name) const
        {
            return val_.get(name_).has_member(name);
        }

        bool is_empty() const
        {
            return val_.get(name_).is_empty();
        }

        bool is_string() const
        {
            return val_.get(name_).is_string();
        }

        bool is_number() const
        {
            return val_.get(name_).is_number();
        }

        bool is_longlong() const
        {
            std::cout << "proxy is_longlong " << val_.get(name_).type() << std::endl;
            return val_.get(name_).is_longlong();
        }

        bool is_ulonglong() const
        {
            std::cout << "proxy is_ulonglong " << val_.get(name_).type() << std::endl;
            return val_.get(name_).is_ulonglong();
        }

        bool is_double() const
        {
            std::cout << "proxy is_double " << val_.get(name_).type() << std::endl;
            return val_.get(name_).is_double();
        }

        bool is_bool() const
        {
            return val_.get(name_).is_bool();
        }

        bool is_object() const
        {
            return val_.get(name_).is_object();
        }

        bool is_array() const
        {
            return val_.get(name_).is_array();
        }

        bool is_custom() const
        {
            return val_.get(name_).is_custom();
        }

        std::basic_string<Char> as_string() const
        {
            return val_.get(name_).as_string();
        }

        bool as_bool() const
        {
            return val_.get(name_).as_bool();
        }

        double as_double() const
        {
            return val_.get(name_).as_double();
        }

        int as_int() const
        {
            return val_.get(name_).as_int();
        }

        unsigned int as_uint() const
        {
            return val_.get(name_).as_uint();
        }

        long long as_longlong() const
        {
            return val_.get(name_).as_longlong();
        }

        unsigned long long as_ulonglong() const
        {
            return val_.get(name_).as_ulonglong();
        }

        template <class T>
        const T& custom_cast() const
        {
            return val_.get(name_).custom_cast<T>();
        }
        // Returns a const reference to the custom data associated with name

        template <class T>
        T& custom_cast() 
        {
            return val_.get(name_).custom_cast<T>();
        }
        // Returns a reference to the custom data associated with name

        operator basic_json&()
        {
            return val_.get(name_);
        }

        operator const basic_json&() const
        {
            return val_.get(name_);
        }

        proxy& operator=(const basic_json& val);

        basic_json<Char>& operator[](size_t i)
        {
            return val_.get(name_)[i];
        }

        const basic_json<Char>& operator[](size_t i) const
        {
            return val_.get(name_)[i];
        }

        proxy operator[](const std::basic_string<Char>& name)
        {
            return proxy(val_.get(name_),name);
        }

        const proxy operator[](const std::basic_string<Char>& name) const
        {
            return proxy(val_.get(name_),name);
        }

        basic_json<Char>& get(const std::basic_string<Char>& name)
        {
            return val_.get(name_).get(name);
        }

        const basic_json<Char>& get(const std::basic_string<Char>& name) const
        {
            return val_.get(name_).get(name);
        }

        basic_json<Char>& get(const std::basic_string<Char>& name, basic_json<Char>& default_val)
        {
            return val_.get(name_).get(name,default_val);
        }

        const basic_json<Char>& get(const std::basic_string<Char>& name, const basic_json<Char>& default_val) const
        {
            return val_.get(name_).get(name,default_val);
        }

        void clear()
        {
            val_.get(name_).clear();
        }
        // Remove all elements from an array or object

        void set(const std::basic_string<Char>& name, const basic_json<Char>& value)
        {
            return val_.get(name_).set(name,value);
        }

        void set(std::basic_string<Char>&& name, basic_json<Char>&& value)

        {
            return val_.get(name_).set(name,value);
        }

        template <class T>
        void set_custom(const std::basic_string<Char>& name, const T& value)
        {
            return val_.get(name_).set_custom(name,value);
        }

        template <class T>
        void set_custom(const std::basic_string<Char>& name, T&& value)
        {
            return val_.get(name_).set_custom(name,value);
        }

        void add(const basic_json<Char>& value)
        {
            val_.get(name_).add(value);
        }

        void add(basic_json<Char>&& value)
        {
            val_.get(name_).add(value);
        }

        template <class T>
        void add_custom(const T& value)
        {
            val_.get(name_).add_custom(value);
        }

        template <class T>
        void add_custom(T&& value)
        {
            val_.get(name_).add_custom(value);
        }

        std::basic_string<Char> to_string() const
        {
            return val_.get(name_).to_string();
        }

        std::basic_string<Char> to_string(const basic_output_format<Char>& format) const
        {
            return val_.get(name_).to_string(format);
        }

        void to_stream(std::basic_ostream<Char>& os) const
        {
            val_.get(name_).to_stream(os);
        }

        void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format) const
        {
            val_.get(name_).to_stream(os,format);
        }

        friend std::ostream& operator<<(std::ostream& os, const proxy& o)
        {
            o.to_stream(os);
            return os;
        }
    private:
        proxy(const proxy& proxy)
            : val_(proxy.val_), name_(proxy.name_)
        {
        }
        proxy& operator = (const proxy& other); // noop

        proxy(basic_json<Char>& var, 
              const std::basic_string<Char>& name);

        basic_json<Char>& val_;

        const std::basic_string<Char>& name_;
    };

    static basic_json parse(std::basic_istream<Char>& is);

    static basic_json parse_string(const std::basic_string<Char>& s);

    static basic_json parse_file(const std::string& s);

    explicit basic_json();

    basic_json(const basic_json& val);

    basic_json(basic_json&& val);

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
    basic_json(InputIterator first, InputIterator last);

    explicit basic_json(json_object<Char>* var);

    explicit basic_json(json_array<Char>* var);

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

    basic_json& operator=(basic_json<Char> rhs);

    size_t size() const; 

    basic_json<Char>& operator[](size_t i);

    const basic_json<Char>& operator[](size_t i) const;

    proxy operator[](const std::basic_string<Char>& name);

    const proxy operator[](const std::basic_string<Char>& name) const;

    std::basic_string<Char> to_string() const;

    std::basic_string<Char> to_string(const basic_output_format<Char>& format) const;

    void to_stream(std::basic_ostream<Char>& os) const;

    void to_stream(std::basic_ostream<Char>& os, const basic_output_format<Char>& format) const;

    bool is_null() const
    {
        return type_ == null_t;
    }

    bool has_member(const std::basic_string<Char>& name) const;

    bool is_string() const
    {
        return type_ == string_t;
    }

    bool is_number() const
    {
        return type_ == double_t || type_ == longlong_t || type_ == ulonglong_t;
    }

    bool is_longlong() const
    {
        return type_ == longlong_t;
    }

    bool is_ulonglong() const
    {
        std::cout << "json is_ulonglong " << type() << std::endl;
        return type_ == ulonglong_t;
    }

    bool is_double() const
    {
        std::cout << "json is_double " << type() << std::endl;
        return type_ == double_t;
    }

    bool is_bool() const
    {
        return type_ == bool_t;
    }

    bool is_object() const
    {
        return type_ == object_t;
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

    bool as_bool() const;

    double as_double() const;

    int as_int() const;

    unsigned int as_uint() const;

    long long as_longlong() const;

    unsigned long long as_ulonglong() const;

    template <class T>
    const T& custom_cast() const;
    // Returns a const reference to the custom data associated with name

    template <class T>
    T& custom_cast();
    // Returns a reference to the custom data associated with name

    std::basic_string<Char> as_string() const;

    void swap(basic_json<Char>& o) throw();

    basic_json<Char>& at(size_t i);

    const basic_json<Char>& at(size_t i) const;

    basic_json<Char>& get(const std::basic_string<Char>& name);

    const basic_json<Char>& get(const std::basic_string<Char>& name) const;

    basic_json<Char>& get(const std::basic_string<Char>& name, basic_json<Char>& default_val);

    const basic_json<Char>& get(const std::basic_string<Char>& name, const basic_json<Char>& default_val) const;

    // Modifiers

    void clear();
    // Remove all elements from an array or object

    void set(const std::basic_string<Char>& name, const basic_json<Char>& value);

    void set(std::basic_string<Char>&& name, basic_json<Char>&& value);

    template <class T>
    void set_custom(const std::basic_string<Char>& name, const T& value);

    template <class T>
    void set_custom(std::basic_string<Char>&& name, T&& value);

    void add(const basic_json<Char>& value);

    void add(basic_json<Char>&& value);

    template <class T>
    void add_custom(const T& value);

    template <class T>
    void add_custom(T&& value);

    value_type type() const
    {
        return type_;
    }

    void serialize(basic_json_out_stream<Char>& serializer) const;

private:
    value_type type_;
    union
    {
        double double_value_;
        long long longlong_value_;
        unsigned long long ulonglong_value_;
        bool bool_value_;
        json_object<Char>* object_;
        json_array<Char>* array_;
        std::basic_string<Char>* string_value_;
        basic_custom_data<Char>* userdata_;
    } value_;
};

}

#endif
