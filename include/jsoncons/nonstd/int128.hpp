// Copyright 2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_NONSTD_INT128_HPP
#define JSONCONS_NONSTD_INT128_HPP

#include <limits>
#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <string>
#include <stdexcept>

#include <jsoncons/nonstd/compiler_support.hpp>

namespace jsoncons { 
namespace nonstd {

#include <cstdint>
#include <stdexcept>

class int128 {
    uint64_t lo_, hi_;

    struct raw_tag {};

    constexpr int128(uint64_t hi, uint64_t lo, raw_tag)
        : lo_(lo), hi_(hi) {}

    static constexpr int128 raw(uint64_t hi, uint64_t lo) {
        return int128(hi, lo, raw_tag{});
    }

    bool negative() const {
        return (hi_ >> 63) != 0;
    }

    // Unsigned comparison of the 128-bit bit patterns.
    static int cmpu(int128 a, int128 b) {
        if (a.hi_ != b.hi_)
            return a.hi_ < b.hi_ ? -1 : 1;
        return a.lo_ == b.lo_ ? 0 :
               (a.lo_ < b.lo_ ? -1 : 1);
    }

    // Unsigned binary long division.
    static void divmodu(int128 n, int128 d,
                        int128& q, int128& r) {
        if (!d)
            throw std::domain_error("int128 division by zero");

        q = 0;
        r = 0;

        for (int i = 127; i >= 0; --i) {
            // Save the bit that would overflow the remainder.
            const bool carry = (r.hi_ >> 63) != 0;

            r <<= 1;

            // Bring down the next dividend bit.
            r.lo_ |= i >= 64
                ? ((n.hi_ >> (i - 64)) & 1)
                : ((n.lo_ >> i) & 1);

            if (carry || cmpu(r, d) >= 0) {
                r -= d;

                if (i >= 64)
                    q.hi_ |= uint64_t(1) << (i - 64);
                else
                    q.lo_ |= uint64_t(1) << i;
            }
        }
    }

    // Signed division with quotient truncated toward zero.
    static void divmods(int128 a, int128 b,
                        int128& q, int128& r) {
        if (!b)
            throw std::domain_error("int128 division by zero");

        if (a == min_() && b == int128(-1))
            throw std::overflow_error("int128 division overflow");

        bool an = a.negative();
        bool bn = b.negative();

        if (an) a = -a;
        if (bn) b = -b;

        divmodu(a, b, q, r);

        if (an != bn) q = -q;
        if (an) r = -r;
    }

public:
    // Constructors
    constexpr int128() : lo_(0), hi_(0) {}

    constexpr int128(int64_t x)
        : lo_(static_cast<uint64_t>(x)),
          hi_(x < 0 ? UINT64_MAX : 0) {}

    static constexpr int128 from_parts(uint64_t hi,
                                     uint64_t lo) {
        return raw(hi, lo);
    }

    template<typename T>
    explicit operator T() const
    {
        static_assert(std::is_integral<T>::value, "T must be an integral type");
        static_assert(sizeof(T) <= sizeof(uint64_t), "T must be 64 bits or less");

        return static_cast<T>(lo_);
    }    

    static constexpr int128 min_() {
        return raw(uint64_t(1) << 63, 0);
    }

    static constexpr int128 max_() {
        return raw((uint64_t(1) << 63) - 1, UINT64_MAX);
    }

    explicit operator bool() const {
        return hi_ != 0 || lo_ != 0;
    }

    uint64_t low() const  { return lo_; }
    uint64_t high() const { return hi_; }

    // Comparisons
    friend bool operator==(int128 a, int128 b) {
        return a.lo_ == b.lo_ && a.hi_ == b.hi_;
    }

    friend bool operator!=(int128 a, int128 b) {
        return !(a == b);
    }

    friend bool operator<(int128 a, int128 b) {
        uint64_t ah = a.hi_ ^ (uint64_t(1) << 63);
        uint64_t bh = b.hi_ ^ (uint64_t(1) << 63);

        return ah != bh ? ah < bh : a.lo_ < b.lo_;
    }

    friend bool operator>(int128 a, int128 b) {
        return b < a;
    }

