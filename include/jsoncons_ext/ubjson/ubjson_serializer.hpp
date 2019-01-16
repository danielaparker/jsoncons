// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_UBJSON_SERIALIZER_HPP
#define JSONCONS_UBJSON_UBJSON_SERIALIZER_HPP

#include <string>
#include <vector>
#include <limits> // std::numeric_limits
#include <memory>
#include <utility> // std::move
#include <jsoncons/json_exception.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_detail.hpp>
#include <jsoncons/result.hpp>
#include <jsoncons/detail/parse_number.hpp>
#include <jsoncons_ext/ubjson/ubjson_detail.hpp>
#include <jsoncons_ext/ubjson/ubjson_error.hpp>

namespace jsoncons { namespace ubjson {

enum class ubjson_container_type {object, indefinite_length_object, array, indefinite_length_array};

template<class CharT,class Result=jsoncons::binary_stream_result>
class basic_ubjson_serializer final : public basic_json_content_handler<CharT>
{

    enum class decimal_parse_state { start, integer, exp1, exp2, fraction1 };
public:
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Result result_type;

private:
    struct stack_item
    {
        ubjson_container_type type_;
        size_t length_;
        size_t count_;

        stack_item(ubjson_container_type type, size_t length = 0)
           : type_(type), length_(length), count_(0)
        {
        }

        size_t length() const
        {
            return length_;
        }

        size_t count() const
        {
            return count_;
        }

        bool is_object() const
        {
            return type_ == ubjson_container_type::object || type_ == ubjson_container_type::indefinite_length_object;
        }

        bool is_indefinite_length() const
        {
            return type_ == ubjson_container_type::indefinite_length_array || type_ == ubjson_container_type::indefinite_length_object;
        }

    };
    std::vector<stack_item> stack_;
    Result result_;

    // Noncopyable and nonmoveable
    basic_ubjson_serializer(const basic_ubjson_serializer&) = delete;
    basic_ubjson_serializer& operator=(const basic_ubjson_serializer&) = delete;
public:
    basic_ubjson_serializer(result_type result)
       : result_(std::move(result))
    {
    }

    ~basic_ubjson_serializer()
    {
        try
        {
            result_.flush();
        }
        catch (...)
        {
        }
    }

private:
    // Implementing methods

    void do_flush() override
    {
        result_.flush();
    }

    bool do_begin_object(semantic_tag_type, const serializing_context&) override
    {
        stack_.push_back(stack_item(ubjson_container_type::indefinite_length_object));
        result_.push_back(ubjson_format::start_object_marker);

        return true;
    }

    bool do_begin_object(size_t length, semantic_tag_type, const serializing_context&) override
    {
        stack_.push_back(stack_item(ubjson_container_type::object, length));
        result_.push_back(ubjson_format::start_object_marker);
        result_.push_back(ubjson_format::count_marker);
        put_length(length);

        return true;
    }

    bool do_end_object(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (stack_.back().is_indefinite_length())
        {
            result_.push_back(ubjson_format::end_object_marker);
        }
        else
        {
            if (stack_.back().count() < stack_.back().length())
            {
                throw serialization_error(ubjson_errc::too_few_items);
            }
            if (stack_.back().count() > stack_.back().length())
            {
                throw serialization_error(ubjson_errc::too_many_items);
            }
        }
        stack_.pop_back();
        end_value();
        return true;
    }

    bool do_begin_array(semantic_tag_type, const serializing_context&) override
    {
        stack_.push_back(stack_item(ubjson_container_type::indefinite_length_array));
        result_.push_back(ubjson_format::start_array_marker);

        return true;
    }

    bool do_begin_array(size_t length, semantic_tag_type, const serializing_context&) override
    {
        stack_.push_back(stack_item(ubjson_container_type::array, length));
        result_.push_back(ubjson_format::start_array_marker);
        result_.push_back(ubjson_format::count_marker);
        put_length(length);

        return true;
    }

