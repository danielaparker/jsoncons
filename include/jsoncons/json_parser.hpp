// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_PARSER_HPP
#define JSONCONS_JSON_PARSER_HPP

#include <cstddef>
#include <cstdint>
#include <functional> // std::function
#include <limits> // std::numeric_limits
#include <memory> // std::allocator
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/utility/read_number.hpp>
#include <jsoncons/json_error.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/json_tokenizer.hpp>
#include <jsoncons/utility/unicode_traits.hpp>

namespace jsoncons {

template <typename CharT,typename TempAlloc  = std::allocator<char>>
class basic_json_parser 
{
public:
    using char_type = CharT;
    using string_view_type = typename basic_json_visitor<CharT>::string_view_type;
private:
    struct string_maps_to_double
    {
        string_view_type s;

        bool operator()(const std::pair<string_view_type,double>& val) const
        {
            return val.first == s;
        }
    };

    using temp_allocator_type = TempAlloc;

    basic_json_tokenizer<CharT,TempAlloc> tokenizer_;

    // Noncopyable and nonmoveable
    basic_json_parser(const basic_json_parser&) = delete;
    basic_json_parser& operator=(const basic_json_parser&) = delete;

public:

    basic_json_parser()
        : basic_json_parser(basic_json_decode_options<char_type>())
    {
    }

    explicit basic_json_parser(const TempAlloc& temp_alloc)
        : basic_json_parser(basic_json_decode_options<char_type>(), temp_alloc)
    {
    }

    basic_json_parser(const basic_json_decode_options<char_type>& options,
        const TempAlloc& temp_alloc = TempAlloc())
      : tokenizer_(options, temp_alloc)
    {
    }

    ~basic_json_parser() = default;

    bool source_exhausted() const
    {
        return tokenizer_.source_exhausted();
    }

    const char_type* current() const
    {
        return tokenizer_.current();
    }

    parse_state parent() const
    {
        return tokenizer_.parent();
    }

    bool done() const
    {
        return tokenizer_.done();
    }

    bool enter() const
    {
        return tokenizer_.enter();
    }

    bool accept() const
    {
        return tokenizer_.accept();
    }

    bool stopped() const
    {
        return tokenizer_.stopped();
    }

    parse_state state() const
    {
        return tokenizer_.state();
    }

    bool finished() const
    {
        return tokenizer_.finished();
    }

    void skip_whitespace()
    {
        tokenizer_.skip_whitespace();
    }

    void reinitialize()
    {
        tokenizer_.reinitialize();
    }

    void reset()
    {
        tokenizer_.reset();
    }

    void restart()
    {
        tokenizer_.restart();
    }

    std::size_t line() const
    {
        return tokenizer_.line();
    }

    std::size_t column() const
    {
        return tokenizer_.column();
    }

    void check_done()
    {
        std::error_code ec;
        check_done(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,tokenizer_.line(), tokenizer_.column()));
        }
    }

    void check_done(std::error_code& ec)
    {
        tokenizer_.check_done(ec);
    }

    void update(const string_view_type sv)
    {
        tokenizer_.try_update(sv.data(),sv.length());
    }

    void update(const char_type* data, std::size_t length)
    {
        tokenizer_.try_update(data, length);
    }

    void parse_some(basic_json_visitor<char_type>& visitor)
    {
        std::error_code ec;
        parse_some(visitor, ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            if (ec != json_errc::unexpected_eof)
            {
                JSONCONS_THROW(ser_error(ec,tokenizer_.line(), tokenizer_.column()));
            }
        }
    }

    void parse_some(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        while (!tokenizer_.done() && !tokenizer_.source_exhausted())
        {
            switch (tokenizer_.token_kind())
            {
                case generic_token_kind::string_value:
                    if (tokenizer_.is_key())
                    {
                        visitor.key(tokenizer_.get_string_view(),
                            tokenizer_.get_context());
                    }
                    else
                    {
                        visitor.string_value(tokenizer_.get_string_view(),
                            tokenizer_.tag(), tokenizer_.get_context());
                    }
                    break;
                case generic_token_kind::null_value:
                    visitor.null_value(tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::bool_value:
                    visitor.bool_value(tokenizer_.get_bool_value(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::int64_value:
                    visitor.int64_value(tokenizer_.get_int64_value(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::uint64_value:
                    visitor.uint64_value(tokenizer_.get_uint64_value(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::double_value:
                    visitor.double_value(tokenizer_.get_double_value(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::begin_map:
                    visitor.begin_object(tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::end_map:
                    visitor.end_object(tokenizer_.get_context());
                    break;
                case generic_token_kind::begin_array:
                    visitor.begin_array(tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::end_array:
                    visitor.end_array(tokenizer_.get_context());
                    break;
                default:
                    break;
            }
            auto r = tokenizer_.try_next();
            if (!r)
            {
                ec = r.ec;
                if (ec == json_errc::unexpected_eof)
                {
                    ec.clear();
                }
                return;
            }
        }
        visitor.flush();
        if (ec == json_errc::unexpected_eof)
        {
            ec.clear();
        }
    }

    void finish_parse(basic_json_visitor<char_type>& visitor)
    {
        std::error_code ec{};
        finish_parse(visitor, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,tokenizer_.line(), tokenizer_.column()));
        }
    }

    void finish_parse(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        while (!tokenizer_.done())
        {
            switch (tokenizer_.token_kind())
            {
                case generic_token_kind::string_value:
                    if (tokenizer_.is_key())
                    {
                        visitor.key(tokenizer_.get_string_view(),
                            tokenizer_.get_context());
                    }
                    else
                    {
                        visitor.string_value(tokenizer_.get_string_view(),
                            tokenizer_.tag(), tokenizer_.get_context());
                    }
                    break;
                case generic_token_kind::null_value:
                    visitor.null_value(tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::bool_value:
                    visitor.bool_value(tokenizer_.get_bool_value(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::int64_value:
                    visitor.int64_value(tokenizer_.get_int64_value(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::uint64_value:
                    visitor.uint64_value(tokenizer_.get_uint64_value(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::double_value:
                    visitor.double_value(tokenizer_.get_double_value(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::begin_map:
                    visitor.begin_object(tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::end_map:
                    visitor.end_object(tokenizer_.get_context());
                    break;
                case generic_token_kind::begin_array:
                    visitor.begin_array(tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::end_array:
                    visitor.end_array(tokenizer_.get_context());
                    break;
                default:
                    break;
            }
            auto r = tokenizer_.try_next();
            if (!r)
            {
                ec = r.ec;
                if (ec == json_errc::unexpected_eof)
                {
                    ec.clear();
                }
                return;
            }
        }
    }
};

using json_parser = basic_json_parser<char>;
using wjson_parser = basic_json_parser<wchar_t>;

}

#endif