    friend bool operator<=(int128 a, int128 b) {
        return !(b < a);
    }

    friend bool operator>=(int128 a, int128 b) {
        return !(a < b);
    }

    // Negation and subtraction
    friend int128 operator~(int128 a) {
        return raw(~a.hi_, ~a.lo_);
    }

    friend int128 operator-(int128 a) {
        return ~a + int128(1);
    }

    // Addition with carry
    friend int128 operator+(int128 a, int128 b) {
        uint64_t lo = a.lo_ + b.lo_;

        return raw(
            a.hi_ + b.hi_ + (lo < a.lo_),
            lo
        );
    }

    friend int128 operator-(int128 a, int128 b) {
        return a + (-b);
    }

    // Logical left shift, modulo 2^128
    friend int128 operator<<(int128 a, unsigned s) {
        if (s >= 128) return 0;
        if (s == 0) return a;

        if (s >= 64)
            return raw(a.lo_ << (s - 64), 0);

        return raw(
            (a.hi_ << s) | (a.lo_ >> (64 - s)),
            a.lo_ << s
        );
    }

    // Arithmetic right shift (sign extending)
    friend int128 operator>>(int128 a, unsigned s) {
        if (s >= 128)
            return a.negative() ? int128(-1) : int128(0);

        if (s == 0) return a;

        const uint64_t fill =
            a.negative() ? UINT64_MAX : 0;

        if (s >= 64) {
            unsigned k = s - 64;

            uint64_t lo = k
                ? ((a.hi_ >> k) | (fill << (64 - k)))
                : a.hi_;

            return raw(fill, lo);
        }

        return raw(
            (a.hi_ >> s) | (fill << (64 - s)),
            (a.lo_ >> s) | (a.hi_ << (64 - s))
        );
    }

    // Multiplication using four 32-bit limbs.
    // Only the low 128 bits are retained.
    friend int128 operator*(int128 a, int128 b) {
        uint64_t x[4] = {
            uint32_t(a.lo_), a.lo_ >> 32,
            uint32_t(a.hi_), a.hi_ >> 32
        };

        uint64_t y[4] = {
            uint32_t(b.lo_), b.lo_ >> 32,
            uint32_t(b.hi_), b.hi_ >> 32
        };

        uint64_t z[4] = {};

        for (int i = 0; i < 4; ++i) {
            uint64_t carry = 0;

            for (int j = 0; j < 4 - i; ++j) {
                uint64_t t =
                    x[i] * y[j] + z[i + j] + carry;

                z[i + j] = uint32_t(t);
                carry = t >> 32;
            }
        }

        return raw(
            (z[3] << 32) | z[2],
            (z[1] << 32) | z[0]
        );
    }

    // Division and remainder
    friend int128 operator/(int128 a, int128 b) {
        int128 q, r;
        divmods(a, b, q, r);
        return q;
    }

    friend int128 operator%(int128 a, int128 b) {
        int128 q, r;
        divmods(a, b, q, r);
        return r;
    }

    // Compound assignment
    int128& operator+=(int128 b) {
        return *this = *this + b;
    }

    int128& operator-=(int128 b) {
        return *this = *this - b;
    }

    int128& operator*=(int128 b) {
        return *this = *this * b;
    }

    int128& operator/=(int128 b) {
        return *this = *this / b;
    }

    int128& operator%=(int128 b) {
        return *this = *this % b;
    }

    int128& operator<<=(unsigned s) {
        return *this = *this << s;
    }

    int128& operator>>=(unsigned s) {
        return *this = *this >> s;
    }

    template<typename T>
    friend typename std::enable_if<std::is_integral<T>::value && (sizeof(T) <= sizeof(int64_t)),bool>::type
    operator==(int128 lhs, T rhs)
    {
        return lhs.hi_ == 0 && lhs.lo_ == rhs;
    }

    template<typename T>
    friend typename std::enable_if<
        std::is_integral<T>::value &&
        (sizeof(T) <= sizeof(int64_t)),
        bool
    >::type
    operator==(T lhs, int128 rhs)
    {
        return rhs == lhs;
    }
};

} // namespace nonstd
} // namespace jsoncons

#endif // JSONCONS_NONSTD_INT128_HPP