    bool do_end_array(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        if (stack_.back().is_indefinite_length())
        {
            result_.push_back(ubjson_format::end_array_marker);
        }
        else
        {
            if (stack_.back().count() < stack_.back().length())
            {
                throw serialization_error(ubjson_errc::too_few_items);
            }
            if (stack_.back().count() > stack_.back().length())
            {
                throw serialization_error(ubjson_errc::too_many_items);
            }
        }
        stack_.pop_back();
        end_value();
        return true;
    }

    bool do_name(const string_view_type& name, const serializing_context&) override
    {
        std::basic_string<uint8_t> target;
        auto result = unicons::convert(
            name.begin(), name.end(), std::back_inserter(target), 
            unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            throw serialization_error(ubjson_errc::invalid_utf8_text_string);
        }

        put_length(target.length());

        for (auto c : target)
        {
            result_.push_back(c);
        }
        return true;
    }

    bool do_null_value(semantic_tag_type, const serializing_context&) override
    {
        // nil
        jsoncons::detail::to_big_endian(static_cast<uint8_t>(ubjson_format::null_type), std::back_inserter(result_));
        end_value();
        return true;
    }

    bool do_string_value(const string_view_type& sv, semantic_tag_type tag, const serializing_context&) override
    {
        switch (tag)
        {
            case semantic_tag_type::big_integer:
            case semantic_tag_type::big_decimal:
            {
                result_.push_back(ubjson_format::high_precision_number_type);
                break;
            }
            default:
            {
                result_.push_back(ubjson_format::string_type);
                break;
            }
        }

        std::basic_string<uint8_t> target;
        auto result = unicons::convert(
            sv.begin(), sv.end(), std::back_inserter(target), 
            unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
        }

        put_length(sv.length());

        for (auto c : target)
        {
            result_.push_back(c);
        }

        end_value();
        return true;
    }

    void put_length(size_t length)
    {
        if (length <= (std::numeric_limits<uint8_t>::max)())
        {
            result_.push_back('U');
            jsoncons::detail::to_big_endian(static_cast<uint8_t>(length), std::back_inserter(result_));
        }
        else if (length <= (size_t)(std::numeric_limits<int16_t>::max)())
        {
            result_.push_back('I');
            jsoncons::detail::to_big_endian(static_cast<uint16_t>(length), std::back_inserter(result_));
        }
        else if (length <= (std::numeric_limits<int32_t>::max)())
        {
            result_.push_back('l');
            jsoncons::detail::to_big_endian(static_cast<uint32_t>(length),std::back_inserter(result_));
        }
        else if (length <= (uint64_t)(std::numeric_limits<int64_t>::max)())
        {
            result_.push_back('L');
            jsoncons::detail::to_big_endian(static_cast<uint32_t>(length),std::back_inserter(result_));
        }
    }

    bool do_byte_string_value(const byte_string_view& b, 
                              semantic_tag_type, 
                              const serializing_context&) override
    {

        const size_t length = b.length();
        result_.push_back(ubjson_format::start_array_marker);
        jsoncons::detail::to_big_endian(static_cast<uint8_t>(ubjson_format::type_marker), std::back_inserter(result_));
        jsoncons::detail::to_big_endian(static_cast<uint8_t>(ubjson_format::uint8_type), std::back_inserter(result_));
        put_length(length);

        for (auto c : b)
        {
            result_.push_back(c);
        }

        end_value();
        return true;
    }

