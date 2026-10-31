// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_NONSTD_UINT128_HPP
#define JSONCONS_NONSTD_UINT128_HPP

#include <limits>
#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <string>
#include <stdexcept>

#include <jsoncons/nonstd/compiler_support.hpp>

namespace jsoncons { 
namespace nonstd {

struct uint128 
{
    uint64_t hi_;
    uint64_t lo_;

    // Constructors
    constexpr uint128() : hi_(0), lo_(0) {}
    constexpr uint128(uint64_t lo) : hi_(0), lo_(lo) {}
    constexpr uint128(uint64_t hi, uint64_t lo) : hi_(hi), lo_(lo) {}

    explicit operator bool() const {
        return hi_ != 0 || lo_ != 0;
    }

    bool operator!() const {
        return hi_ == 0 && lo_ == 0;
    }

    template<typename T>
    explicit operator T() const
    {
        static_assert(std::is_integral<T>::value, "T must be an integral type");
        static_assert(sizeof(T) <= sizeof(uint64_t), "T must be 64 bits or less");

        return static_cast<T>(lo_);
    }    

    // Addition
    uint128 operator+(uint128 rhs) const {
        uint64_t new_low = lo_ + rhs.lo_;
        // Carry occurs if the result wrapped around and is less than one of the inputs
        uint64_t carry = (new_low < lo_) ? 1 : 0;
        uint64_t new_high = hi_ + rhs.hi_ + carry;
        return {new_high, new_low};
    }

    uint128& operator+=(uint128 rhs) {
        *this = *this + rhs;
        return *this;
    } 

    bool operator<(uint128 rhs) const
    {
        if (hi_ != rhs.hi_)
            return hi_ < rhs.hi_;

        return lo_ < rhs.lo_;
    }

    bool operator>(uint128 rhs) const
    {
        if (hi_ != rhs.hi_)
            return hi_ > rhs.hi_;

        return lo_ > rhs.lo_;
    }

    bool operator>=(uint128 rhs) const
    {
        if (hi_ != rhs.hi_)
            return hi_ > rhs.hi_;

        return lo_ >= rhs.lo_;
    }

    // Left Shift
    uint128 operator<<(unsigned int shift) const {
        if (shift == 0) return *this;
        if (shift >= 128) return {0, 0};

        if (shift >= 64) {
            return {lo_ << (shift - 64), 0};
        }
        else {
            return {
                (hi_ << shift) | (lo_ >> (64 - shift)),
                lo_ << shift
            };
        }
    }

    // Right Shift
    uint128 operator>>(unsigned int shift) const {
        if (shift == 0) return *this;
        if (shift >= 128) return {0, 0};

        if (shift >= 64) {
            return {0, hi_ >> (shift - 64)};
        }
        else {
            return {
                hi_ >> shift,
                (lo_ >> shift) | (hi_ << (64 - shift))
            };
        }
    }

    uint128 operator*(uint128 rhs) const {
        // Split components into 32-bit chunks to avoid 64-bit overflow during multiplication
        uint64_t a3 = hi_ >> 32;
        uint64_t a2 = hi_ & 0xFFFFFFFFFFFFFFULL; // lower 32 bits of hi_
        uint64_t a1 = lo_ >> 32;
        uint64_t a0 = lo_ & 0xFFFFFFFFULL;

        uint64_t b3 = rhs.hi_ >> 32;
        uint64_t b2 = rhs.hi_ & 0xFFFFFFFFFFFFFFULL;
        uint64_t b1 = rhs.lo_ >> 32;
        uint64_t b0 = rhs.lo_ & 0xFFFFFFFFULL;

        // Long multiplication matrix
        // We only care about terms that fit in the final 128 bits.
        // Chunks exceeding indices that add up to 3 (e.g., a2*b2, a3*b1) drop out of range.

        uint64_t c0 = a0 * b0;
        uint64_t c1 = a0 * b1 + a1 * b0;
        uint64_t c2 = a0 * b2 + a1 * b1 + a2 * b0;
        uint64_t c3 = a0 * b3 + a1 * b2 + a2 * b1 + a3 * b0;

        // Carry propagation
        c1 += (c0 >> 32);
        c2 += (c1 >> 32);
        c3 += (c2 >> 32);

        // Reconstruct hi_ and lo_ 64-bit values
        uint64_t new_low = (c0 & 0xFFFFFFFFULL) | (c1 << 32);
        uint64_t new_high = (c2 & 0xFFFFFFFFULL) | (c3 << 32);

        return {new_high, new_low};
    }

