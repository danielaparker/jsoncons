// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_SERIALIZER_HPP
#define JSONCONS_BSON_BSON_SERIALIZER_HPP

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
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons/detail/writer.hpp>
#include <jsoncons/detail/parse_number.hpp>
#include <jsoncons_ext/bson/bson_detail.hpp>

namespace jsoncons { namespace bson {

template<class CharT,class Writer=jsoncons::detail::stream_byte_writer>
class basic_bson_serializer final : public basic_json_content_handler<CharT>
{

    enum class decimal_parse_state { start, integer, exp1, exp2, fraction1 };
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Writer writer_type;
    typedef typename Writer::output_type output_type;

private:
    struct stack_item
    {
        bson_structure_type type_;
        size_t count_;

        stack_item(bson_structure_type type)
           : type_(type), count_(0)
        {
        }

        size_t count() const
        {
            return count_;
        }

        bool is_object() const
        {
            return type_ == bson_structure_type::object || type_ == bson_structure_type::indefinite_length_object;
        }

        bool is_indefinite_length() const
        {
            return type_ == bson_structure_type::indefinite_length_array || type_ == bson_structure_type::indefinite_length_object;
        }

    };
    std::vector<stack_item> stack_;
    Writer writer_;

    // Noncopyable and nonmoveable
    basic_bson_serializer(const basic_bson_serializer&) = delete;
    basic_bson_serializer& operator=(const basic_bson_serializer&) = delete;
public:
    basic_bson_serializer(output_type& os)
       : writer_(os)
    {
    }

    ~basic_bson_serializer()
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
        stack_.push_back(stack_item(bson_structure_type::object));

        if (length <= std::numeric_limits<int32_t>::max())
        {
            // map 32
            binary::to_big_endian(static_cast<uint8_t>(bson_format::document_cd), 
                                  std::back_inserter(writer_));
            binary::to_big_endian(static_cast<int32_t>(length),
                                  std::back_inserter(writer_));
            writer_.push_back(0x00);
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
        if (length <= (std::numeric_limits<int32_t>::max)())
        {
            // array 32
            binary::to_big_endian(static_cast<uint8_t>(bson_format::array_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<int32_t>(length),std::back_inserter(writer_));
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
        binary::to_big_endian(static_cast<uint8_t>(bson_format::null_cd), std::back_inserter(writer_));
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
        if (length <= (std::numeric_limits<int32_t>::max)())
        {
            // str 32 stores a byte array whose length is upto (2^32)-1 bytes
            binary::to_big_endian(static_cast<uint8_t>(bson_format::string_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint32_t>(length),std::back_inserter(writer_));
        }

        for (auto c : target)
        {
            writer_.push_back(c);
        }
        writer_.push_back(0x00);

        end_value();
        return true;
    }

    bool do_byte_string_value(const byte_string_view& b, 
                              byte_string_chars_format,
                              semantic_tag_type, 
                              const serializing_context&) override
    {

        const size_t length = b.length();
        if (length <= (std::numeric_limits<int32_t>::max)())
        {
            // str 32 stores a byte array whose length is upto (2^32)-1 bytes
            binary::to_big_endian(static_cast<uint8_t>(bson_format::binary_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<int32_t>(length),std::back_inserter(writer_));
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
        binary::to_big_endian(static_cast<uint8_t>(bson_format::double_cd), std::back_inserter(writer_));
        binary::to_big_endian(val,std::back_inserter(writer_));

        end_value();
        return true;
    }

    bool do_int64_value(int64_t val, 
                        semantic_tag_type, 
                        const serializing_context&) override
    {
        if (val <= (std::numeric_limits<int32_t>::max)
            && val <= (std::numeric_limits<int32_t>::max))
        {
            binary::to_big_endian(static_cast<uint8_t>(bson_format::uint32_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint32_t>(val),std::back_inserter(writer_));
        }
        else if ()
        {
            binary::to_big_endian(static_cast<uint8_t>(bson_format::int64_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<int64_t>(val),std::back_inserter(writer_));
        }
        end_value();
        return true;
    }

    bool do_uint64_value(uint64_t val, 
                         semantic_tag_type, 
                         const serializing_context&) override
    {
        if (val <= (std::numeric_limits<int32_t>::max)
            &&  <= (std::numeric_limits<int32_t>::max))
        {
            binary::to_big_endian(static_cast<uint8_t>(bson_format::uint32_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<uint32_t>(val),std::back_inserter(writer_));
        }
        else if ()
        {
            binary::to_big_endian(static_cast<uint8_t>(bson_format::int64_cd), std::back_inserter(writer_));
            binary::to_big_endian(static_cast<int64_t>(val),std::back_inserter(writer_));
        }
        end_value();
        return true;
    }

    bool do_bool_value(bool val, semantic_tag_type, const serializing_context&) override
    {
        // true and false
        writer_.push_back(static_cast<uint8_t>(val ? bson_format::true_cd : bson_format::false_cd));
        //binary::to_big_endian(static_cast<uint8_t>(val ? bson_format::true_cd : bson_format::false_cd), std::back_inserter(writer_));

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

typedef basic_bson_serializer<char,jsoncons::detail::stream_byte_writer> bson_serializer;

typedef basic_bson_serializer<char,jsoncons::detail::bytes_writer> bson_bytes_serializer;

}}
#endif