    bool do_double_value(double val, 
                         semantic_tag_type,
                         const serializing_context&) override
    {
        float valf = (float)val;
        if ((double)valf == val)
        {
            // float 32
            result_.push_back(static_cast<uint8_t>(ubjson_format::float32_type));
            jsoncons::detail::to_big_endian(valf,std::back_inserter(result_));
        }
        else
        {
            // float 64
            result_.push_back(static_cast<uint8_t>(ubjson_format::float64_type));
            jsoncons::detail::to_big_endian(val,std::back_inserter(result_));
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
            if (val <= (std::numeric_limits<uint8_t>::max)())
            {
                // uint 8 stores a 8-bit unsigned integer
                result_.push_back(ubjson_format::uint8_type);
                jsoncons::detail::to_big_endian(static_cast<uint8_t>(val),std::back_inserter(result_));
            }
            else if (val <= (std::numeric_limits<int16_t>::max)())
            {
                // uint 16 stores a 16-bit big-endian unsigned integer
                result_.push_back(ubjson_format::int16_type);
                jsoncons::detail::to_big_endian(static_cast<int16_t>(val),std::back_inserter(result_));
            }
            else if (val <= (std::numeric_limits<int32_t>::max)())
            {
                // uint 32 stores a 32-bit big-endian unsigned integer
                result_.push_back(ubjson_format::int32_type);
                jsoncons::detail::to_big_endian(static_cast<int32_t>(val),std::back_inserter(result_));
            }
            else if (val <= (std::numeric_limits<int64_t>::max)())
            {
                // int 64 stores a 64-bit big-endian signed integer
                result_.push_back(ubjson_format::int64_type);
                jsoncons::detail::to_big_endian(static_cast<int64_t>(val),std::back_inserter(result_));
            }
            else
            {
                // big integer
            }
        }
        else
        {
            if (val >= (std::numeric_limits<int8_t>::lowest)())
            {
                // int 8 stores a 8-bit signed integer
                result_.push_back(ubjson_format::int8_type);
                jsoncons::detail::to_big_endian(static_cast<int8_t>(val),std::back_inserter(result_));
            }
            else if (val >= (std::numeric_limits<int16_t>::lowest)())
            {
                // int 16 stores a 16-bit big-endian signed integer
                result_.push_back(ubjson_format::int16_type);
                jsoncons::detail::to_big_endian(static_cast<int16_t>(val),std::back_inserter(result_));
            }
            else if (val >= (std::numeric_limits<int32_t>::lowest)())
            {
                // int 32 stores a 32-bit big-endian signed integer
                result_.push_back(ubjson_format::int32_type);
                jsoncons::detail::to_big_endian(static_cast<int32_t>(val),std::back_inserter(result_));
            }
            else if (val >= (std::numeric_limits<int64_t>::lowest)())
            {
                // int 64 stores a 64-bit big-endian signed integer
                result_.push_back(ubjson_format::int64_type);
                jsoncons::detail::to_big_endian(static_cast<int64_t>(val),std::back_inserter(result_));
            }
        }
        end_value();
        return true;
    }

    bool do_uint64_value(uint64_t val, 
                         semantic_tag_type, 
                         const serializing_context&) override
    {
        if (val <= (std::numeric_limits<uint8_t>::max)())
        {
            result_.push_back(ubjson_format::uint8_type);
            jsoncons::detail::to_big_endian(static_cast<uint8_t>(val),std::back_inserter(result_));
        }
        else if (val <= (std::numeric_limits<int16_t>::max)())
        {
            result_.push_back(ubjson_format::int16_type);
            jsoncons::detail::to_big_endian(static_cast<int16_t>(val),std::back_inserter(result_));
        }
        else if (val <= (std::numeric_limits<int32_t>::max)())
        {
            result_.push_back(ubjson_format::int32_type);
            jsoncons::detail::to_big_endian(static_cast<int32_t>(val),std::back_inserter(result_));
        }
        else if (val <= (uint64_t)(std::numeric_limits<int64_t>::max)())
        {
            result_.push_back(ubjson_format::int64_type);
            jsoncons::detail::to_big_endian(static_cast<int64_t>(val),std::back_inserter(result_));
        }
        end_value();
        return true;
    }

    bool do_bool_value(bool val, semantic_tag_type, const serializing_context&) override
    {
        // true and false
        result_.push_back(static_cast<uint8_t>(val ? ubjson_format::true_type : ubjson_format::false_type));

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

typedef basic_ubjson_serializer<char,jsoncons::binary_stream_result> ubjson_serializer;
typedef basic_ubjson_serializer<char,jsoncons::buffer_result> ubjson_buffer_serializer;

typedef basic_ubjson_serializer<wchar_t,jsoncons::binary_stream_result> wubjson_serializer;
typedef basic_ubjson_serializer<wchar_t,jsoncons::buffer_result> wubjson_buffer_serializer;

}}
#endif
