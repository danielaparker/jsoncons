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
#include <locale>
#include <limits> // std::numeric_limits
#include <jsoncons/jsoncons_config.hpp>
#include <jsoncons/osequencestream.hpp>

namespace jsoncons
{

template <class CharT>
class buffered_output
{
    static const size_t default_buffer_length = 16384;

    std::basic_ostream<CharT>& os_;
    std::vector<CharT> buffer_;
    CharT * const begin_buffer_;
    const CharT* const end_buffer_;
    CharT* p_;


    // Noncopyable and nonmoveable
    buffered_output(const buffered_output&) = delete;
    buffered_output& operator=(const buffered_output&) = delete;

public:
    buffered_output(std::basic_ostream<CharT>& os)
        : os_(os), buffer_(default_buffer_length), begin_buffer_(buffer_.data()), end_buffer_(buffer_.data()+default_buffer_length), p_(buffer_.data())
    {
    }
    ~buffered_output()
    {
        os_.write(begin_buffer_, (p_ - begin_buffer_));
        os_.flush();
    }

    void flush()
    {
        os_.write(begin_buffer_, (p_ - begin_buffer_));
        p_ = begin_buffer_;
        os_.flush();
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
            os_.write(begin_buffer_, (p_ - begin_buffer_));
            os_.write(s, length);
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
            os_.write(begin_buffer_, (p_-begin_buffer_));
            p_ = begin_buffer_;
            *p_++ = c;
        }
    }

};

template <class CharT>
class ostringstream_float_printer
{
    uint8_t precision_;
public:
    ostringstream_float_printer(uint8_t precision)
        : precision_(precision)
    {
    }
    void print(double val, uint8_t precision, buffered_output<CharT>& os)
    {
        uint8_t prec = (precision == 0) ? precision_ : precision;

        std::basic_ostringstream<CharT> ss;
        ss.imbue(std::locale::classic());
        ss << std::showpoint << std::setprecision(prec) << val;
        std::basic_string<CharT> str(ss.str());

        bool dot = false;
        typename std::basic_string<CharT>::size_type exp_pos= str.find('e');
        if (exp_pos != std::basic_string<CharT>::npos)
        {
            size_t len = exp_pos;
            while (len >= 2 && str[len - 1] == '0' && str[len - 2] != '.')
            {
                --len;
            }
            for (size_t i = 0; i < len;++i)
            {
                if (str[i] == '.')
                {
                    dot = true;
                }
                os.put(str[i]);
            }
            if (!dot)
            {
                os.put('.');
                os.put('0');
                dot = true;
            }
            for (size_t i = exp_pos; i < str.size();++i)
            {
                os.put(str[i]);
            }
        }
        else
        {
            size_t len = str.length();
            while (len >= 2 && str[len - 1] == '0' && str[len - 2] != '.')
            {
                --len;
            }
            const CharT* s = str.data();
            const CharT* se = s + len;
            while (s < se)
            {
                if (*s == '.')
                {
                    dot = true;
                }
                os.put(*s);
                ++s;
            }
        }

        if (!dot)
        {
            os.put('.');
            os.put('0');
        }
    }
};

template <class CharT>
class osequencestream_float_printer
{
    uint8_t precision_;
    basic_osequencestream<CharT> oss_;
public:
    osequencestream_float_printer(uint8_t precision)
        : precision_(precision)
    {
        oss_.imbue(std::locale::classic());
        oss_.precision(precision);
    }
    void print(double val, uint8_t precision, buffered_output<CharT>& os)
    {
        oss_.clear_sequence();
        oss_.precision((precision == 0) ? precision_ : precision);
        oss_ << val;

        const CharT* sbeg = oss_.data();
        const CharT* send = sbeg + oss_.length();
        const CharT* pexp = send;

        if (sbeg != send)
        {
            bool dot = false;
            for (pexp = sbeg; *pexp != 'e' && *pexp != 'E' && pexp < send; ++pexp)
            {
            }

            if (pexp != send)
            {
                const CharT* p = pexp;
                while (p >= sbeg+2 && *(p-1) == '0' && *(p-2) != '.')
                {
                    --p;
                }
                for (const CharT* q = sbeg; q < p; ++q)
                {
                    if (*q == '.')
                    {
                        dot = true;
                    }
                    os.put(*q);
                }
                if (!dot)
                {
                    os.put('.');
                    os.put('0');
                    dot = true;
                }
                for (const CharT* q = pexp; q < send; ++q)
                {
                    os.put(*q);
                }
            }
            else
            {
                const CharT* p = send;
                while (p >= sbeg+2 && *(p-1) == '0' && *(p-2) != '.')
                {
                    --p;
                }
                const CharT* qend = *(p-2) == '.' ? p : send;
                for (const CharT* q = sbeg; q < qend; ++q)
                {
                    if (*q == '.')
                    {
                        dot = true;
                    }
                    os.put(*q);
                }
            }

            if (!dot)
            {
                os.put('.');
                os.put('0');
            }
        }
    }
};

