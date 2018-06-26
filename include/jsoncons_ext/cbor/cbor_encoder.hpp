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
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/detail/writer.hpp>

namespace jsoncons { namespace cbor {

enum class cbor_structure_type {object, array};

template<class CharT,class Writer=jsoncons::detail::ostream_buffered_writer<CharT>>
class basic_cbor_encoder final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Writer writer_type;
    typedef typename Writer::output_type output_type;

private:
    static const size_t default_buffer_length = 16384;

    struct stack_item
    {
        cbor_structure_type type_;
        size_t count_;

        stack_item(cbor_structure_type type)
           : type_(type), count_(0)
        {
        }

        size_t count() const
        {
            return count_;
        }

        bool is_object() const
        {
            return is_object_ == cbor_structure_type::object;
        }

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

    void do_begin_object(const serializing_context& context) override
    {
        stack_.push_back(stack_item(cbor_structure_type::object));
        
        writer_.put('{');
    }

    void do_begin_object(size_t length, const serializing_context& context) override
    {
        stack_.push_back(stack_item(cbor_structure_type::object));

        std::vector<uint8_t> v;
        if (length <= 0x17)
        {
            binary::to_big_endian(static_cast<uint8_t>(static_cast<uint8_t>(0xa0 + length)), v);
        } else if (length <= 0xff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0xb8), v);
            binary::to_big_endian(static_cast<uint8_t>(static_cast<uint8_t>(length)), v);
        } else if (length <= 0xffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0xb9), v);
            binary::to_big_endian(static_cast<uint16_t>(length),v);
        } else if (length <= 0xffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0xba), v);
            binary::to_big_endian(static_cast<uint32_t>(length),v);
        } else if (length <= 0xffffffffffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0xbb), v);
            binary::to_big_endian(static_cast<uint64_t>(length),v);
        }
    }

    void do_end_object(const serializing_context& context) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        stack_.pop_back();
        writer_.put('}');

        end_value();
    }

    void do_end_object(size_t length, const serializing_context& context) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        stack_.pop_back();
        writer_.put('}');

        end_value();
    }

    void do_begin_array(const serializing_context& context) override
    {
        stack_.push_back(stack_item(cbor_structure_type::array));
        writer_.put('[');
    }

    void do_begin_array(size_t length, const serializing_context& context) override
    {
        std::vector<uint8_t> v;
        stack_.push_back(stack_item(cbor_structure_type::array));
        if (length <= 0x17)
        {
            binary::to_big_endian(static_cast<uint8_t>(static_cast<uint8_t>(0x80 + length)), v);
        } 
        else if (length <= 0xff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x98), v);
            binary::to_big_endian(static_cast<uint8_t>(static_cast<uint8_t>(length)), v);
        } 
        else if (length <= 0xffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x99), v);
            binary::to_big_endian(static_cast<uint16_t>(length),v);
        } 
        else if (length <= 0xffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x9a), v);
            binary::to_big_endian(static_cast<uint32_t>(length),v);
        } 
        else if (length <= 0xffffffffffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x9b), v);
            binary::to_big_endian(static_cast<uint64_t>(length),v);
        }
    }

    void do_end_array(const serializing_context& context) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        stack_.pop_back();
        writer_.put(']');
        end_value();
    }

    void do_name(const string_view_type& name, const serializing_context& context) override
    {

        writer_.put('\"');
        // write string
        writer_.put('\"');
        writer_.put(':');
    }

    void do_null_value(const serializing_context& context) override
    {

        auto buf = detail::null_literal<CharT>();
        writer_.write(buf, 4);

        end_value();
    }

    void do_string_value(const string_view_type& value, const serializing_context& context) override
    {

        writer_. put('\"');
        // write string
        writer_. put('\"');

        end_value();
    }

    void do_byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) override
    {
        std::basic_string<CharT> s;
        encode_base64url(data,data+length,s);
        do_string_value(s);
    }

    void do_double_value(double value, const floating_point_options& fmt, const serializing_context& context) override
    {

        // write double

        end_value();
    }

    void do_integer_value(int64_t value, const serializing_context& context) override
    {
        detail::print_integer(value, writer_);
        end_value();
    }

    void do_uinteger_value(uint64_t value, const serializing_context& context) override
    {
        detail::print_uinteger(value, writer_);
        end_value();
    }

    void do_bool_value(bool value, const serializing_context& context) override
    {

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

    void end_value()
    {
        if (!stack_.empty())
        {
            ++stack_.back().count_;
        }
    }
};

typedef basic_cbor_encoder<char,jsoncons::detail::ostream_buffered_writer<char>> cbor_encoder;
typedef basic_cbor_encoder<wchar_t, jsoncons::detail::ostream_buffered_writer<wchar_t>> wcbor_encoder;

}}
#endif
