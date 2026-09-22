// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UTILITY_BIGDEC_HPP
#define JSONCONS_UTILITY_BIGDEC_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/utility/number_readers.hpp>
#include <jsoncons/utility/number_writers.hpp>
#include <jsoncons/utility/bignum_common.hpp>
#include <jsoncons/utility/bigint.hpp>

#include <array>

namespace jsoncons {

template <typename Allocator>
class basic_bigdec;

template <typename CharT, typename Allocator>
to_number_result<CharT> to_bigdec(const CharT* s, std::size_t length, basic_bigdec<Allocator>& value);

template <typename Allocator>
class basic_bigdec
{
    basic_bigint<Allocator> unscaled_;
    int64_t scale_{0};

public:
    basic_bigdec() = default;
    explicit basic_bigdec(basic_bigint<Allocator>&& unscaled)
        : unscaled_{unscaled}, scale_{0}
    {
    }

    basic_bigdec(basic_bigint<Allocator>&& unscaled, int64_t scale)
        : unscaled_{unscaled}, scale_{scale}
    {
    }

    template <typename CharT>
    explicit basic_bigdec(jsoncons::basic_string_view<CharT> sv)
    {
        auto r = to_bigdec(sv.data(), sv.size(), *this);
        if (!r)
        {
            JSONCONS_THROW(std::runtime_error(r.error_code().message()));
        }
    }

    template <typename CharT>
    explicit basic_bigdec(const CharT* s)
    {
        auto r = to_bigdec(s, std::char_traits<CharT>::length(s), *this);
        if (!r)
        {
            JSONCONS_THROW(std::runtime_error(r.error_code().message()));
        }
    }

    basic_bigdec(const basic_bigdec&) = default;
    basic_bigdec(basic_bigdec&&) = default;

    basic_bigdec& operator=(const basic_bigdec&) = default;
    basic_bigdec& operator=(basic_bigdec&&) = default;

    basic_bigint<Allocator> unscaled() const
    {
        return unscaled_;
    }

    int64_t scale() const
    {
        return scale_;
    }

    int signum() const
    {
        return unscaled_.signum();
    }

    static basic_bigint<Allocator> big_ten_to_the(std::size_t n) 
    {
        if (n == 0)
        {
            return basic_bigint<Allocator>{0};
        }

        if (n < uint64_pow10_table.size())
        {
            return uint64_pow10_table[n];
        }       

        return powb(basic_bigint<Allocator>(10), n);
    }

    static std::size_t big_digit_length(const basic_bigint<Allocator>& b) {
        if (b.signum() == 0)
            return 1;
        std::size_t r = ((b.bit_width() + 1) * 646456993u) >> 31;
        return b.compare_magnitude(big_ten_to_the(r)) < 0u ? r : r+1;
    }