#ifdef JSONCONS_HAS__ECVT_S

template <class CharT>
class float_printer
{
    uint8_t precision_;
public:
    float_printer(uint8_t precision)
        : precision_(precision)
    {
    }

    void print(double val, uint8_t precision, buffered_output<CharT>& os)
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
                os.put(static_cast<CharT>(i) + '0');
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
#define float_printer ostringstream_float_printer
#endif

template <class CharT>
class string_to_double
{
};

#if defined(_MSC_VER)
template <>
class string_to_double<char>
{
private:
    _locale_t locale_;
public:
    string_to_double()
    {
        locale_ = _create_locale(LC_NUMERIC, "C");
    }
    ~string_to_double()
    {
        _free_locale(locale_);
    }

    double operator()(const char* s, size_t)
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
private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double&) = delete;
    string_to_double& operator=(const string_to_double&) = delete;
};

template <>
class string_to_double<wchar_t>
{
private:
    _locale_t locale_;
public:
    string_to_double()
    {
        locale_ = _create_locale(LC_NUMERIC, "C");
    }
    ~string_to_double()
    {
        _free_locale(locale_);
    }

    double operator()(const wchar_t* s, size_t)
    {
        const wchar_t *begin = s;
        wchar_t *end = nullptr;
        double val = _wcstod_l(begin, &end, locale_);
        if (begin == end)
        {
            throw std::invalid_argument("Invalid float value");
        }
        return val;
    }
private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double&) = delete;
    string_to_double& operator=(const string_to_double&) = delete;
};
#elif defined(JSONCONS_HAS_STRTOD_L)
template<>
class string_to_double<char>
{
private:
    locale_t locale_;
public:
    string_to_double()
    {
        locale_ = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
    }
    ~string_to_double()
    {
        freelocale(locale_);
    }

    double operator()(const char* s, size_t length)
    {
        const char *begin = s;
        char *end = nullptr;
        double val = strtod_l(begin, &end, locale_);
        if (begin == end)
        {
            throw std::invalid_argument("Invalid float value");
        }
        return val;
    }

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};

template<>
class string_to_double<wchar_t>
{
private:
    locale_t locale_;
public:
    string_to_double()
    {
        locale_ = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
    }
    ~string_to_double()
    {
        freelocale(locale_);
    }

    double operator()(const wchar_t* s, size_t length)
    {
        const wchar_t *begin = s;
        wchar_t *end = nullptr;
        double val = wcstod_l(begin, &end, locale_);
        if (begin == end)
        {
            throw std::invalid_argument("Invalid float value");
        }
        return val;
    }

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};
#else
template<>
class string_to_double<char>
{
private:
    std::vector<char> buffer_;
    std::string decimal_point_;
    bool is_dot_;
public:
    string_to_double()
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

    double operator()(const char* s, size_t length)
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

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};
template<>
class string_to_double<wchar_t>
{
private:
    std::vector<wchar_t> buffer_;
    std::string decimal_point_;
    bool is_dot_;
public:
    string_to_double()
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

    double operator()(const wchar_t* s, size_t length)
    {
        double val;
        if (is_dot_)
        {
            const wchar_t *begin = s;
            wchar_t *end = nullptr;
            val = wcstod(begin, &end);
            if (begin == end)
            {
                throw std::invalid_argument("Invalid float value");
            }
        }
        else
        {
            buffer_.clear();
            size_t j = 0;
            const wchar_t* pe = s + length;
            for (const wchar_t* p = s; p < pe; ++p)
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
            const wchar_t *begin = buffer_.data();
            wchar_t *end = nullptr;
            val = wcstod(begin, &end);
            if (begin == end)
            {
                throw std::invalid_argument("Invalid float value");
            }
        }
        return val;
    }

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};
#endif

}

#endif
