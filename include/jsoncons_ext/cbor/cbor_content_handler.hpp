// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_CONTENT_HANDLER_HPP
#define JSONCONS_CBOR_CBOR_CONTENT_HANDLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_config.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>

namespace jsoncons { namespace cbor {

class cbor_content_handler : public basic_json_content_handler<char>
{
};

class default_cbor_content_handler : public cbor_content_handler
{
    using super_type = basic_default_json_content_handler<char>;

    bool parse_more_;
public:
    using char_type = char;
    using string_view_type = typename super_type::string_view_type;

    default_cbor_content_handler(bool accept_more = true)
        : parse_more_(accept_more)
    {
    }
private:
    void do_flush() override
    {
    }

    bool do_begin_object(semantic_tag, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_end_object(const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_begin_array(semantic_tag, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_end_array(const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_name(const string_view_type&, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_null_value(semantic_tag, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_string_value(const string_view_type&, semantic_tag, const ser_context&, std::error_code&) override
    {
        return parse_more_;
    }

    bool do_byte_string_value(const byte_string_view&,
                              semantic_tag, 
                              const ser_context&,
                              std::error_code&) override
    {
        return parse_more_;
    }

    bool do_int64_value(int64_t, 
                        semantic_tag, 
                        const ser_context&,
                        std::error_code&) override
    {
        return parse_more_;
    }

    bool do_uint64_value(uint64_t, 
                         semantic_tag, 
                         const ser_context&,
                         std::error_code&) override
    {
        return parse_more_;
    }

    bool do_double_value(double, 
                         semantic_tag,
                         const ser_context&,
                         std::error_code&) override
    {
        return parse_more_;
    }

    bool do_half_value(uint16_t, 
                       semantic_tag,
                       const ser_context&,
                       std::error_code&) override
    {
        return parse_more_;
    }

    bool do_bool_value(bool, 
                       semantic_tag, 
                       const ser_context&, 
                       std::error_code&) override
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const uint8_t>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const uint16_t>&, 
                        semantic_tag, 
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const uint32_t>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const uint64_t>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const int8_t>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const int16_t>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const int32_t>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const int64_t>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(half_arg_t, 
                        const span<const uint16_t>&, 
                        semantic_tag, 
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const float>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const span<const double>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }
/*
    bool do_typed_array(const span<const float128_type>&, 
                        semantic_tag,
                        const ser_context&, 
                        std::error_code&) override 
    {
        return parse_more_;
    }
*/
    bool do_begin_multi_dim(const span<const size_t>&,
                            semantic_tag,
                            const ser_context&, 
                            std::error_code&) override
    {
        return parse_more_;
    }

    bool do_end_multi_dim(const ser_context&,
                       std::error_code&) override
    {
        return parse_more_;
    }
};


}}

#endif
