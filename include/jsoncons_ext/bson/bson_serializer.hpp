// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_SERIALIZER_HPP
#define JSONCONS_BSON_BSON_SERIALIZER_HPP

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
#include <jsoncons_ext/bson/bson_detail.hpp>

namespace jsoncons { namespace bson {

template<class CharT,class Result=jsoncons::binary_stream_result>
class basic_bson_serializer final : public basic_json_content_handler<CharT>
{
    enum class decimal_parse_state { start, integer, exp1, exp2, fraction1 };
public:
    typedef CharT char_type;
    using typename basic_json_content_handler<CharT>::string_view_type;
    typedef Result result_type;

private:
    struct stack_item
    {
        bson_container_type type_;
        size_t offset_;
        size_t name_offset_;
        size_t index_;

        stack_item(bson_container_type type, size_t offset)
           : type_(type), offset_(offset), name_offset_(0), index_(0)
        {
        }

        size_t offset() const
        {
            return offset_;
        }

        size_t member_offset() const
        {
            return name_offset_;
        }

        void member_offset(size_t offset) 
        {
            name_offset_ = offset;
        }

        size_t next_index()
        {
            return index_++;
        }

        bool is_object() const
        {
            return type_ == bson_container_type::document;
        }


    };

    std::vector<stack_item> stack_;
    std::vector<uint8_t> buffer_;
    result_type result_;

    // Noncopyable and nonmoveable
    basic_bson_serializer(const basic_bson_serializer&) = delete;
    basic_bson_serializer& operator=(const basic_bson_serializer&) = delete;
public:
    explicit basic_bson_serializer(result_type result)
       : result_(std::move(result))
    {
    }

    ~basic_bson_serializer()
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
        if (buffer_.size() > 0)
        {
            before_value(bson_format::document_cd);
        }
        stack_.emplace_back(bson_container_type::document, buffer_.size());
        buffer_.insert(buffer_.end(), sizeof(int32_t), 0);

        return true;
    }

    bool do_end_object(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());

        buffer_.push_back(0x00);

        size_t length = buffer_.size() - stack_.back().offset();
        jsoncons::detail::to_little_endian(static_cast<uint32_t>(length), buffer_.begin()+stack_.back().offset());

        stack_.pop_back();
        if (stack_.empty())
        {
            for (auto c : buffer_)
            {
                result_.push_back(c);
            }
        }
        return true;
    }

    bool do_begin_array(semantic_tag_type, const serializing_context&) override
    {
        if (buffer_.size() > 0)
        {
            before_value(bson_format::array_cd);
        }
        stack_.emplace_back(bson_container_type::array, buffer_.size());
        buffer_.insert(buffer_.end(), sizeof(int32_t), 0);
        return true;
    }

    bool do_end_array(const serializing_context&) override
    {
        JSONCONS_ASSERT(!stack_.empty());

        buffer_.push_back(0x00);

        size_t length = buffer_.size() - stack_.back().offset();
        jsoncons::detail::to_little_endian(static_cast<uint32_t>(length), buffer_.begin()+stack_.back().offset());

        stack_.pop_back();
        if (stack_.empty())
        {
            for (auto c : buffer_)
            {
                result_.push_back(c);
            }
        }
        return true;
    }

    bool do_name(const string_view_type& name, const serializing_context&) override
    {
        stack_.back().member_offset(buffer_.size());
        buffer_.push_back(0x00); // reserve space for code
        for (auto c : name)
        {
            buffer_.push_back(c);
        }
        buffer_.push_back(0x00);
        return true;
    }

    bool do_null_value(semantic_tag_type, const serializing_context&) override
    {
        before_value(bson_format::null_cd);
        return true;
    }

    bool do_bool_value(bool val, semantic_tag_type, const serializing_context&) override
    {
        before_value(bson_format::bool_cd);
        if (val)
        {
            buffer_.push_back(0x01);
        }
        else
        {
            buffer_.push_back(0x00);
        }

        return true;
    }

