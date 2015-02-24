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

template <typename Char,class Alloc>
class basic_json_deserializer : public basic_json_input_handler<Char>
{
    struct stack_item
    {
        stack_item(bool is_object, size_t minimum_structure_capacity)
            : is_object_(is_object)
        {
            minimum_structure_capacity_ = minimum_structure_capacity;
            if (is_object_)
            {
                object_ = new json_object_impl<Char,Alloc>();
                object_->reserve(minimum_structure_capacity);
            }
            else
            {
                array_ = new json_array_impl<Char,Alloc>();
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

        json_object_impl<Char,Alloc>* release_object() {json_object_impl<Char,Alloc>* p(0); std::swap(p,object_); return p;}

        json_array_impl<Char,Alloc>* release_array() {json_array_impl<Char,Alloc>* p(0); std::swap(p,array_); return p;}

        std::basic_string<Char> name_;
        bool is_object_;
        json_object_impl<Char,Alloc>* object_;
        json_array_impl<Char,Alloc>* array_;
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

    basic_json<Char,Alloc>& root()
    {
        return root_;
    }

private:

    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
    }

    void do_begin_object(const basic_parsing_context<Char>& context) override
    {
        stack_.push_back(stack_item(true,context.minimum_structure_capacity()));
    }

    void do_end_object(const basic_parsing_context<Char>&) override
    {
        stack_.back().object_->sort_members();
        basic_json<Char,Alloc> val(stack_.back().release_object());	    
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

    void do_begin_array(const basic_parsing_context<Char>& context) override
    {
        stack_.push_back(stack_item(false,context.minimum_structure_capacity()));
    }

    void do_end_array(const basic_parsing_context<Char>&) override
    {
        basic_json<Char,Alloc> val(stack_.back().release_array());	    
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

    void do_name(const Char* p, size_t length, const basic_parsing_context<Char>&) override
    {
        stack_.back().name_ = std::basic_string<Char>(p,length);
    }

    void do_string_value(const Char* p, size_t length, const basic_parsing_context<Char>&) override
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<Char,Alloc>(p,length));
        } 
        else 
        {
            stack_.back().array_->push_back(basic_json<Char,Alloc>(p,length));
        }
    }

    void do_longlong_value(long long value, const basic_parsing_context<Char>&) override
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<Char,Alloc>::make_integer(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char,Alloc>::make_integer(value));
        }
    }

    void do_ulonglong_value(unsigned long long value, const basic_parsing_context<Char>&) override
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<Char,Alloc>::make_unsigned(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char,Alloc>::make_unsigned(value));
        }
    }

    void do_double_value(double value, const basic_parsing_context<Char>&) override
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<Char,Alloc>::make_float(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char,Alloc>::make_float(value));
        }
    }

    void do_bool_value(bool value, const basic_parsing_context<Char>&) override
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<Char,Alloc>(value));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char,Alloc>(value));
        }
    }

    void do_null_value(const basic_parsing_context<Char>&) override
    {
        if (stack_.back().is_object())
        {
            stack_.back().object_->push_back(std::move(stack_.back().name_),basic_json<Char,Alloc>(basic_json<Char,Alloc>::null));
        } 
        else
        {
            stack_.back().array_->push_back(basic_json<Char,Alloc>::null);
        }
    }

	basic_json<Char,Alloc> root_;
    std::vector<stack_item> stack_;
};

typedef basic_json_deserializer<char,std::allocator<void>> json_deserializer;
typedef basic_json_deserializer<wchar_t,std::allocator<void>> wjson_deserializer;

}

#endif
