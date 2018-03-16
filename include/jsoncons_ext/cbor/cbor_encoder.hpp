// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBORENCODER_HPP
#define JSONCONS_CBOR_CBORENCODER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include <limits> // std::numeric_limits
#include <fstream>
#include <memory>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/json_output_handler.hpp>
#include <jsoncons/detail/writer.hpp>

namespace jsoncons { cbor {

template<class CharT,class Writer=detail::ostream_buffered_writer<CharT>>
class basic_cbor_encoder final : public basic_json_output_handler<CharT>
{
public:
    using typename basic_json_output_handler<CharT>::string_view_type;
    typedef Writer writer_type;
    typedef typename Writer::output_type output_type;

private:
    static const size_t default_buffer_length = 16384;

    struct stack_item
    {
        stack_item(bool is_object)
           : is_object_(is_object), count_(0)
        {
        }

        size_t count() const
        {
            return count_;
        }

        bool is_object() const
        {
            return is_object_;
        }

        bool is_object_;
        size_t count_;
    };
    std::vector<stack_item> stack_;
    Writer writer_;

    // Noncopyable and nonmoveable
    basic_cbor_encoder(const basic_cbor_encoder&) = delete;
    basic_cbor_encoder& operator=(const basic_cbor_encoder&) = delete;
public:
    basic_cbor_encoder(output_type& os)
       : writer_(os)
    {
    }

    ~basic_cbor_encoder()
    {
    }

private:
    // Implementing methods

    void do_begin_json() override
    {
    }

    void do_end_json() override
    {
        writer_.flush();
    }

    void do_begin_object() override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            if (!stack_.empty())
            {
                if (stack_.back().count_ > 0)
                {
                    writer_. put(',');
                }
            }
        }
        
        stack_.push_back(stack_item(true));
        
        writer_.put('{');
    }

    void do_end_object() override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (indenting_)
        {
            unindent();
            if (stack_.back().unindent_at_end())
            {
                write_indent();
            }
        }
        stack_.pop_back();
        writer_.put('}');

        end_value();
    }


    void do_begin_array() override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            if (!stack_.empty())
            {
                if (stack_.back().count_ > 0)
                {
                    writer_. put(',');
                }
            }
        }
        stack_.push_back(stack_item(false));
        writer_.put('[');
    }

    void do_end_array() override
    {
        JSONCONS_ASSERT(!stack_.empty());
        stack_.pop_back();
        writer_.put(']');
        end_value();
    }

    void do_name(const string_view_type& name) override
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                writer_. put(',');
            }
        }

        writer_.put('\"');
        // write string
        writer_.put('\"');
        writer_.put(':');
    }

    void do_null_value() override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        auto buf = detail::null_literal<CharT>();
        writer_.write(buf, 4);

        end_value();
    }

    void do_string_value(const string_view_type& value) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        writer_. put('\"');
        // write string
        writer_. put('\"');

        end_value();
    }

    void do_byte_string_value(const uint8_t* data, size_t length) override
    {
        std::basic_string<CharT> s;
        encode_base64url(data,data+length,s);
        do_string_value(s);
    }

    void do_double_value(double value, const number_format& fmt) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        // write double

        end_value();
    }

    void do_integer_value(int64_t value) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }
        detail::print_integer(value, writer_);
        end_value();
    }

    void do_uinteger_value(uint64_t value) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }
        detail::print_uinteger(value, writer_);
        end_value();
    }

    void do_bool_value(bool value) override
    {
        if (!stack_.empty() && !stack_.back().is_object())
        {
            begin_scalar_value();
        }

        if (value)
        {
            auto buf = detail::true_literal<CharT>();
            writer_.write(buf,4);
        }
        else
        {
            auto buf = detail::false_literal<CharT>();
            writer_.write(buf,5);
        }

        end_value();
    }

    void begin_scalar_value()
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                writer_. put(',');
            }
        }
    }

    void begin_value()
    {
        if (!stack_.empty())
        {
            if (stack_.back().count_ > 0)
            {
                writer_. put(',');
            }
        }
    }

    void end_value()
    {
        if (!stack_.empty())
        {
            ++stack_.back().count_;
        }
    }
};

typedef basic_cbor_encoder<char,detail::ostream_buffered_writer<char>> cbor_encoder;
typedef basic_cbor_encoder<wchar_t, detail::ostream_buffered_writer<wchar_t>> wcbor_encoder;

}}
#endif