    bool do_string_value(const string_view_type& sv, semantic_tag_type, const serializing_context&) override
    {
        before_value(bson_format::string_cd);

        size_t offset = buffer_.size();
        buffer_.insert(buffer_.end(), sizeof(int32_t), 0);
        size_t string_offset = buffer_.size();
        auto result = unicons::convert(
            sv.begin(), sv.end(), std::back_inserter(buffer_), 
            unicons::conv_flags::strict);
        if (result.ec != unicons::conv_errc())
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Illegal unicode"));
        }
        buffer_.push_back(0x00);
        size_t length = buffer_.size() - string_offset;
        jsoncons::detail::to_little_endian(static_cast<uint32_t>(length), buffer_.begin()+offset);

        return true;
    }

    bool do_byte_string_value(const byte_string_view& b, 
                              semantic_tag_type, 
                              const serializing_context&) override
    {
        before_value(bson_format::binary_cd);

        size_t offset = buffer_.size();
        buffer_.insert(buffer_.end(), sizeof(int32_t), 0);
        size_t string_offset = buffer_.size();

        for (auto c : b)
        {
            buffer_.push_back(c);
        }
        size_t length = buffer_.size() - string_offset;
        jsoncons::detail::to_little_endian(static_cast<uint32_t>(length), buffer_.begin()+offset);

        return true;
    }

    bool do_int64_value(int64_t val, 
                        semantic_tag_type tag, 
                        const serializing_context&) override
    {
        if (tag == semantic_tag_type::timestamp)
        {
            before_value(bson_format::datetime_cd);
        }
        else
        {
            before_value(bson_format::int64_cd);
        }
        if (val >= (std::numeric_limits<int32_t>::lowest)() && val <= (std::numeric_limits<int32_t>::max)())
        {
            jsoncons::detail::to_little_endian(static_cast<uint32_t>(val),std::back_inserter(buffer_));
        }
        else if (val >= (std::numeric_limits<int64_t>::lowest)() && val <= (std::numeric_limits<int64_t>::max)())
        {
            jsoncons::detail::to_little_endian(static_cast<int64_t>(val),std::back_inserter(buffer_));
        }
        else
        {
            // error
        }

        return true;
    }

    bool do_uint64_value(uint64_t val, 
                         semantic_tag_type tag, 
                         const serializing_context&) override
    {
        if (tag == semantic_tag_type::timestamp)
        {
            before_value(bson_format::datetime_cd);
        }
        else
        {
            before_value(bson_format::int64_cd);
        }
        if (val <= (std::numeric_limits<int32_t>::max)())
        {
            jsoncons::detail::to_little_endian(static_cast<uint32_t>(val),std::back_inserter(buffer_));
        }
        else if (val <= (uint64_t)(std::numeric_limits<int64_t>::max)())
        {
            jsoncons::detail::to_little_endian(static_cast<uint64_t>(val),std::back_inserter(buffer_));
        }
        else
        {
            // error
        }

        return true;
    }

    bool do_double_value(double val, 
                         semantic_tag_type,
                         const serializing_context&) override
    {
        before_value(bson_format::double_cd);

        jsoncons::detail::to_little_endian(val,std::back_inserter(buffer_));

        return true;
    }

    void before_value(uint8_t code) 
    {
        if (stack_.back().is_object())
        {
            buffer_[stack_.back().member_offset()] = code;
        }
        else
        {
            buffer_.push_back(code);
            std::string name = std::to_string(stack_.back().next_index());
            buffer_.insert(buffer_.end(), name.begin(), name.end());
            buffer_.push_back(0x00);
        }
    }
};

typedef basic_bson_serializer<char,jsoncons::binary_stream_result> bson_serializer;
typedef basic_bson_serializer<char,jsoncons::buffer_result> bson_buffer_serializer;

typedef basic_bson_serializer<wchar_t,jsoncons::binary_stream_result> wbson_serializer;
typedef basic_bson_serializer<wchar_t,jsoncons::buffer_result> wbson_buffer_serializer;

}}
#endif
