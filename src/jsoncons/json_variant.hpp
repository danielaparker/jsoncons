// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_VARIANT_HPP
#define JSONCONS_JSON_VARIANT_HPP

#include <string>
#include <vector>
#include <assert.h> 
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "jsoncons/json1.hpp"

namespace jsoncons {

template <class Char>
class json_object;

template <class Char>
class json_array;

template <class Char>
class json_variant
{
public:
    enum value_type {object_t,string_t,double_t,long_t,ulong_t,array_t,bool_t,null_t};
    friend class basic_json<Char>;

    typedef Char char_type;

    json_variant(value_type type)
        : type_(type)
    {
    }

    size_t size() const {
        switch (type_)
        {
        case object_t:
            return object_cast()->size();
        case array_t:
            return array_cast()->size();
        default:
            return 0;
        }
    }

    basic_json<Char>& get(size_t i) {
        switch (type_)
        {
        case object_t:
            return object_cast()->at(i);
        case array_t:
            return array_cast()->at(i);
        default:
            JSONCONS_THROW_EXCEPTION("Not an array or object");
        }
    }

    const basic_json<Char>& get(size_t i) const {
        switch (type_)
        {
        case object_t:
            return object_cast()->at(i);
        case array_t:
            return array_cast()->at(i);
        default:
            JSONCONS_THROW_EXCEPTION("Not an array or object");
        }
    }

    basic_json<Char>& get(const std::string& name) {
        switch (type_)
        {
        case object_t:
            return object_cast()->get(name);
        default:
            JSONCONS_THROW_EXCEPTION("Not an object");
        }
    }

    const basic_json<Char>& get(const std::string& name) const {
        switch (type_)
        {
        case object_t:
            return object_cast()->get(name);
        default:
            JSONCONS_THROW_EXCEPTION("Not an object");
        }
    }


    void set_member(const std::string& name, json_variant* var) {
        switch (type_)
        {
        case object_t:
            return object_cast()->set_member(name,var);
        default:
            JSONCONS_THROW_EXCEPTION("Not an object");
        }
    }

    bool is_null() const
    {
        return type_ == null_t;
    }

    bool is_object() const
    {
        return type_ == object_t;
    }

    bool is_array() const
    {
        return type_ == array_t;
    }

    bool is_string() const
    {
        return type_ == string_t;
    }

    bool is_double() const
    {
        return type_ == double_t;
    }

    virtual ~json_variant()
    {
    }

    virtual json_variant* clone() = 0;

    double as_double() const;

    int as_int() const;

    std::basic_string<Char> to_string() const;

    virtual void to_stream(std::ostream& os) const
    {
    }

    static std::basic_string<Char> escape_string(const std::basic_string<Char>& s);

    json_object<Char>* object_cast();
    json_array<Char>* array_cast();
    const json_object<Char>* object_cast() const;
    const json_array<Char>* array_cast() const;
    double double_value() const;
    integer_type long_value() const;
    uinteger_type ulong_value() const;
    std::basic_string<Char> string_value() const;
    bool bool_value() const;
private:
    value_type type_; 

};

template <class Char>
class name_value_pair 
{
public:
    name_value_pair()
//        : value_(0)
    {
    }
    name_value_pair(std::basic_string<Char> name, json_variant<Char>* value)
        : name_(name), value_(basic_json<Char>(value))
    {
    }
    name_value_pair(std::basic_string<Char> name, const basic_json<Char>& value)
        : name_(name), value_(value)
    {
    }
    std::basic_string<Char> name_;
    basic_json<Char> value_;
};

template <class Char>
class member_compare
{
public:
    bool operator()(const name_value_pair<Char>& a, 
                    const name_value_pair<Char>& b) const
    {
        return a.name_ < b.name_;
    }
};

template <class Char>
class json_string : public json_variant<Char>
{
public:
    json_string()
        : json_variant<Char>(json_variant<Char>::string_t)
    {
    }
    json_string(std::basic_string<Char> s)
        : json_variant<Char>(json_variant<Char>::string_t), value_(s)
    {
    }

    virtual json_variant<Char>* clone() 
    {
        return new json_string(value_);
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << "\"" << value_ << "\"";
    }

    std::basic_string<Char> value_;
};

template <class Char>
class json_null : public json_variant<Char>
{
public:
    json_null()
        : json_variant<Char>(json_variant<Char>::null_t)
    {
    }

    virtual json_variant<Char>* clone() 
    {
        return new json_null();
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << "null";
    }
};

template <class Char>
class json_double : public json_variant<Char>
{
public:
    json_double(double value)
        : json_variant<Char>(json_variant<Char>::double_t), value_(value)
    {
    }

    virtual json_variant<Char>* clone() 
    {
        return new json_double(value_);
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << value_;
    }

    double value_;
};

template <class Char>
class json_long  : public json_variant<Char>
{
public:
    json_long(integer_type value)
        : json_variant<Char>(json_variant<Char>::long_t), value_(value)
    {
    }

