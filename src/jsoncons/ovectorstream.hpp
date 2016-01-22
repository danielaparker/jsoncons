// Copyright 2016 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_OVECTORSTREAM_HPP
#define JSONCONS_OVECTORSTREAM_HPP

#include <ios>
#include <ostream>
#include <string>
#include <cstddef>
#include <vector>
#include "jsoncons/jsoncons_config.hpp"

namespace jsoncons {

template< 
    class CharT, 
    class Traits = std::char_traits<CharT>
> class basic_ovectorstream;

template<class CharT, class CharTraits>
class basic_ovectorbuf
    : public std::basic_streambuf<CharT, CharTraits>
{
private:
    std::ios_base::openmode mode_;
    std::vector<CharT> buf_;

public:
    typedef CharT                                         char_type;
    typedef typename CharTraits::int_type                 int_type;
    typedef typename CharTraits::pos_type                 pos_type;
    typedef typename CharTraits::off_type                 off_type;
    typedef CharTraits                                    traits_type;
    typedef std::basic_streambuf<char_type, traits_type>  base_streambuf;

public:

    explicit basic_ovectorbuf(std::size_t length) JSONCONS_NOEXCEPT
        : base_streambuf(), 
          mode_(std::ios_base::out | std::ios_base::binary), 
          buf_(length)
    {  
        // Set write position to beginning of buffer.
        this->setp(buf_.data(), buf_.data() + buf_.size());
    }

    virtual ~basic_ovectorbuf() {}

    const CharT* data() const
    {
        return buf_.data();
    }

protected:
    int_type underflow() override
    {
        return this->gptr() != this->egptr() ?
               CharTraits::to_int_type(*this->gptr()) : CharTraits::eof();
    }

    int_type pbackfail(int_type c = CharTraits::eof()) override
    {
        if (this->gptr() != this->eback())
        {
            if (!CharTraits::eq_int_type(c, CharTraits::eof()))
            {
                if (CharTraits::eq(CharTraits::to_char_type(c), this->gptr()[-1]))
                {
                    this->gbump(-1);
                    return c;
                }
                this->gbump(-1);
                *this->gptr() = c;
                return c;
            } 
            else
            {
                this->gbump(-1);
                return CharTraits::not_eof(c);
            }
        } 
        else 
        {
            return CharTraits::eof();
        }
    }

    int_type overflow(int_type c = CharTraits::eof()) override
    {
        if (!CharTraits::eq_int_type(c, CharTraits::eof()))
        {
            if (this->pptr() == this->epptr())
            {
                size_t pos = buf_.size();
                buf_.resize(pos*2);
                this->setp(buf_.data(), buf_.data() + buf_.size());
                this->pubseekpos(pos, std::ios_base::out);
                *this->pptr() = CharTraits::to_char_type(c);
                this->pbump(1);
                this->pubsync();
                return c;
            } 
            else
            {
                *this->pptr() = CharTraits::to_char_type(c);
                this->pbump(1);
                return c;
            }
        } 
        else  
        {
            return CharTraits::not_eof(c);
        }
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                     std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) override
    {
        bool in  = false;
        bool out = false;

        const std::ios_base::openmode inout =
            std::ios_base::in | std::ios_base::out;

        if ((mode & inout) == inout)
        {
            if (dir == std::ios_base::beg || dir == std::ios_base::end) in = out = true;
        } 
        else if (mode & std::ios_base::in) 
            in = true;
        else if (mode & std::ios_base::out) 
            out = true;

        if (!in && !out) 
            return pos_type(off_type(-1));
        else 
            if ((in  && (!(mode_ & std::ios_base::in) || (off != 0 && this->gptr() == 0))) ||
                 (out && (!(mode_ & std::ios_base::out) || (off != 0 && this->pptr() == 0)))) 
                return pos_type(off_type(-1));

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
            newoff = in ? static_cast<std::streamoff>(this->gptr() - this->eback())
                : static_cast<std::streamoff>(this->pptr() - this->pbase());
            break;
        default:
            return pos_type(off_type(-1));
        }

        off += newoff;

        if (in)
        {
            std::ptrdiff_t n = this->egptr() - this->eback();

            if (off < 0 || off > n) return pos_type(off_type(-1));
            else this->setg(this->eback(), this->eback() + off, this->eback() + n);
        }

        if (out)
        {
            std::ptrdiff_t n = this->epptr() - this->pbase();

            if (off < 0 || off > n) return pos_type(off_type(-1));
            else
            {
                this->setp(this->pbase(), this->pbase() + n);
                this->pbump(static_cast<int>(off));
            }
        }

        return pos_type(off);
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode mode
                     = std::ios_base::in | std::ios_base::out) override
    {  
        return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);  
    }
};

template<class CharT, class CharTraits>
class basic_ovectorstream :
    private basic_ovectorbuf<CharT, CharTraits>,
    public std::basic_ostream<CharT, CharTraits>
{
public:
    typedef typename std::basic_ios
        <CharT, CharTraits>::char_type          char_type;
    typedef typename std::basic_ios<char_type, CharTraits>::int_type     int_type;
    typedef typename std::basic_ios<char_type, CharTraits>::pos_type     pos_type;
    typedef typename std::basic_ios<char_type, CharTraits>::off_type     off_type;
    typedef typename std::basic_ios<char_type, CharTraits>::traits_type  traits_type;

private:
    typedef basic_ovectorbuf<CharT, CharTraits>         base_ouputbuf;
    typedef std::basic_ios<char_type, CharTraits>      base_ios;
    typedef std::basic_ostream<char_type, CharTraits>  base_streambuf;
    base_ouputbuf& get_buf() {return *this;}
    const base_ouputbuf& get_buf() const {return *this;}

public:
    basic_ovectorstream(std::size_t length) JSONCONS_NOEXCEPT
        : base_ouputbuf(length),  
          base_streambuf(&get_buf())
    {}

    ~basic_ovectorstream() {}

public:

    size_t length()
    {
        return pptr() - pbase();
    }

    void set_locale(const std::locale& loc)
    {
        std::locale result = std::basic_ostream<CharT, CharTraits>::imbue(loc);
        pubimbue(loc);
    }

    void reset()
    {
        clear();
        seekp(0, std::ios::beg);
    }

    const CharT* data() const
    {
        return get_buf().data();
    }
};

} 

#endif 
