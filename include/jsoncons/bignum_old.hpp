// Copyright 2018 vDaniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BIGNUM_HPP
#define JSONCONS_BIGNUM_HPP

#include <cstdint>
#include <vector> // std::vector
#include <iostream>
#include <climits>
#include <cassert> // assert
#include <limits> // std::numeric_limits
#include <algorithm> // std::max, std::min, std::reverse
#include <string> // std::string
#include <cstring> // std::memcpy
#include <cmath> // std::fmod
#include <memory> // std::allocator
#include <initializer_list> // std::initializer_list
#include <type_traits> // std::enable_if
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {

/*
This implementation is based on Chapter 2 and Appendix A of
Ammeraal, L. (1996) Algorithms and Data Structures in C++,
Chichester: John Wiley.

*/

template <class Allocator>
class basic_bignum_base
{
public:
    using allocator_type = Allocator;
    using basic_type_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<uint64_t>;

private:
    basic_type_allocator_type alloc_;

public:
    basic_bignum_base()
        : alloc_()
    {
    }
    explicit basic_bignum_base(const allocator_type& alloc)
        : alloc_(basic_type_allocator_type(alloc))
    {
    }

    basic_type_allocator_type get_allocator() const
    {
        return alloc_;
    }
};

template <class Allocator = std::allocator<uint64_t>>
class basic_bignum : protected basic_bignum_base<Allocator>
{
private:
    using base_t = basic_bignum_base<Allocator>;
    using bignum_type = basic_bignum<Allocator>;
    using base_t::get_allocator;

    static constexpr uint64_t max_basic_type = (std::numeric_limits<uint64_t>::max)();
    static constexpr uint64_t basic_type_bits = sizeof(uint64_t) * 8;  // Number of bits
    static constexpr uint64_t basic_type_halfBits = basic_type_bits/2;

    static constexpr uint16_t word_length = 4; // Use multiples of word_length words
    static constexpr uint64_t r_mask = (uint64_t(1) << basic_type_halfBits) - 1;
    static constexpr uint64_t l_mask = max_basic_type - r_mask;
    static constexpr uint64_t l_bit = max_basic_type - (max_basic_type >> 1);

    union
    {
        std::size_t capacity_;
        uint64_t values_[2];
    };
    uint64_t* data_;
    bool      is_negative_;
    bool      is_dynamic_;
    std::size_t    length_;
public:
    bool is_dynamic() const {return is_dynamic_;}
    bool is_negative() const {return is_negative_;}

//  Constructors and Destructor
    basic_bignum()
        : values_{0,0}, data_(values_), is_negative_(false), is_dynamic_(false), length_(0)
    {
    }

    explicit basic_bignum(const Allocator& alloc)
        : basic_bignum_base<Allocator>(alloc), values_{0,0}, data_(values_), is_negative_(false), is_dynamic_(false), length_(0)
    {
    }

    basic_bignum(const basic_bignum<Allocator>& n)
        : basic_bignum_base<Allocator>(n.get_allocator()), is_negative_(n.is_negative()), length_(n.length_)
    {
        if (!n.is_dynamic())
        {
            values_[0] = n.values_[0];
            values_[1] = n.values_[1];
            data_ = values_;
            is_dynamic_ = false;
        }
        else
        {
            capacity_ = n.capacity_;
            data_ = get_allocator().allocate(capacity_);
            is_dynamic_ = true;
            std::memcpy( data_, n.data_, n.length_*sizeof(uint64_t) );
        }
    }

    basic_bignum(basic_bignum<Allocator>&& other) noexcept
        : basic_bignum_base<Allocator>(other.get_allocator()), is_negative_(other.is_negative()), is_dynamic_(other.is_dynamic()), length_(other.length_)
    {
        if (other.is_dynamic())
        {
            capacity_ = other.capacity_;
            data_ = other.data_;

            other.data_ = other.values_;
            other.is_dynamic_ = false;
            other.length_ = 0;
            other.is_negative_ = false;
        }
        else
        {
            values_[0] = other.data_[0];
            values_[1] = other.data_[1];
            data_ = values_;
        }
    }

    basic_bignum(int signum, std::initializer_list<uint8_t> l)
        : values_{0,0}
    {
        if (l.size() > 0)
        {
            basic_bignum<Allocator> v = 0;
            for (auto c: l)
            {
                v = (v * 256) + (uint64_t)(c);
            }

            if (signum == -1)
            {
                v = -1 - v;
            }

            initialize(v);
        }
        else
        {
            is_negative_ = false;
            initialize_from_integer(0u);
        }
    }

    basic_bignum(short i)
        : values_{0,0}
    {
        is_negative_ = i < 0;
        uint64_t u = is_negative() ? -i : i;
        initialize_from_integer( u );
    }

    basic_bignum(unsigned short u)
        : values_{0,0}
    {
        is_negative_ = false;
        initialize_from_integer( u );
    }

    basic_bignum(int i)
        : values_{0,0}
    {
        is_negative_ = i < 0;
        uint64_t u = is_negative() ? -i : i;
        initialize_from_integer( u );
    }

    basic_bignum(unsigned int u)
        : values_{0,0}
    {
        is_negative_ = false;
        initialize_from_integer( u );
    }

    basic_bignum(long i)
        : values_{0,0}
    {
        is_negative_ = i < 0;
        uint64_t u = is_negative() ? -i : i;
        initialize_from_integer( u );
    }

    basic_bignum(unsigned long u)
        : values_{0,0}
    {
        is_negative_ = false;
        initialize_from_integer( u );
    }

    basic_bignum(long long i)
        : values_{0,0}
    {
        is_negative_ = i < 0;
        uint64_t u = is_negative() ? -i : i;
        initialize_from_integer( u );
    }

    basic_bignum(unsigned long long u)
        : values_{0,0}
    {
        is_negative_ = false;
        initialize_from_integer( u );
    }

    explicit basic_bignum( double x )
        : values_{0,0}
    {
        bool neg = false;

        if ( x < 0 )
        {
            neg = true;
            x = -x;
        }
        basic_bignum<Allocator> v = 0;

        double values = (double)max_basic_type + 1.0;
        basic_bignum<Allocator> factor = 1;

        while ( x >= 1 )
        {
            uint64_t u = (uint64_t) std::fmod( x, values );
            v = v + factor* basic_bignum<Allocator>(u);
            x /= values;
            factor = factor*(basic_bignum<Allocator>( max_basic_type ) + basic_bignum<Allocator>(1));
        }

        if ( neg )
        {
            v.is_negative_ = true;
        }
        initialize( v );
    }

    explicit basic_bignum(long double x)
        : values_{0,0}
    {
        bool neg = false;

        if ( x < 0 )
        {
            neg = true;
            x = -x;
        }

        basic_bignum<Allocator> v = 0;

        long double values = (long double)max_basic_type + 1.0;
        basic_bignum<Allocator> factor = 1;

        while ( x >= 1.0 )
        {
            uint64_t u = (uint64_t) std::fmod( x, values );
            v = v + factor* basic_bignum<Allocator>(u);
            x /= values;
            factor = factor*(basic_bignum<Allocator>( max_basic_type ) + basic_bignum<Allocator>(1));
        }

        if ( neg )
        {
            v.is_negative_ = true;
        }
        initialize( v );
    }

    ~basic_bignum() noexcept
    {
        if ( is_dynamic() )
        {
            get_allocator().deallocate(data_, capacity_);
        }
    }

    static basic_bignum from_be(const uint8_t* str, std::size_t n)
    {
        double radix_log2 = std::log2(next_power_of_two(256));
        // Estimate how big the result will be, so we can pre-allocate it.
        double bits = radix_log2 * n;
        double big_digits = std::ceil(bits / 64.0);
        std::cout << "ESTIMATED: " << big_digits << "\n";

        bignum_type v = 0;
        v.reserve(static_cast<std::size_t>(big_digits));

        if (n > 0)
        {
            for (std::size_t i = 0; i < n; i++)
            {
                v = (v * 256) + (uint64_t)(str[i]);
            }
        }
        std::cout << "ACTUAL: " << v.length() << "\n";

        return v;
    }

    std::size_t capacity() const { return is_dynamic() ? capacity_ : 2; }

//  Operators
    bool operator!() const
    {
        return length() == 0 ? true : false;
    }

    basic_bignum operator-() const
    {
        basic_bignum<Allocator> v = *this;
        v.is_negative_ = !v.is_negative();
        return v;
    }

    basic_bignum& operator=( const basic_bignum<Allocator>& y )
    {
        if ( this != &y )
        {
            resize( y.length() );
            is_negative_ = y.is_negative();
            if ( y.length() > 0 )
            {
                std::memcpy( data_, y.data_, y.length()*sizeof(uint64_t) );
            }
        }
        return *this;
    }

    basic_bignum& operator+=( const basic_bignum<Allocator>& y )
    {
        if ( is_negative() != y.is_negative() )
            return *this -= -y;
        uint64_t d;
        uint64_t carry = 0;

        incr_length( (std::max)(y.length(), length()) + 1 );

        for (std::size_t i = 0; i < length(); i++ )
        {
            if ( i >= y.length() && carry == 0 )
                break;
            d = data_[i] + carry;
            carry = d < carry;
            if ( i < y.length() )
            {
                data_[i] = d + y.data_[i];
                if ( data_[i] < d )
                    carry = 1;
            }
            else
                data_[i] = d;
        }
        reduce();
        return *this;
    }

    basic_bignum& operator-=( const basic_bignum<Allocator>& y )
    {
        if ( is_negative() != y.is_negative() )
            return *this += -y;
        if ( (!is_negative() && y > *this) || (is_negative() && y < *this) )
            return *this = -(y - *this);
        uint64_t borrow = 0;
        uint64_t d;
        for (std::size_t i = 0; i < length(); i++ )
        {
            if ( i >= y.length() && borrow == 0 )
                break;
            d = data_[i] - borrow;
            borrow = d > data_[i];
            if ( i < y.length())
            {
                data_[i] = d - y.data_[i];
                if ( data_[i] > d )
                    borrow = 1;
            }
            else 
                data_[i] = d;
        }
        reduce();
        return *this;
    }

    basic_bignum& operator*=( int64_t y )
    {
        *this *= uint64_t(y < 0 ? -y : y);
        if ( y < 0 )
            is_negative_ = !is_negative();
        return *this;
    }

    basic_bignum& operator*=( uint64_t y )
    {
        std::size_t len0 = length();
        uint64_t hi;
        uint64_t lo;
        uint64_t dig = data_[0];
        uint64_t carry = 0;

        incr_length( length() + 1 );

        std::size_t i = 0;
        for (i = 0; i < len0; i++ )
        {
            DDproduct( dig, y, hi, lo );
            data_[i] = lo + carry;
            dig = data_[i+1];
            carry = hi + (data_[i] < lo);
        }
        data_[i] = carry;
        reduce();
        return *this;
    }

    basic_bignum& operator*=( basic_bignum<Allocator> y )
    {
        if ( length() == 0 || y.length() == 0 )
                    return *this = 0;
        bool difSigns = is_negative() != y.is_negative();
        if ( length() + y.length() == 2 ) // length() = y.length() = 1
        {
            uint64_t a = data_[0], b = y.data_[0];
            data_[0] = a * b;
            if ( data_[0] / a != b )
            {
                resize( 2 );
                DDproduct( a, b, data_[1], data_[0] );
            }
            is_negative_ = difSigns;
            return *this;
        }
        if ( length() == 1 )  //  && y.length() > 1
        {
            uint64_t digit = data_[0];
            *this = y;
            *this *= digit;
        }
        else
        {
            if ( y.length() == 1 )
                *this *= y.data_[0];
            else
            {
                std::size_t lenProd = length() + y.length(), jA, jB;
                uint64_t sumHi = 0, sumLo, hi, lo,
                sumLo_old, sumHi_old, carry=0;
                basic_bignum<Allocator> x = *this;
                resize( lenProd ); // Give *this length lenProd

                for (std::size_t i = 0; i < lenProd; i++ )
                {
                    sumLo = sumHi;
                    sumHi = carry;
                    carry = 0;
                    for ( jA=0; jA < x.length(); jA++ )
                    {
                        jB = i - jA;
                        if ( jB >= 0 && jB < y.length() )
                        {
                            DDproduct( x.data_[jA], y.data_[jB], hi, lo );
                            sumLo_old = sumLo;
                            sumHi_old = sumHi;
                            sumLo += lo;
                            if ( sumLo < sumLo_old )
                                sumHi++;
                            sumHi += hi;
                            carry += (sumHi < sumHi_old);
                        }
                    }
                    data_[i] = sumLo;
                }
            }
        }
       reduce();
       is_negative_ = difSigns;
       return *this;
    }

    basic_bignum& operator/=( const basic_bignum<Allocator>& divisor )
    {
        basic_bignum<Allocator> r;
        divide( divisor, *this, r, false );
        return *this;
    }

    basic_bignum& operator%=( const basic_bignum<Allocator>& divisor )
    {
        basic_bignum<Allocator> q;
        divide( divisor, q, *this, true );
        return *this;
    }

    basic_bignum& operator<<=( uint64_t k )
    {
        std::size_t q = (std::size_t)(k / basic_type_bits);
        if ( q ) // Increase length_ by q:
        {
            incr_length(length() + q);
            for (std::size_t i = length(); i-- > 0; )
                data_[i] = ( i < q ? 0 : data_[i - q]);
            k %= basic_type_bits;
        }
        if ( k )  // 0 < k < basic_type_bits:
        {
            uint64_t k1 = basic_type_bits - k;
            uint64_t mask = (1 << k) - 1;
            incr_length( length() + 1 );
            for (std::size_t i = length(); i-- > 0; )
            {
                data_[i] <<= k;
                if ( i > 0 )
                    data_[i] |= (data_[i-1] >> k1) & mask;
            }
        }
        reduce();
        return *this;
    }

    basic_bignum& operator>>=(uint64_t k)
    {
        std::size_t q = (std::size_t)(k / basic_type_bits);
        if ( q >= length() )
        {
            resize( 0 );
            return *this;
        }
        if (q > 0)
        {
            memmove( data_, data_+q, (std::size_t)((length() - q)*sizeof(uint64_t)) );
            resize( length() - q );
            k %= basic_type_bits;
            if ( k == 0 )
            {
                reduce();
                return *this;
            }
        }

        std::size_t n = (std::size_t)(length() - 1);
        int64_t k1 = basic_type_bits - k;
        uint64_t mask = (1 << k) - 1;
        for (std::size_t i = 0; i <= n; i++)
        {
            data_[i] >>= k;
            if ( i < n )
                data_[i] |= ((data_[i+1] & mask) << k1);
        }
        reduce();
        return *this;
    }

    basic_bignum& operator++()
    {
        *this += 1;
        return *this;
    }

    basic_bignum<Allocator> operator++(int)
    {
        basic_bignum<Allocator> old = *this;
        ++(*this);
        return old;
    }

    basic_bignum<Allocator>& operator--()
    {
        *this -= 1;
        return *this;
    }

    basic_bignum<Allocator> operator--(int)
    {
        basic_bignum<Allocator> old = *this;
        --(*this);
        return old;
    }

    basic_bignum& operator|=( const basic_bignum<Allocator>& a )
    {
        if ( length() < a.length() )
        {
            incr_length( a.length() );
        }

        const uint64_t* qBegin = a.begin();
        const uint64_t* q =      a.end() - 1;
        uint64_t*       p =      begin() + a.length() - 1;

        while ( q >= qBegin )
        {
            *p-- |= *q--;
        }

        reduce();

        return *this;
    }

    basic_bignum& operator^=( const basic_bignum<Allocator>& a )
    {
        if ( length() < a.length() )
        {
            incr_length( a.length() );
        }

        const uint64_t* qBegin = a.begin();
        const uint64_t* q = a.end() - 1;
        uint64_t* p = begin() + a.length() - 1;

        while ( q >= qBegin )
        {
            *p-- ^= *q--;
        }

        reduce();

        return *this;
    }

    basic_bignum& operator&=( const basic_bignum<Allocator>& a )
    {
        std::size_t old_length = length();

        resize( (std::min)( length(), a.length() ) );

        const uint64_t* pBegin = begin();
        uint64_t* p = end() - 1;
        const uint64_t* q = a.begin() + length() - 1;

        while ( p >= pBegin )
        {
            *p-- &= *q--;
        }

        if ( old_length > length() )
        {
            memset( data_ + length(), 0, old_length - length() );
        }

        reduce();

        return *this;
    }

    explicit operator bool() const
    {
       return length() != 0 ? true : false;
    }

    explicit operator int64_t() const
    {
       int64_t x = 0;
       if ( length() > 0 )
       {
           x = data_ [0];
       }

       return is_negative() ? -x : x;
    }

    explicit operator uint64_t() const
    {
       uint64_t u = 0;
       if ( length() > 0 )
       {
           u = data_ [0];
       }

       return u;
    }

    explicit operator double() const
    {
        double x = 0.0;
        double factor = 1.0;
        double values = (double)max_basic_type + 1.0;

        const uint64_t* p = begin();
        const uint64_t* pEnd = end();
        while ( p < pEnd )
        {
            x += *p*factor;
            factor *= values;
            ++p;
        }

       return is_negative() ? -x : x;
    }

    explicit operator long double() const
    {
        long double x = 0.0;
        long double factor = 1.0;
        long double values = (long double)max_basic_type + 1.0;

        const uint64_t* p = begin();
        const uint64_t* pEnd = end();
        while ( p < pEnd )
        {
            x += *p*factor;
            factor *= values;
            ++p;
        }

       return is_negative() ? -x : x;
    }

    template <typename Alloc>
    void dump(int& signum, std::vector<uint8_t,Alloc>& data) const
    {
        basic_bignum<Allocator> n(*this);
        if (is_negative())
        {
            signum = -1;
            n = - n -1;
        }
        else
        {
            signum = 1;
        }
        basic_bignum<Allocator> divisor(256);

        while (n >= 256)
        {
            basic_bignum<Allocator> q;
            basic_bignum<Allocator> r;
            n.divide(divisor, q, r, true);
            n = q;
            data.push_back((uint8_t)(uint64_t)r);
        }
        if (n >= 0)
        {
            data.push_back((uint8_t)(uint64_t)n);
        }
        std::reverse(data.begin(),data.end());
    }

    template <typename Ch, typename Traits, typename Alloc>
    void dump(std::basic_string<Ch,Traits,Alloc>& data) const
    {
        basic_bignum<Allocator> v(*this);

        int len = int(uint32_t(v.length()) * basic_bignum<Allocator>::basic_type_bits / 3) + 2;
        data.resize(len);

        int n = len;
        int i = 0;
                                      // 1/3 > ln(2)/ln(10)
        static uint64_t p10 = 1;
        static uint64_t ip10 = 0;

        if ( v.length() == 0 )
        {
            data[0] = '0';
            i = 1;
        }
        else
        {
            uint64_t r;
            if ( p10 == 1 )
            {
                while ( p10 <= (std::numeric_limits<uint64_t>::max)()/10 )
                {
                    p10 *= 10;
                    ip10++;
                }
            }                     // p10 is max unsigned power of 10
            basic_bignum<Allocator> R;
            basic_bignum<Allocator> LP10 = p10; // LP10 = p10 = ::pow(10, ip10)
            if ( v.is_negative() )
            {
                data[0] = '-';
                i = 1;
            }
            do
            {
                v.divide( LP10, v, R, true );
                r = (R.length() ? R.data_[0] : 0);
                for ( std::size_t j=0; j < ip10; j++ )
                {
                    data[--n] = char(r % 10 + '0');
                    r /= 10;
                    if ( r + v.length() == 0 )
                        break;
                }
            } while ( v.length() );
            while ( n < len )
                data[i++] = data[n++];
        }
        data.resize(i);
    }

    template <typename Ch, typename Traits, typename Alloc>
    void dump_hex_string(std::basic_string<Ch,Traits,Alloc>& data) const
    {
        basic_bignum<Allocator> v(*this);

        int len = int(uint32_t(v.length()) * basic_bignum<Allocator>::basic_type_bits / 3) + 2;
        data.resize(len);

        int n = len;
        int i = 0;
                                      // 1/3 > ln(2)/ln(10)
        static uint64_t p10 = 1;
        static uint64_t ip10 = 0;

        if ( v.length() == 0 )
        {
            data[0] = '0';
            i = 1;
        }
        else
        {
            uint64_t r;
            if ( p10 == 1 )
            {
                while ( p10 <= (std::numeric_limits<uint64_t>::max)()/16 )
                {
                    p10 *= 16;
                    ip10++;
                }
            }                     // p10 is max unsigned power of 16
            basic_bignum<Allocator> R;
            basic_bignum<Allocator> LP10 = p10; // LP10 = p10 = ::pow(16, ip10)
            if ( v.is_negative() )
            {
                data[0] = '-';
                i = 1;
            }
            do
            {
                v.divide( LP10, v, R, true );
                r = (R.length() ? R.data_[0] : 0);
                for ( std::size_t j=0; j < ip10; j++ )
                {
                    uint8_t c = r % 16;
                    data[--n] = (c < 10) ? ('0' + c) : ('A' - 10 + c);
                    r /= 16;
                    if ( r + v.length() == 0 )
                        break;
                }
            } while ( v.length() );
            while ( n < len )
                data[i++] = data[n++];
        }
        data.resize(i);
    }

//  Global Operators

    friend bool operator==( const basic_bignum<Allocator>& x, const basic_bignum<Allocator>& y )
    {
        return x.compare(y) == 0 ? true : false;
    }

    friend bool operator==( const basic_bignum<Allocator>& x, int y )
    {
        return x.compare(y) == 0 ? true : false;
    }

    friend bool operator!=( const basic_bignum<Allocator>& x, const basic_bignum<Allocator>& y )
    {
        return x.compare(y) != 0 ? true : false;
    }

    friend bool operator!=( const basic_bignum<Allocator>& x, int y )
    {
        return x.compare(basic_bignum<Allocator>(y)) != 0 ? true : false;
    }

    friend bool operator<( const basic_bignum<Allocator>& x, const basic_bignum<Allocator>& y )
    {
       return x.compare(y) < 0 ? true : false;
    }

    friend bool operator<( const basic_bignum<Allocator>& x, int64_t y )
    {
       return x.compare(y) < 0 ? true : false;
    }

    friend bool operator>( const basic_bignum<Allocator>& x, const basic_bignum<Allocator>& y )
    {
        return x.compare(y) > 0 ? true : false;
    }

    friend bool operator>( const basic_bignum<Allocator>& x, int y )
    {
        return x.compare(basic_bignum<Allocator>(y)) > 0 ? true : false;
    }

    friend bool operator<=( const basic_bignum<Allocator>& x, const basic_bignum<Allocator>& y )
    {
        return x.compare(y) <= 0 ? true : false;
    }

    friend bool operator<=( const basic_bignum<Allocator>& x, int y )
    {
        return x.compare(y) <= 0 ? true : false;
    }

    friend bool operator>=( const basic_bignum<Allocator>& x, const basic_bignum<Allocator>& y )
    {
        return x.compare(y) >= 0 ? true : false;
    }

    friend bool operator>=( const basic_bignum<Allocator>& x, int y )
    {
        return x.compare(y) >= 0 ? true : false;
    }

    friend basic_bignum<Allocator> operator+( basic_bignum<Allocator> x, const basic_bignum<Allocator>& y )
    {
        return x += y;
    }

    friend basic_bignum<Allocator> operator+( basic_bignum<Allocator> x, int64_t y )
    {
        return x += y;
    }

    friend basic_bignum<Allocator> operator-( basic_bignum<Allocator> x, const basic_bignum<Allocator>& y )
    {
        return x -= y;
    }

    friend basic_bignum<Allocator> operator-( basic_bignum<Allocator> x, int64_t y )
    {
        return x -= y;
    }

    friend basic_bignum<Allocator> operator*( int64_t x, const basic_bignum<Allocator>& y )
    {
        return basic_bignum<Allocator>(y) *= x;
    }

    friend basic_bignum<Allocator> operator*( basic_bignum<Allocator> x, const basic_bignum<Allocator>& y )
    {
        return x *= y;
    }

    friend basic_bignum<Allocator> operator*( basic_bignum<Allocator> x, int64_t y )
    {
        return x *= y;
    }

    friend basic_bignum<Allocator> operator/( basic_bignum<Allocator> x, const basic_bignum<Allocator>& y )
    {
        return x /= y;
    }

    friend basic_bignum<Allocator> operator/( basic_bignum<Allocator> x, int y )
    {
        return x /= y;
    }

    friend basic_bignum<Allocator> operator%( basic_bignum<Allocator> x, const basic_bignum<Allocator>& y )
    {
        return x %= y;
    }

    friend basic_bignum<Allocator> operator<<( basic_bignum<Allocator> u, unsigned k )
    {
        return u <<= k;
    }

    friend basic_bignum<Allocator> operator<<( basic_bignum<Allocator> u, int k )
    {
        return u <<= k;
    }

    friend basic_bignum<Allocator> operator>>( basic_bignum<Allocator> u, unsigned k )
    {
        return u >>= k;
    }

    friend basic_bignum<Allocator> operator>>( basic_bignum<Allocator> u, int k )
    {
        return u >>= k;
    }

    friend basic_bignum<Allocator> operator|( basic_bignum<Allocator> x, const basic_bignum<Allocator>& y )
    {
        return x |= y;
    }

    friend basic_bignum<Allocator> operator|( basic_bignum<Allocator> x, int y )
    {
        return x |= y;
    }

    friend basic_bignum<Allocator> operator|( basic_bignum<Allocator> x, unsigned y )
    {
        return x |= y;
    }

    friend basic_bignum<Allocator> operator^( basic_bignum<Allocator> x, const basic_bignum<Allocator>& y )
    {
        return x ^= y;
    }

    friend basic_bignum<Allocator> operator^( basic_bignum<Allocator> x, int y )
    {
        return x ^= y;
    }

    friend basic_bignum<Allocator> operator^( basic_bignum<Allocator> x, unsigned y )
    {
        return x ^= y;
    }

    friend basic_bignum<Allocator> operator&( basic_bignum<Allocator> x, const basic_bignum<Allocator>& y )
    {
        return x &= y;
    }

    friend basic_bignum<Allocator> operator&( basic_bignum<Allocator> x, int y )
    {
        return x &= y;
    }

    friend basic_bignum<Allocator> operator&( basic_bignum<Allocator> x, unsigned y )
    {
        return x &= y;
    }

    friend basic_bignum<Allocator> abs( const basic_bignum<Allocator>& a )
    {
        if ( a.is_negative() )
        {
            return -a;
        }
        return a;
    }

    friend basic_bignum<Allocator> power( basic_bignum<Allocator> x, unsigned n )
    {
        basic_bignum<Allocator> y = 1;

        while ( n )
        {
            if ( n & 1 )
            {
                y *= x;
            }
            x *= x;
            n >>= 1;
        }

        return y;
    }

    friend basic_bignum<Allocator> sqrt( const basic_bignum<Allocator>& a )
    {
        basic_bignum<Allocator> x = a;
        basic_bignum<Allocator> b = a;
        basic_bignum<Allocator> q;

        b <<= 1;
        while ( b >>= 2, b > 0 )
        {
            x >>= 1;
        }
        while ( x > (q = a/x) + 1 || x < q - 1 )
        {
            x += q;
            x >>= 1;
        }
        return x < q ? x : q;
    }

    template <class CharT>
    friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const basic_bignum<Allocator>& v)
    {
        std::basic_string<CharT> s; 
        v.dump(s);
        os << s;

        return os;
    }

    int compare( const basic_bignum<Allocator>& y ) const
    {
        if ( is_negative() != y.is_negative() )
            return y.is_negative() - is_negative();
        int code = 0;
        if ( length() == 0 && y.length() == 0 )
            code = 0;
        else if ( length() < y.length() )
            code = -1;
        else if ( length() > y.length() )
            code = +1;
        else
        {
            for (std::size_t i = length(); i-- > 0; )
            {
                if (data_[i] > y.data_[i])
                {
                    code = 1;
                    break;
                }
                else if (data_[i] < y.data_[i])
                {
                    code = -1;
                    break;
                }
            }
        }
        return is_negative() ? -code : code;
    }

    template <typename CharT>
    static basic_bignum<Allocator> from_string(const std::basic_string<CharT>& s)
    {
        return from_string(s.data(), s.length());
    }

    template <typename CharT>
    static basic_bignum<Allocator> from_string(const CharT* s)
    {
        return from_string(s, std::char_traits<CharT>::length(s));
    }

    template <typename CharT>
    static basic_bignum<Allocator> from_string(const CharT* data, std::size_t length)
    {
        bool neg;
        if (*data == '-')
        {
            neg = true;
            data++;
            --length;
        }
        else
        {
            neg = false;
        }

        basic_bignum<Allocator> v = 0;
        for (std::size_t i = 0; i < length; i++)
        {
            CharT c = data[i];
            switch (c)
            {
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    v = (v * 10) + (uint64_t)(c - '0');
                    break;
                default:
                    JSONCONS_THROW(std::runtime_error(std::string("Invalid digit ") + "\'" + (char)c + "\'"));
            }
        }

        if ( neg )
        {
            v.is_negative_ = true;
        }

        return v;
    }

    template <typename CharT>
    static basic_bignum<Allocator> from_string_radix(const CharT* data, std::size_t length, uint8_t base)
    {
        if (!(base >= 2 && base <= 16))
        {
            JSONCONS_THROW(std::runtime_error("Unsupported base"));
        }

        bool neg;
        if (*data == '-')
        {
            neg = true;
            data++;
            --length;
        }
        else
        {
            neg = false;
        }

        basic_bignum<Allocator> v = 0;
        for (std::size_t i = 0; i < length; i++)
        {
            CharT c = data[i];
            uint64_t d;
            switch (c)
            {
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                    d = (uint64_t)(c - '0');
                    break;
                case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
                    d = (uint64_t)(c - ('a' - 10));
                    break;
                case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
                    d = (uint64_t)(c - ('A' - 10));
                    break;
                default:
                    JSONCONS_THROW(std::runtime_error(std::string("Invalid digit in base ") + std::to_string(base) + ": \'" + (char)c + "\'"));
            }
            if (d >= base)
            {
                JSONCONS_THROW(std::runtime_error(std::string("Invalid digit in base ") + std::to_string(base) + ": \'" + (char)c + "\'"));
            }
            v = (v * base) + d;
        }

        if ( neg )
        {
            v.is_negative_ = true;
        }

        return v;
    }

private:
    void DDproduct( uint64_t A, uint64_t B,
                    uint64_t& hi, uint64_t& lo ) const
    // Multiplying two digits: (hi, lo) = A * B
    {
        uint64_t hiA = A >> basic_type_halfBits, loA = A & r_mask,
                   hiB = B >> basic_type_halfBits, loB = B & r_mask,
                   mid1, mid2, old;

        lo = loA * loB;
        hi = hiA * hiB;
        mid1 = loA * hiB;
        mid2 = hiA * loB;
        old = lo;
        lo += mid1 << basic_type_halfBits;
            hi += (lo < old) + (mid1 >> basic_type_halfBits);
        old = lo;
        lo += mid2 << basic_type_halfBits;
            hi += (lo < old) + (mid2 >> basic_type_halfBits);
    }

    uint64_t DDquotient( uint64_t A, uint64_t B, uint64_t d ) const
    // Divide double word (A, B) by d. Quotient = (qHi, qLo)
    {
        uint64_t left, middle, right, qHi, qLo, x, dLo1,
                   dHi = d >> basic_type_halfBits, dLo = d & r_mask;
        qHi = A/(dHi + 1);
        // This initial guess of qHi may be too small.
        middle = qHi * dLo;
        left = qHi * dHi;
        x = B - (middle << basic_type_halfBits);
        A -= (middle >> basic_type_halfBits) + left + (x > B);
        B = x;
        dLo1 = dLo << basic_type_halfBits;
        // Increase qHi if necessary:
        while ( A > dHi || (A == dHi && B >= dLo1) )
        {
            x = B - dLo1;
            A -= dHi + (x > B);
            B = x;
            qHi++;
        }
        qLo = ((A << basic_type_halfBits) | (B >> basic_type_halfBits))/(dHi + 1);
        // This initial guess of qLo may be too small.
        right = qLo * dLo;
        middle = qLo * dHi;
        x = B - right;
        A -= (x > B);
        B = x;
        x = B - (middle << basic_type_halfBits);
            A -= (middle >> basic_type_halfBits) + (x > B);
        B = x;
        // Increase qLo if necessary:
        while ( A || B >= d )
        {
            x = B - d;
            A -= (x > B);
            B = x;
            qLo++;
        }
        return (qHi << basic_type_halfBits) + qLo;
    }

    void subtractmul( uint64_t* a, uint64_t* b, std::size_t n, uint64_t& q ) const
    // a -= q * b: b in n positions; correct q if necessary
    {
        uint64_t hi, lo, d, carry = 0;
        std::size_t i;
        for ( i = 0; i < n; i++ )
        {
            DDproduct( b[i], q, hi, lo );
            d = a[i];
            a[i] -= lo;
            if ( a[i] > d )
                carry++;
            d = a[i + 1];
            a[i + 1] -= hi + carry;
            carry = a[i + 1] > d;
        }
        if ( carry ) // q was too large
        {
            q--;
            carry = 0;
            for ( i = 0; i < n; i++ )
            {
                d = a[i] + carry;
                carry = d < carry;
                a[i] = d + b[i];
                if ( a[i] < d )
                    carry = 1;
            }
            a[n] = 0;
        }
    }

    int normalize( basic_bignum<Allocator>& denom, basic_bignum<Allocator>& num, int& x ) const
    {
        std::size_t r = denom.length() - 1;
        uint64_t y = denom.data_[r];

        x = 0;
        while ( (y & l_bit) == 0 )
        {
            y <<= 1;
            x++;
        }
        denom <<= x;
        num <<= x;
        if ( r > 0 && denom.data_[r] < denom.data_[r-1] )
        {
            denom *= max_basic_type;
                    num *= max_basic_type;
            return 1;
        }
        return 0;
    }

    void unnormalize( basic_bignum<Allocator>& rem, int x, int secondDone ) const
    {
        if ( secondDone )
        {
            rem /= max_basic_type;
        }
        if ( x > 0 )
        {
            rem >>= x;
        }
        else
        {
            rem.reduce();
        }
    }

    void divide( basic_bignum<Allocator> denom, basic_bignum<Allocator>& quot, basic_bignum<Allocator>& rem, bool remDesired ) const
    {
        if ( denom.length() == 0 )
        {
            JSONCONS_THROW(std::runtime_error( "Zero divide." ));
        }
        bool quot_neg = is_negative() ^ denom.is_negative();
        bool rem_neg = is_negative();
        int x = 0;
        basic_bignum<Allocator> num = *this;
        num.is_negative_ = denom.is_negative_ = false;
        if ( num < denom )
        {
            quot = uint64_t(0);
            rem = num;
            rem.is_negative_ = rem_neg;
            return;
        }
        if ( denom.length() == 1 && num.length() == 1 )
        {
            quot = uint64_t( num.data_[0]/denom.data_[0] );
            rem = uint64_t( num.data_[0]%denom.data_[0] );
            quot.is_negative_ = quot_neg;
            rem.is_negative_ = rem_neg;
            return;
        }
        else if (denom.length() == 1 && (denom.data_[0] & l_mask) == 0 )
        {
            // Denominator fits into a half word
            uint64_t divisor = denom.data_[0], dHi = 0,
                     q1, r, q2, dividend;
            quot.resize(length());
            for (std::size_t i=length(); i-- > 0; )
            {
                dividend = (dHi << basic_type_halfBits) | (data_[i] >> basic_type_halfBits);
                q1 = dividend/divisor;
                r = dividend % divisor;
                dividend = (r << basic_type_halfBits) | (data_[i] & r_mask);
                q2 = dividend/divisor;
                dHi = dividend % divisor;
                quot.data_[i] = (q1 << basic_type_halfBits) | q2;
            }
            quot.reduce();
            rem = dHi;
            quot.is_negative_ = quot_neg;
            rem.is_negative_ = rem_neg;
            return;
        }
        basic_bignum<Allocator> num0 = num, denom0 = denom;
        int second_done = normalize(denom, num, x);
        std::size_t l = denom.length() - 1;
        std::size_t n = num.length() - 1;
        quot.resize(n - l);
        for (std::size_t i=quot.length(); i-- > 0; )
            quot.data_[i] = 0;
        rem = num;
        if ( rem.data_[n] >= denom.data_[l] )
        {
            rem.incr_length(rem.length() + 1);
            n++;
            quot.incr_length(quot.length() + 1);
        }
        uint64_t d = denom.data_[l];
        for ( std::size_t k = n; k > l; k-- )
        {
            uint64_t q = DDquotient(rem.data_[k], rem.data_[k-1], d);
            subtractmul( rem.data_ + k - l - 1, denom.data_, l + 1, q );
            quot.data_[k - l - 1] = q;
        }
        quot.reduce();
        quot.is_negative_ = quot_neg;
        if ( remDesired )
        {
            unnormalize(rem, x, second_done);
            rem.is_negative_ = rem_neg;
        }
    }

    std::size_t length() const { return length_; }
    uint64_t* begin() { return data_; }
    const uint64_t* begin() const { return data_; }
    uint64_t* end() { return data_ + length_; }
    const uint64_t* end() const { return data_ + length_; }

    void resize(std::size_t n)
    {
        std::size_t len_old = length_;
       length_ = n;
       if ( length_ > capacity() )
       {
           std::size_t capacity_new = round_up(length_);
           uint64_t* data_old = data_;
           data_ = get_allocator().allocate(capacity_new);
           if ( len_old > 0 )
           {
               std::memcpy( data_, data_old, len_old*sizeof(uint64_t));
           }
           if ( is_dynamic() )
           {
               get_allocator().deallocate(data_old,capacity_);
           }
           capacity_ = capacity_new;
           is_dynamic_ = true;
       }
    }

    void reserve(std::size_t n)
    {
       if (capacity() < n)
       {
           std::size_t capacity_new = round_up(n);
           uint64_t* data_old = data_;
           data_ = get_allocator().allocate(capacity_new);
           if (length_ > 0)
           {
               std::memcpy( data_, data_old, length_*sizeof(uint64_t));
           }
           if ( is_dynamic() )
           {
               get_allocator().deallocate(data_old,capacity_);
           }
           capacity_ = capacity_new;
           is_dynamic_ = true;
       }
    }

    std::size_t round_up(std::size_t i) const // Find suitable new block size
    {
        return (i/word_length + 1) * word_length;
    }

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && 
                            !std::is_signed<T>::value &&
                            sizeof(T) <= sizeof(int64_t),void>::type
    initialize_from_integer(T u)
    {
        data_ = values_;
        is_dynamic_ = false;
        length_ = u != 0 ? 1 : 0;

        data_ [0] = u;
    }
 
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value &&
                           !std::is_signed<T>::value &&
                           sizeof(int64_t) < sizeof(T) &&
                           sizeof(T) <= sizeof(int64_t)*2, void>::type
        initialize_from_integer(T u)
    {
        data_ = values_;
        is_dynamic_ = false;
        length_ = u != 0 ? 2 : 0;

        data_[0] = uint64_t(u & max_basic_type);
        u >>= basic_type_bits;
        data_[1] = uint64_t(u & max_basic_type);
    }

    void initialize( const basic_bignum<Allocator>& n )
    {
        is_negative_ = n.is_negative();
        length_ = n.length_;

        if ( length_ <= 2 )
        {
            data_ = values_;
            is_dynamic_ = false;
            values_ [0] = n.data_ [0];
            values_ [1] = n.data_ [1];
        }
        else
        {
            capacity_ = round_up( length_ );
            data_ = get_allocator().allocate(capacity_);
            is_dynamic_ = true;
            if ( length_ > 0 )
            {
                std::memcpy( data_, n.data_, length_*sizeof(uint64_t) );
            }
            reduce();
        }
    }

    void initialize(basic_bignum<Allocator>&& other)
    {
        is_negative_ = other.is_negative();
        length_ = other.length_;
        is_dynamic_ = other.is_dynamic();

        if (other.is_dynamic())
        {
            capacity_ = other.capacity_;
            data_ = other.data_;

            other.data_ = other.values_;
            other.is_dynamic_ = false;
            other.length_ = 0;
            other.is_negative_ = false;
        }
        else
        {
            values_[0] = other.data_[0];
            values_[1] = other.data_[1];
            data_ = values_;
        }
    }

    void reduce()
    {
        uint64_t* p = end() - 1;
        uint64_t* pBegin = begin();
        while ( p >= pBegin )
        {
            if ( *p )
            {
                break;
            }
            --length_;
            --p;
        }
        if ( length_ == 0 )
        {
            is_negative_ = false;
        }
    }

    void incr_length( std::size_t len_new )
    {
        std::size_t len_old = length_;
        length_ = len_new;  // length_ > len_old

        if ( length_ > capacity() )
        {
            std::size_t capacity_new = round_up( length_ );

            uint64_t* data_old = data_;

            data_ = get_allocator().allocate(capacity_new);

            if ( len_old > 0 )
            {
                std::memcpy( data_, data_old, len_old*sizeof(uint64_t) );
            }
            if ( is_dynamic() )
            {
                get_allocator().deallocate(data_old,capacity_);
            }
            capacity_ = capacity_new;
            is_dynamic_ = true;
        }

        if ( length_ > len_old )
        {
            memset( data_+len_old, 0, (length_ - len_old)*sizeof(uint64_t) );
        }
    }

    static uint64_t next_power_of_two(uint64_t n) {
        n = n - 1;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        return n + 1;
    }
};

using bignum = basic_bignum<std::allocator<uint8_t>>;

}

#endif
