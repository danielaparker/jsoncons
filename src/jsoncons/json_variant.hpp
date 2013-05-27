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
    friend class basic_json<Char>;

    virtual ~json_variant()
    {
    }

    virtual json_variant* clone() = 0;

    std::basic_string<Char> to_string() const;

    virtual void to_stream(std::ostream& os) const
    {
    }

    static std::basic_string<Char> escape_string(const std::basic_string<Char>& s);

    json_object<Char>* object_cast();
    json_array<Char>* array_cast();
    const json_object<Char>* object_cast() const;
    const json_array<Char>* array_cast() const;
    std::basic_string<Char> string_value() const;
};

template <class Char>
class name_value_pair 
{
public:
    name_value_pair()
//        : value_(0)
    {
    }
    name_value_pair(std::basic_string<Char> name)
        : name_(name), value_(nullptr)
    {
    }
    name_value_pair(const std::basic_string<Char>& name, const basic_json<Char>& value)
        : name_(name), value_(value)
    {
    }
    name_value_pair(std::basic_string<Char>&& name, basic_json<Char>&& value)
        : name_(name), value_(value)
    {
    }
    std::basic_string<Char> name_;
    basic_json<Char> value_;
};

template <class Char>
class member_key
{
public:
    member_key(const std::basic_string<Char>& name)
        : name_(name)
    {
    }
    const std::basic_string<Char>& name_;
};

template <class Char>
class key_compare
{
public:
    bool operator()(const name_value_pair<Char>& a, 
                    const member_key<Char>& b) const
    {
        return a.name_ < b.name_;
    }
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
    {
    }
    json_string(std::basic_string<Char> s)
        : value_(s)
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
class json_array : public json_variant<Char>
{
public:
    typedef typename std::vector<basic_json<Char>>::iterator iterator;
    typedef typename std::vector<basic_json<Char>>::const_iterator const_iterator;

    json_array()
    {
    }
    template <class Iterator>
    json_array(Iterator begin, Iterator end)
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
class json_object : public json_variant<Char>
{
public:
    typedef typename std::vector<name_value_pair<Char>>::iterator iterator;
    typedef typename std::vector<name_value_pair<Char>>::const_iterator const_iterator;

    json_object()
    {
    }

    json_object(std::vector<name_value_pair<Char>> members)
        : members_(members)
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

    void set_member(const std::basic_string<Char>& name, basic_json<Char> value);

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
std::basic_string<Char> json_variant<Char>::string_value() const {return static_cast<const json_string<Char>*>(this)->value_;}

template <class Char>
json_object<Char>* json_variant<Char>::object_cast() {return static_cast<json_object<Char>*>(this);}

template <class Char>
json_array<Char>* json_variant<Char>::array_cast() {return static_cast<json_array<Char>*>(this);}

template <class Char>
const json_object<Char>* json_variant<Char>::object_cast() const {return static_cast<const json_object<Char>*>(this);}

template <class Char>
const json_array<Char>* json_variant<Char>::array_cast() const {return static_cast<const json_array<Char>*>(this);}

template <class Char>
std::basic_string<Char> json_variant<Char>::to_string() const
{
    std::basic_ostringstream<Char> os;
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
    member_key<Char> key(name);
    iterator it = std::lower_bound(begin(),end(),key,key_compare<Char>());
    if (it != end() && (*it).name_ == name)
    {
        remove(it);
    }
    insert(it,name_value_pair<Char>(name,value));
}

template <class Char>
void json_object<Char>::set_member(const std::basic_string<Char>& name, basic_json<Char> value)
{
    member_key<Char> key(name);
    iterator it = std::lower_bound(begin(),end(),key,key_compare<Char>());
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
    member_key<Char> key(name);
    key_compare<Char> comp;
    iterator it = std::lower_bound(begin(),end(),key,comp);
    return (it != end() && !comp(*it,key)) ? it : end();
}

template <class Char>
typename json_object<Char>::const_iterator json_object<Char>::find(const std::basic_string<Char>& name) const
{
    member_key<Char> key(name);
    key_compare comp;
    const_iterator it = std::lower_bound(begin(),end(),key,comp);
    return (it != end() && !comp(*it,key)) ? it : end();
}

}

#endif
