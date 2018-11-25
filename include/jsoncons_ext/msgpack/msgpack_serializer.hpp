// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MSGPACK_MSGPACK_SERIALIZER_HPP
#define JSONCONS_MSGPACK_MSGPACK_SERIALIZER_HPP

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
#include <jsoncons/config/binary_utilities.hpp>
#include <jsoncons/detail/writer.hpp>
#include <jsoncons/detail/parse_number.hpp>

namespace jsoncons { namespace msgpack {

namespace msgpack_format
{
    const uint8_t positive_fixint_base_cd = 0x00;
    const uint8_t nil_cd = 0xc0;
    const uint8_t false_cd = 0xc2;
    const uint8_t true_cd = 0xc3;
    const uint8_t float32_cd = 0xca;
    const uint8_t float64_cd = 0xcb;
    const uint8_t uint8_cd = 0xcc;
    const uint8_t uint16_cd = 0xcd;
    const uint8_t uint32_cd = 0xce;
    const uint8_t uint64_cd = 0xcf;
    const uint8_t int8_cd = 0xd0;
    const uint8_t int16_cd = 0xd1;
    const uint8_t int32_cd = 0xd2;
    const uint8_t int64_cd = 0xd3;
    const uint8_t fixmap_base_cd = 0x80;
    const uint8_t fixarray_base_cd = 0x90;
    const uint8_t fixstr_base_cd = 0xa0;
    const uint8_t str8_cd = 0xd9;
    const uint8_t str16_cd = 0xda;
    const uint8_t str32_cd = 0xdb;
    const uint8_t bin8_cd = 0xc7;
    const uint8_t bin16_cd = 0xc8;
    const uint8_t bin32_cd = 0xc9;
    const uint8_t array16_cd = 0xdc;
    const uint8_t array32_cd = 0xdd;
    const uint8_t map16_cd = 0xde;
    const uint8_t map32_cd = 0xdf;
    const uint8_t negative_fixint_base_cd = 0xe0;
}

enum class msgpack_structure_type {object, indefinite_length_object, array, indefinite_length_array};

template<class CharT,class Writer=jsoncons::detail::stream_byte_writer>
class basic_msgpack_serializer final : public basic_json_content_handler<CharT>
{

    enum class decimal_parse_state { start, integer, exp1, exp2, fraction1 };
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Writer writer_type;
    typedef typename Writer::output_type output_type;

private:
    struct stack_item
    {
        msgpack_structure_type type_;
        size_t count_;

        stack_item(msgpack_structure_type type)
           : type_(type), count_(0)
        {
        }

        size_t count() const
        {
            return count_;
        }

        bool is_object() const
        {
            return type_ == msgpack_structure_type::object || type_ == msgpack_structure_type::indefinite_length_object;
        }

        bool is_indefinite_length() const
        {
            return type_ == msgpack_structure_type::indefinite_length_array || type_ == msgpack_structure_type::indefinite_length_object;
        }

    };
    std::vector<stack_item> stack_;
    Writer writer_;

    // Noncopyable and nonmoveable
    basic_msgpack_serializer(const basic_msgpack_serializer&) = delete;
    basic_msgpack_serializer& operator=(const basic_msgpack_serializer&) = delete;
public:
    basic_msgpack_serializer(output_type& os)
       : writer_(os)
    {
    }

    ~basic_msgpack_serializer()
    {
        try
        {
            writer_.flush();
        }
        catch (...)
        {
        }
    }

private:
    // Implementing methods

    void do_flush() override
    {
        writer_.flush();
    }

    bool do_begin_object(semantic_tag_type, const serializing_context&) override
    {
        
        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Indefinite object length not supported."));
    }

