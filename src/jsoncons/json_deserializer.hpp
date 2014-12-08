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
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json1.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/json_structures.hpp"

namespace jsoncons {

template <typename CharT,class Alloc>
class basic_json_deserializer : public basic_json_input_handler<CharT>
{
    struct stack_item
    {
        stack_item(bool is_object, size_t minimum_structure_capacity)
            : is_object_(is_object)
        {
            minimum_structure_capacity_ = minimum_structure_capacity;
            if (is_object_)
            {
                object_ = new json_object<CharT,Alloc>();
                object_->reserve(minimum_structure_capacity);
            }
            else
            {
                array_ = new json_array<CharT,Alloc>();
                array_->reserve(minimum_structure_capacity);
            }
        }

        void destroy()
        {
            try
            {
                if (is_object_)
                {
                    delete object_;
                }
                else
                {
                    delete array_;
                }
            }
            catch (...)
            {
                // no throw
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

        json_object<CharT,Alloc>* release_object() {json_object<CharT,Alloc>* p(0); std::swap(p,object_); return p;}

        json_array<CharT,Alloc>* release_array() {json_array<CharT,Alloc>* p(0); std::swap(p,array_); return p;}

        std::basic_string<CharT> name_;
        bool is_object_;
        json_object<CharT,Alloc>* object_;
        json_array<CharT,Alloc>* array_;
        size_t minimum_structure_capacity_;
    };

public:
    virtual ~basic_json_deserializer()
    {
        for (size_t i = 0; i < stack_.size(); ++i)
        {
            stack_[i].destroy();
        }
    }

    virtual void begin_json()
    {
    }

    virtual void end_json()
    {
    }

    virtual void begin_object(const basic_parsing_context<CharT>& context)
    {
        stack_.push_back(stack_item(true,context.minimum_structure_capacity()));
    }

    virtual void end_object(const basic_parsing_context<CharT>&)
    {
        stack_.back().object_->sort_members();
        basic_json<CharT,Alloc> val(stack_.back().release_object());	    
        stack_.pop_back();
        if (stack_.size() > 0)
        {
            if (stack_.back().is_object())
            {
                stack_.back().object_->push_back(std::move(stack_.back().name_),std::move(val));
            }
            else
            {
                stack_.back().array_->push_back(std::move(val));
            }
        }
        else
        {
            root_.swap(val);
        }
    }

    virtual void begin_array(const basic_parsing_context<CharT>& context)
    {
        stack_.push_back(stack_item(false,context.minimum_structure_capacity()));
    }

    virtual void end_array(const basic_parsing_context<CharT>&)
    {
        basic_json<CharT,Alloc> val(stack_.back().release_array());	    
        stack_.pop_back();
        if (stack_.size() > 0)
        {
            if (stack_.back().is_object())
            {
                stack_.back().object_->push_back(std::move(stack_.back().name_),std::move(val));
            }
            else
            {
                stack_.back().array_->push_back(std::move(val));
            }
        }
        else
        {
            root_ = std::move(val);
        }
    }

    virtual void name(const std::basic_string<CharT>& name, const basic_parsing_context<CharT>&)
    {
        stack_.back().name_ = name;
    }

    virtual void null_value(const basic_parsing_context<CharT>&)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<CharT,Alloc>(basic_json<CharT,Alloc>::null));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<CharT,Alloc>::null);
        }
    }

    basic_json<CharT,Alloc>& root()
    {
        return root_;
    }

// value(...) implementation

    virtual void string_value(const CharT* value, size_t length, const basic_parsing_context<CharT>&)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<CharT,Alloc>(value,length));
        } 
        else 
        {
            stack_.back().array_->push_back(basic_json<CharT,Alloc>(value,length));
        }
    }

    virtual void double_value(double value, const basic_parsing_context<CharT>&)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<CharT,Alloc>(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<CharT,Alloc>(value));
        }
    }

    virtual void longlong_value(long long value, const basic_parsing_context<CharT>&)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<CharT,Alloc>(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<CharT,Alloc>(value));
        }
    }

    virtual void ulonglong_value(unsigned long long value, const basic_parsing_context<CharT>&)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<CharT,Alloc>(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<CharT,Alloc>(value));
        }
    }

    virtual void bool_value(bool value, const basic_parsing_context<CharT>&)
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<CharT,Alloc>(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<CharT,Alloc>(value));
        }
    }

private:
	basic_json<CharT,Alloc> root_;
    std::vector<stack_item> stack_;
};

typedef basic_json_deserializer<char,std::allocator<char>> json_deserializer;
typedef basic_json_deserializer<wchar_t,std::allocator<wchar_t>> wjson_deserializer;

}

#endif
