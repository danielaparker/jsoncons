#include <string>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "jsoncons/json_variant.hpp"

namespace jsoncons {

using std::ostringstream;
using std::string;

std::string json_variant::to_string() const
{
    ostringstream os;
    os.precision(16);
    to_stream(os);
    return os.str();
}

std::string json_variant::escape_string(const std::string& s)
{
    size_t pos = s.find_first_of("\\\"\b\f\n\r\t");
    if (pos ==  string::npos)
    {
        return s;
    }
    else
    {
        const size_t len = s.length();
        std::string buf(s,0,pos);
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

double json_variant::as_double() const
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
        JSONCONS_THROW_EXCEPTION("Not a double");
    }
}

void json_array::push_back(json_variant* value)
{
    elements_.push_back(value);
}

void json_object::sort_members()
{
    std::sort(members_.begin(),members_.end(),member_compare());
}

void json_object::insert(const_iterator it, name_value_pair member)
{
    members_.insert(it,member);
}

void json_object::remove(iterator at)
{
    delete (*at).value_;
    members_.erase(at);
}

void json_object::set_member(const std::string& name, json_variant* value)
{
    name_value_pair key(name,0);
    iterator it = std::lower_bound(begin(),end(),key,member_compare());
    if (it != end() && (*it).name_ == name)
    {
        remove(it);
    }
    insert(it,name_value_pair(name,value));
}

json_variant* json_object::get(const std::string& name)
{
    iterator it = find(name);
    JSONCONS_ASSERT((it != end()));
    return (*it).value_;
}

json_object::iterator json_object::find(const std::string& name)
{
    name_value_pair key(name,0);
    member_compare comp;
    iterator it = std::lower_bound(begin(),end(),key,comp);
    return (it != end() && !comp(key,*it)) ? it : end();
}

json_object::const_iterator json_object::find(const std::string& name) const
{
    name_value_pair key(name,0);
    member_compare comp;
    const_iterator it = std::lower_bound(begin(),end(),key,comp);
    return (it != end() && !comp(key,*it)) ? it : end();
}


}

