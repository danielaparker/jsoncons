// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BIGNUM_HPP
#define JSONCONS_BIGNUM_HPP

#include <cstdint>
#include <iostream>
#include <climits>
#include <limits>
#include <algorithm>
#include <string>
#include <cstring>

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
    typedef uint64_t basic_type;
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<basic_type> byte_allocator_type;

private:
    byte_allocator_type byte_allocator_;

public:
    basic_bignum_base()
        : byte_allocator_()
    {
    }
    explicit basic_bignum_base(const allocator_type& allocator)
        : byte_allocator_(byte_allocator_type(allocator))
    {
    }

    byte_allocator_type allocator() const
    {
        return byte_allocator_;
    }
};

template <class Allocator = std::allocator<uint8_t>>
class basic_bignum : protected basic_bignum_base<Allocator>
{
private:
    using typename basic_bignum_base<Allocator>::basic_type;
    using typename basic_bignum_base<Allocator>::byte_allocator_type;
    using basic_bignum_base<Allocator>::allocator;

    static const uint64_t max_basic_type;
    static const uint64_t basic_type_bits;  // Number of bits
    static const uint64_t basic_type_halfBits;

    static const uint16_t word_length; // Use multiples of word_length words
    static const uint64_t r_mask;
    static const uint64_t l_mask;
    static const uint64_t l_bit;

    union
    {
        size_t capacity_;
        basic_type values_[2];
    };
    basic_type* data_;
    bool        neg_;
    bool        dynamic_;
    size_t      length_;
public:
//  Constructors and Destructor
    basic_bignum()
        : data_(values_), neg_(false), dynamic_(false), length_(0)
    {
        values_[0] = 0;
        values_[1] = 0;
    }

    basic_bignum(const Allocator& allocator)
        : basic_bignum_base<Allocator>(allocator), data_(values_), neg_(false), dynamic_(false), length_(0)
    {
        values_[0] = 0;
        values_[1] = 0;
    }

    basic_bignum(const basic_bignum<Allocator>& n)
        : basic_bignum_base<Allocator>(n.allocator()), data_(values_), neg_(n.neg_), dynamic_(false), length_(n.length_)
    {
        if ( length_ <= 2 )
        {
            values_ [0] = n.data_ [0];
            values_ [1] = n.data_ [1];
        }
        else
        {
            capacity_ = n.capacity_;
            data_ = allocator().allocate(capacity_);
            dynamic_ = true;
            memcpy( data_, n.data_, n.length_*sizeof(basic_type) );
        }
    }

    basic_bignum(basic_bignum<Allocator>&& n)
        : data_(values_), neg_(n.neg_), dynamic_(false), length_(n.length_)
    {
        if (n.dynamic_)
        {
            data_ = n.data_;
            dynamic_ = true;

            n.data_ = n.values_;
            n.dynamic_ = false;
            n.length_ = 0;
            n.neg_ = false;
        }
        else
        {
            values_[0] = n.data_[0];
            values_[1] = n.data_[1];
            data_ = values_;
            dynamic_ = false;
        }
    }

    template <typename CharT>
    explicit basic_bignum(const CharT* str)
    {
        initialize(str, strlen(str));
    }

    template <typename CharT>
    basic_bignum(const CharT* data, size_t length)
    {
        initialize(data, length);
    }

    template <typename CharT>
    void initialize(const CharT* data, size_t length)
    {
        bool neg = false;

        const CharT* end = data+length;
        while (data != end && isspace(*data))
        {
            ++data;
            --length;
        }

        if ( *data == '-' )
        {
            neg = true;
            data++;
            --length;
        }

        basic_bignum<Allocator> v = 0;
        for (size_t i = 0; i < length; i++)
        {
            v = (v * 10) + (uint64_t)(data[i] - '0');
        }

        if ( neg )
        {
            v.neg_ = true;
        }
        initialize( v );
    }

