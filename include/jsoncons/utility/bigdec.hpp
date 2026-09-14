// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UTILITY_BIGDEC_HPP
#define JSONCONS_UTILITY_BIGDEC_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/utility/number_readers.hpp>
#include <jsoncons/utility/bigint.hpp>

namespace jsoncons {

template <typename Allocator>
class basic_bigdec
{
    basic_bigint<Allocator> unscaled_;
    int64_t scale_{0};

public:
    basic_bigdec() = default;

    basic_bigdec(basic_bigint<Allocator>&& unscaled_value, int64_t scale)
        : unscaled_{unscaled_value}, scale_{scale}
    {
    }

    basic_bigint<Allocator> unscaled() const
    {
        return unscaled_;
    }

    int64_t scale() const
    {
        return scale_;
    }

    friend bool operator==(const basic_bigdec& lhs, const basic_bigdec& rhs)
    {
        if (&lhs == &rhs)
        {
            return true;
        }
        if (lhs.scale_ != rhs.scale_)
        {
            return false;
        }
        return lhs.unscaled_ == rhs.unscaled_;
    }
};

template <typename CharT, typename Allocator>
to_number_result<CharT> to_bigdec(const CharT* s, std::size_t length, basic_bigdec<Allocator>& value)
{
    if (JSONCONS_UNLIKELY(length == 0))
    {
        return to_number_result<CharT>{s, std::errc::invalid_argument};
    }

    const CharT* cur = s;
    const CharT* end = s + length;

    int64_t scale = 0;

    cur += (*cur == '-');
    if (JSONCONS_UNLIKELY(cur == end || !is_digit_char(*cur)))
    {
        return to_number_result<CharT>{s, std::errc::invalid_argument};
    }
    if (*cur == '0')
    {
        cur++;
        if (JSONCONS_UNLIKELY(cur < end && is_digit_char(*cur)))
        {
            return to_number_result<CharT>{s, std::errc::invalid_argument};
        }
        value = basic_bigdec<Allocator>{};
        return to_number_result<CharT>(cur);
    }
    else
    {
        while (cur != end && is_digit_char(*cur))
        {
            cur++;
        }
        if (cur == end || !is_float_char(*cur))
        {
            value = basic_bigdec<Allocator>{basic_bigint<Allocator>{s,std::size_t(cur-s)}, 0};
            return to_number_result<CharT>(cur);
        }
    }
    const CharT* mark = cur;
    if (cur != end && *cur == '.')
    {
        cur++;
        if (cur != end && !is_digit_char(*cur))
        {
            return to_number_result<CharT>{cur, std::errc::invalid_argument};
        }
        cur++;
        while (cur != end && is_digit_char(*cur))
        {
            cur++;
        }
    }
    std::basic_string<CharT> buf{s, std::size_t(mark-s)};
    mark++;
    scale = static_cast<int64_t>(cur - mark);
    buf.append(mark, cur-mark);
    if (cur != end && is_exp_char(*cur))
    {
        bool negexp = false;
        if (*cur == '-')
        {
            negexp = true;
            ++cur;
            if (cur == end || !is_digit_char(*cur))
            {
                return to_number_result<CharT>{cur, std::errc::invalid_argument};
            }
        }
        mark = cur;
        ++cur;
        while (cur != end && is_digit_char(*cur))
        {
            cur++;
        }
        int64_t exp;
        auto r = dec_to_integer<int64_t>(mark, (cur-mark), exp);
        if (!r)
        {
            return r;
        }

        if (negexp)
        {
            if ((exp > 0 && scale > (std::numeric_limits<int64_t>::max)() - exp)) // overflow
            {
                return to_number_result<CharT>{cur, std::errc::result_out_of_range};
            }        
            value = basic_bigdec<Allocator>{basic_bigint<Allocator>{buf.data(), buf.size()}, scale + exp};
        }
        else
        {
            if (exp > 0 && scale < (std::numeric_limits<int64_t>::min)() + exp) // underflow
            {
                return to_number_result<CharT>{cur, std::errc::result_out_of_range};
            } 
            value = basic_bigdec<Allocator>{basic_bigint<Allocator>{buf.data(), buf.size()}, scale - exp};
        }
    }
    else
    {
        value = basic_bigdec<Allocator>{basic_bigint<Allocator>{buf.data(), buf.size()}, scale};
    }
    return to_number_result<CharT>(cur);
}

template <typename Alloc,typename CharT,typename BAlloc>
void to_buffer(const basic_bigdec<Alloc>& value, std::basic_string<CharT,std::char_traits<CharT>,BAlloc>& buf)
{
    buf.clear();
    if (value.scale() == 0)
    {
        buf = to_string(value.unscaled());
        return;
    }
    if (value.unscaled().is_negative())
    {
        buf.push_back('-');
    }
    std::string coeff = to_string(value.unscaled().is_negative() ? -value.unscaled() : value.unscaled());
    std::size_t coeffLen = coeff.size();
    int64_t adjusted = -value.scale() + (int64_t)(coeffLen-1);
    if ((value.scale() >= 0) && (adjusted >= -6)) 
    { // plain number
        int64_t pad = value.scale() - coeffLen;         // count of padding zeros
        if (pad >= 0) {                     // 0.xxx form
            buf.push_back('0');
            buf.push_back('.');
            for (; pad>0; pad--) {
                buf.push_back('0');
            }
            buf.append(coeff.data(), coeffLen);
        } else 
        {                         // xx.xx form
            buf.append(coeff.data(), -pad);
            buf.push_back('.');
            buf.append(coeff.data() -pad, value.scale());
        }
    }
}

using bigdec = basic_bigdec<std::allocator<uint64_t>>;

} // namespace jsoncons

#endif // JSONCONS_UTILITY_BIGDEC_HPP
