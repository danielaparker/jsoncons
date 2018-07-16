#ifndef JSONCONS_BIGINT_HPP
#define JSONCONS_BIGINT_HPP

#include <cstdint>
#include <iostream>
#include <limits>

namespace jsoncons {

/*
Description:  Multi-precision integer arithmetic.

Remarks:

This program text is based on Chapter 2 and Appendix A of
Ammeraal, L. (1996) Algorithms and Data Structures in C++,
Chichester: John Wiley.

*/

class bignum 
{
public:
    typedef uint32_t basic_type;
    union
    {
        short capacity_;
        basic_type values_[2];
    };
private:
    const basic_type max_basic_type = std::numeric_limits<basic_type>::max();
    const int basic_type_bits = sizeof(basic_type) * 8;  // Number of bits
    const int basic_type_halfBits = basic_type_bits/2;

    const int word_length = 2; // Use multiples of word_length words
    const basic_type r_mask = (1 << basic_type_halfBits) - 1;
    const basic_type l_mask = max_basic_type - r_mask;
    const basic_type l_bit = max_basic_type - (max_basic_type >> 1);

    basic_type* data_;
    bool        neg_;
    bool        dynamic_;
    short       length_;
public:
//  Constructors and Destructor
    bignum()
        : data_(values_), neg_(false), dynamic_(false), length_(0)
    {
        values_[0] = 0;
        values_[1] = 0;
    }

    bignum(const char* str)
    {
        bool neg = false;

        while ( isspace(*str) )
        {
            ++str;
        }

        if ( *str == '-' )
        {
            neg = true;
            str++;
        }

        size_t n = strlen( str );
        bignum v = 0;
        for (size_t i = 0; i < n; i++)
        {
            v = (v * 10) + (int)(str[i] - '0');
        }

        if ( neg )
        {
            v.neg_ = true;
        }
        initialize( v );
    }

    bignum(int i)
    {
        neg_ = i < 0;
        unsigned long u = neg_ ? -i : i;
        initialize( u );
    }

    bignum(unsigned u)
    {
        initialize( (unsigned long) u );
    }

    bignum(long i)
    {
        neg_ = i < 0;
        unsigned long u = neg_ ? -i : i;
        initialize( u );
    }

    bignum(unsigned long u)
    {
        initialize( u );
    }

    bignum(const int64_t& i)
    {
        neg_ = i < 0;
        uint64_t u = neg_ ? -i : i;
        initialize( u );
    }

    bignum(const uint64_t& u)
    {
        initialize( u );
    }

    bignum(const bignum& n)
        : data_(values_), dynamic_(false), length_(n.length_), neg_(n.neg_)
    {
        if ( length_ <= 2 )
        {
            values_ [0] = n.data_ [0];
            values_ [1] = n.data_ [1];
        }
        else
        {
            capacity_ = n.capacity_;
            data_ = new basic_type [capacity_];
            dynamic_ = true;
            memcpy( data_, n.data_, n.length_*sizeof(basic_type) );
        }
    }

    bignum( double x )
    {
        bool neg = false;

        if ( x < 0 )
        {
            neg = true;
            x = -x;
        }
        bignum v = 0;

        double values = (double)max_basic_type + 1.0;
        bignum factor = 1;

        while ( x >= 1 )
        {
            basic_type u = (basic_type) fmod( x, values );
            v = v + factor* bignum(u);
            x /= values;
            factor = factor*(bignum( max_basic_type ) + bignum(1));
        }

        if ( neg )
        {
            v.neg_ = true;
        }
        initialize( v );
    }

    bignum(long double x)
    {
        bool neg = false;

        if ( x < 0 )
        {
            neg = true;
            x = -x;
        }

        bignum v = 0;

        long double values = (long double)max_basic_type + 1.0;
        bignum factor = 1;

        while ( x >= 1.0 )
        {
            basic_type u = (basic_type) fmod( x, values );
            v = v + factor* bignum(u);
            x /= values;
            factor = factor*(bignum( max_basic_type ) + bignum(1));
        }

        if ( neg )
        {
            v.neg_ = true;
        }
        initialize( v );
    }

    ~bignum()
    {
        if ( dynamic_ )
        {
            delete[] data_;
        }
    }

    short capacity() const {return capacity_;}

//  Operators
    bool operator!() const
    {
        return length() == 0 ? true : false;
    }

    bignum operator-() const
    {
        bignum v = *this;
        v.neg_ = !v.neg_;
        return v;
    }

