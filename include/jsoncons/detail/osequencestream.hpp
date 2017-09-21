// Copyright 2016 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_OSEQUENCESTREAM_HPP
#define JSONCONS_DETAIL_OSEQUENCESTREAM_HPP

#include <ios>
#include <ostream>
#include <string>
#include <cstddef>
#include <vector>
#include <locale>
#include <jsoncons/detail/jsoncons_config.hpp>

namespace jsoncons {

template< 
    class CharT, 
    class Traits = std::char_traits<CharT>
> class basic_osequencestream;

template<class CharT, class Traits>
class basic_osequencebuf
    : public std::basic_streambuf<CharT, Traits>
{
private:
    std::ios_base::openmode mode_;
    std::vector<CharT> buf_;

public:
    typedef CharT                                     char_type;
    typedef typename Traits::int_type                 int_type;
    typedef typename Traits::pos_type                 pos_type;
    typedef typename Traits::off_type                 off_type;
    typedef Traits                                    traits_type;

public:

    explicit basic_osequencebuf() JSONCONS_NOEXCEPT
        : mode_(std::ios_base::out | std::ios_base::binary), 
          buf_(100u)
    {  
        // Set write position to beginning of buffer.
        this->setp(buf_.data(), buf_.data() + buf_.size());
        this->setg(buf_.data(), 0, buf_.data());
    }

    explicit basic_osequencebuf(std::size_t length) JSONCONS_NOEXCEPT
        : mode_(std::ios_base::out | std::ios_base::binary), 
          buf_(length)
    {  
        // Set write position to beginning of buffer.
        this->setp(buf_.data(), buf_.data() + buf_.size());
        this->setg(buf_.data(), 0, buf_.data());
    }

    virtual ~basic_osequencebuf() JSONCONS_NOEXCEPT {}

    basic_osequencebuf(const basic_osequencebuf<CharT,Traits>&) = delete;

    //basic_osequencebuf(basic_osequencebuf<CharT,Traits>&&) = default;

    basic_osequencebuf<CharT,Traits>& operator=(const basic_osequencebuf<CharT,Traits>&) = delete;

    //basic_osequencebuf<CharT,Traits>& operator=(basic_osequencebuf<CharT,Traits>&&) = default;

    const CharT* data() const
    {
        return buf_.data();
    }

    size_t length() const
    {
        return this->pptr() - this->pbase();
    }

    virtual int sync() override
    {
        return EOF;
    }

protected:
    int_type underflow() override
    {
        return this->gptr() != this->egptr() ?
               Traits::to_int_type(*this->gptr()) : Traits::eof();
    }

    int_type pbackfail(int_type c = Traits::eof()) override
    {
        if (this->gptr() != this->eback())
        {
            if (!Traits::eq_int_type(c, Traits::eof()))
            {
                if (Traits::eq(Traits::to_char_type(c), this->gptr()[-1]))
                {
                    this->gbump(-1);
                    return c;
                }
                this->gbump(-1);
                *this->gptr() = static_cast<CharT>(c);
                return c;
            } 
            else
            {
                this->gbump(-1);
                return Traits::not_eof(c);
            }
        } 
        else 
        {
            return Traits::eof();
        }
    }

    int_type overflow(int_type c = Traits::eof()) override
    {
        if (!Traits::eq_int_type(c, Traits::eof()))
        {
            size_t pos = buf_.size()+1;
            buf_.resize(pos*2);
            this->setp(buf_.data(), buf_.data() + buf_.size());
            this->pubseekpos(pos, std::ios_base::out);
            *this->pptr() = Traits::to_char_type(c);
            this->pbump(1);
            this->pubsync();
            return c;
        } 
        else  
        {
            return Traits::not_eof(c);
        }
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                     std::ios_base::openmode mode = std::ios_base::out) override
    {
        (void)mode; // Always out

        std::streamoff newoff;
        switch (dir)
        {
        case std::ios_base::beg:
            newoff = 0;
            break;
        case std::ios_base::end:
            newoff = static_cast<std::streamoff>(buf_.size());
            break;
        case std::ios_base::cur:
            newoff = static_cast<std::streamoff>(this->pptr() - this->pbase());
            break;
        default:
            return pos_type(off_type(-1));
        }

        off += newoff;

        std::ptrdiff_t n = this->epptr() - this->pbase();

        if (off < 0 || off > n) return pos_type(off_type(-1));
        else
        {
            this->setp(this->pbase(), this->pbase() + n);
            this->pbump(static_cast<int>(off));
        }

        return pos_type(off);
    }

    pos_type seekoff_beg(off_type off)
    {
        std::ptrdiff_t n = this->epptr() - this->pbase();

        if (off < 0 || off > n) 
        {
            return pos_type(off_type(-1));
        }
        else
        {
            this->setp(this->pbase(), this->pbase() + n);
            this->pbump(static_cast<int>(off));
        }

        return pos_type(off);
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode mode
                     = std::ios_base::out) override
    {
          
        (void)mode; // Always out

        return seekoff_beg(pos - pos_type(off_type(0)));
    }
};

template<class CharT, class Traits>
class basic_osequencestream :
    public std::basic_ostream<CharT, Traits>
{
public:
    typedef typename std::basic_ios<CharT, Traits>::char_type       char_type;
    typedef typename std::basic_ios<char_type, Traits>::int_type    int_type;
    typedef typename std::basic_ios<char_type, Traits>::pos_type    pos_type;
    typedef typename std::basic_ios<char_type, Traits>::off_type    off_type;
    typedef typename std::basic_ios<char_type, Traits>::traits_type traits_type;

private:
    typedef basic_osequencebuf<CharT, Traits>      base_ouputbuf;
    typedef std::basic_ios<char_type, Traits>      base_ios;

    basic_osequencebuf<CharT, Traits> buf_;

public:
    basic_osequencestream() JSONCONS_NOEXCEPT
        : std::basic_ostream<CharT, Traits>( (std::basic_streambuf<CharT, Traits>*)(&buf_)),
          buf_()
    {}
    basic_osequencestream(std::size_t length) JSONCONS_NOEXCEPT
        : std::basic_ostream<CharT, Traits>( (std::basic_streambuf<CharT, Traits>*)(&buf_)),
          buf_(length)
    {}

    basic_osequencestream(const basic_osequencestream<CharT,Traits>&) = delete;

    //basic_osequencestream(basic_osequencestream<CharT,Traits>&&) = default;

    virtual ~basic_osequencestream() JSONCONS_NOEXCEPT 
    { 
    }

    basic_osequencestream<CharT,Traits>& operator=(const basic_osequencestream<CharT,Traits>&) = delete;

    //basic_osequencestream<CharT,Traits>& operator=(basic_osequencestream<CharT,Traits>&&) = default;

    const CharT* data() const
    {
        return buf_.data();
    }

    size_t length() const
    {
        return buf_.length();
    }

    void set_locale(const std::locale& loc)
    {
        std::locale result = std::basic_ostream<CharT, Traits>::imbue(loc);
        //this->pubimbue(loc);
    }

    void clear_sequence()
    {
        this->clear();
        this->seekp(0, std::ios::beg);
    }
};

} 

#endif 
