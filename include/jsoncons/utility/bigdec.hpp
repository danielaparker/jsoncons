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

enum class rounding_mode {down, up, half_up, half_even};

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
            return basic_bigint<Allocator>(uint64_pow10_table[n]);
        }       

        return powb(basic_bigint<Allocator>(10), n);
    }

    static uint64_t big_digit_length(const basic_bigint<Allocator>& b) {
        if (b.signum() == 0)
            return 1;
        uint64_t r = ((b.bit_width() + 1) * 646456993u) >> 31;
        return b.compare_abs(big_ten_to_the(r)) < 0 ? r : r+1;
    }

    uint64_t precision() const
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

    bool need_to_round_up(const basic_bigint<Allocator>& quotient, 
        const basic_bigint<Allocator>& remainder, 
        const basic_bigint<Allocator>& divisor, rounding_mode rounding) 
    {
        // Conceptual rounding logic helper
        // Compares (remainder * 2) against the divisor to see if we are past the halfway point (0.5)
        int compare = (absb(remainder) * basic_bigint<Allocator>(2)).compare(absb(divisor));

        switch (rounding) 
        {
            case rounding_mode::down: 
                return false; // Always truncate toward zero
            case rounding_mode::up: 
                return true;  // Always round away from zero
            case rounding_mode::half_up: 
                return compare >= 0; // Round up if remainder >= 0.5 of divisor
            case rounding_mode::half_even:
                if (compare > 0) return true;
                if (compare < 0) return false;
                // If exactly 0.5, round up only if the last digit of the quotient is odd
                //return quotient.testBit(0);
                return quotient.is_odd();
            default:
                JSONCONS_UNREACHABLE();
        } 
    }

    int compare_abs(const basic_bigdec& val) 
    {
        int64_t sdiff = this->scale - val.scale;
        if (sdiff != 0) {
            // Avoid matching scales if the (adjusted) exponents differ
            int64_t xae = (int64_t)this->precision() - this->scale();   // [-1]
            int64_t yae = (int64_t)val.precision() - val.scale();     // [-1]
            if (xae < yae)
                return -1;
            if (xae > yae)
                return 1;
            if (sdiff < 0) {
                basic_bigint<Allocator> rb = bigMultiplyPowerTen((int)-sdiff);
                return rb.compare_abs(val.unscaled());
            } else { // sdiff > 0
                // The cases sdiff > Integer.MAX_VALUE intentionally fall through.
                basic_bigint<Allocator> rb = val.bigMultiplyPowerTen((int)sdiff);
                return this->intVal.compare_abs(rb);
            }
        }
        return this->unscaled().compare_abs(val.unscaled());
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

    bignum_result divide(const basic_bigdec<Allocator>& divisor, 
        basic_bigdec<Allocator>& value,
        int64_t preferred_scale = 34, 
        rounding_mode rounding = rounding_mode::half_even)
    {
        if (divisor.signum() == 0)
        {
            return bignum_result{bignum_errc::divide_by_zero};
        }

        int64_t scale_difference = preferred_scale + divisor.scale() - scale();
        basic_bigint<Allocator> scaled_dividend = unscaled();
        basic_bigint<Allocator> adjusted_divisor = divisor.unscaled();
        if (scale_difference > 0) 
        {
            // Multiply dividend by 10^scale_difference to make room for decimal precision
            scaled_dividend = scaled_dividend * powb(basic_bigint<Allocator>(10), scale_difference);
        } 
        else if (scale_difference < 0) 
        {
            // If scale difference is negative, the divisor needs to be scaled up instead
            adjusted_divisor = adjusted_divisor * powb(basic_bigint<Allocator>(10), scale_difference);
        }

        // Perform the integer division (yields quotient and remainder)
        basic_bigint<Allocator> quotient;
        basic_bigint<Allocator> remainder;

        scaled_dividend.divide(adjusted_divisor, quotient, remainder, true);

        // Handle Rounding if there is a remainder left over
        if (remainder.signum() != 0)
        {
            if (need_to_round_up(quotient, remainder, adjusted_divisor, rounding)) 
            {
                quotient += (basic_bigint<Allocator>(quotient.signum() >= 0 ? 1 : -1));
            }
        }
        
        // Assigns a standard basic_bigdec<Allocator> with the rounded unscaled value and proper scale
        value = basic_bigdec<Allocator>(std::move(quotient), preferred_scale);
        
        return bignum_result{};
    }

    template <typename CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const basic_bigdec& b)
    {
        std::basic_string<CharT> s;
        append_to_string(b, s); 
        os << s;

        return os;
    }

    int compare(const basic_bigdec<Allocator>& other) const
    {
        int xsign = this->signum();
        int ysign = other.signum();
        if (xsign != ysign)
            return (xsign > ysign) ? 1 : -1;
        if (xsign == 0)
            return 0;
        int cmp = compare_abs(other);
        return (xsign > 0) ? cmp : -cmp;
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
void append_to_string(const basic_bigdec<Alloc>& value, std::basic_string<CharT,std::char_traits<CharT>,BAlloc>& buf)
{
    if (value.scale() == 0)
    {
        append_to_string(value.unscaled(), buf);
        return;
    }
    if (value.unscaled().is_negative())
    {
        buf.push_back('-');
    }
    std::basic_string<CharT> coeff;
    append_to_string(value.unscaled().is_negative() ? -value.unscaled() : value.unscaled(), coeff);
    std::size_t coeffLen = coeff.size();
    if ((value.scale() >= 0) && (value.scale() <= static_cast<int64_t>(coeffLen) + 5)) 
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
        int64_t digits = static_cast<int64_t>(coeffLen) - 1;
        if (value.scale() != digits) 
        {             
            buf.push_back('e');
            if (value.scale() < digits)
            {
                from_integer(static_cast<uint64_t>(digits) - static_cast<uint64_t>(value.scale()), buf);
            }
            else
            {
                from_integer(digits - value.scale(), buf);
            }
        }
    }
}

using bigdec = basic_bigdec<std::allocator<uint64_t>>;

} // namespace jsoncons

#endif // JSONCONS_UTILITY_BIGDEC_HPP