    bignum& operator=( const bignum& y )
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

    bignum& operator+=( const bignum& y )
    {
        if ( neg_ != y.neg_ )
            return *this -= -y;
        int i;
        basic_type d;
        basic_type carry = 0;

        incr_length( std::max(y.length(), length()) + 1 );

        for ( i = 0; i < length(); i++ )
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

    bignum& operator-=( const bignum& y )
    {
        if ( neg_ != y.neg_ )
            return *this += -y;
        if ( !neg_ && y > *this || neg_ && y < *this )
            return *this = -(y - *this);
        int i, borrow = 0;
        basic_type d;
        for ( i = 0; i < length(); i++ )
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
            else data_[i] = d;
        }
        reduce();
        return *this;
    }

    bignum& operator*=( int y )
    {
        *this *= basic_type(y < 0 ? -y : y);
        if ( y < 0 )
            neg_ = !neg_;
        return *this;
    }

    bignum& operator*=( unsigned y )
    {
        int len0 = length();
        int i;
        basic_type Hi;
        basic_type Lo;
        basic_type dig = data_[0];
        basic_type carry = 0;

        incr_length( length() + 1 );

        for ( i = 0; i < len0; i++ )
        {
            DDproduct( dig, y, Hi, Lo );
            data_[i] = Lo + carry;
            dig = data_[i+1];
            carry = Hi + (data_[i] < Lo);
        }
        data_[i] = carry;
        reduce();
        return *this;
    }

    bignum& operator*=( bignum y )
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
            if ( y.length() == 1 )
                *this *= y.data_[0];
            else
            {
                int lenProd = length() + y.length(), i, jA, jB;
                basic_type sumHi = 0, sumLo, Hi, Lo,
                sumLo_old, sumHi_old, carry=0;
                bignum x = *this;
                set_length( lenProd ); // Give *this length lenProd
                for ( i = 0; i < lenProd; i++ )
                {
                    sumLo = sumHi;
                    sumHi = carry;
                    carry = 0;
                    for ( jA=0; jA < x.length(); jA++ )
                    {
                        jB = i - jA;
                        if ( jB >= 0 && jB < y.length() )
                        {
                            DDproduct( x.data_[jA], y.data_[jB], Hi, Lo );
                            sumLo_old = sumLo;
                            sumHi_old = sumHi;
                            sumLo += Lo;
                            if ( sumLo < sumLo_old )
                                sumHi++;
                            sumHi += Hi;
                            carry += (sumHi < sumHi_old);
                        }
                    }
                data_[i] = sumLo;
            }
       }
       reduce();
       neg_ = difSigns;
       return *this;
    }

    bignum& operator/=( const bignum& divisor )
    {
        bignum r;
        divide( divisor, *this, r, false );
        return *this;
    }

    bignum& operator%=( const bignum& divisor )
    {
        bignum q;
        divide( divisor, q, *this, true );
        return *this;
    }

    bignum& operator<<=( unsigned k )
    {
        int q = k / basic_type_bits;
        if ( q ) // Increase length_ by q:
        {
            int i;
            incr_length( length() + q );
            for ( i = length() - 1; i >= 0; i-- )
                data_[i] = ( i < q ? 0 : data_[i - q]);
            k %= basic_type_bits;
        }
        if ( k )  // 0 < k < basic_type_bits:
        {
            int k1 = basic_type_bits - k;
            basic_type mask = (1 << k) - 1;
            incr_length( length() + 1 );
            for ( int i = length() - 1; i >= 0; i-- )
            {
                data_[i] <<= k;
                if ( i > 0 )
                    data_[i] |= (data_[i-1] >> k1) & mask;
            }
        }
        reduce();
        return *this;
    }

    bignum& operator>>=( unsigned k )
    {
        int q = k / basic_type_bits;
        if ( q >= length() )
        {
            set_length( 0 );
            return *this;
        }
        if ( q )
        {
            memmove( data_, data_+q, (length() - q)*sizeof(basic_type) );
            set_length( length() - q );
            k %= basic_type_bits;
            if ( k == 0 )
            {
                reduce();
                return *this;
            }
        }

        int n = length() - 1;
        int k1 = basic_type_bits - k;
        basic_type mask = (1 << k) - 1;
        for ( int i = 0; i <= n; i++ )
        {
            data_[i] >>= k;
            if ( i < n )
                data_[i] |= ((data_[i+1] & mask) << k1);
        }
        reduce();
        return *this;
    }

    bignum& operator++();
    bignum  operator++(int);

