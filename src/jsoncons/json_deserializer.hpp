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
#include <memory>
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/json1.hpp"
#include "jsoncons/json_input_handler.hpp"

namespace jsoncons {

template <class Char>
class basic_json_deserializer : public basic_json_input_handler<Char>
{
    typedef std::pair<std::basic_string<Char>,basic_json<Char>> member_type;

    struct stack_item
    {
        stack_item(bool is_object, size_t minimum_structure_capacity)
            : is_object_(is_object)
        {
            minimum_structure_capacity_ = minimum_structure_capacity;
            if (is_object_)
            {
                object_.reset(new json_object<Char>());
                object_->reserve(minimum_structure_capacity);
            }
            else
            {
                array_.reset(new json_array<Char>());
                array_->reserve(minimum_structure_capacity);
            }
        }
        bool is_object() const
        {
            return is_object_;
        }
        bool is_array() const
        {
            return !is_object_;
        }

        std::basic_string<Char> name_;
        bool is_object_;
        std::auto_ptr<json_object<Char>> object_;
        std::auto_ptr<json_array<Char>> array_;
        size_t minimum_structure_capacity_;
    };

public:
    ~basic_json_deserializer()
    {
    }

    virtual void begin_json()
    {
    }

    virtual void end_json()
    {
    }

    virtual void begin_object(const basic_parsing_context<Char>& context)
    {
        stack_.push_back(stack_item(true,context.minimum_structure_capacity()));
    }

    virtual void end_object(const basic_parsing_context<Char>& context)
    {
        stack_.back().object_->sort_members();
        basic_json<Char> val(stack_.back().object_.release());	    
        stack_.pop_back();
        if (stack_.size() > 0)
        {
            if (stack_.back().is_object())
            {
                stack_.back().object_->push_back(JSONCONS_MOVE(stack_.back().name_),JSONCONS_MOVE(val));
            }
            else
            {
                stack_.back().array_->push_back(JSONCONS_MOVE(val));
            }
        }
        else
        {
            root_.swap(val);
        }
    }

    virtual void begin_array(const basic_parsing_context<Char>& context)
    {
        stack_.push_back(stack_item(false,context.minimum_structure_capacity()));
    }

    virtual void end_array(const basic_parsing_context<Char>& context)
    {
        basic_json<Char> val(stack_.back().array_.release());	    
        stack_.pop_back();
        if (stack_.size() > 0)
        {
            if (stack_.back().is_object())
            {
                stack_.back().object_->push_back(JSONCONS_MOVE(stack_.back().name_),JSONCONS_MOVE(val));
            }
            else
            {
                stack_.back().array_->push_back(JSONCONS_MOVE(val));
            }
        }
        else
        {
            root_ = JSONCONS_MOVE(val);
        }
    }

    virtual void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context)
    {
        stack_.back().name_ = name;
    }

    virtual void value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(JSONCONS_MOVE(stack_.back().name_),basic_json<Char>(value));
        } 
        else 
        {
            stack_.back().array_->push_back(basic_json<Char>(value));
        }
    }

    virtual void value(double value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(JSONCONS_MOVE(stack_.back().name_),basic_json<Char>(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char>(value));
        }
    }

    virtual void value(long long value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(JSONCONS_MOVE(stack_.back().name_),basic_json<Char>(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char>(value));
        }
    }

    virtual void value(unsigned long long value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(JSONCONS_MOVE(stack_.back().name_),basic_json<Char>(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char>(value));
        }
    }

    virtual void value(bool value, const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(JSONCONS_MOVE(stack_.back().name_),basic_json<Char>(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char>(value));
        }
    }

    virtual void null_value(const basic_parsing_context<Char>& context)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(JSONCONS_MOVE(stack_.back().name_),basic_json<Char>(basic_json<Char>::null));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char>::null);
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
