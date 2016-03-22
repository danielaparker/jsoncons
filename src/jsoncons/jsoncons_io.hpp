// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONS_IO_HPP
#define JSONCONS_JSONCONS_IO_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <limits> // std::numeric_limits
#include "jsoncons_config.hpp"
#include "ovectorstream.hpp"

namespace jsoncons
{

template <typename CharT>
class buffered_ostream
{
    static const size_t default_buffer_length = 16384;

    std::basic_ostream<CharT>* os_;
    std::vector<CharT> buffer_;
    CharT * const begin_buffer_;
    const CharT* const end_buffer_;
    CharT* p_;
public:
    buffered_ostream(std::basic_ostream<CharT>& os)
        : os_(std::addressof(os)), buffer_(default_buffer_length), begin_buffer_(buffer_.data()), end_buffer_(buffer_.data()+default_buffer_length), p_(buffer_.data())
    {
    }
    ~buffered_ostream()
    {
        os_->write(begin_buffer_, (p_ - begin_buffer_));
        os_->flush();
    }

    void flush()
    {
        os_->write(begin_buffer_, (p_ - begin_buffer_));
        p_ = begin_buffer_;
        os_->flush();
    }

    void write(const CharT* s, size_t length)
    {
        size_t diff = end_buffer_ - p_;
        if (diff >= length)
        {
            std::memcpy(p_, s, length*sizeof(CharT));
            p_ += length;
        }
        else
        {
            os_->write(begin_buffer_, (p_ - begin_buffer_));
            os_->write(s, length);
            p_ = begin_buffer_;
        }
    }

    void write(const std::basic_string<CharT>& s)
    {
        write(s.data(),s.length());
    }

    void put(CharT c)
    {
        if (p_ < end_buffer_)
        {
            *p_++ = c;
        }
        else
        {
            os_->write(begin_buffer_, (p_-begin_buffer_));
            p_ = begin_buffer_;
            *p_++ = c;
        }
    }

};

#ifdef _MSC_VER

template <typename CharT>
class float_printer
{
    uint8_t precision_;
public:
    float_printer(int precision)
        : precision_(precision)
    {
    }

    void print(double val, uint8_t precision, buffered_ostream<CharT>& os)
    {
        char buf[_CVTBUFSIZE];
        int decimal_point = 0;
        int sign = 0;

        int prec = (precision == 0) ? precision_ : precision;

        int err = _ecvt_s(buf, _CVTBUFSIZE, val, prec, &decimal_point, &sign);
        if (err != 0)
        {
            throw std::runtime_error("Failed attempting double to string conversion");
        }
        char* s = buf;
        char* se = s + prec;

        int i, k;
        int j;

        if (sign)
        {
            os.put('-');
        }
        if (decimal_point <= -4 || decimal_point > se - s + 5) 
        {
            os.put(*s++);
            if (s < se) 
            {
                os.put('.');
                while ((se-1) > s && *(se-1) == '0')
                {
                    --se;
                }

                while(s < se)
                {
                    os.put(*s++);
                }
            }
            os.put('e');
            /* sprintf(b, "%+.2d", decimal_point - 1); */
            if (--decimal_point < 0) {
                os.put('-');
                decimal_point = -decimal_point;
                }
            else
                os.put('+');
            for(j = 2, k = 10; 10*k <= decimal_point; j++, k *= 10);
            for(;;) 
            {
                i = decimal_point / k;
                os.put(i + '0');
                if (--j <= 0)
                    break;
                decimal_point -= i*k;
                decimal_point *= 10;
            }
        }
        else if (decimal_point <= 0) 
        {
            os.put('0');
            os.put('.');
            while ((se-1) > s && *(se-1) == '0')
            {
                --se;
            }
            for(; decimal_point < 0; decimal_point++)
            {
                os.put('0');
            }
            while(s < se)
            {
                os.put(*s++);
            }
        }
        else {
            while(s < se) 
            {
                os.put(*s++);
                if ((--decimal_point == 0) && s < se)
                {
                    os.put('.');
                    while ((se-1) > s && *(se-1) == '0')
                    {
                        --se;
                    }
                }
            }
            for(; decimal_point > 0; decimal_point--)
            {
                os.put('0');
            }
        }
    }
};

#else

template <typename CharT>
class float_printer
{
    jsoncons::basic_ovectorstream<CharT> vs_;
    uint8_t precision_;
public:
    float_printer(uint8_t precision)
        : vs_(255), precision_(precision)
    {
        vs_.set_locale(std::locale::classic());
        vs_.precision(precision);
    }

    void print(double val, uint8_t precision, buffered_ostream<CharT>& os)
    {
        vs_.reset();
        vs_.precision(precision == 0 ? precision_ : precision);
        vs_ << val;

        const CharT* s = vs_.data();
        const CharT* se = s + vs_.length();

        bool dot = false;
        while (s < se)
        {
            if (*s == '.')
            {
                dot = true;
            }
            else if (*s == 'e')
            {
                if (!dot)
                {
                    os.put('.');
                    os.put('0');
                    dot = true;
                }
            }
            os.put(*s);
            ++s;
        }
        if (!dot)
        {
            os.put('.');
            os.put('0');
        }
    }
};

#endif

// string_to_float only requires narrow char
#ifdef _MSC_VER
class float_reader
{
private:
    _locale_t locale_;
public:
    float_reader()
    {
        locale_ = _create_locale(LC_NUMERIC, "C");
    }
    ~float_reader()
    {
        _free_locale(locale_);
    }

    double read(const char* s, size_t length)
    {
        const char *begin = s;
        char *end = nullptr;
        double val = _strtod_l(begin, &end, locale_);
        if (begin == end)
        {
            throw std::invalid_argument("Invalid float value");
        }
        return val;
    }

    float_reader(const float_reader& fr) = delete;
    float_reader& operator=(const float_reader& fr) = delete;
};

#else
class float_reader
{
private:
    std::vector<char> buffer_;
    std::string decimal_point_;
    bool is_dot_;
public:
    float_reader()
        : buffer_()
    {
        struct lconv * lc = localeconv();
        if (lc != nullptr)
        {
            decimal_point_ = std::string(lc->decimal_point);    
        }
        else
        {
            decimal_point_ = std::string("."); 
        }
        buffer_.reserve(100);
        is_dot_ = decimal_point_ == ".";
    }

    double read(const char* s, size_t length)
    {
        double val;
        if (is_dot_)
        {
            const char *begin = s;
            char *end = nullptr;
            val = strtod(begin, &end);
            if (begin == end)
            {
                throw std::invalid_argument("Invalid float value");
            }
        }
        else
        {
            buffer_.clear();
            size_t j = 0;
            const char* pe = s + length;
            for (const char* p = s; p < pe; ++p)
            {
                if (*p == '.')
                {
                    buffer_.insert(buffer_.begin() + j, decimal_point_.begin(), decimal_point_.end());
                    j += decimal_point_.length();
                }
                else
                {
                    buffer_.push_back(*p);
                    ++j;
                }
            }
            const char *begin = buffer_.data();
            char *end = nullptr;
            val = strtod(begin, &end);
            if (begin == end)
            {
                throw std::invalid_argument("Invalid float value");
            }
        }
        return val;
    }

    float_reader(const float_reader& fr) = delete;
    float_reader& operator=(const float_reader& fr) = delete;
};
#endif

}

#endif
