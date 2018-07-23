// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBORSERIALIZER_HPP
#define JSONCONS_CBOR_CBORSERIALIZER_HPP

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
#include <jsoncons/binary/binary_utilities.hpp>
#include <jsoncons/detail/writer.hpp>

namespace jsoncons { namespace cbor {

enum class cbor_structure_type {object, indefinite_length_object, array, indefinite_length_array};

template<class CharT,class Writer=jsoncons::detail::ostream_buffered_writer<CharT>>
class basic_cbor_serializer final : public basic_json_content_handler<CharT>
{
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Writer writer_type;
    typedef typename Writer::output_type output_type;

private:
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
            return type_ == cbor_structure_type::object || type_ == cbor_structure_type::indefinite_length_object;
        }

        bool is_indefinite_length() const
        {
            return type_ == cbor_structure_type::indefinite_length_array || type_ == cbor_structure_type::indefinite_length_object;
        }

    };
    std::vector<stack_item> stack_;
    Writer writer_;

    // Noncopyable and nonmoveable
    basic_cbor_serializer(const basic_cbor_serializer&) = delete;
    basic_cbor_serializer& operator=(const basic_cbor_serializer&) = delete;
public:
    basic_cbor_serializer(output_type& os)
       : writer_(os)
    {
    }

    ~basic_cbor_serializer()
    {
    }

private:
    // Implementing methods

    void do_begin_document() override
    {
    }

    void do_end_document() override
    {
        writer_.flush();
    }

    void do_begin_object(const serializing_context& context) override
    {
        stack_.push_back(stack_item(cbor_structure_type::indefinite_length_object));
        
        writer_.put((CharT)0xbf);
    }

    void do_begin_object(size_t length, const serializing_context& context) override
    {
        stack_.push_back(stack_item(cbor_structure_type::object));

        std::vector<uint8_t> v;
        if (length <= 0x17)
        {
            binary::to_big_endian(static_cast<uint8_t>(0xa0 + length), v);
        } else if (length <= 0xff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0xb8), v);
            binary::to_big_endian(static_cast<uint8_t>(length), v);
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

