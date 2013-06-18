// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_STREAM_LISTENER_HPP
#define JSONCONS_JSON_STREAM_LISTENER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>
#include <cstdlib>
#include "jsoncons/json_char_traits.hpp"

namespace jsoncons {

template <class Char>
class basic_json_stream_listener
{
    struct stack_item
    {
        enum structure_type {object_t, array_t};
        stack_item(json_object<Char>* var)
            : type_(object_t)
        {
            structure_.object_ = var;
        }
        stack_item(json_array<Char>* var)
            : type_(array_t)
        {
            structure_.array_ = var;
        }
        bool is_object() const
        {
            return type_ == object_t;
        }
        bool is_array() const
        {
            return type_ == array_t;
        }

        std::basic_string<Char> name_;
        structure_type type_;
        union {
            json_object<Char>* object_;
            json_array<Char>* array_;
        } structure_;
    };

public:
    void begin_json()
    {
    }

    void end_json()
    {
    }

    void begin_object()
    {
        json_object<Char>* var = new json_object<Char>();
        stack_.push_back(stack_item(var));
    }

    void end_object()
    {
		json_object<Char>* var = stack_.back().structure_.object_;
        var->sort_members();
        basic_json<Char> val(var);	    
		stack_.pop_back();
        if (stack_.size() > 0)
        {
            if (stack_.back().is_object())
            {
                stack_.back().structure_.object_->push_back(basic_name_value_pair<Char>(std::move(stack_.back().name_),std::move(val)));
            }
            else
            {
                stack_.back().structure_.array_->push_back(std::move(val));
            }
        }
        else
        {
            root_ = std::move(val);
        }
    }

    void begin_array()
    {
        json_array<Char>* var = new json_array<Char>();
        stack_.push_back(stack_item(var));
    }

    void end_array()
    {
        json_array<Char>* var = stack_.back().structure_.array_;
        stack_.pop_back();
        basic_json<Char> val(var);	    
        if (stack_.size() > 0)
        {
            if (stack_.back().is_object())
            {
                stack_.back().structure_.object_->push_back(basic_name_value_pair<Char>(std::move(stack_.back().name_),std::move(val)));
            }
            else
            {
                stack_.back().structure_.array_->push_back(std::move(val));
            }
        }
        else
        {
            root_ = std::move(val);
        }
    }

    void name(const std::basic_string<Char>& name)
    {
        stack_.back().name_ = std::move(name);
    }

    void value(const std::basic_string<Char>& value)
    {
        basic_json<Char> val(std::move(value));
        if (stack_.back().is_object())
        {
            stack_.back().structure_.object_->push_back(std::move(basic_name_value_pair<Char>(std::move(stack_.back().name_),std::move(val))));
        } 
        else 
        {
            stack_.back().structure_.array_->push_back(std::move(val));
        }
    }

    void value(double value)
    {
        basic_json<Char> val(value);
        if (stack_.back().is_object())
        {
            stack_.back().structure_.object_->push_back(basic_name_value_pair<Char>(std::move(stack_.back().name_),std::move(val)));
        } 
        else
        {
            stack_.back().structure_.array_->push_back(val);
        }
    }

    void value(long long value)
    {
        basic_json<Char> val(value);
        if (stack_.back().is_object())
        {
            stack_.back().structure_.object_->push_back(basic_name_value_pair<Char>(std::move(stack_.back().name_),std::move(val)));
        } 
        else
        {
            stack_.back().structure_.array_->push_back(val);
        }
    }

    void value(unsigned long long value)
    {
        basic_json<Char> val(value);
        if (stack_.back().is_object())
        {
            stack_.back().structure_.object_->push_back(basic_name_value_pair<Char>(std::move(stack_.back().name_),std::move(val)));
        } 
        else
        {
            stack_.back().structure_.array_->push_back(val);
        }
    }

    void value(bool value)
    {
        basic_json<Char> val(value);
        if (stack_.back().is_object())
        {
            stack_.back().structure_.object_->push_back(basic_name_value_pair<Char>(std::move(stack_.back().name_),std::move(val)));
        } 
        else
        {
            stack_.back().structure_.array_->push_back(val);
        }
    }

    void null()
    {
        basic_json<Char> val;
        if (stack_.back().is_object())
        {
            stack_.back().structure_.object_->push_back(basic_name_value_pair<Char>(std::move(stack_.back().name_),std::move(val)));
        } 
        else
        {
            stack_.back().structure_.array_->push_back(val);
        }
    }

	basic_json<Char> root_;
private:
    std::vector<stack_item> stack_;
};

typedef basic_json_stream_listener<char> json_stream_listener;

}

#endif
