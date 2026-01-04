// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_CURSOR_HPP
#define JSONCONS_JSON_CURSOR_HPP

#include <cstddef>
#include <functional>
#include <ios>
#include <memory> // std::allocator
#include <system_error>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/utility/byte_string.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/conv_error.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_tokenizer.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/source_adaptor.hpp>
#include <jsoncons/staj_cursor.hpp>
#include <jsoncons/utility/unicode_traits.hpp>

namespace jsoncons {

template <typename CharT,typename Source=jsoncons::stream_source<CharT>,typename Allocator=std::allocator<char>>
class basic_json_cursor : public basic_staj_cursor<CharT>, private virtual ser_context
{
public:
    using source_type = Source;
    using char_type = CharT;
    using allocator_type = Allocator;
    using string_view_type = jsoncons::basic_string_view<CharT>;
private:
    using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT>;
    static constexpr size_t default_max_buffer_size = 16384;

    json_source_adaptor<Source> source_;
    basic_json_tokenizer<CharT,Allocator> tokenizer_;
    bool done_{false};
    basic_staj_event<char_type> current_;

public:

    // Constructors that throw parse exceptions
    template <typename Sourceable>
    basic_json_cursor(Sourceable&& source, 
        const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>(),
        const Allocator& alloc = Allocator(),
        typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(std::forward<Sourceable>(source)),
         tokenizer_(options, alloc),
         current_(staj_event_type::null_value)
    {
        std::error_code local_ec;
        read_next(local_ec);
        if (local_ec)
        {
            if (local_ec == json_errc::unexpected_eof)
            {
                done_ = true;
            }
            else
            {
                JSONCONS_THROW(ser_error(local_ec, 1, 1));
            }
        }
    }
    template <typename Sourceable>
    basic_json_cursor(Sourceable&& source, 
        const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>(),
        const Allocator& alloc = Allocator(),
        typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(),
         tokenizer_(options, alloc),
         current_(staj_event_type::null_value)
    {
        initialize_with_string_view(std::forward<Sourceable>(source));
    }

    // Constructors that set parse error codes
    template <typename Sourceable>
    basic_json_cursor(Sourceable&& source, std::error_code& ec)
        : basic_json_cursor(std::allocator_arg, Allocator(), 
              std::forward<Sourceable>(source),
              basic_json_decode_options<CharT>(),
              ec)
    {
    }

    template <typename Sourceable>
    basic_json_cursor(Sourceable&& source, 
        const basic_json_decode_options<CharT>& options,
        std::error_code& ec)
        : basic_json_cursor(std::allocator_arg, Allocator(), 
              std::forward<Sourceable>(source),
              options,
              ec)
    {
    }

    template <typename Sourceable>
    basic_json_cursor(std::allocator_arg_t, const Allocator& alloc,
        Sourceable&& source, 
        const basic_json_decode_options<CharT>& options,
        std::error_code& ec,
        typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(std::forward<Sourceable>(source)),
         tokenizer_(options, alloc),
         current_(staj_event_type::null_value) 
    {
        std::error_code local_ec;
        read_next(local_ec);
        if (local_ec)
        {
            if (local_ec == json_errc::unexpected_eof)
            {
                done_ = true;
            }
            else
            {
                ec = local_ec;
            }
        }
    }

    template <typename Sourceable>
    basic_json_cursor(std::allocator_arg_t, const Allocator& alloc,
        Sourceable&& source, 
        const basic_json_decode_options<CharT>& options,
        std::error_code& ec,
        typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type* = 0)
       : source_(),
         tokenizer_(options, alloc),
         current_(staj_event_type::null_value)
    {
        initialize_with_string_view(std::forward<Sourceable>(source), ec);
    }
    
    basic_json_cursor(const basic_json_cursor&) = delete;
    basic_json_cursor(basic_json_cursor&&) = default;
    
    ~basic_json_cursor() = default;

    // Noncopyable and nonmoveable

    basic_json_cursor& operator=(const basic_json_cursor&) = delete;
    basic_json_cursor& operator=(basic_json_cursor&&) = default;

    void reset()
    {
        tokenizer_.reset();
        done_ = false;
        if (!done_)
        {
            read_next();
        }
    }

    template <typename Sourceable>
    typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type
    reset(Sourceable&& source)
    {
        source_ = std::forward<Sourceable>(source);
        tokenizer_.reinitialize();
        done_ = false;
        if (!done_)
        {
            read_next();
        }
    }

    template <typename Sourceable>
    typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type
    reset(Sourceable&& source)
    {
        source_ = {};
        tokenizer_.reinitialize();
        done_ = false;
        initialize_with_string_view(std::forward<Sourceable>(source));
    }

    void reset(std::error_code& ec)
    {
        tokenizer_.reset();
        done_ = false;
        if (!done_)
        {
            read_next(ec);
        }
    }

    template <typename Sourceable>
    typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type
    reset(Sourceable&& source, std::error_code& ec)
    {
        source_ = std::forward<Sourceable>(source);
        tokenizer_.reinitialize();
        done_ = false;
        if (!done_)
        {
            read_next(ec);
        }
    }

    template <typename Sourceable>
    typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Sourceable>::value>::type
    reset(Sourceable&& source, std::error_code& ec)
    {
        source_ = {};
        tokenizer_.reinitialize();
        done_ = false;
        initialize_with_string_view(std::forward<Sourceable>(source), ec);
    }

    bool done() const override
    {
        return tokenizer_.done() /*|| done_*/;
    }

    string_view_type get_string_view() const
    {
        return tokenizer_.get_string_view();
    }

    const basic_staj_event<CharT>& current() const override
    {
        return current_;
    }

    void read_to(basic_json_visitor<CharT>& visitor) override
    {
        std::error_code ec;
        read_to(visitor, ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,tokenizer_.line(),tokenizer_.column()));
        }
    }

    void read_to(basic_json_visitor<CharT>& visitor,
        std::error_code& ec) override
    {
        if (is_begin_container(current().event_type()))
        {
            int level = 1;
            if (tokenizer_.token_kind() == generic_token_kind::begin_map)
            {
                visitor.begin_object(tokenizer_.tag(), tokenizer_.get_context());
            }
            else
            {
                visitor.begin_array(tokenizer_.tag(), tokenizer_.get_context());
            }
            while (!tokenizer_.done() && level != 0)
            {
                auto r = tokenizer_.try_next_token();
                if (JSONCONS_UNLIKELY(!r))
                {
                    if (r.ec != json_errc::unexpected_eof)
                    {
                        ec = r.ec;
                    }
                    return;
                }
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
                        visitor.bool_value(tokenizer_.get_bool(),
                            tokenizer_.tag(), tokenizer_.get_context());
                        break;
                    case generic_token_kind::int64_value:
                        visitor.int64_value(tokenizer_.get_int64(),
                            tokenizer_.tag(), tokenizer_.get_context());
                        break;
                    case generic_token_kind::uint64_value:
                        visitor.uint64_value(tokenizer_.get_uint64(),
                            tokenizer_.tag(), tokenizer_.get_context());
                        break;
                    case generic_token_kind::double_value:
                        visitor.double_value(tokenizer_.get_double(),
                            tokenizer_.tag(), tokenizer_.get_context());
                        break;
                    case generic_token_kind::begin_map:
                        visitor.begin_object(tokenizer_.tag(), tokenizer_.get_context());
                        ++level;
                        break;
                    case generic_token_kind::end_map:
                        visitor.end_object(tokenizer_.get_context());
                        --level;
                        break;
                    case generic_token_kind::begin_array:
                        visitor.begin_array(tokenizer_.tag(), tokenizer_.get_context());
                        ++level;
                        break;
                    case generic_token_kind::end_array:
                        visitor.end_array(tokenizer_.get_context());
                        --level;
                        break;
                    default:
                        break;
                }
            }
            update_current();
            visitor.flush();
            if (ec == json_errc::unexpected_eof)
            {
                ec.clear();
            }
        }
        else
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
                    visitor.bool_value(tokenizer_.get_bool(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::int64_value:
                    visitor.int64_value(tokenizer_.get_int64(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::uint64_value:
                    visitor.uint64_value(tokenizer_.get_uint64(),
                        tokenizer_.tag(), tokenizer_.get_context());
                    break;
                case generic_token_kind::double_value:
                    visitor.double_value(tokenizer_.get_double(),
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
        }
    }

    void next() override
    {
        read_next();
    }

    void next(std::error_code& ec) override
    {
        read_next(ec);
    }

    void check_done()
    {
        std::error_code ec;
        check_done(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,tokenizer_.line(),tokenizer_.column()));
        }
    }

    const ser_context& context() const override
    {
        return *this;
    }

    void check_done(std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = json_errc::source_error;
            return;
        }   
        if (source_.eof())
        {
            tokenizer_.check_done(ec);
            if (JSONCONS_UNLIKELY(ec)) {return;}
        }
        else
        {
            do
            {
                if (tokenizer_.source_exhausted())
                {
                    auto s = source_.read_buffer(ec);
                    if (JSONCONS_UNLIKELY(ec)) {return;}
                    if (!s.empty())
                    {
                        tokenizer_.update(s.data(),s.size());
                    }
                }
                if (!tokenizer_.source_exhausted())
                {
                    tokenizer_.check_done(ec);
                    if (JSONCONS_UNLIKELY(ec)) {return;}
                }
            }
            while (!eof());
        }
    }

    bool eof() const
    {
        return tokenizer_.source_exhausted() && source_.eof();
    }

    std::size_t line() const override
    {
        return tokenizer_.line();
    }

    std::size_t column() const override
    {
        return tokenizer_.column();
    }

    friend
    basic_staj_filter_view<CharT> operator|(basic_json_cursor& cursor, 
        std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> pred)
    {
        return basic_staj_filter_view<CharT>(cursor, pred);
    }

private:

    void initialize_with_string_view(string_view_type sv)
    {
        std::error_code local_ec;
        initialize_with_string_view(sv, local_ec);
        if (local_ec)
        {
            JSONCONS_THROW(ser_error(local_ec, 1, 1));
        }
    }

    void initialize_with_string_view(string_view_type sv, std::error_code& ec)
    {
        auto r = unicode_traits::detect_json_encoding(sv.data(), sv.size());
        if (!(r.encoding == unicode_traits::encoding_kind::utf8 || r.encoding == unicode_traits::encoding_kind::undetected))
        {
            ec = json_errc::illegal_unicode_character;
            return;
        }
        std::size_t offset = (r.ptr - sv.data());
        tokenizer_.update(sv.data()+offset,sv.size()-offset);
        if (!done_)
        {
            std::error_code local_ec;
            read_next(local_ec);
            if (local_ec)
            {
                if (local_ec == json_errc::unexpected_eof)
                {
                    done_ = true;
                }
                else
                {
                    ec = local_ec;
                }
            }
        }
    }

    void read_next()
    {
        std::error_code ec;
        read_next(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,tokenizer_.line(),tokenizer_.column()));
        }
    }

