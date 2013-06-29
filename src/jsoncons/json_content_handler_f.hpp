// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_CONTENT_HANDLER_F_HPP
#define JSONCONS_JSON_CONTENT_HANDLER_F_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>
#include <cstdlib>
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/json_exception.hpp"

namespace jsoncons {

template <class Char>
class basic_json_content_handler_f
{
    enum structure_type {object_t, array_t};
    struct stack_item
    {
        stack_item(structure_type type)
            : type_(type)
        {
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

        std::vector<std::pair<std::basic_string<Char>,basic_json<Char>>> members_;
        std::vector<basic_json<Char>> elements_;
    };

public:
    basic_json_content_handler_f()
        : level_(0)
    {
    }

    void begin_json()
    {
        level_ = 0;
    }

    void end_json()
    {
        JSONCONS_ASSERT(level_ == 0);
    }

    void begin_object()
    {
        if (++level_ >= stack_.size())
        {
            stack_.push_back(stack_item(object_t));
        }
        else
        {
            stack_[level_-1].type_ = object_t; 
        }
    }

    void end_object()
    {
		json_object<Char>* var = new json_object<Char>(stack_[level_-1].members_.size());
        for (size_t i = 0; i < var->size(); ++i)
        {
            var->members_[i].swap(stack_[level_-1].members_[i]);
        }
        var->sort_members();
        basic_json<Char> val(var);	    
        stack_[level_-1].members_.clear();
        if (--level_ > 0)
        {
            if (stack_[level_-1].is_object())
            {
                stack_[level_-1].members_.push_back(std::move(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_[level_-1].name_),std::move(val))));
            }
            else
            {
                stack_[level_-1].elements_.push_back(std::move(val));
            }
        }
        else
        {
            root_ = std::move(val);
        }
    }

    void begin_array()
    {
        if (++level_ >= stack_.size())
        {
            stack_.push_back(stack_item(array_t));
        }
        else
        {
            stack_[level_-1].type_ = array_t; 
        }
    }

    void end_array()
    {
        json_array<Char>* var = new json_array<Char>(stack_[level_-1].elements_.size());
        for (size_t i = 0; i < var->size(); ++i)
        {
            var->elements_[i].swap(stack_[level_-1].elements_[i]);
        }
        basic_json<Char> val(var);	    
        stack_[level_-1].elements_.clear();
        if (--level_ > 0)
        {
            if (stack_[level_-1].is_object())
            {
                stack_[level_-1].members_.push_back(std::move(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_[level_-1].name_),std::move(val))));
            }
            else
            {
                stack_[level_-1].elements_.push_back(std::move(val));
            }
        }
        else
        {
            root_ = std::move(val);
        }
    }

    void name(const std::basic_string<Char>& name)
    {
        stack_[level_-1].name_ = name;
    }

    void value(const std::basic_string<Char>& value)
    {
        basic_json<Char> val(value);
        if (stack_[level_-1].is_object())
        {
            stack_[level_-1].members_.push_back(std::move(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_[level_-1].name_),std::move(val))));
        } 
        else 
        {
            stack_[level_-1].elements_.push_back(std::move(val));
        }
    }

    void value(double value)
    {
        basic_json<Char> val(value);
        if (stack_[level_-1].is_object())
        {
            stack_[level_-1].members_.push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_[level_-1].name_),std::move(val)));
        } 
        else
        {
            stack_[level_-1].elements_.push_back(val);
        }
    }

    void value(long long value)
    {
        basic_json<Char> val(value);
        if (stack_[level_-1].is_object())
        {
            stack_[level_-1].members_.push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_[level_-1].name_),std::move(val)));
        } 
        else
        {
            stack_[level_-1].elements_.push_back(val);
        }
    }

    void value(unsigned long long value)
    {
        basic_json<Char> val(value);
        if (stack_[level_-1].is_object())
        {
            stack_[level_-1].members_.push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_[level_-1].name_),std::move(val)));
        } 
        else
        {
            stack_[level_-1].elements_.push_back(val);
        }
    }

    void value(bool value)
    {
        basic_json<Char> val(value);
        if (stack_[level_-1].is_object())
        {
            stack_[level_-1].members_.push_back(std::move(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_[level_-1].name_),std::move(val))));
        } 
        else
        {
            stack_[level_-1].elements_.push_back(val);
        }
    }

    void null()
    {
        basic_json<Char> val;
        if (stack_[level_-1].is_object())
        {
            stack_[level_-1].members_.push_back(std::pair<std::basic_string<Char>,basic_json<Char>>(std::move(stack_[level_-1].name_),std::move(val)));
        } 
        else
        {
            stack_[level_-1].elements_.push_back(val);
        }
    }

    void swap_root(basic_json<Char>& root)
    {
        root.swap(root_);
    }

private:
    basic_json<Char> root_;
    std::vector<stack_item> stack_;
    size_t level_;
};

typedef basic_json_content_handler_f<char> json_content_handler_f;

}

#endif
