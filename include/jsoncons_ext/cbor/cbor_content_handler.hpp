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

template <class Float128T=void>
class cbor_content_handler : public basic_json_content_handler<char>
{
public:
    using super_type = basic_json_content_handler<char>;
public:
    using char_type = char;
    using string_view_type = typename super_type::string_view_type;

    bool typed_array(const Float128T* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const uint8_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const uint16_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const uint32_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const uint64_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const int8_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const int16_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const int32_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const int64_t* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const float* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }

    bool typed_array(const double* data, size_t size, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context_arg)
    {
        return do_typed_array(data, size, tag, context);
    }
private:
    virtual bool do_typed_array(const uint8_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const uint16_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const uint32_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const uint64_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const int8_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const int16_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const int32_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const int64_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const float* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const double* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) = 0;

    virtual bool do_typed_array(const Float128T* /*data*/, size_t /*size*/, 
                                semantic_tag /*tag*/,
                                const ser_context& /*context*/) = 0;
};

template <class Float128T=void>
class default_cbor_content_handler : public cbor_content_handler<Float128T>
{
    using super_type = basic_default_json_content_handler<char>;

    bool parse_more_;
public:
    using char_type = char;
    using string_view_type = typename super_type::string_view_type;

    default_cbor_content_handler(bool parse_more = true)
        : parse_more_(parse_more)
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

    bool do_string_value(const string_view_type&, semantic_tag, const ser_context&) override
    {
        return parse_more_;
    }

    bool do_byte_string_value(const byte_string_view&,
                              semantic_tag, 
                              const ser_context&) override
    {
        return parse_more_;
    }

    bool do_int64_value(int64_t, 
                        semantic_tag, 
                        const ser_context&) override
    {
        return parse_more_;
    }

    bool do_uint64_value(uint64_t, 
                         semantic_tag, 
                         const ser_context&) override
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

    bool do_bool_value(bool, semantic_tag, const ser_context&) override
    {
        return parse_more_;
    }

    bool do_typed_array(const uint8_t*, size_t, 
                        semantic_tag,
                        const ser_context&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const uint16_t*, size_t, semantic_tag, const ser_context&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const uint32_t*, size_t, 
                        semantic_tag,
                        const ser_context&) override
    {
        return parse_more_;
    }

    bool do_typed_array(const uint64_t*, size_t, 
                        semantic_tag,
                        const ser_context&) override
    {
        return parse_more_;
    }

    bool do_typed_array(const int8_t*, size_t, 
                        semantic_tag,
                        const ser_context&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const int16_t*, size_t, 
                        semantic_tag,
                        const ser_context&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const int32_t*, size_t, 
                        semantic_tag,
                        const ser_context&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const int64_t*, size_t, 
                        semantic_tag,
                        const ser_context&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const float*, size_t, 
                        semantic_tag,
                        const ser_context&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const double*, size_t, 
                        semantic_tag,
                        const ser_context&) override 
    {
        return parse_more_;
    }

    bool do_typed_array(const Float128T*, size_t, 
                        semantic_tag,
                        const ser_context&) override 
    {
        return parse_more_;
    }
};

template <class Float128T=void>
class cbor_to_json_content_handler_adaptor : public cbor_content_handler<Float128T>
{
    using super_type = cbor_content_handler<Float128T>;
public:
    using char_type = char;
    using string_view_type = typename super_type::string_view_type;
    using super_type::typed_array;
private:
    basic_json_content_handler<char_type>& to_handler_;
public:
    cbor_to_json_content_handler_adaptor(basic_json_content_handler<char>& handler)
        : to_handler_(handler)
    {
    }
private:
    void do_flush() override
    {
        to_handler_.flush();
    }

    bool do_begin_object(semantic_tag tag, const ser_context& context, std::error_code& ec) override
    {
        return to_handler_.begin_object(tag, context, ec);
    }

    bool do_begin_object(size_t length, semantic_tag tag, const ser_context& context, std::error_code&) override
    {
        return to_handler_.begin_object(length, tag, context);
    }

    bool do_end_object(const ser_context& context, std::error_code& ec) override
    {
        return to_handler_.end_object(context, ec);
    }

    bool do_begin_array(semantic_tag tag, const ser_context& context, std::error_code& ec) override
    {
        return to_handler_.begin_array(tag, context, ec);
    }

    bool do_begin_array(size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec) override
    {
        return to_handler_.begin_array(length, tag, context, ec);
    }

    bool do_end_array(const ser_context& context, std::error_code& ec) override
    {
        return to_handler_.end_array(context, ec);
    }

    bool do_name(const string_view_type& name,
                 const ser_context& context,
                 std::error_code& ec) override
    {
        return to_handler_.name(name, context, ec);
    }

    bool do_string_value(const string_view_type& value,
                         semantic_tag tag,
                         const ser_context& context) override
    {
        return to_handler_.string_value(value, tag, context);
    }

    bool do_byte_string_value(const byte_string_view& b, 
                              semantic_tag tag,
                              const ser_context& context) override
    {
        return to_handler_.byte_string_value(b, tag, context);
    }

    bool do_double_value(double value, 
                         semantic_tag tag,
                         const ser_context& context,
                         std::error_code&) override
    {
        return to_handler_.double_value(value, tag, context);
    }

    bool do_int64_value(int64_t value,
                        semantic_tag tag,
                        const ser_context& context) override
    {
        return to_handler_.int64_value(value, tag, context);
    }

    bool do_uint64_value(uint64_t value,
                         semantic_tag tag,
                         const ser_context& context) override
    {
        return to_handler_.uint64_value(value, tag, context);
    }

    bool do_bool_value(bool value, semantic_tag tag, const ser_context& context) override
    {
        return to_handler_.bool_value(value, tag, context);
    }

    bool do_null_value(semantic_tag tag, const ser_context& context, std::error_code& ec) override
    {
        return to_handler_.null_value(tag, context, ec);
    }

    bool do_typed_array(const uint8_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) override
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.uint64_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const uint16_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) override
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.uint64_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const uint32_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) override
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.uint64_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const uint64_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) override
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.uint64_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const int8_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) override
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.int64_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const int16_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) override
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.int64_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const int32_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) override
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.int64_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const int64_t* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) override
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.int64_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const float* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context) override
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.double_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const double* data, size_t size, 
                                semantic_tag tag,
                                const ser_context& context)
    {
        bool more = to_handler_.begin_array(tag,context);
        for (auto p = data; more && p < data+size; ++p)
        {
            more = to_handler_.double_value(*p,semantic_tag::none,context);
        }
        if (more)
        {
            more = to_handler_.end_array(context);
        }
        return more;
    }

    bool do_typed_array(const Float128T* /*data*/, size_t /*size*/, 
                        semantic_tag /*tag*/,
                        const ser_context& /*context*/) override
    {
        return true;
    }
};

}}

#endif