    basic_bignum(int signum, std::initializer_list<uint8_t> l)
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
            neg_ = false;
            initialize_from_integer(0u);
        }
    }

    basic_bignum(int signum, const uint8_t* str, size_t n)
    {
        if (n > 0)
        {
            basic_bignum<Allocator> v = 0;
            for (size_t i = 0; i < n; i++)
            {
                v = (v * 256) + (uint64_t)(str[i]);
            }

            if (signum == -1)
            {
                v = -1 - v;
            }

            initialize(v);
        }
        else
        {
            neg_ = false;
            initialize_from_integer(0u);
        }
    }

    basic_bignum(short i)
    {
        neg_ = i < 0;
        uint64_t u = neg_ ? -i : i;
        initialize_from_integer( u );
    }

    basic_bignum(unsigned short u)
    {
        neg_ = false;
        initialize_from_integer( u );
    }

    basic_bignum(int i)
    {
        neg_ = i < 0;
        uint64_t u = neg_ ? -i : i;
        initialize_from_integer( u );
    }

    basic_bignum(unsigned int u)
    {
        neg_ = false;
        initialize_from_integer( u );
    }

    basic_bignum(long i)
    {
        neg_ = i < 0;
        uint64_t u = neg_ ? -i : i;
        initialize_from_integer( u );
    }

    basic_bignum(unsigned long u)
    {
        neg_ = false;
        initialize_from_integer( u );
    }

    basic_bignum(long long i)
    {
        neg_ = i < 0;
        uint64_t u = neg_ ? -i : i;
        initialize_from_integer( u );
    }

    basic_bignum(unsigned long long u)
    {
        neg_ = false;
        initialize_from_integer( u );
    }

    explicit basic_bignum( double x )
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
            basic_type u = (basic_type) fmod( x, values );
            v = v + factor* basic_bignum<Allocator>(u);
            x /= values;
            factor = factor*(basic_bignum<Allocator>( max_basic_type ) + basic_bignum<Allocator>(1));
        }

        if ( neg )
        {
            v.neg_ = true;
        }
        initialize( v );
    }

    explicit basic_bignum(long double x)
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
            basic_type u = (basic_type) fmod( x, values );
            v = v + factor* basic_bignum<Allocator>(u);
            x /= values;
            factor = factor*(basic_bignum<Allocator>( max_basic_type ) + basic_bignum<Allocator>(1));
        }

        if ( neg )
        {
            v.neg_ = true;
        }
        initialize( v );
    }

    ~basic_bignum()
    {
        if ( dynamic_ )
        {
            allocator().deallocate(data_, capacity_);
        }
    }

    size_t capacity() const { return dynamic_ ? capacity_ : 2; }

