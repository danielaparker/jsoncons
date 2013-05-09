#ifndef JSONCONS_JSONVARIANT_HPP
#define JSONCONS_JSONVARIANT_HPP

#include <string>
#include <vector>
#include <assert.h> 
#include <exception>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace jsoncons {

class json_exception : public std::exception
{
public:
    json_exception(std::string s)
        : message_(s)
    {
    }
    const char* what() const 
    {
        return message_.c_str();
    }
private:
    std::string message_;
};

#define JSONCONS_THROW_EXCEPTION(x) throw json_exception((x))
#define JSONCONS_ASSERT(x) if (!x) throw json_exception(#x)

class json_variant;
class json_object;
class json_array;
class name_value_pair;

class name_value_pair 
{
public:
    name_value_pair()
        : value_(0)
    {
    }
    name_value_pair(std::string name, json_variant* value)
        : name_(name), value_(value)
    {
    }
    std::string name_;
    json_variant* value_;
};

class member_compare
{
public:
    bool operator()(const name_value_pair& a, const name_value_pair& b) const
    {
        return a.name_ < b.name_;
    }
};

class json_variant
{
public:
    enum value_type {object_t,string_t,double_t,long_t,ulong_t,pair_t,array_t,bool_t,null_t};

    json_variant(value_type type)
        : type_(type)
    {
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

    std::string to_string() const;

    virtual void to_stream(std::ostream& os) const
    {
    }

    static std::string escape_string(const std::string& s);

    json_object* object_cast();
    json_array* array_cast();
    double double_value() const;
    long long_value() const;
    unsigned long ulong_value() const;
    std::string string_value() const;
    bool bool_value() const;
private:
    value_type type_; 

};


class json_string : public json_variant
{
public:
    json_string()
        : json_variant(json_variant::string_t)
    {
    }
    json_string(std::string s)
        : json_variant(json_variant::string_t), value_(s)
    {
    }

    virtual json_variant* clone() 
    {
        return new json_string(value_);
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << "\"" << value_ << "\"";
    }

    std::string value_;
};

class json_null : public json_variant
{
public:
    json_null()
        : json_variant(json_variant::null_t)
    {
    }

    virtual json_variant* clone() 
    {
        return new json_null();
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << "null";
    }
};

class json_double : public json_variant
{
public:
    json_double(double value)
        : json_variant(json_variant::double_t), value_(value)
    {
    }

    virtual json_variant* clone() 
    {
        return new json_double(value_);
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << value_;
    }

    double value_;
};

class json_long  : public json_variant
{
public:
    json_long (long value)
        : json_variant(json_variant::long_t), value_(value)
    {
    }

    virtual json_variant* clone() 
    {
        return new json_long (value_);
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << value_;
    }

    long value_;
};

class json_ulong : public json_variant
{
public:
    json_ulong(unsigned long value)
        : json_variant(json_variant::ulong_t), value_(value)
    {
    }

    virtual json_variant* clone() 
    {
        return new json_ulong(value_);
    }

    virtual void to_stream(std::ostream& os) const
    {
        os << value_;
    }

    unsigned long value_;
};

class json_array : public json_variant
{
public:
    typedef std::vector<json_variant*>::iterator iterator;
    typedef std::vector<json_variant*>::const_iterator const_iterator;

    json_array()
        : json_variant(json_variant::array_t)
    {
    }
    json_array(std::vector<json_variant*> elements)
        : json_variant(json_variant::array_t), elements_(elements)
    {
    }

    virtual json_variant* clone() 
    {
        std::vector<json_variant*> elements(elements_.size());
        for (size_t i = 0; i < elements_.size(); ++i)
        {
            elements[i] = elements_[i]->clone();
        }
        return new json_array(elements);
    }

    ~json_array()
    {
        for (size_t i = 0; i < elements_.size(); ++i)
        {
            delete elements_[i];
        }
    }

    size_t size() const {return elements_.size();}

    json_variant* at(size_t i) {return elements_[i];}

    const json_variant* at(size_t i) const {return elements_[i];}

    void push_back(json_variant* value);

    virtual void to_stream(std::ostream& os) const
    {
		os << "[";
        for (size_t i = 0; i < elements_.size(); ++i)
        {
            if (i > 0)
            {
                os << ",";
            }
            elements_[i]->to_stream(os);
        }
		os << "]";
    }

    std::vector<json_variant*> elements_;
};

class json_bool : public json_variant
{
public:
	json_bool(bool value)
		: json_variant(json_variant::bool_t), value_(value)
	{
	}

    virtual json_variant* clone() 
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

class json_object : public json_variant
{
public:
    typedef std::vector<name_value_pair>::iterator iterator;
    typedef std::vector<name_value_pair>::const_iterator const_iterator;

    json_object()
        : json_variant(json_variant::object_t)
    {
    }

    json_object(std::vector<name_value_pair> members)
        : json_variant(json_variant::object_t), members_(members)
    {
    }

    ~json_object()
    {
        for (size_t i = 0; i < members_.size(); ++i)
        {
            delete members_[i].value_;
        }
    }

    virtual json_variant* clone() 
    {
        std::vector<name_value_pair> members(members_.size());
        for (size_t i = 0; i < members_.size(); ++i)
        {
            
            members[i] = name_value_pair(members_[i].name_,members_[i].value_->clone());
        }
        return new json_object(members);
    }

    size_t size() const {return members_.size();}

    json_variant* at(size_t i) {return members_[i].value_;}

    const json_variant* at(size_t i) const {return members_[i].value_;}

    //json_value& get(const std::string& name, json_value default_value);

    //const json_value& get(const std::string& name, json_value default_value) const;

    void set_member(const std::string& name, json_variant* value);

    void remove(iterator at); 

    json_variant* get(const std::string& name);

    iterator find(const std::string& name);

    const_iterator find(const std::string& name) const;

    void insert(const_iterator it, name_value_pair member);

    void push_back(name_value_pair member)
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
			members_[i].value_->to_stream(os);
        }
		os << "}";
    }

    std::vector<name_value_pair> members_;
};

inline
double json_variant::double_value() const {assert(type_ == double_t); return static_cast<const json_double*>(this)->value_;}

inline
long json_variant::long_value() const {assert(type_ == long_t); return static_cast<const json_long *>(this)->value_;}

inline
unsigned long json_variant::ulong_value() const {assert(type_ == ulong_t); return static_cast<const json_ulong*>(this)->value_;}

inline
std::string json_variant::string_value() const {assert(type_ == string_t); return static_cast<const json_string*>(this)->value_;}

inline
bool json_variant::bool_value() const {assert(type_ == bool_t); return static_cast<const json_bool*>(this)->value_;}

inline
json_object* json_variant::object_cast() {assert(type_ == object_t); return static_cast<json_object*>(this);}

inline
json_array* json_variant::array_cast() {assert(type_ == array_t); return static_cast<json_array*>(this);}

}

#endif
