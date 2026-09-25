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
            return basic_bigint<Allocator>(uint64_pow10_table[n]);
        }       

        return powb(basic_bigint<Allocator>(10), n);
    }

    static uint64_t big_digit_length(const basic_bigint<Allocator>& b) {
        if (b.signum() == 0)
            return 1;
        uint64_t r = ((b.bit_width() + 1) * 646456993u) >> 31;
        return b.compare_magnitude(big_ten_to_the(r)) < 0u ? r : r+1;
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
/*
    if (checkScale(dividend,(long)scale + divisorScale) > dividendScale) {
        int newScale = scale + divisorScale;
        int raise = newScale - dividendScale;
        BigInteger scaledDividend = bigMultiplyPowerTen(dividend, raise);
        return divideAndRound(scaledDividend, divisor, scale, roundingMode, scale);
    } else {
        int newScale = checkScale(divisor,(long)dividendScale - scale);
        int raise = newScale - divisorScale;
        BigInteger scaledDivisor = bigMultiplyPowerTen(divisor, raise);
        return divideAndRound(dividend, scaledDivisor, scale, roundingMode, scale);
    }
 
    if (add_overflow(a.scale(), b.scale()))
    {
        return bignum_result{bignum_errc::result_out_of_range};
    }

*/

    /*static int checkScale(BigInteger intVal, long val) {
        int asInt = (int)val;
        if (asInt != val) {
            asInt = val>Integer.MAX_VALUE ? Integer.MAX_VALUE : Integer.MIN_VALUE;
            if (intVal.signum() != 0)
                throw new ArithmeticException(asInt>0 ? "Underflow":"Overflow");
        }
        return asInt;
    }*/

    static bignum_result create_and_strip_zeros_to_match_scale(const basic_bigint<Allocator>& intVal, 
        int64_t scale, int64_t preferred_scale, basic_bigint<Allocator>& result) 
    {
        if (subtract_overflow(scale, preferred_scale))
        {
            return bignum_result{bignum_errc::result_out_of_range};
        }
/*
        // avoid overflow of scale - preferred_scale
 
 
        preferred_scale = Math.clamp(preferred_scale, Integer.MIN_VALUE - 1L, Integer.MAX_VALUE);
        int powsOf2 = intVal.getLowestSetBit();
        // scale - preferred_scale >= remainingZeros >= max{n : (intVal % 10^n) == 0 && n <= scale - preferred_scale}
        // a multiple of 10^n must be a multiple of 2^n
        long remainingZeros = Math.min(scale - preferred_scale, powsOf2);
        if (remainingZeros <= 0L)
            return valueOf(intVal, scale, 0);

        final int sign = intVal.signum;
        if (sign < 0)
            intVal = intVal.negate(); // speed up computation of shiftRight() and bitLength()

        intVal = intVal.shiftRight(powsOf2); // remove powers of 2
        // Let k = max{n : (intVal % 5^n) == 0}, m = max{n : 5^n <= intVal}, so m >= k.
        // Let b = intVal.bitLength(). It can be shown that
        // | b * LOG_5_OF_2 - b log5(2) | < 2^(-21) (fp viz. real arithmetic),
        // which entails m <= maxPowsOf5 <= m + 1, where maxPowsOf5 is as below.
        // Hence, maxPowsOf5 >= k.
        long maxPowsOf5 = Math.round(intVal.bitLength() * LOG_5_OF_2);
        remainingZeros = Math.min(remainingZeros, maxPowsOf5);

        basic_bigint<Allocator>         [] qr; // quotient-remainder pair
        // Remove 5^(2^i) from the factors of intVal, until 5^remainingZeros < 5^(2^i).
        // Let z = max{n >= 0 : ((intVal * 2^powsOf2) % 10^n) == 0 && n <= scale - preferred_scale},
        // then the condition min(scale - preferred_scale, powsOf2) >= remainingZeros >= z
        // and the values ((intVal * 2^powsOf2) / 10^z) and (scale - z)
        // are preserved invariants after each iteration.
        // Note that if intVal % 5^(2^i) != 0, the loop condition will become false.
        for (int i = 0; remainingZeros >= 1L << i; i++) {
            final int exp = 1 << i;
            qr = intVal.divideAndRemainder(fiveToTwoToThe(i));
            if (qr[1].signum != 0) { // non-0 remainder
                remainingZeros = exp - 1;
            } else {
                intVal = qr[0];
                scale = checkScale(intVal, (long) scale - exp); // could Overflow
                remainingZeros -= exp;
                powsOf2 -= exp;
            }
        }

        // bitLength(remainingZeros) == min{n >= 0 : 5^(2^n) > 5^remainingZeros}
        // so, while the loop condition is true,
        // the invariant i == max{n : 5^(2^n) <= 5^remainingZeros},
        // which is equivalent to i == bitLength(remainingZeros) - 1,
        // is preserved at the beginning of each iteration.
        // Note that the loop stops exactly when remainingZeros == 0.
        // Using the same definition of z for the first loop, the invariants
        // min(scale - preferred_scale, powsOf2) >= remainingZeros >= z,
        // ((intVal * 2^powsOf2) / 10^z) and (scale - z)
        // are preserved in this loop as well, so, when the loop ends,
        // remainingZeros == 0 implies z == 0, hence (intVal * 2^powsOf2) and scale
        // have the correct values to return.
        for (int i = basic_bigint<Allocator>::bitLengthForLong(remainingZeros) - 1; i >= 0; i--) {
            final int exp = 1 << i;
            qr = intVal.divideAndRemainder(fiveToTwoToThe(i));
            if (qr[1].signum != 0) { // non-0 remainder
                remainingZeros = exp - 1;
            } else {
                intVal = qr[0];
                scale = checkScale(intVal, (long) scale - exp); // could Overflow
                remainingZeros -= exp;
                powsOf2 -= exp;

                if (remainingZeros < exp >> 1) // else i == bitLength(remainingZeros) already
                    i = basic_bigint<Allocator>         .bitLengthForLong(remainingZeros);
            }
        }

        intVal = intVal.shiftLeft(powsOf2); // restore remaining powers of 2
        return valueOf(sign >= 0 ? intVal : intVal.negate(), scale, 0);
 */
        return bignum_result{};
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

/*
private static BigDecimal divideAndRound(BigInteger bdividend, BigInteger bdivisor, int scale, int roundingMode,
                                         int preferred_scale) {
    boolean isRemainderZero; // record remainder is zero or not
    int qsign; // quotient sign
    // Descend into mutables for faster remainder checks
    MutableBigInteger mdividend = new MutableBigInteger(bdividend.mag);
    MutableBigInteger mq = new MutableBigInteger();
    MutableBigInteger mdivisor = new MutableBigInteger(bdivisor.mag);
    MutableBigInteger mr = mdividend.divide(mdivisor, mq);
    isRemainderZero = mr.isZero();
    qsign = (bdividend.signum != bdivisor.signum) ? -1 : 1;
    if (!isRemainderZero) {
        if (needIncrement(mdivisor, roundingMode, qsign, mq, mr)) {
            mq.add(MutableBigInteger.ONE);
        }
        return mq.toBigDecimal(qsign, scale);
    } else {
        if (preferred_scale != scale) {
            long compactVal = mq.toCompactValue(qsign);
            if (compactVal != INFLATED) {
                return create_and_strip_zeros_to_match_scale(compactVal, scale, preferred_scale);
            }
            BigInteger intVal = mq.toBigInteger(qsign);
            return create_and_strip_zeros_to_match_scale(intVal, scale, preferred_scale);
        } else {
            return mq.toBigDecimal(qsign, scale);
        }
    }
}
private static BigDecimal create_and_strip_zeros_to_match_scale(BigInteger intVal, int scale, long preferred_scale) {
    // avoid overflow of scale - preferred_scale
    preferred_scale = Math.clamp(preferred_scale, Integer.MIN_VALUE - 1L, Integer.MAX_VALUE);
    int powsOf2 = intVal.getLowestSetBit();
    // scale - preferred_scale >= remainingZeros >= max{n : (intVal % 10^n) == 0 && n <= scale - preferred_scale}
    // a multiple of 10^n must be a multiple of 2^n
    long remainingZeros = Math.min(scale - preferred_scale, powsOf2);
    if (remainingZeros <= 0L)
        return valueOf(intVal, scale, 0);

    final int sign = intVal.signum;
    if (sign < 0)
        intVal = intVal.negate(); // speed up computation of shiftRight() and bitLength()

    intVal = intVal.shiftRight(powsOf2); // remove powers of 2
    // Let k = max{n : (intVal % 5^n) == 0}, m = max{n : 5^n <= intVal}, so m >= k.
    // Let b = intVal.bitLength(). It can be shown that
    // | b * LOG_5_OF_2 - b log5(2) | < 2^(-21) (fp viz. real arithmetic),
    // which entails m <= maxPowsOf5 <= m + 1, where maxPowsOf5 is as below.
    // Hence, maxPowsOf5 >= k.
    long maxPowsOf5 = Math.round(intVal.bitLength() * LOG_5_OF_2);
    remainingZeros = Math.min(remainingZeros, maxPowsOf5);

    BigInteger[] qr; // quotient-remainder pair
    // Remove 5^(2^i) from the factors of intVal, until 5^remainingZeros < 5^(2^i).
    // Let z = max{n >= 0 : ((intVal * 2^powsOf2) % 10^n) == 0 && n <= scale - preferred_scale},
    // then the condition min(scale - preferred_scale, powsOf2) >= remainingZeros >= z
    // and the values ((intVal * 2^powsOf2) / 10^z) and (scale - z)
    // are preserved invariants after each iteration.
    // Note that if intVal % 5^(2^i) != 0, the loop condition will become false.
    for (int i = 0; remainingZeros >= 1L << i; i++) {
        final int exp = 1 << i;
        qr = intVal.divideAndRemainder(fiveToTwoToThe(i));
        if (qr[1].signum != 0) { // non-0 remainder
            remainingZeros = exp - 1;
        } else {
            intVal = qr[0];
            scale = checkScale(intVal, (long) scale - exp); // could Overflow
            remainingZeros -= exp;
            powsOf2 -= exp;
        }
    }

    // bitLength(remainingZeros) == min{n >= 0 : 5^(2^n) > 5^remainingZeros}
    // so, while the loop condition is true,
    // the invariant i == max{n : 5^(2^n) <= 5^remainingZeros},
    // which is equivalent to i == bitLength(remainingZeros) - 1,
    // is preserved at the beginning of each iteration.
    // Note that the loop stops exactly when remainingZeros == 0.
    // Using the same definition of z for the first loop, the invariants
    // min(scale - preferred_scale, powsOf2) >= remainingZeros >= z,
    // ((intVal * 2^powsOf2) / 10^z) and (scale - z)
    // are preserved in this loop as well, so, when the loop ends,
    // remainingZeros == 0 implies z == 0, hence (intVal * 2^powsOf2) and scale
    // have the correct values to return.
    for (int i = BigInteger.bitLengthForLong(remainingZeros) - 1; i >= 0; i--) {
        final int exp = 1 << i;
        qr = intVal.divideAndRemainder(fiveToTwoToThe(i));
        if (qr[1].signum != 0) { // non-0 remainder
            remainingZeros = exp - 1;
        } else {
            intVal = qr[0];
            scale = checkScale(intVal, (long) scale - exp); // could Overflow
            remainingZeros -= exp;
            powsOf2 -= exp;

            if (remainingZeros < exp >> 1) // else i == bitLength(remainingZeros) already
                i = BigInteger.bitLengthForLong(remainingZeros);
        }
    }

    intVal = intVal.shiftLeft(powsOf2); // restore remaining powers of 2
    return valueOf(sign >= 0 ? intVal : intVal.negate(), scale, 0);
}
public BigDecimal[] divideAndRemainder(BigDecimal divisor) {
    // we use the identity  x = i * y + r to determine r
    BigDecimal[] result = new BigDecimal[2];

    result[0] = this.divideToIntegralValue(divisor);
    result[1] = this.subtract(result[0].multiply(divisor));
    return result;
}
*/
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
        uint64_t mcp = 34;
        uint64_t dividend_scale = a.precision();
        uint64_t divisor_scale = b.precision();

        if (!add_overflow(preferred_scale, divisor_scale) && preferred_scale > divisor_scale)
        {
            uint64_t new_scale = preferred_scale + divisor_scale;
            uint64_t n = new_scale - dividend_scale;
            basic_bigint<Alloc> scaled_dividend = a * big_ten_to_the(n);
            return divide_and_round(scaled_dividend, b);
        }
        else if (!subtract_overflow(dividend_scale, preferred_scale))
        {
            uint64_t new_scale = dividend_scale - preferred_scale;
            uint64_t n = new_scale - divisor_scale;
            basic_bigint<Alloc> scaled_divisor = b * big_ten_to_the(n);
            return divide_and_round(a, scaled_divisor);
        }
        else
        {
            return bignum_result{bignum_errc::result_out_of_range};
        }
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