    bool do_begin_object(size_t length, semantic_tag_type, const serializing_context&) override
    {
        stack_.push_back(stack_item(msgpack_structure_type::object));

        if (length <= 15)
        {
            // fixmap
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::fixmap_base_cd | (length & 0xf)), 
                                  std::back_inserter(writer_));
        }
        else if (length <= 65535)
        {
            // map 16
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::map16_cd), 
                                  std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint16_t>(length), 
                                  std::back_inserter(writer_));
        }
        else if (length <= 4294967295)
        {
            // map 32
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::map32_cd), 
                                  std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint32_t>(length),
                                  std::back_inserter(writer_));
        }

        return true;
    }

    bool do_end_object(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        stack_.pop_back();
        end_value();
        return true;
    }

    bool do_begin_array(semantic_tag_type, const serializing_context&) override
    {
        JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Indefinite array length not supported."));
    }

    bool do_begin_array(size_t length, semantic_tag_type, const serializing_context&) override
    {
        stack_.push_back(stack_item(msgpack_structure_type::array));
        if (length <= 15)
        {
            // fixarray
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::fixarray_base_cd | (length & 0xf)), std::back_inserter(writer_));
        }
        else if (length <= (std::numeric_limits<uint16_t>::max)())
        {
            // array 16
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::array16_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint16_t>(length),std::back_inserter(writer_));
        }
        else if (length <= (std::numeric_limits<uint32_t>::max)())
        {
            // array 32
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::array32_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint32_t>(length),std::back_inserter(writer_));
        }
        return true;
    }

    bool do_end_array(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        stack_.pop_back();
        end_value();
        return true;
    }

    bool do_name(const string_view_type& name, const serializing_context& context) override
    {
        do_string_value(name, semantic_tag_type::none, context);
        return true;
    }

    bool do_null_value(semantic_tag_type, const serializing_context&) override
    {
        // nil
        binary::to_big_endian(static_cast<uint8_t>(msgpack_format::nil_cd), std::back_inserter(writer_));
        end_value();
        return true;
    }

    bool do_string_value(const string_view_type& sv, semantic_tag_type, const serializing_context&) override
    {
        std::basic_string<uint8_t> target;
        auto result = unicons::convert(
            sv.begin(), sv.end(), std::back_inserter(target), 
            unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
        }

        const size_t length = target.length();
        if (length <= 31)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::fixstr_base_cd | length), std::back_inserter(writer_));
        }
        else if (length <= (std::numeric_limits<uint8_t>::max)())
        {
            // str 8 stores a byte array whose length is upto (2^8)-1 bytes
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::str8_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint8_t>(length), std::back_inserter(writer_));
        }
        else if (length <= (std::numeric_limits<uint16_t>::max)())
        {
            // str 16 stores a byte array whose length is upto (2^16)-1 bytes
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::str16_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint16_t>(length), std::back_inserter(writer_));
        }
        else if (length <= (std::numeric_limits<uint32_t>::max)())
        {
            // str 32 stores a byte array whose length is upto (2^32)-1 bytes
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::str32_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint32_t>(length),std::back_inserter(writer_));
        }

        for (auto c : target)
        {
            writer_.push_back(c);
        }

        end_value();
        return true;
    }

    bool do_byte_string_value(const byte_string_view& b, 
                              byte_string_chars_format,
                              semantic_tag_type, 
                              const serializing_context&) override
    {

        const size_t length = b.length();
        if (length <= (std::numeric_limits<uint8_t>::max)())
        {
            // str 8 stores a byte array whose length is upto (2^8)-1 bytes
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::bin8_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint8_t>(length), std::back_inserter(writer_));
        }
        else if (length <= (std::numeric_limits<uint16_t>::max)())
        {
            // str 16 stores a byte array whose length is upto (2^16)-1 bytes
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::bin16_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint16_t>(length), std::back_inserter(writer_));
        }
        else if (length <= (std::numeric_limits<uint32_t>::max)())
        {
            // str 32 stores a byte array whose length is upto (2^32)-1 bytes
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::bin32_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint32_t>(length),std::back_inserter(writer_));
        }

        for (auto c : b)
        {
            writer_.push_back(c);
        }

        end_value();
        return true;
    }

    bool do_double_value(double val, 
                         const floating_point_options&, 
                         semantic_tag_type,
                         const serializing_context&) override
    {
        float valf = (float)val;
        if ((double)valf == val)
        {
            // float 32
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::float32_cd), std::back_inserter(writer_));
            binary::to_big_endian(valf,std::back_inserter(writer_));
        }
        else
        {
            // float 64
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::float64_cd), std::back_inserter(writer_));
            binary::to_big_endian(val,std::back_inserter(writer_));
        }

        // write double

        end_value();
        return true;
    }

    bool do_int64_value(int64_t val, 
                        semantic_tag_type, 
                        const serializing_context&) override
    {
        if (val >= 0)
        {
            if (val <= 0x7f)
            {
                // positive fixnum stores 7-bit positive integer
                binary::to_big_endian(static_cast<uint8_t>(val),std::back_inserter(writer_));
            }
            else if (val <= (std::numeric_limits<uint8_t>::max)())
            {
                // uint 8 stores a 8-bit unsigned integer
                binary::to_big_endian(static_cast<uint8_t>(msgpack_format::uint8_cd), std::back_inserter(writer_));
                binary::to_big_endian(static_cast<uint8_t>(val),std::back_inserter(writer_));
            }
            else if (val <= (std::numeric_limits<uint16_t>::max)())
            {
                // uint 16 stores a 16-bit big-endian unsigned integer
                binary::to_big_endian(static_cast<uint8_t>(msgpack_format::uint16_cd), std::back_inserter(writer_));
                binary::to_big_endian(static_cast<uint16_t>(val),std::back_inserter(writer_));
            }
            else if (val <= (std::numeric_limits<uint32_t>::max)())
            {
                // uint 32 stores a 32-bit big-endian unsigned integer
                binary::to_big_endian(static_cast<uint8_t>(msgpack_format::uint32_cd), std::back_inserter(writer_));
                binary::to_big_endian(static_cast<uint32_t>(val),std::back_inserter(writer_));
            }
            else if (val <= (std::numeric_limits<int64_t>::max)())
            {
                // int 64 stores a 64-bit big-endian signed integer
                binary::to_big_endian(static_cast<uint8_t>(msgpack_format::uint64_cd), std::back_inserter(writer_));
                binary::to_big_endian(static_cast<uint64_t>(val),std::back_inserter(writer_));
            }
        }
        else
        {
            if (val >= -32)
            {
                // negative fixnum stores 5-bit negative integer
                binary::to_big_endian(static_cast<int8_t>(val), std::back_inserter(writer_));
            }
            else if (val >= (std::numeric_limits<int8_t>::lowest)())
            {
                // int 8 stores a 8-bit signed integer
                binary::to_big_endian(static_cast<uint8_t>(msgpack_format::int8_cd), std::back_inserter(writer_));
                binary::to_big_endian(static_cast<int8_t>(val),std::back_inserter(writer_));
            }
            else if (val >= (std::numeric_limits<int16_t>::lowest)())
            {
                // int 16 stores a 16-bit big-endian signed integer
                binary::to_big_endian(static_cast<uint8_t>(msgpack_format::int16_cd), std::back_inserter(writer_));
                binary::to_big_endian(static_cast<int16_t>(val),std::back_inserter(writer_));
            }
            else if (val >= (std::numeric_limits<int32_t>::lowest)())
            {
                // int 32 stores a 32-bit big-endian signed integer
                binary::to_big_endian(static_cast<uint8_t>(msgpack_format::int32_cd), std::back_inserter(writer_));
                binary::to_big_endian(static_cast<int32_t>(val),std::back_inserter(writer_));
            }
            else if (val >= (std::numeric_limits<int64_t>::lowest)())
            {
                // int 64 stores a 64-bit big-endian signed integer
                binary::to_big_endian(static_cast<uint8_t>(msgpack_format::int64_cd), std::back_inserter(writer_));
                binary::to_big_endian(static_cast<int64_t>(val),std::back_inserter(writer_));
            }
        }
        end_value();
        return true;
    }

    bool do_uint64_value(uint64_t val, 
                         semantic_tag_type, 
                         const serializing_context&) override
    {
        if (val <= (std::numeric_limits<int8_t>::max)())
        {
            // positive fixnum stores 7-bit positive integer
            binary::to_big_endian(static_cast<uint8_t>(val), std::back_inserter(writer_));
        }
        else if (val <= (std::numeric_limits<uint8_t>::max)())
        {
            // uint 8 stores a 8-bit unsigned integer
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::uint8_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint8_t>(val), std::back_inserter(writer_));
        }
        else if (val <= (std::numeric_limits<uint16_t>::max)())
        {
            // uint 16 stores a 16-bit big-endian unsigned integer
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::uint16_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint16_t>(val),std::back_inserter(writer_));
        }
        else if (val <= (std::numeric_limits<uint32_t>::max)())
        {
            // uint 32 stores a 32-bit big-endian unsigned integer
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::uint32_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint32_t>(val),std::back_inserter(writer_));
        }
        else if (val <= (std::numeric_limits<uint64_t>::max)())
        {
            // uint 64 stores a 64-bit big-endian unsigned integer
            binary::to_big_endian(static_cast<uint8_t>(msgpack_format::uint64_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint64_t>(val),std::back_inserter(writer_));
        }
        end_value();
        return true;
    }

    bool do_bool_value(bool val, semantic_tag_type, const serializing_context&) override
    {
        // true and false
        writer_.push_back(static_cast<uint8_t>(val ? msgpack_format::true_cd : msgpack_format::false_cd));
        //binary::to_big_endian(static_cast<uint8_t>(val ? msgpack_format::true_cd : msgpack_format::false_cd), std::back_inserter(writer_));

        end_value();
        return true;
    }

    void end_value()
    {
        if (!stack_.empty())
        {
            ++stack_.back().count_;
        }
    }
};

typedef basic_msgpack_serializer<char,jsoncons::detail::stream_byte_writer> msgpack_serializer;

typedef basic_msgpack_serializer<char,jsoncons::detail::bytes_writer> msgpack_bytes_serializer;

}}
#endif
