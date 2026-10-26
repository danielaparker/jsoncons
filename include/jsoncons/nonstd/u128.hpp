// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_NONSTD_U128_HPP
#define JSONCONS_NONSTD_U128_HPP

#include <limits>
#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <string>
#include <stdexcept>

#include <jsoncons/nonstd/a5hash.hpp>
#include <jsoncons/nonstd/compiler_support.hpp>

namespace jsoncons { 
namespace nonstd {

struct u128 
{
    uint64_t hi;
    uint64_t lo;

    // Constructors
    constexpr u128() : hi(0), lo(0) {}
    constexpr u128(uint64_t lo) : hi(0), lo(lo) {}
    constexpr u128(uint64_t hi, uint64_t lo) : hi(hi), lo(lo) {}

    explicit operator bool() const {
        return hi != 0 || lo != 0;
    }

    bool operator!() const {
        return hi == 0 && lo == 0;
    }

    // Addition
    u128 operator+(const u128& rhs) const {
        uint64_t new_low = lo + rhs.lo;
        // Carry occurs if the result wrapped around and is less than one of the inputs
        uint64_t carry = (new_low < lo) ? 1 : 0;
        uint64_t new_high = hi + rhs.hi + carry;
        return {new_high, new_low};
    }

    u128& operator+=(const u128& rhs) {
        *this = *this + rhs;
        return *this;
    } 

    bool operator<(const u128& rhs) const
    {
        if (hi != rhs.hi)
            return hi < rhs.hi;

        return lo < rhs.lo;
    }

    bool operator>(const u128& rhs) const
    {
        if (hi != rhs.hi)
            return hi > rhs.hi;

        return lo > rhs.lo;
    }

    bool operator>=(const u128& rhs) const
    {
        if (hi != rhs.hi)
            return hi > rhs.hi;

        return lo >= rhs.lo;
    }

    // Left Shift
    u128 operator<<(unsigned int shift) const {
        if (shift == 0) return *this;
        if (shift >= 128) return {0, 0};

        if (shift >= 64) {
            return {lo << (shift - 64), 0};
        }
        else {
            return {
                (hi << shift) | (lo >> (64 - shift)),
                lo << shift
            };
        }
    }

    // Right Shift
    u128 operator>>(unsigned int shift) const {
        if (shift == 0) return *this;
        if (shift >= 128) return {0, 0};

        if (shift >= 64) {
            return {0, hi >> (shift - 64)};
        }
        else {
            return {
                hi >> shift,
                (lo >> shift) | (hi << (64 - shift))
            };
        }
    }


    u128 operator*(const u128& rhs) const {
        uint64_t new_low, new_high;
        a5hash_umul128(lo, rhs.lo, &new_low, &new_high);
        return {new_high + hi * rhs.lo + lo * rhs.hi, new_low};
    }

    u128& operator*=(const u128& rhs) {
        *this = *this * rhs;
        return *this;
    }
    // Equality Comparisons
    bool operator==(const u128& rhs) const { return hi == rhs.hi && lo == rhs.lo; }
    bool operator!=(const u128& rhs) const { return !(*this == rhs); }

    // Explicit conversion operator to uint64_t
        // Returns the lower 64 bits and discards the hi 64 bits
    explicit operator uint64_t() const {
        return lo;
    }
    // Bitwise OR
    constexpr u128 operator|(const u128& rhs) const {
        return {hi | rhs.hi, lo | rhs.lo};
    }

    u128& operator|=(const u128& rhs) {
        hi |= rhs.hi;
        lo |= rhs.lo;
        return *this;
    }
/*
    bool operator<(const u128& rhs) const {
        if (hi != rhs.hi) {
            return hi < rhs.hi;
        }
        return lo < rhs.lo;
    }
*/
    bool operator<=(const u128& rhs) const {
        return *this < rhs || *this == rhs;
    }

    // 128-bit Division Operator
    u128 operator/(const u128& rhs) const {
        // 1. Handle division by zero
        if (rhs.hi == 0 && rhs.lo == 0) {
            JSONCONS_THROW(std::runtime_error("Division by zero"));
        }

        // 2. Handle cases where the divisor is larger than the dividend
        if (*this < rhs) {
            return {0, 0};
        }

        // 3. Handle identical values
        if (*this == rhs) {
            return {0, 1};
        }

        // 4. Core Shift-and-Subtract Loop
        u128 quotient = {0, 0};
        u128 remainder = {0, 0};

        // Standard binary long division tracking bit-by-bit from MSB to LSB
        for (int i = 127; i >= 0; --i) {
            // Shift remainder left by 1 and pull down the next bit from dividend (*this)
            remainder = remainder << 1;

            // Extract the i-th bit of the dividend
            u128 current_bit = (*this >> i) & u128(1);
            remainder = remainder | current_bit;

            // If the remainder is larger than or equal to the divisor, subtract
            if (rhs <= remainder) {
                remainder = remainder - rhs; // Note: Requires operator- (shown below)
                quotient = quotient | (u128(1) << i);
            }
        }

        return quotient;
    }

    u128& operator/=(const u128& rhs) {
        *this = *this / rhs;
        return *this;
    }

    // Helper operator- required for the division algorithm subtraction step
    u128 operator-(const u128& rhs) const {
        uint64_t new_low = lo - rhs.lo;
        // Borrow occurs if lo wrapped around
        uint64_t borrow = (lo < rhs.lo) ? 1 : 0;
        uint64_t new_high = hi - rhs.hi - borrow;
        return {new_high, new_low};
    }

    u128& operator>>=(unsigned int shift) {
        *this = *this >> shift;
        return *this;
    }
    // Bitwise AND
    constexpr u128 operator&(const u128& rhs) const {
        return {hi & rhs.hi, lo & rhs.lo};
    }

    u128& operator&=(const u128& rhs) {
        hi &= rhs.hi;
        lo &= rhs.lo;
        return *this;
    }
    // 128-bit Modulo Operator
    u128 operator%(const u128& rhs) const {
        // 1. Handle division by zero
        if (rhs.hi == 0 && rhs.lo == 0) {
            JSONCONS_THROW(std::runtime_error("Division by zero in modulo operation"));
        }

        // 2. If divisor is larger than dividend, the remainder is the dividend itself
        if (*this < rhs) {
            return *this;
        }

        // 3. If identical, the remainder is 0
        if (*this == rhs) {
            return {0, 0};
        }

        // 4. Shift-and-Subtract Loop
        u128 remainder = {0, 0};

        for (int i = 127; i >= 0; --i) {
            // Shift remainder left by 1 and pull down the next bit from the dividend
            remainder = remainder << 1;

            u128 current_bit = (*this >> i) & u128(1);
            remainder = remainder | current_bit;

            // If the remainder is larger than or equal to the divisor, subtract
            if (rhs <= remainder) {
                remainder = remainder - rhs; // Uses operator-
            }
        }

        return remainder;
    }

    u128& operator%=(const u128& rhs) {
        *this = *this % rhs;
        return *this;
    }
};

} // namespace nonstd
} // namespace jsoncons

#endif // JSONCONS_NONSTD_U128_HPP