    virtual json_variant<Char>* clone() 
    {
        return new json_long (value_);
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << value_;
    }

    integer_type value_;
};

template <class Char>
class json_ulong : public json_variant<Char>
{
public:
    json_ulong(uinteger_type value)
        : json_variant<Char>(json_variant<Char>::ulong_t), value_(value)
    {
    }

    virtual json_variant<Char>* clone() 
    {
        return new json_ulong(value_);
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << value_;
    }

    uinteger_type value_;
};

template <class Char>
class json_array : public json_variant<Char>
{
public:
    typedef typename std::vector<basic_json<Char>>::iterator iterator;
    typedef typename std::vector<basic_json<Char>>::const_iterator const_iterator;

    json_array()
        : json_variant<Char>(json_variant<Char>::array_t)
    {
    }
    template <class Iterator>
    json_array(Iterator begin, Iterator end)
        : json_variant<Char>(json_variant<Char>::array_t)
    {
        for (Iterator it = begin; it != end; ++it)
        {
            elements_.push_back(*it);
        }
    }

    virtual json_variant<Char>* clone() 
    {
        return new json_array(elements_.begin(),elements_.end());
    }

    ~json_array()
    {
    }

    size_t size() const {return elements_.size();}

    basic_json<Char>& at(size_t i) {return elements_[i];}

    const basic_json<Char>& at(size_t i) const {return elements_[i];}

    void push_back(basic_json<Char> value);

    virtual void to_stream(std::ostream& os) const
    {
		os << "[";
        for (size_t i = 0; i < elements_.size(); ++i)
        {
            if (i > 0)
            {
                os << ",";
            }
            elements_[i].to_stream(os);
        }
		os << "]";
    }

    std::vector<basic_json<Char>> elements_;
};

template <class Char>
class json_bool : public json_variant<Char>
{
public:
	json_bool(bool value)
		: json_variant<Char>(json_variant<Char>::bool_t), value_(value)
	{
	}

    virtual json_variant<Char>* clone() 
    {
        return new json_bool(value_);
    }

    virtual void to_stream(std::ostream& os) const
    {
        if (value_)
        {
            os << "true";
        }
        else
        {
            os << "false";
        }
    }

    bool value_;
};

template <class Char>
class json_object : public json_variant<Char>
{
public:
    typedef typename std::vector<name_value_pair<Char>>::iterator iterator;
    typedef typename std::vector<name_value_pair<Char>>::const_iterator const_iterator;

    json_object()
        : json_variant<Char>(json_variant<Char>::object_t)
    {
    }

    json_object(std::vector<name_value_pair<Char>> members)
        : json_variant<Char>(json_variant<Char>::object_t), members_(members)
    {
    }

    ~json_object()
    {
        //for (size_t i = 0; i < members_.size(); ++i)
        //{
            //delete members_[i].value_;
        //}
    }

    virtual json_variant<Char>* clone() 
    {
        std::vector<name_value_pair<Char>> members(members_.size());
        for (size_t i = 0; i < members_.size(); ++i)
        {
            
            members[i] = name_value_pair<Char>(members_[i].name_,members_[i].value_);
        }
        return new json_object(members);
    }

    size_t size() const {return members_.size();}

    basic_json<Char>& at(size_t i) {return members_[i].value_;}

    const basic_json<Char>& at(size_t i) const {return members_[i].value_;}

    void set_member(const std::basic_string<Char>& name, json_variant<Char>* value);

    void remove(iterator at); 

    basic_json<Char>& get(const std::basic_string<Char>& name);

    const basic_json<Char>& get(const std::basic_string<Char>& name) const;

    iterator find(const std::basic_string<Char>& name);

    const_iterator find(const std::basic_string<Char>& name) const;

    void insert(const_iterator it, name_value_pair<Char> member);

    void push_back(name_value_pair<Char> member)
    {
        members_.push_back(member);
    }

    void sort_members();

    iterator begin() {return members_.begin();}

    iterator end() {return members_.end();}

    const_iterator begin() const {return members_.begin();}

    const_iterator end() const {return members_.end();}

    virtual void to_stream(std::ostream& os) const
    {
		os << "{";
        for (size_t i = 0; i < members_.size(); ++i)
        {
            if (i > 0)
            {
                os << ",";
            }
            os << "\"" << members_[i].name_ << "\":";
			members_[i].value_.to_stream(os);
        }
		os << "}";
    }