    uint128& operator*=(uint128 rhs) {
        *this = *this * rhs;
        return *this;
    }
    // Equality Comparisons
    bool operator==(uint128 rhs) const { return hi_ == rhs.hi_ && lo_ == rhs.lo_; }
    bool operator!=(uint128 rhs) const { return !(*this == rhs); }

    // Explicit conversion operator to uint64_t
        // Returns the lower 64 bits and discards the hi_ 64 bits
    explicit operator uint64_t() const {
        return lo_;
    }
    // Bitwise OR
    constexpr uint128 operator|(uint128 rhs) const {
        return {hi_ | rhs.hi_, lo_ | rhs.lo_};
    }

    uint128& operator|=(uint128 rhs) {
        hi_ |= rhs.hi_;
        lo_ |= rhs.lo_;
        return *this;
    }
/*
    bool operator<(uint128 rhs) const {
        if (hi_ != rhs.hi_) {
            return hi_ < rhs.hi_;
        }
        return lo_ < rhs.lo_;
    }
*/
    bool operator<=(uint128 rhs) const {
        return *this < rhs || *this == rhs;
    }

    // 128-bit Division Operator
    uint128 operator/(uint128 rhs) const {
        // 1. Handle division by zero
        if (rhs.hi_ == 0 && rhs.lo_ == 0) {
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
        uint128 quotient = {0, 0};
        uint128 remainder = {0, 0};

        // Standard binary long division tracking bit-by-bit from MSB to LSB
        for (int i = 127; i >= 0; --i) {
            // Shift remainder left by 1 and pull down the next bit from dividend (*this)
            remainder = remainder << 1;

            // Extract the i-th bit of the dividend
            uint128 current_bit = (*this >> i) & uint128(1);
            remainder = remainder | current_bit;

            // If the remainder is larger than or equal to the divisor, subtract
            if (rhs <= remainder) {
                remainder = remainder - rhs; // Note: Requires operator- (shown below)
                quotient = quotient | (uint128(1) << i);
            }
        }

        return quotient;
    }

    uint128& operator/=(uint128 rhs) {
        *this = *this / rhs;
        return *this;
    }

    // Helper operator- required for the division algorithm subtraction step
    uint128 operator-(uint128 rhs) const {
        uint64_t new_low = lo_ - rhs.lo_;
        // Borrow occurs if lo_ wrapped around
        uint64_t borrow = (lo_ < rhs.lo_) ? 1 : 0;
        uint64_t new_high = hi_ - rhs.hi_ - borrow;
        return {new_high, new_low};
    }

    uint128& operator>>=(unsigned int shift) {
        *this = *this >> shift;
        return *this;
    }
    // Bitwise AND
    constexpr uint128 operator&(uint128 rhs) const {
        return {hi_ & rhs.hi_, lo_ & rhs.lo_};
    }

    uint128& operator&=(uint128 rhs) {
        hi_ &= rhs.hi_;
        lo_ &= rhs.lo_;
        return *this;
    }
    // 128-bit Modulo Operator
    uint128 operator%(uint128 rhs) const {
        // 1. Handle division by zero
        if (rhs.hi_ == 0 && rhs.lo_ == 0) {
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
        uint128 remainder = {0, 0};

        for (int i = 127; i >= 0; --i) {
            // Shift remainder left by 1 and pull down the next bit from the dividend
            remainder = remainder << 1;

            uint128 current_bit = (*this >> i) & uint128(1);
            remainder = remainder | current_bit;

            // If the remainder is larger than or equal to the divisor, subtract
            if (rhs <= remainder) {
                remainder = remainder - rhs; // Uses operator-
            }
        }

        return remainder;
    }

    uint128& operator%=(uint128 rhs) {
        *this = *this % rhs;
        return *this;
    }

    template<typename T>
    friend typename std::enable_if<std::is_integral<T>::value && (sizeof(T) <= sizeof(uint64_t)),bool>::type
    operator==(uint128 lhs, T rhs)
    {
        return lhs.hi_ == 0 && lhs.lo_ == rhs;
    }

    template<typename T>
    friend typename std::enable_if<std::is_integral<T>::value &&(sizeof(T) <= sizeof(uint64_t)),bool>::type
    operator==(T lhs, uint128 rhs)
    {
        return rhs == lhs;
    }
};

} // namespace nonstd
} // namespace jsoncons

#endif // JSONCONS_NONSTD_UINT128_HPP