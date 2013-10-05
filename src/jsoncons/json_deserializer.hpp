// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_DESERIALIZER_HPP
#define JSONCONS_JSON_DESERIALIZER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/json1.hpp"
#include "jsoncons/json_listener.hpp"

namespace jsoncons {

template <class Char>
class basic_json_deserializer : public basic_json_listener<Char>
{
    enum structure_type {object_t, array_t};
    struct stack_item
    {
        stack_item(structure_type type)
            : type_(type)
        {
            switch (type)
            {
            case object_t:
                structure_.object_ = new json_object<Char>();
                //structure_.object_->reserve(4);
                break;
            case array_t:
                structure_.array_ =  new json_array<Char>();
                //structure_.array_->reserve(4);
                break;
            }
        }
        bool is_object() const
        {
            return type_ == object_t;
        }
        bool is_array() const
        {
            return type_ == array_t;
        }

        std::pair<std::basic_string<Char>,basic_json<Char>> pair_;
        structure_type type_;
        union {
            json_object<Char>* object_;
            json_array<Char>* array_;
        } structure_;
    };

public:
    virtual void begin_json()
    {
    }

    virtual void end_json()
    {
    }

    virtual void begin_object(const basic_parsing_context<Char>& context)
    {
        stack_.push_back(stack_item(object_t));
    }

    virtual void end_object(const basic_parsing_context<Char>& context)
    {
		json_object<Char>* var = stack_.back().structure_.object_;
        var->sort_members();
		stack_.pop_back();
        if (stack_.size() > 0)
        {
            if (stack_.back().is_object())
            {
                stack_.back().pair_.second = basic_json<Char>(var);	    
                stack_.back().structure_.object_->push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_.back().pair_)));
            }
            else
            {
                basic_json<Char> val(var);	    
                stack_.back().structure_.array_->push_back(std::move(val));
            }
        }
        else
        {
            basic_json<Char> val(var);	    
            root_ = std::move(val);
        }
    }

    virtual void begin_array(const basic_parsing_context<Char>& context)
    {
        stack_.push_back(stack_item(array_t));
    }

    virtual void end_array(const basic_parsing_context<Char>& context)
    {
        json_array<Char>* var = stack_.back().structure_.array_;
        stack_.pop_back();
        if (stack_.size() > 0)
        {
            if (stack_.back().is_object())
            {
                stack_.back().pair_.second = basic_json<Char>(var);	    
                stack_.back().structure_.object_->push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_.back().pair_)));
            }
            else
            {
                basic_json<Char> val(var);	    
                stack_.back().structure_.array_->push_back(std::move(val));
            }
        }
        else
        {
            basic_json<Char> val(var);	    
            root_ = std::move(val);
        }
    }

    virtual void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context)
    {
        stack_.back().pair_.first = name;
    }

    virtual void value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().pair_.second = basic_json<Char>(value);	    
            stack_.back().structure_.object_->push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_.back().pair_)));
        } 
        else 
        {
            basic_json<Char> val(value);
            stack_.back().structure_.array_->push_back(std::move(val));
        }
    }

    virtual void value(double value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().pair_.second = basic_json<Char>(value);	    
            stack_.back().structure_.object_->push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_.back().pair_)));
        } 
        else
        {
            basic_json<Char> val(value);
            stack_.back().structure_.array_->push_back(val);
        }
    }

    virtual void value(long long value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().pair_.second = basic_json<Char>(value);	    
            stack_.back().structure_.object_->push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_.back().pair_)));
        } 
        else
        {
            basic_json<Char> val(value);
            stack_.back().structure_.array_->push_back(val);
        }
    }

    virtual void value(unsigned long long value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().pair_.second = basic_json<Char>(value);	    
            stack_.back().structure_.object_->push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_.back().pair_)));
        } 
        else
        {
            basic_json<Char> val(value);
            stack_.back().structure_.array_->push_back(val);
        }
    }

    virtual void value(bool value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().pair_.second = basic_json<Char>(value);	    
            stack_.back().structure_.object_->push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_.back().pair_)));
        } 
        else
        {
            basic_json<Char> val(value);
            stack_.back().structure_.array_->push_back(val);
        }
    }

    virtual void null_value(const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().pair_.second = basic_json<Char>();	    
            stack_.back().structure_.object_->push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_.back().pair_)));
        } 
        else
        {
            basic_json<Char> val;
            stack_.back().structure_.array_->push_back(val);
        }
    }

    basic_json<Char>& root()
    {
        return root_;
    }

private:
	basic_json<Char> root_;
    std::vector<stack_item> stack_;
};

typedef basic_json_deserializer<char> json_deserializer;

}

#endif