    void read_next(std::error_code& ec)
    {
        auto r = tokenizer_.try_next_token();
        if (JSONCONS_UNLIKELY(!r))
        {
            if (r.ec != json_errc::unexpected_eof)
            {
                ec = r.ec;
            }
            return;
        }
        while (!tokenizer_.done() && tokenizer_.token_kind() == generic_token_kind{})
        {
            if (tokenizer_.source_exhausted())
            {
                if (!source_.eof())
                {
                    auto buf = source_.read_buffer(ec);
                    if (JSONCONS_UNLIKELY(ec)) {return;}
                    if (!buf.empty())
                    {
                        tokenizer_.update(buf.data(), buf.size());
                        r = tokenizer_.try_next_token();
                        if (JSONCONS_UNLIKELY(!r))
                        {
                            if (r.ec != json_errc::unexpected_eof)
                            {
                                ec = r.ec;
                            }
                            return;
                        }
                    }
                    else
                    {
                        r = tokenizer_.try_next_token();
                        if (JSONCONS_UNLIKELY(!r))
                        {
                            if (!r)
                            {
                                ec = r.ec;
                            }
                            return;
                        }
                    }
                }
                else
                {
                    if (!tokenizer_.started())
                    {
                        return;
                    }
                    r = tokenizer_.try_next_token();
                    if (JSONCONS_UNLIKELY(!r))
                    {
                        if (!r)
                        {
                            ec = r.ec;
                        }
                        return;
                    }
                }
            }
            else
            {
                r = tokenizer_.try_next_token();
                if (JSONCONS_UNLIKELY(!r))
                {
                    if (!r)
                    {
                        ec = r.ec;
                    }
                    return;
                }
            }
        }
        update_current();
    }