//  Operators
    bool operator!() const
    {
        return length() == 0 ? true : false;
    }

    basic_bignum operator-() const
    {
        basic_bignum<Allocator> v = *this;
        v.neg_ = !v.neg_;
        return v;
    }

    basic_bignum& operator=( const basic_bignum<Allocator>& y )
    {
        if ( this != &y )
        {
            set_length( y.length() );
            neg_ = y.neg_;
            if ( y.length() > 0 )
            {
                memcpy( data_, y.data_, y.length()*sizeof(basic_type) );
            }
        }
        return *this;
    }

    basic_bignum& operator+=( const basic_bignum<Allocator>& y )
    {
        if ( neg_ != y.neg_ )
            return *this -= -y;
        basic_type d;
        basic_type carry = 0;

        incr_length( (std::max)(y.length(), length()) + 1 );

        for (size_t i = 0; i < length(); i++ )
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
        if ( neg_ != y.neg_ )
            return *this += -y;
        if ( (!neg_ && y > *this) || (neg_ && y < *this) )
            return *this = -(y - *this);
        uint64_t borrow = 0;
        basic_type d;
        for (size_t i = 0; i < length(); i++ )
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
        *this *= basic_type(y < 0 ? -y : y);
        if ( y < 0 )
            neg_ = !neg_;
        return *this;
    }

    basic_bignum& operator*=( uint64_t y )
    {
        size_t len0 = length();
        basic_type hi;
        basic_type lo;
        basic_type dig = data_[0];
        basic_type carry = 0;

        incr_length( length() + 1 );

        size_t i = 0;
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
        bool difSigns = neg_ != y.neg_;
        if ( length() + y.length() == 2 ) // length() = y.length() = 1
        {
            basic_type a = data_[0], b = y.data_[0];
            data_[0] = a * b;
            if ( data_[0] / a != b )
            {
                set_length( 2 );
                DDproduct( a, b, data_[1], data_[0] );
            }
            neg_ = difSigns;
            return *this;
        }
        if ( length() == 1 )  //  && y.length() > 1
        {
            basic_type digit = data_[0];
            *this = y;
            *this *= digit;
        }
        else
        {
            if ( y.length() == 1 )
                *this *= y.data_[0];
            else
            {
                size_t lenProd = length() + y.length(), jA, jB;
                basic_type sumHi = 0, sumLo, hi, lo,
                sumLo_old, sumHi_old, carry=0;
                basic_bignum<Allocator> x = *this;
                set_length( lenProd ); // Give *this length lenProd

                for (size_t i = 0; i < lenProd; i++ )
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
       neg_ = difSigns;
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
        size_t q = (size_t)(k / basic_type_bits);
        if ( q ) // Increase length_ by q:
        {
            incr_length(length() + q);
            for (size_t i = length(); i-- > 0; )
                data_[i] = ( i < q ? 0 : data_[i - q]);
            k %= basic_type_bits;
        }
        if ( k )  // 0 < k < basic_type_bits:
        {
            uint64_t k1 = basic_type_bits - k;
            basic_type mask = (1 << k) - 1;
            incr_length( length() + 1 );
            for (size_t i = length(); i-- > 0; )
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
        size_t q = (size_t)(k / basic_type_bits);
        if ( q >= length() )
        {
            set_length( 0 );
            return *this;
        }
        if (q > 0)
        {
            memmove( data_, data_+q, (size_t)((length() - q)*sizeof(basic_type)) );
            set_length( length() - q );
            k %= basic_type_bits;
            if ( k == 0 )
            {
                reduce();
                return *this;
            }
        }

        size_t n = (size_t)(length() - 1);
        int64_t k1 = basic_type_bits - k;
        basic_type mask = (1 << k) - 1;
        for (size_t i = 0; i <= n; i++)
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

        const basic_type* qBegin = a.begin();
        const basic_type* q =      a.end() - 1;
        basic_type*       p =      begin() + a.length() - 1;

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

        const basic_type* qBegin = a.begin();
        const basic_type* q = a.end() - 1;
        basic_type* p = begin() + a.length() - 1;

        while ( q >= qBegin )
        {
            *p-- ^= *q--;
        }

        reduce();

        return *this;
    }

    basic_bignum& operator&=( const basic_bignum<Allocator>& a )
    {
        size_t old_length = length();

        set_length( (std::min)( length(), a.length() ) );

        const basic_type* pBegin = begin();
        basic_type* p = end() - 1;
        const basic_type* q = a.begin() + length() - 1;

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

       return neg_ ? -x : x;
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

        const basic_type* p = begin();
        const basic_type* pEnd = end();
        while ( p < pEnd )
        {
            x += *p*factor;
            factor *= values;
            ++p;
        }

       return neg_ ? -x : x;
    }

    explicit operator long double() const
    {
        long double x = 0.0;
        long double factor = 1.0;
        long double values = (long double)max_basic_type + 1.0;

        const basic_type* p = begin();
        const basic_type* pEnd = end();
        while ( p < pEnd )
        {
            x += *p*factor;
            factor *= values;
            ++p;
        }

       return neg_ ? -x : x;
    }

    template <typename Alloc>
    void dump(int& signum, std::vector<uint8_t,Alloc>& data) const
    {
        basic_bignum<Allocator> n(*this);
        if (neg_)
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
            if ( v.neg_ )
            {
                data[0] = '-';
                i = 1;
            }
            do
            {
                v.divide( LP10, v, R, true );
                r = (R.length() ? R.data_[0] : 0);
                for ( size_t j=0; j < ip10; j++ )
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
        if ( a.neg_ )
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

    friend std::ostream& operator<<( std::ostream& os, const basic_bignum<Allocator>& v )
    {
        std::string s; 
        v.dump(s);

        os << s;

        return os;
    }

    int compare( const basic_bignum<Allocator>& y ) const
    {
        if ( neg_ != y.neg_ )
            return y.neg_ - neg_;
        int code = 0;
        if ( length() == 0 && y.length() == 0 )
            code = 0;
        else if ( length() < y.length() )
            code = -1;
        else if ( length() > y.length() )
            code = +1;
        else
        {
            for (size_t i = length(); i-- > 0; )
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
        return neg_ ? -code : code;
    }

    void DDproduct( basic_type A, basic_type B,
                    basic_type& hi, basic_type& lo ) const
    // Multiplying two digits: (hi, lo) = A * B
    {
        basic_type hiA = A >> basic_type_halfBits, loA = A & r_mask,
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

    basic_type DDquotient( basic_type A, basic_type B, basic_type d ) const
    // Divide double word (A, B) by d. Quotient = (qHi, qLo)
    {
        basic_type left, middle, right, qHi, qLo, x, dLo1,
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

    void subtractmul( basic_type* a, basic_type* b, size_t n, basic_type& q ) const
    // a -= q * b: b in n positions; correct q if necessary
    {
        basic_type hi, lo, d, carry = 0;
        size_t i;
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
        size_t r = denom.length() - 1;
        basic_type y = denom.data_[r];

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
            throw std::runtime_error( "Zero divide." );
        }
        bool quot_neg = neg_ ^ denom.neg_;
        bool rem_neg = neg_;
        int x = 0;
        basic_bignum<Allocator> num = *this;
        num.neg_ = denom.neg_ = false;
        if ( num < denom )
        {
            quot = uint64_t(0);
            rem = num;
            rem.neg_ = rem_neg;
            return;
        }
        if ( denom.length() == 1 && num.length() == 1 )
        {
            quot = basic_type( num.data_[0]/denom.data_[0] );
            rem = basic_type( num.data_[0]%denom.data_[0] );
            quot.neg_ = quot_neg;
            rem.neg_ = rem_neg;
            return;
        }
        else if (denom.length() == 1 && (denom.data_[0] & l_mask) == 0 )
        {
            // Denominator fits into a half word
            basic_type divisor = denom.data_[0], dHi = 0,
                     q1, r, q2, dividend;
            quot.set_length(length());
            for (size_t i=length(); i-- > 0; )
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
            quot.neg_ = quot_neg;
            rem.neg_ = rem_neg;
            return;
        }
        basic_bignum<Allocator> num0 = num, denom0 = denom;
        int second_done = normalize(denom, num, x);
        size_t l = denom.length() - 1;
        size_t n = num.length() - 1;
        quot.set_length(n - l);
        for (size_t i=quot.length(); i-- > 0; )
            quot.data_[i] = 0;
        rem = num;
        if ( rem.data_[n] >= denom.data_[l] )
        {
            rem.incr_length(rem.length() + 1);
            n++;
            quot.incr_length(quot.length() + 1);
        }
        basic_type d = denom.data_[l];
        for ( size_t k = n; k > l; k-- )
        {
            basic_type q = DDquotient(rem.data_[k], rem.data_[k-1], d);
            subtractmul( rem.data_ + k - l - 1, denom.data_, l + 1, q );
            quot.data_[k - l - 1] = q;
        }
        quot.reduce();
        quot.neg_ = quot_neg;
        if ( remDesired )
        {
            unnormalize(rem, x, second_done);
            rem.neg_ = rem_neg;
        }
    }

    size_t length() const { return length_; }
    basic_type* begin() { return data_; }
    const basic_type* begin() const { return data_; }
    basic_type* end() { return data_ + length_; }
    const basic_type* end() const { return data_ + length_; }

    void set_length(size_t n)
    {
       length_ = n;
       if ( length_ > capacity() )
       {
           if ( dynamic_ )
           {
               delete[] data_;
           }
           capacity_ = round_up(length_);
           data_ = allocator().allocate(capacity_);
           dynamic_ = true;
       }
    }

    size_t round_up(size_t i) const // Find suitable new block size
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
        dynamic_ = false;
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
        dynamic_ = false;
        length_ = u != 0 ? 2 : 0;

        data_[0] = basic_type(u & max_basic_type);
        u >>= basic_type_bits;
        data_[1] = basic_type(u & max_basic_type);
    }

    void initialize( const basic_bignum<Allocator>& n )
    {
        neg_ = n.neg_;
        length_ = n.length_;

        if ( length_ <= 2 )
        {
            data_ = values_;
            dynamic_ = false;
            values_ [0] = n.data_ [0];
            values_ [1] = n.data_ [1];
        }
        else
        {
            capacity_ = round_up( length_ );
            data_ = allocator().allocate(capacity_);
            dynamic_ = true;
            if ( length_ > 0 )
            {
                memcpy( data_, n.data_, length_*sizeof(basic_type) );
            }
            reduce();
        }
    }

    void reduce()
    {
        basic_type* p = end() - 1;
        basic_type* pBegin = begin();
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
            neg_ = false;
        }
    }

    void incr_length( size_t len_new )
    {
        size_t len_old = length_;
        length_ = len_new;  // length_ > len_old

        if ( length_ > capacity() )
        {
            size_t capacity_new = round_up( length_ );

            basic_type* data_old = data_;

            data_ = allocator().allocate(capacity_new);

            if ( len_old > 0 )
            {
                memcpy( data_, data_old, len_old*sizeof(basic_type) );
            }
            if ( dynamic_ )
            {
                allocator().deallocate(data_old,capacity_);
            }
            capacity_ = capacity_new;
            dynamic_ = true;
        }

        if ( length_ > len_old )
        {
            memset( data_+len_old, 0, (length_ - len_old)*sizeof(basic_type) );
        }
    }
};  

template <class Allocator>
const uint64_t basic_bignum<Allocator>::max_basic_type = std::numeric_limits<uint64_t>::max();
template <class Allocator>
const uint64_t basic_bignum<Allocator>::basic_type_bits = sizeof(uint64_t) * 8;  // Number of bits
template <class Allocator>
const uint64_t basic_bignum<Allocator>::basic_type_halfBits = basic_bignum<Allocator>::basic_type_bits/2;
template <class Allocator>
const uint16_t basic_bignum<Allocator>::word_length = 4; // Use multiples of word_length words
template <class Allocator>
const uint64_t basic_bignum<Allocator>::r_mask = (uint64_t(1) << basic_bignum<Allocator>::basic_type_halfBits) - 1;
template <class Allocator>
const uint64_t basic_bignum<Allocator>::l_mask = basic_bignum<Allocator>::max_basic_type - basic_bignum<Allocator>::r_mask;
template <class Allocator>
const uint64_t basic_bignum<Allocator>::l_bit = basic_bignum<Allocator>::max_basic_type - (basic_bignum<Allocator>::max_basic_type >> 1);

typedef basic_bignum<std::allocator<uint8_t>> bignum;

}

#endif
