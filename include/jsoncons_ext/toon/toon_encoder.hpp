// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_TOON_TOON_ENCODER_HPP
#define JSONCONS_TOON_TOON_ENCODER_HPP

#include <array> // std::array
#include <cstddef>
#include <cstdint>
#include <cmath> // std::isfinite, std::isnan
#include <limits> // std::numeric_limits
#include <memory>
#include <string>
#include <utility> // std::move
#include <vector>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/utility/write_number.hpp>
#include <jsoncons/json_error.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/sink.hpp>
#include <jsoncons/utility/bigint.hpp>
#include <jsoncons/utility/byte_string.hpp>
#include <jsoncons/utility/unicode_traits.hpp>

namespace jsoncons { 
namespace toon { 
namespace detail {

    inline
    bool is_control_character(uint32_t c)
    {
        return c <= 0x1F || c == 0x7f;
    }

    inline
    bool is_non_ascii_codepoint(uint32_t cp)
    {
        return cp >= 0x80;
    }

    template <typename CharT,typename Sink>
    void escape_string(const CharT* s, std::size_t length,
                         bool escape_all_non_ascii, bool escape_solidus,
                         Sink& sink)
    {
        std::size_t count = 0;
        const CharT* begin = s;
        const CharT* end = s + length;
        for (const CharT* it = begin; it != end; ++it)
        {
            CharT c = *it;
            switch (c)
            {
                case '\\':
                    sink.push_back('\\');
                    sink.push_back('\\');
                    count += 2;
                    break;
                case '"':
                    sink.push_back('\\');
                    sink.push_back('\"');
                    count += 2;
                    break;
                case '\b':
                    sink.push_back('\\');
                    sink.push_back('b');
                    count += 2;
                    break;
                case '\f':
                    sink.push_back('\\');
                    sink.push_back('f');
                    count += 2;
                    break;
                case '\n':
                    sink.push_back('\\');
                    sink.push_back('n');
                    count += 2;
                    break;
                case '\r':
                    sink.push_back('\\');
                    sink.push_back('r');
                    count += 2;
                    break;
                case '\t':
                    sink.push_back('\\');
                    sink.push_back('t');
                    count += 2;
                    break;
                default:
                    if (escape_solidus && c == '/')
                    {
                        sink.push_back('\\');
                        sink.push_back('/');
                        count += 2;
                    }
                    else if (is_control_character(c) || escape_all_non_ascii)
                    {
                        // convert to codepoint
                        uint32_t cp;
                        auto r = unicode_traits::to_codepoint(it, end, cp, unicode_traits::conv_flags::strict);
                        if (r.ec != unicode_traits::conv_errc())
                        {
                            JSONCONS_THROW(ser_error(json_errc::illegal_codepoint));
                        }
                        it = r.ptr - 1;
                        if (is_non_ascii_codepoint(cp) || is_control_character(c))
                        {
                            if (cp > 0xFFFF)
                            {
                                cp -= 0x10000;
                                uint32_t first = (cp >> 10) + 0xD800;
                                uint32_t second = ((cp & 0x03FF) + 0xDC00);

                                sink.push_back('\\');
                                sink.push_back('u');
                                sink.push_back(jsoncons::to_hex_character(first >> 12 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(first >> 8 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(first >> 4 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(first & 0x000F));
                                sink.push_back('\\');
                                sink.push_back('u');
                                sink.push_back(jsoncons::to_hex_character(second >> 12 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(second >> 8 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(second >> 4 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(second & 0x000F));
                                count += 12;
                            }
                            else
                            {
                                sink.push_back('\\');
                                sink.push_back('u');
                                sink.push_back(jsoncons::to_hex_character(cp >> 12 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(cp >> 8 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(cp >> 4 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(cp & 0x000F));
                                count += 6;
                            }
                        }
                        else
                        {
                            sink.push_back(c);
                            ++count;
                        }
                    }
                    else
                    {
                        sink.push_back(c);
                        ++count;
                    }
                    break;
            }
        }
    }

    inline
    byte_string_chars_format resolve_byte_string_chars_format(byte_string_chars_format format1,
                                                              byte_string_chars_format format2,
                                                              byte_string_chars_format default_format = byte_string_chars_format::base64url)
    {
        byte_string_chars_format sink;
        switch (format1)
        {
            case byte_string_chars_format::base16:
            case byte_string_chars_format::base64:
            case byte_string_chars_format::base64url:
                sink = format1;
                break;
            default:
                switch (format2)
                {
                    case byte_string_chars_format::base64url:
                    case byte_string_chars_format::base64:
                    case byte_string_chars_format::base16:
                        sink = format2;
                        break;
                    default: // base64url
                    {
                        sink = default_format;
                        break;
                    }
                }
                break;
        }
        return sink;
    }

} // namespace detail

    template <typename CharT,typename Sink=jsoncons::stream_sink<CharT>,typename Allocator=std::allocator<char>>
    class basic_toon_encoder final : public basic_json_visitor<CharT>
    {
        static jsoncons::basic_string_view<CharT> null_literal()
        {
            static jsoncons::basic_string_view<CharT> lit = JSONCONS_STRING_VIEW_CONSTANT(CharT, "null");
            return lit;
        }
        static jsoncons::basic_string_view<CharT> true_literal()
        {
            static jsoncons::basic_string_view<CharT> lit = JSONCONS_STRING_VIEW_CONSTANT(CharT, "true");
            return lit;
        }
        static jsoncons::basic_string_view<CharT> false_literal()
        {
            static jsoncons::basic_string_view<CharT> lit = JSONCONS_STRING_VIEW_CONSTANT(CharT, "false");
            return lit;
        }
    public:
        using allocator_type = Allocator;
        using char_type = CharT;
        using typename basic_json_visitor<CharT>::string_view_type;
        using sink_type = Sink;
        using string_type = typename basic_json_encode_options<CharT>::string_type;

    private:
        enum class container_type {object, array};

        class encoding_context
        {
            container_type type_;
            std::size_t count_{0};
        public:
            encoding_context(container_type type) noexcept
               : type_(type)
            {
            }

            std::size_t count() const
            {
                return count_;
            }

            void increment_count()
            {
                ++count_;
            }

            bool is_array() const
            {
                return type_ == container_type::array;
            }
        };
        using encoding_context_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<encoding_context>;

        Sink sink_;
        basic_json_encode_options<CharT> options_;
        jsoncons::write_double fp_;
        std::vector<encoding_context,encoding_context_allocator_type> stack_;
        int nesting_depth_;
    public:

        // Noncopyable and nonmoveable
        basic_toon_encoder(const basic_toon_encoder&) = delete;
        basic_toon_encoder(basic_toon_encoder&&) = delete;

        basic_toon_encoder(Sink&& sink, 
            const Allocator& alloc = Allocator())
            : basic_toon_encoder(std::forward<Sink>(sink), basic_json_encode_options<CharT>(), alloc)
        {
        }

        basic_toon_encoder(Sink&& sink, 
            const basic_json_encode_options<CharT>& options, 
            const Allocator& alloc = Allocator())
           : sink_(std::forward<Sink>(sink)),
             options_(options),
             fp_(options.float_format(), options.precision()),
             stack_(alloc),
             nesting_depth_(0)          
        {
        }

        ~basic_toon_encoder() noexcept
        {
            JSONCONS_TRY
            {
                sink_.flush();
            }
            JSONCONS_CATCH(...)
            {
            }
        }

        basic_toon_encoder& operator=(const basic_toon_encoder&) = delete;
        basic_toon_encoder& operator=(basic_toon_encoder&&) = delete;

        void reset()
        {
            stack_.clear();
            nesting_depth_ = 0;
        }

        void reset(Sink&& sink)
        {
            sink_ = std::move(sink);
            reset();
        }

    private:
        // Implementing methods
        void visit_flush() final
        {
            sink_.flush();
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(semantic_tag, const ser_context&, std::error_code& ec) final
        {
            if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
            {
                ec = json_errc::max_nesting_depth_exceeded;
                JSONCONS_VISITOR_RETURN;
            } 
            if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
            {
                sink_.push_back(',');
            }

            stack_.emplace_back(container_type::object);
            sink_.push_back('{');
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_end_object(const ser_context&, std::error_code&) final
        {
            JSONCONS_ASSERT(!stack_.empty());
            --nesting_depth_;

            stack_.pop_back();
            sink_.push_back('}');

            if (!stack_.empty())
            {
                stack_.back().increment_count();
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(std::size_t length, semantic_tag, const ser_context&, std::error_code& ec) final
        {
            if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
            {
                ec = json_errc::max_nesting_depth_exceeded;
                JSONCONS_VISITOR_RETURN;
            } 
            if (!stack_.empty() && stack_.back().is_array())
            {
                sink_.push_back('\n');
                for (std::size_t i=0; i < stack_.size(); ++i)
                {
                    sink_.push_back(' ');
                    sink_.push_back(' ');
                }
                sink_.push_back('-');
                sink_.push_back(' ');
            }
            stack_.emplace_back(container_type::array);
            sink_.push_back('[');
            jsoncons::from_integer(length, sink_);
            sink_.push_back(']');
            sink_.push_back(':');
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(semantic_tag, const ser_context&, std::error_code& ec) final
        {
            if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
            {
                ec = json_errc::max_nesting_depth_exceeded;
                JSONCONS_VISITOR_RETURN;
            } 
            if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
            {
                sink_.push_back(',');
            }
            stack_.emplace_back(container_type::array);
            sink_.push_back('[');
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_end_array(const ser_context&, std::error_code&) final
        {
            JSONCONS_ASSERT(!stack_.empty());
            --nesting_depth_;

            stack_.pop_back();
            if (!stack_.empty())
            {
                stack_.back().increment_count();
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_key(const string_view_type& name, const ser_context&, std::error_code&) final
        {
            if (!stack_.empty() && stack_.back().count() > 0)
            {
                sink_.push_back(',');
            }

            sink_.push_back('\"');
            jsoncons::toon::detail::escape_string(name.data(), name.length(),options_.escape_all_non_ascii(),options_.escape_solidus(),sink_);
            sink_.push_back('\"');
            sink_.push_back(':');
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_null(semantic_tag, const ser_context&, std::error_code&) final
        {
            if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
            {
                sink_.push_back(',');
            }

            sink_.append(null_literal().data(), null_literal().size());

            if (!stack_.empty())
            {
                stack_.back().increment_count();
            }
            JSONCONS_VISITOR_RETURN;
        }

        void write_bignum_value(const string_view_type& sv)
        {
            switch (options_.bignum_format())
            {
                case bignum_format_kind::raw:
                {
                    sink_.append(sv.data(),sv.size());
                    break;
                }
                case bignum_format_kind::base64:
                {
                    bigint n(sv.data(), sv.length());
                    bool is_neg = n < 0;
                    if (is_neg)
                    {
                        n = - n -1;
                    }
                    int signum;
                    std::vector<uint8_t> v;
                    n.write_bytes_be(signum, v);

                    sink_.push_back('\"');
                    if (is_neg)
                    {
                        sink_.push_back('~');
                    }
                    bytes_to_base64(v.begin(), v.end(), sink_);
                    sink_.push_back('\"');
                    break;
                }
                case bignum_format_kind::base64url:
                {
                    bigint n(sv.data(), sv.length());
                    bool is_neg = n < 0;
                    if (is_neg)
                    {
                        n = - n -1;
                    }
                    int signum;
                    std::vector<uint8_t> v;
                    n.write_bytes_be(signum, v);

                    sink_.push_back('\"');
                    if (is_neg)
                    {
                        sink_.push_back('~');
                    }
                    bytes_to_base64url(v.begin(), v.end(), sink_);
                    sink_.push_back('\"');
                    break;
                }
                default:
                {
                    sink_.push_back('\"');
                    sink_.append(sv.data(),sv.size());
                    sink_.push_back('\"');
                    break;
                }
            }
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type& sv, semantic_tag tag, const ser_context& context, std::error_code& ec) final
        {
            if (!stack_.empty() && stack_.back().is_array())
            {
                if (stack_.back().count() == 0)
                {
                    sink_.push_back(' ');
                }
                else
                {
                    sink_.push_back(',');
                }
            }

            if (!is_unquoted_safe(sv, ','))
            {
                sink_.push_back('\"');
                write_string(sv, tag, context, ec);
                sink_.push_back('\"');
            }
            else
            {
                write_string(sv, tag, context, ec);
            }

            if (!stack_.empty())
            {
                stack_.back().increment_count();
            }
            JSONCONS_VISITOR_RETURN;
        }

        void write_string(const string_view_type& sv, semantic_tag tag, const ser_context&, std::error_code&) 
        {
            if (JSONCONS_LIKELY(tag == semantic_tag::noesc && !options_.escape_all_non_ascii() && !options_.escape_solidus()))
            {
                //std::cout << "noesc\n";
                //sink_.push_back('\"');
                const CharT* begin = sv.data();
                const CharT* end = begin + sv.length();
                for (const CharT* it = begin; it != end; ++it)
                {
                    sink_.push_back(*it);
                }
                //sink_.push_back('\"');
            }
            else if (tag == semantic_tag::bigint)
            {
                write_bignum_value(sv);
            }
            else if (tag == semantic_tag::bigdec && options_.bignum_format() == bignum_format_kind::raw)
            {
                write_bignum_value(sv);
            }
            else
            {
                //if (tag != semantic_tag::bigdec)
                //    std::cout << "esc\n";
                //sink_.push_back('\"');
                jsoncons::toon::detail::escape_string(sv.data(), sv.length(),options_.escape_all_non_ascii(),options_.escape_solidus(),sink_);
                //sink_.push_back('\"');
            }           
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& b, 
            semantic_tag tag,
            const ser_context&,
            std::error_code&) final
        {
            if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
            {
                sink_.push_back(',');
            }

            byte_string_chars_format encoding_hint;
            switch (tag)
            {
                case semantic_tag::base16:
                    encoding_hint = byte_string_chars_format::base16;
                    break;
                case semantic_tag::base64:
                    encoding_hint = byte_string_chars_format::base64;
                    break;
                case semantic_tag::base64url:
                    encoding_hint = byte_string_chars_format::base64url;
                    break;
                default:
                    encoding_hint = byte_string_chars_format::none;
                    break;
            }

            byte_string_chars_format format = jsoncons::detail::resolve_byte_string_chars_format(options_.byte_string_format(), 
                                                                                       encoding_hint, 
                                                                                       byte_string_chars_format::base64url);
            switch (format)
            {
                case byte_string_chars_format::base16:
                {
                    sink_.push_back('\"');
                    bytes_to_base16(b.begin(),b.end(),sink_);
                    sink_.push_back('\"');
                    break;
                }
                case byte_string_chars_format::base64:
                {
                    sink_.push_back('\"');
                    bytes_to_base64(b.begin(), b.end(), sink_);
                    sink_.push_back('\"');
                    break;
                }
                case byte_string_chars_format::base64url:
                {
                    sink_.push_back('\"');
                    bytes_to_base64url(b.begin(),b.end(),sink_);
                    sink_.push_back('\"');
                    break;
                }
                default:
                {
                    JSONCONS_UNREACHABLE();
                }
            }

            if (!stack_.empty())
            {
                stack_.back().increment_count();
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_double(double value, 
                             semantic_tag,
                             const ser_context& context,
                             std::error_code& ec) final
        {
            if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
            {
                sink_.push_back(',');
            }

            if (JSONCONS_UNLIKELY(!std::isfinite(value)))
            {
                if ((std::isnan)(value))
                {
                    if (options_.enable_nan_to_num())
                    {
                        sink_.append(options_.nan_to_num().data(), options_.nan_to_num().length());
                    }
                    else if (options_.enable_nan_to_str())
                    {
                        write_string(options_.nan_to_str(), semantic_tag::none, context, ec);
                    }
                    else
                    {
                        sink_.append(null_literal().data(), null_literal().size());
                    }
                }
                else if (value == std::numeric_limits<double>::infinity())
                {
                    if (options_.enable_inf_to_num())
                    {
                        sink_.append(options_.inf_to_num().data(), options_.inf_to_num().length());
                    }
                    else if (options_.enable_inf_to_str())
                    {
                        write_string(options_.inf_to_str(), semantic_tag::none, context, ec);
                    }
                    else
                    {
                        sink_.append(null_literal().data(), null_literal().size());
                    }
                }
                else 
                {
                    if (options_.enable_neginf_to_num())
                    {
                        sink_.append(options_.neginf_to_num().data(), options_.neginf_to_num().length());
                    }
                    else if (options_.enable_neginf_to_str())
                    {
                        write_string(options_.neginf_to_str(), semantic_tag::none, context, ec);
                    }
                    else
                    {
                        sink_.append(null_literal().data(), null_literal().size());
                    }
                }
            }
            else
            {
                fp_(value, sink_);
            }

            if (!stack_.empty())
            {
                stack_.back().increment_count();
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_int64(int64_t value, 
                            semantic_tag,
                            const ser_context&,
                            std::error_code&) final
        {
            if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
            {
                sink_.push_back(',');
            }
            jsoncons::from_integer(value, sink_);
            if (!stack_.empty())
            {
                stack_.back().increment_count();
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_uint64(uint64_t value, 
                             semantic_tag, 
                             const ser_context&,
                             std::error_code&) final
        {
            if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
            {
                sink_.push_back(',');
            }
            jsoncons::from_integer(value, sink_);
            if (!stack_.empty())
            {
                stack_.back().increment_count();
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_bool(bool value, semantic_tag, const ser_context&, std::error_code&) final
        {
            if (!stack_.empty() && stack_.back().is_array() && stack_.back().count() > 0)
            {
                sink_.push_back(',');
            }

            if (value)
            {
                sink_.append(true_literal().data(), true_literal().size());
            }
            else
            {
                sink_.append(false_literal().data(), false_literal().size());
            }

            if (!stack_.empty())
            {
                stack_.back().increment_count();
            }
            JSONCONS_VISITOR_RETURN;
        }

        static bool is_unquoted_safe(jsoncons::basic_string_view<CharT> str, CharT delimiter = ',')
        {
            if (str.empty())
            {
                return false;
            }
            if (is_number(str))
            {
                return false;
            }
            if (str == null_literal() || str == true_literal() || str == false_literal())
            {
                return false;
            }
            if (str.front() == '-')
            {
                return false;
            }
            for (auto c : str)
            {
                switch (c)
                {
                    case ':':
                    case '[':
                    case ']':
                    case '{':
                    case '}':
                    case '\"':
                    case '\\':
                    case '\n':
                    case '\r':
                    case '\t':
                        return false;
                }
                if (c == delimiter)
                {
                    return false;
                }
            }
            return true;
        }
    public:
        static bool is_number(jsoncons::basic_string_view<CharT> str) 
        {
            int state = 0;

            for (auto c : str)
            {
                switch (state)
                {
                    case 0:
                        if (c == '-')
                        {
                            state = 1;
                        }
                        else if (c == '0')
                        {
                            state = 2;
                        }
                        else if (c >= '1' && c <= '9')
                        {
                            state = 3;
                        }
                        else
                        {
                            state = 9;
                        }
                        break;
                    case 1: // leading minus
                        if (c == '0')
                        {
                            state = 2;
                        }
                        else if (c >= '1' && c <= '9')
                        {
                            state = 3;
                        }
                        else
                        {
                            state = 9;
                        }
                        break;
                    case 2: // after 0
                        if (c == '0')
                        {
                            state = 9;
                        }
                        else if (c == '.')
                        {
                            state = 4;
                        }
                        else if (c >= '1' && c <= '9')
                        {
                            state = 3;
                        }
                        else
                        {
                            state = 9;
                        }
                        break;
                    case 3: // expect digits or dot
                        if (c == '.')
                        {
                            state = 4;
                        }
                        else if (!(c >= '0' && c <= '9'))
                        {
                            state = 9;
                        }
                        break;
                    case 4: // expect digits
                        if (c >= '0' && c <= '9')
                        {
                            state = 5;
                        }
                        else
                        {
                            state = 9;
                        }
                        break;
                    case 5: // expect digits
                        if (!(c >= '0' && c <= '9'))
                        {
                            state = 9;
                        }
                        break;
                    default:
                        break;
                }
            }
            if (state == 2 || state == 3 || state == 5)
            {
                return true;
            }
            return false;
        }
    };

    using toon_stream_encoder = basic_toon_encoder<char,jsoncons::stream_sink<char>>;
    using wtoon_stream_encoder = basic_toon_encoder<wchar_t,jsoncons::stream_sink<wchar_t>>;

    using toon_string_encoder = basic_toon_encoder<char,jsoncons::string_sink<std::string>>;
    using wtoon_string_encoder = basic_toon_encoder<wchar_t,jsoncons::string_sink<std::wstring>>;

} // namespace toon
} // namespace jsoncons

#endif // JSONCONS_TOON_TOON_ENCODER_HPP