        for (auto c : v)
        {
            writer_.put(c);
        }
    }

    void do_end_object(const serializing_context& context) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (stack_.back().is_indefinite_length())
        {
            writer_.put((CharT)0xff);
        }
        stack_.pop_back();

        end_value();
    }

    void do_begin_array(const serializing_context& context) override
    {
        stack_.push_back(stack_item(cbor_structure_type::indefinite_length_array));
        writer_.put((CharT)0x9f);
    }

    void do_begin_array(size_t length, const serializing_context& context) override
    {
        std::vector<uint8_t> v;
        stack_.push_back(stack_item(cbor_structure_type::array));
        if (length <= 0x17)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x80 + length), v);
        } 
        else if (length <= 0xff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x98), v);
            binary::to_big_endian(static_cast<uint8_t>(length), v);
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
        for (auto c : v)
        {
            writer_.put(c);
        }
    }

    void do_end_array(const serializing_context& context) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (stack_.back().is_indefinite_length())
        {
            writer_.put((CharT)0xff);
        }
        stack_.pop_back();
        end_value();
    }

    void do_name(const string_view_type& name, const serializing_context& context) override
    {
        do_string_value(name,context);
    }

    void do_null_value(const serializing_context& context) override
    {
        writer_.put((CharT)0xf6);

        end_value();
    }

    void do_string_value(const string_view_type& sv, const serializing_context& context) override
    {
        std::vector<uint8_t> v;
        std::basic_string<uint8_t> target;
        auto result = unicons::convert(
            sv.begin(), sv.end(), std::back_inserter(target), 
            unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
        }

        const size_t length = target.length();
        if (length <= 0x17)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            binary::to_big_endian(static_cast<uint8_t>(0x60 + length), v);
        }
        else if (length <= 0xff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x78), v);
            binary::to_big_endian(static_cast<uint8_t>(length), v);
        }
        else if (length <= 0xffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x79), v);
            binary::to_big_endian(static_cast<uint16_t>(length), v);
        }
        else if (length <= 0xffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x7a), v);
            binary::to_big_endian(static_cast<uint32_t>(length), v);
        }
        else if (length <= 0xffffffffffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x7b), v);
            binary::to_big_endian(static_cast<uint64_t>(length),v);
        }

        for (size_t i = 0; i < length; ++i)
        {
            binary::to_big_endian(static_cast<uint8_t>(target.data()[i]), v);
        }
        for (auto c : v)
        {
            writer_.put(c);
        }

        end_value();
    }

    void do_byte_string_value(const uint8_t* data, size_t length, const serializing_context& context) override
    {
        std::vector<uint8_t> v;

        if (length <= 0x17)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            binary::to_big_endian(static_cast<uint8_t>(0x40 + length), v);
        }
        else if (length <= 0xff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x58), v);
            binary::to_big_endian(static_cast<uint8_t>(length), v);
        }
        else if (length <= 0xffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x59), v);
            binary::to_big_endian(static_cast<uint16_t>(length), v);
        }
        else if (length <= 0xffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x5a), v);
            binary::to_big_endian(static_cast<uint32_t>(length), v);
        }
        else if (length <= 0xffffffffffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x5b), v);
            binary::to_big_endian(static_cast<uint64_t>(length),v);
        }

        for (size_t i = 0; i < length; ++i)
        {
            binary::to_big_endian(static_cast<uint8_t>(data[i]), v);
        }
        for (auto c : v)
        {
            writer_.put(c);
        }

        end_value();
    }

    void do_bignum_value(int signum, const uint8_t* data, size_t length, const serializing_context& context) override
    {
        std::vector<uint8_t> v;

        if (signum == -1)
        {
            v.push_back(0xc3);
        }
        else
        {
            v.push_back(0xc2);
        }

        if (length <= 0x17)
        {
            // fixstr stores a byte array whose length is upto 31 bytes
            binary::to_big_endian(static_cast<uint8_t>(0x40 + length), v);
        }
        else if (length <= 0xff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x58), v);
            binary::to_big_endian(static_cast<uint8_t>(length), v);
        }
        else if (length <= 0xffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x59), v);
            binary::to_big_endian(static_cast<uint16_t>(length), v);
        }
        else if (length <= 0xffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x5a), v);
            binary::to_big_endian(static_cast<uint32_t>(length), v);
        }
        else if (length <= 0xffffffffffffffff)
        {
            binary::to_big_endian(static_cast<uint8_t>(0x5b), v);
            binary::to_big_endian(static_cast<uint64_t>(length),v);
        }

        for (size_t i = 0; i < length; ++i)
        {
            v.push_back(data[i]);
            //binary::to_big_endian(static_cast<uint8_t>(data[i]), v);
        }

        for (auto c : v)
        {
            writer_.put(c);
        }

        end_value();
    }

    void do_double_value(double value, const floating_point_options& fmt, const serializing_context& context) override
    {
        std::vector<uint8_t> v;
        binary::to_big_endian(static_cast<uint8_t>(0xfb), v);
        binary::to_big_endian(value,v);
        for (auto c : v)
        {
            writer_.put(c);
        }

        // write double

        end_value();
    }

    void do_integer_value(int64_t value, const serializing_context& context) override
    {
        std::vector<uint8_t> v;
        if (value >= 0)
        {
            if (value <= 0x17)
            {
                binary::to_big_endian(static_cast<uint8_t>(value), v);
            } 
            else if (value <= (std::numeric_limits<uint8_t>::max)())
            {
                binary::to_big_endian(static_cast<uint8_t>(0x18), v);
                binary::to_big_endian(static_cast<uint8_t>(value), v);
            } 
            else if (value <= (std::numeric_limits<uint16_t>::max)())
            {
                binary::to_big_endian(static_cast<uint8_t>(0x19), v);
                binary::to_big_endian(static_cast<uint16_t>(value), v);
            } 
            else if (value <= (std::numeric_limits<uint32_t>::max)())
            {
                binary::to_big_endian(static_cast<uint8_t>(0x1a), v);
                binary::to_big_endian(static_cast<uint32_t>(value), v);
            } 
            else if (value <= (std::numeric_limits<int64_t>::max)())
            {
                binary::to_big_endian(static_cast<uint8_t>(0x1b), v);
                binary::to_big_endian(static_cast<int64_t>(value), v);
            }
        } else
        {
            const auto posnum = -1 - value;
            if (value >= -24)
            {
                binary::to_big_endian(static_cast<uint8_t>(0x20 + posnum), v);
            } 
            else if (posnum <= (std::numeric_limits<uint8_t>::max)())
            {
                binary::to_big_endian(static_cast<uint8_t>(0x38), v);
                binary::to_big_endian(static_cast<uint8_t>(posnum), v);
            } 
            else if (posnum <= (std::numeric_limits<uint16_t>::max)())
            {
                binary::to_big_endian(static_cast<uint8_t>(0x39), v);
                binary::to_big_endian(static_cast<uint16_t>(posnum), v);
            } 
            else if (posnum <= (std::numeric_limits<uint32_t>::max)())
            {
                binary::to_big_endian(static_cast<uint8_t>(0x3a), v);
                binary::to_big_endian(static_cast<uint32_t>(posnum), v);
            } 
            else if (posnum <= (std::numeric_limits<int64_t>::max)())
            {
                binary::to_big_endian(static_cast<uint8_t>(0x3b), v);
                binary::to_big_endian(static_cast<int64_t>(posnum), v);
            }
        }
        for (auto c : v)
        {
            writer_.put(c);
        }
        end_value();
    }

    void do_uinteger_value(uint64_t value, const serializing_context& context) override
    {
        std::vector<uint8_t> v;
        if (value <= 0x17)
        {
            binary::to_big_endian(static_cast<uint8_t>(value),v);
        } else if (value <=(std::numeric_limits<uint8_t>::max)())
        {
            binary::to_big_endian(static_cast<uint8_t>(0x18), v);
            binary::to_big_endian(static_cast<uint8_t>(value),v);
        } else if (value <=(std::numeric_limits<uint16_t>::max)())
        {
            binary::to_big_endian(static_cast<uint8_t>(0x19), v);
            binary::to_big_endian(static_cast<uint16_t>(value),v);
        } else if (value <=(std::numeric_limits<uint32_t>::max)())
        {
            binary::to_big_endian(static_cast<uint8_t>(0x1a), v);
            binary::to_big_endian(static_cast<uint32_t>(value),v);
        } else if (value <=(std::numeric_limits<uint64_t>::max)())
        {
            binary::to_big_endian(static_cast<uint8_t>(0x1b), v);
            binary::to_big_endian(static_cast<uint64_t>(value),v);
        }
        for (auto c : v)
        {
            writer_.put(c);
        }
        end_value();
    }

    void do_bool_value(bool value, const serializing_context& context) override
    {

        if (value)
        {
            writer_.put((CharT)0xf5);
        }
        else
        {
            writer_.put((CharT)0xf4);
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

typedef basic_cbor_serializer<char,jsoncons::detail::ostream_buffered_writer<char>> cbor_serializer;

typedef basic_cbor_serializer<char,jsoncons::detail::bytes_writer<uint8_t>> cbor_bytes_serializer;

}}
#endif