    std::size_t precision() const
    {
        return big_digit_length(unscaled_);
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

    static bool need_increment(const basic_bigint<Allocator>& divisor, 
        const basic_bigint<Allocator>& q, const basic_bigint<Allocator>& r) 
    {
        JSONCONS_ASSERT(r != 0);
        int32_t cmp_frac_half = r.compare_half(divisor);
        // half_even rounding
        if (cmp_frac_half < 0 ) // We're closer to higher digit
        {
            return false;
        }
        else if (cmp_frac_half > 0 ) // We're closer to lower digit
        {
            return true;
        }
        else // half-way 
        { 
            return q.is_odd();
        }       
    }
public:
    static basic_bigint<Allocator> divide_and_round(const basic_bigint<Allocator>& dividend, 
        const basic_bigint<Allocator>& divisor) 
    {
        basic_bigint<Allocator> adividend = dividend >= 0 ? dividend : -dividend;
        basic_bigint<Allocator> adivisor = divisor >= 0 ? divisor : -divisor;

        basic_bigint<Allocator> q;
        basic_bigint<Allocator> r;

        adividend.divide(adivisor, q, r, true);
        bool isRemainderZero = r.signum() == 0;
        if (!isRemainderZero) {
            if (need_increment(adivisor, q, r)) {
                q += basic_bigint<Allocator>(1);
            }
        }

        return (dividend.signum() != divisor.signum()) ? -q : q;
    }
};

template <typename Alloc>
bignum_result multiply(const basic_bigdec<Alloc>& a, const basic_bigdec<Alloc>& b, basic_bigdec<Alloc>& c)
{
    if (add_overflow(a.scale(), b.scale()))
    {
        return bignum_result{bignum_errc::result_out_of_range};
    }
    int64_t scale = a.scale() + b.scale();
    c = basic_bigdec<Alloc>(a.unscaled() * b.unscaled(), scale);

    return bignum_result{};
}

template <typename Alloc>
bignum_result divide(const basic_bigdec<Alloc>& a, const basic_bigdec<Alloc>& b, basic_bigdec<Alloc>& c)
{
    if (b.signum() == 0)
    {
        return bignum_result{bignum_errc::division_by_zero};
    }
    if (subtract_overflow(a.scale(), b.scale()))
    {
        return bignum_result{bignum_errc::result_out_of_range};
    }
    int64_t preferred_scale = a.scale() - b.scale();

    if (a.signum() == 0)
    {
        c = basic_bigdec<Alloc>{basic_bigint<Alloc>{}, preferred_scale};
    }
    else
    {
        int xscale = a.precision();
        int yscale = b.precision();
        basic_bigdec<Alloc> quotient;
    }
    return bignum_result{};
}

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
    if (JSONCONS_UNLIKELY(cur == end || !is_char_digit(*cur)))
    {
        return to_number_result<CharT>{s, std::errc::invalid_argument};
    }
    if (*cur == '0')
    {
        cur++;
        if (JSONCONS_UNLIKELY(cur < end && is_char_digit(*cur)))
        {
            return to_number_result<CharT>{s, std::errc::invalid_argument};
        }
        value = basic_bigdec<Allocator>{};
        return to_number_result<CharT>(cur);
    }
    else
    {
        while (cur != end && is_char_digit(*cur))
        {
            cur++;
        }
        if (cur == end || !is_char_float(*cur))
        {
            value = basic_bigdec<Allocator>{basic_bigint<Allocator>{s,std::size_t(cur-s)}, 0};
            return to_number_result<CharT>(cur);
        }
    }
    const CharT* mark1 = cur;
    const CharT* mark = cur;
    if (cur != end && *cur == '.')
    {
        cur++;
        if (cur != end && !is_char_digit(*cur))
        {
            return to_number_result<CharT>{cur, std::errc::invalid_argument};
        }
        cur++;
        while (cur != end && is_char_digit(*cur))
        {
            cur++;
        }
        mark++;
    }
    std::basic_string<CharT> buf{s, std::size_t(mark1-s)};
    scale = static_cast<int64_t>(cur - mark);
    buf.append(mark, cur-mark);
    if (cur != end && is_char_exp(*cur))
    {
        ++cur;
        bool negexp = false;
        if (cur != end)
        {
            if (*cur == '-')
            {
                negexp = true;
            }
            cur += is_char_sign(*cur);
            if (cur == end || !is_char_digit(*cur))
            {
                return to_number_result<CharT>{cur, std::errc::invalid_argument};
            }
        }
        mark = cur;
        ++cur;
        while (cur != end && is_char_digit(*cur))
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
    if (value.scale() == 0)
    {
        to_buffer(value.unscaled(), buf);
        return;
    }
    if (value.unscaled().is_negative())
    {
        buf.push_back('-');
    }
    std::basic_string<CharT> coeff;
    to_buffer(value.unscaled().is_negative() ? -value.unscaled() : value.unscaled(), coeff);
    std::size_t coeffLen = coeff.size();
    int64_t adjusted = -value.scale() + (int64_t)(coeffLen-1);
    if ((value.scale() >= 0) && (adjusted >= -6)) 
    { 
        int64_t pad = value.scale() - coeffLen;         // padding zeros
        if (pad >= 0) {                                 // 0.xxx form
            buf.push_back('0');
            buf.push_back('.');
            for (; pad>0; pad--) {
                buf.push_back('0');
            }
            buf.append(coeff.data(), coeffLen);
        } 
        else 
        {
            buf.append(coeff.data(), -pad);
            buf.push_back('.');
            buf.append(coeff.data() -pad, value.scale());
        }
    }
    else
    {
        buf.push_back(coeff[0]);   // first character
        if (coeffLen > 1) 
        {
            buf.push_back('.');
            buf.append(coeff.data() + 1, coeffLen - 1);
        }
        if (adjusted != 0) 
        {             
            buf.push_back('e');
            from_integer(adjusted, buf);
        }
    }
}

using bigdec = basic_bigdec<std::allocator<uint64_t>>;

} // namespace jsoncons

#endif // JSONCONS_UTILITY_BIGDEC_HPP