    std::vector<name_value_pair<Char>> members_;
};

template <class Char>
double json_variant<Char>::double_value() const {assert(type_ == double_t); return static_cast<const json_double<Char>*>(this)->value_;}

template <class Char>
integer_type json_variant<Char>::long_value() const {assert(type_ == long_t); return static_cast<const json_long<Char> *>(this)->value_;}

template <class Char>
uinteger_type json_variant<Char>::ulong_value() const {assert(type_ == ulong_t); return static_cast<const json_ulong<Char>*>(this)->value_;}

template <class Char>
std::basic_string<Char> json_variant<Char>::string_value() const {assert(type_ == string_t); return static_cast<const json_string<Char>*>(this)->value_;}

template <class Char>
bool json_variant<Char>::bool_value() const {assert(type_ == bool_t); return static_cast<const json_bool<Char>*>(this)->value_;}

template <class Char>
json_object<Char>* json_variant<Char>::object_cast() {assert(type_ == object_t); return static_cast<json_object<Char>*>(this);}

template <class Char>
json_array<Char>* json_variant<Char>::array_cast() {assert(type_ == array_t); return static_cast<json_array<Char>*>(this);}

template <class Char>
const json_object<Char>* json_variant<Char>::object_cast() const {assert(type_ == object_t); return static_cast<const json_object<Char>*>(this);}

template <class Char>
const json_array<Char>* json_variant<Char>::array_cast() const {assert(type_ == array_t); return static_cast<const json_array<Char>*>(this);}

template <class Char>
std::basic_string<Char> json_variant<Char>::to_string() const
{
    std::ostringstream os;
    os.precision(16);
    to_stream(os);
    return os.str();
}

template <class Char>
std::basic_string<Char> json_variant<Char>::escape_string(const std::basic_string<Char>& s)
{
    size_t pos = s.find_first_of("\\\"\b\f\n\r\t");
    if (pos ==  string::npos)
    {
        return s;
    }
    else
    {
        const size_t len = s.length();
        std::basic_string<Char> buf(s,0,pos);
        for (size_t i = pos; i < len; ++i)
        {
            char c = s[i];
            switch (c)
            {
            case '\\':
                buf.push_back('\\');
                break;
            case '"':
                buf.append("\\\"");
                break;
            case '\b':
                buf.append("\\b");
                break;
            case '\f':
                buf.append("\\f");
                break;
            case '\n':
                buf.append("\\n");
                break;
            case '\r':
                buf.append("\\r");
                break;
            case '\t':
                buf.append("\\t");
                break;
            default:
                buf.push_back(c);
                break;
            }
        }
        return buf;
    }
}

template <class Char>
double json_variant<Char>::as_double() const
{
    switch (type_)
    {
    case double_t:
        return double_value();
    case long_t:
        return static_cast<double>(long_value());
    case ulong_t:
        return static_cast<double>(ulong_value());
    default:
        std::cout << "Must be here" << std::endl;
        JSONCONS_THROW_EXCEPTION("Not a double");
    }
}

template <class Char>
int json_variant<Char>::as_int() const
{
    switch (type_)
    {
    case double_t:
        return static_cast<int>(double_value());
    case long_t:
        return static_cast<int>(long_value());
    case ulong_t:
        return static_cast<int>(ulong_value());
    default:
        std::cout << "Must be here" << std::endl;
        JSONCONS_THROW_EXCEPTION("Not a double");
    }
}

template <class Char>
void json_array<Char>::push_back(basic_json<Char> value)
{
    elements_.push_back(value);
}

template <class Char>
void json_object<Char>::sort_members()
{
    std::sort(members_.begin(),members_.end(),member_compare<Char>());
}

template <class Char>
void json_object<Char>::insert(const_iterator it, name_value_pair<Char> member)
{
    members_.insert(it,member);
}

template <class Char>
void json_object<Char>::remove(iterator at)
{
    members_.erase(at);
}

template <class Char>
void json_object<Char>::set_member(const std::basic_string<Char>& name, json_variant<Char>* value)
{
    name_value_pair<Char> key(name,0);
    iterator it = std::lower_bound(begin(),end(),key,member_compare<Char>());
    if (it != end() && (*it).name_ == name)
    {
        remove(it);
    }
    insert(it,name_value_pair<Char>(name,value));
}

template <class Char>
basic_json<Char>& json_object<Char>::get(const std::basic_string<Char>& name) 
{
    iterator it = find(name);
    JSONCONS_ASSERT((it != end()));
    return (*it).value_;
}

template <class Char>
const basic_json<Char>& json_object<Char>::get(const std::basic_string<Char>& name) const
{
    iterator it = find(name);
    JSONCONS_ASSERT((it != end()));
    return (*it).value_;
}

template <class Char>
typename json_object<Char>::iterator json_object<Char>::find(const std::basic_string<Char>& name)
{
    name_value_pair<Char> key(name,0);
    member_compare<Char> comp;
    iterator it = std::lower_bound(begin(),end(),key,comp);
    return (it != end() && !comp(key,*it)) ? it : end();
}

template <class Char>
typename json_object<Char>::const_iterator json_object<Char>::find(const std::basic_string<Char>& name) const
{
    name_value_pair<Char> key(name,0);
    member_compare comp;
    const_iterator it = std::lower_bound(begin(),end(),key,comp);
    return (it != end() && !comp(key,*it)) ? it : end();
}

}

#endif