    bignum& operator--();
    bignum  operator--(int);

    bignum& operator|=( const bignum& a )
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

    bignum& operator^=( const bignum& a )
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
            *p-- ^= *q--;
        }

        reduce();

        return *this;
    }

    bignum& operator&=( const bignum& a )
    {
        int d_len_old = length();

        set_length( std::min( length(), a.length() ) );

        const basic_type* pBegin = begin();
        basic_type*       p      = end() - 1;
        const basic_type* q      = a.begin() + length() - 1;

        while ( p >= pBegin )
        {
            *p-- &= *q--;
        }

        if ( d_len_old > length() )
        {
            memset( data_ + length(), 0, d_len_old - length() );
        }

        reduce();

        return *this;
    }

    operator bool() const;
    operator int() const;
    operator unsigned() const;
    operator long() const;
    operator unsigned long() const;
    operator double() const;
    operator long double() const;
    operator int64_t() const;
    operator uint64_t() const;

//  Global Operators

    friend bool operator==( const bignum& x, const bignum& y );
    friend bool operator==( const bignum& x, int y );
    friend bool operator!=( const bignum& x, const bignum& y );
    friend bool operator!=( const bignum& x, int y );
    friend bool operator<( const bignum& x, const bignum& y );
    friend bool operator<( const bignum& x, int y );
    friend bool operator>( const bignum& x, const bignum& y );
    friend bool operator>( const bignum& x, int y );
    friend bool operator<=( const bignum& x, const bignum& y );
    friend bool operator<=( const bignum& x, int y );
    friend bool operator>=( const bignum& x, const bignum& y );
    friend bool operator>=( const bignum& x, int y );

    friend std::ostream& operator<<( std::ostream &os, const bignum& v );
    friend std::istream& operator>>( std::istream &os, bignum& v );

    friend bignum operator+( bignum x, const bignum& y );
    friend bignum operator-( bignum x, const bignum& y );
    friend bignum operator*( bignum x, bignum y );
    friend bignum operator/( bignum x, const bignum& y );
    friend bignum operator%( bignum x, const bignum& y );
    friend bignum operator<<( bignum u, unsigned k );
    friend bignum operator<<( bignum u, int k );
    friend bignum operator>>( bignum u, unsigned k );
    friend bignum operator>>( bignum u, int k );

    friend bignum operator|( bignum x, const bignum& y );
    friend bignum operator|( bignum x, int y );
    friend bignum operator|( bignum x, unsigned y );
    friend bignum operator^( bignum x, const bignum& y );
    friend bignum operator^( bignum x, int y );
    friend bignum operator^( bignum x, unsigned y );
    friend bignum operator&( bignum x, const bignum& y );
    friend bignum operator&( bignum x, int y );
    friend bignum operator&( bignum x, unsigned y );

    friend bignum abs( const bignum& a )
    {
        if ( a.neg_ )
        {
            return -a;
        }
        return a;
    }

    friend bignum power( bignum x, unsigned n )
    {
        bignum y = 1;

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
    friend bignum sqrt( const bignum& x );

private:
    int compare( const bignum& y ) const
    {
        if ( neg_ != y.neg_ )
            return y.neg_ - neg_;
        int code = 0;
        if ( length() == 0 || y.length() == 0 )
            code = length() - y.length();
        else if ( length() < y.length() )
            code = -1;
        else if ( length() > y.length() )
            code = +1;
        else
        {
            for ( int i = length() - 1; i >= 0; i-- )
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
                                  basic_type& Hi, basic_type& Lo ) const
    // Multiplying two digits: (Hi, Lo) = A * B
    {
        basic_type hiA = A >> basic_type_halfBits, loA = A & r_mask,
                   hiB = B >> basic_type_halfBits, loB = B & r_mask,
                   mid1, mid2, old;

        Lo = loA * loB;
        Hi = hiA * hiB;
        mid1 = loA * hiB;
        mid2 = hiA * loB;
        old = Lo;
        Lo += mid1 << basic_type_halfBits;
            Hi += (Lo < old) + (mid1 >> basic_type_halfBits);
        old = Lo;
        Lo += mid2 << basic_type_halfBits;
            Hi += (Lo < old) + (mid2 >> basic_type_halfBits);
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

    void subtractmul( basic_type* a, basic_type* b, int n,
                                 basic_type& q ) const
    // a -= q * b: b in n positions; correct q if necessary
    {
        basic_type Hi, Lo, d, carry = 0;
        int i;
        for ( i = 0; i < n; i++ )
        {
            DDproduct( b[i], q, Hi, Lo );
            d = a[i];
            a[i] -= Lo;
            if ( a[i] > d )
                carry++;
            d = a[i + 1];
            a[i + 1] -= Hi + carry;
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

    int normalize( bignum& denom, bignum& num, int& x ) const
    {
        int r = denom.length() - 1;
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

    void unnormalize( bignum& rem, int x, int secondDone ) const
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

    void divide( bignum denom, bignum& quot, bignum& rem, bool remDesired ) const
    {
        if ( denom.length() == 0 )
        {
            throw std::runtime_error( "Zero divide." );
        }
        bool QuotNeg = neg_ ^ denom.neg_;
        bool RemNeg = neg_;
        int    i, r, secondDone, x = 0, n;
        basic_type q, d;
        bignum num = *this;
        num.neg_ = denom.neg_ = false;
        if ( num < denom )
        {
            quot = 0;
            rem = num;
            rem.neg_ = RemNeg;
            return;
        }
        if ( denom.length() == 1 && num.length() == 1 )
        {
            quot = basic_type( num.data_[0]/denom.data_[0] );
            rem = basic_type( num.data_[0]%denom.data_[0] );
            quot.neg_ = QuotNeg;
            rem.neg_ = RemNeg;
            return;
        }
        else if (denom.length() == 1 && (denom.data_[0] & l_mask) == 0 )
        {
            // Denominator fits into a half word
            basic_type divisor = denom.data_[0], dHi = 0,
                     q1, r, q2, dividend;
            quot.set_length(length());
            for ( int i=length()-1; i >= 0; i-- )
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
            quot.neg_ = QuotNeg;
            rem.neg_ = RemNeg;
            return;
        }
        bignum num0 = num, denom0 = denom;
        secondDone = normalize(denom, num, x);
        r = denom.length() - 1;
        n = num.length() - 1;
        quot.set_length(n - r);
        for ( i=quot.length()-1; i >= 0; i-- )
            quot.data_[i] = 0;
        rem = num;
        if ( rem.data_[n] >= denom.data_[r] )
        {
            rem.incr_length(rem.length() + 1);
            n++;
            quot.incr_length(quot.length() + 1);
        }
        d = denom.data_[r];
        for ( int k = n; k > r; k-- )
        {
            q = DDquotient(rem.data_[k], rem.data_[k-1], d);
            subtractmul( rem.data_ + k - r - 1, denom.data_, r + 1, q );
            quot.data_[k - r - 1] = q;
        }
        quot.reduce();
        quot.neg_ = QuotNeg;
        if ( remDesired )
        {
            unnormalize(rem, x, secondDone);
            rem.neg_ = RemNeg;
        }
    }

    int length() const { return length_; }
    basic_type* begin() { return data_; }
    const basic_type* begin() const { return data_; }
    basic_type* end() { return data_ + length_; }
    const basic_type* end() const { return data_ + length_; }

    void set_length(int n)
    {
       length_ = n;
       if ( length_ > capacity() )
       {
           if ( dynamic_ )
           {
               delete[] data_;
           }
           capacity_ = round_up(length_);
           data_ = new basic_type[capacity_];
           dynamic_ = true;
       }
    }

    int round_up(int i) const // Find suitable new block size
    {
        return (i/word_length + 1) * word_length;
    }

    void initialize( unsigned long u )
    {
        data_ = values_;
        dynamic_ = false;
        length_ = u != 0 ? 1 : 0;

        data_ [0] = u;
    }

    void initialize( uint64_t u )
    {
        data_ = values_;
        dynamic_ = false;
        length_ = u != 0 ? 2 : 0;

        data_[0] = basic_type(u & max_basic_type);
        u >>= basic_type_bits;
        data_[1] = basic_type(u & max_basic_type);
    }

    void initialize( const bignum& n )
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
            data_ = new basic_type[capacity_];
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
        basic_type* p      = end() - 1;
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

    void incr_length( int len_new )
    {
        int len_old = length_;
        length_ = len_new;  // length_ > len_old

        if ( length_ > capacity() )
        {
            int capacity_new = round_up( length_ );
            basic_type* data_old = data_;

            data_ = new basic_type[capacity_new];

            if ( len_old > 0 )
            {
                memcpy( data_, data_old, len_old*sizeof(basic_type) );
            }
            if ( dynamic_ )
            {
                delete [] data_old;
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

bignum abs( const bignum& x );

}

#endif