    void update_current()
    {
        switch (tokenizer_.token_kind())
        {
            case generic_token_kind::null_value:
                current_ = basic_staj_event<CharT>(staj_event_type::null_value);
                break;
            case generic_token_kind::string_value:
                if (tokenizer_.is_key())
                {
                    current_ = basic_staj_event<CharT>(tokenizer_.get_string_view(),
                        staj_event_type::key);
                }
                else
                {
                    current_ = basic_staj_event<CharT>(tokenizer_.get_string_view(),
                        staj_event_type::string_value, tokenizer_.tag());
                }
                break;
            case generic_token_kind::bool_value:
                current_ = basic_staj_event<CharT>(tokenizer_.get_bool(),
                    tokenizer_.tag());
                break;
            case generic_token_kind::uint64_value:
                current_ = basic_staj_event<CharT>(tokenizer_.get_uint64(),
                    tokenizer_.tag());
                break;
            case generic_token_kind::int64_value:
                current_ = basic_staj_event<CharT>(tokenizer_.get_int64(),
                    tokenizer_.tag());
                break;
            case generic_token_kind::double_value:
                current_ = basic_staj_event<CharT>(tokenizer_.get_double(),
                    tokenizer_.tag());
                break;
            case generic_token_kind::begin_map:
                current_ = basic_staj_event<CharT>(staj_event_type::begin_object);
                break;
            case generic_token_kind::end_map:
                current_ = basic_staj_event<CharT>(staj_event_type::end_object);
                break;
            case generic_token_kind::begin_array:
                current_ = basic_staj_event<CharT>(staj_event_type::begin_array);
                break;
            case generic_token_kind::end_array:
                current_ = basic_staj_event<CharT>(staj_event_type::end_array);
                break;
            default:
                current_ = basic_staj_event<CharT>(staj_event_type::null_value);
                break;
        }
    }
};

using json_stream_cursor = basic_json_cursor<char,jsoncons::stream_source<char>>;
using json_string_cursor = basic_json_cursor<char,jsoncons::string_source<char>>;
using wjson_stream_cursor = basic_json_cursor<wchar_t,jsoncons::stream_source<wchar_t>>;
using wjson_string_cursor = basic_json_cursor<wchar_t,jsoncons::string_source<wchar_t>>;

} // namespace jsoncons

#endif // JSONCONS_JSON_CURSOR_HPP

