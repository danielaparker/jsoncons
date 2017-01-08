// Copyright 2016 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/unicode_traits for latest version

/*
 * Includes code derived from Unicode, Inc decomposition code in ConvertUTF.h and ConvertUTF.c 
 * http://www.unicode.org/  
 *  
 * "Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard."
*/

#ifndef UNICONS_UNICODE_TRAITS_HPP
#define UNICONS_UNICODE_TRAITS_HPP

#if defined (__clang__)
#if defined(_GLIBCXX_USE_NOEXCEPT)
#define UNICONS_NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#else
#define UNICONS_NOEXCEPT noexcept
#endif
#elif defined(__GNUC__)
#define UNICONS_NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#elif defined(_MSC_VER)
#if _MSC_VER >= 1900
#define UNICONS_NOEXCEPT noexcept
#else
#define UNICONS_NOEXCEPT
#endif
#else
#define UNICONS_NOEXCEPT
#endif

#include <string>
#include <iterator>
#include <type_traits>
#include <system_error>

namespace unicons {

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence. Source: ConvertUTF.c
 */
const uint32_t offsets_from_utf8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
              0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total. Source: ConvertUTF.c
 */
const uint8_t first_byte_mark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms. Source: ConvertUTF.c
 */
const uint8_t trailing_bytes_for_utf8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

// Some fundamental constants.  Source: ConvertUTF.h 
const uint32_t uni_replacement_char = 0x0000FFFD;
const uint32_t uni_max_bmp = 0x0000FFFF;
const uint32_t uni_max_utf16 = 0x0010FFFF;
const uint32_t uni_max_utf32 = 0x7FFFFFFF;
const uint32_t uni_max_legal_utf32 = 0x0010FFFF;

const int half_shift  = 10; // used for shifting by 10 bits
const uint32_t half_base = 0x0010000UL;
const uint32_t half_mask = 0x3FFUL;

const uint16_t uni_sur_high_start = 0xD800;
const uint16_t uni_sur_high_end = 0xDBFF;
const uint16_t uni_sur_low_start = 0xDC00;
const uint16_t uni_sur_low_end = 0xDFFF;

enum class conv_flags 
{
    strict = 0,
    lenient
};

// unicode_traits

enum class uni_errc 
{
    ok = 0,                         // conversion successful
    over_long_utf8_sequence = 1,    // over long utf8 sequence
    expected_continuation_byte = 2, // expected continuation byte    
    unpaired_high_surrogate = 3,    // unpaired high surrogate UTF-16
    illegal_surrogate_value = 4,    // UTF-16 surrogate values are illegal in UTF-32
    source_exhausted = 5,           // partial character in source, but hit end
    source_illegal = 6              // source sequence is illegal/malformed
};

class Unicode_traits_error_category_impl_
   : public std::error_category
{
public:
    virtual const char* name() const UNICONS_NOEXCEPT
    {
        return "unicode_traits";
    }
    virtual std::string message(int ev) const
    {
        switch (static_cast<uni_errc>(ev))
        {
        case uni_errc::over_long_utf8_sequence:
            return "Over long utf8 sequence";
        case uni_errc::expected_continuation_byte:
            return "Expected continuation byte";
        case uni_errc::unpaired_high_surrogate:
            return "Unpaired high surrogate UTF-16";
        case uni_errc::illegal_surrogate_value:
            return "UTF-16 surrogate values are illegal in UTF-32";
        case uni_errc::source_exhausted:
            return "Partial character in source, but hit end";
        case uni_errc::source_illegal:
            return "Source sequence is illegal/malformed";
        default:
            return "";
            break;
        }
    }
};

inline
const std::error_category& unicode_traits_error_category()
{
  static Unicode_traits_error_category_impl_ instance;
  return instance;
}

inline 
std::error_code make_error_code(uni_errc result)
{
    return std::error_code(static_cast<int>(result),unicode_traits_error_category());
}

// sequence_generator

template <class Iterator>
class sequence_generator
{
    Iterator begin_;
    Iterator last_;
    size_t length_;
    uni_errc err_cd_;
public:
    sequence_generator(Iterator first, Iterator last)
        : begin_(first), last_(last), length_(0), err_cd_(uni_errc::ok)
    {
        next();
    }

    bool done() const
    {
        return err_cd_ != uni_errc::ok || begin_ == last_;
    }

    uni_errc status() const
    {
        return err_cd_;
    }

    std::pair<Iterator,size_t> get() const 
    {
        return std::make_pair(begin_,length_);
    }

    template <class CharT = typename std::iterator_traits<Iterator>::value_type>
    typename std::enable_if<sizeof(CharT) == sizeof(uint8_t),uint32_t>::type 
    get_codepoint()
    {
        uint32_t ch = 0;
        Iterator it = begin_;
        switch (length_) 
        {
        default:
            throw std::invalid_argument("Invalid sequence");
            break;
        case 4: ch += static_cast<uint8_t>(*it++); ch <<= 6;
        case 3: ch += static_cast<uint8_t>(*it++); ch <<= 6;
        case 2: ch += static_cast<uint8_t>(*it++); ch <<= 6;
        case 1: ch += static_cast<uint8_t>(*it++);
            ch -= offsets_from_utf8[length_ - 1];
            break;
        }
        return ch;
    }

    template <class CharT = typename std::iterator_traits<Iterator>::value_type>
    typename std::enable_if<sizeof(CharT) == sizeof(uint16_t),uint32_t>::type 
    get_codepoint()
    {
        if (length_ == 0)
        {
            throw std::invalid_argument("Invalid sequence");
        }
        if (length_ == 2)
        {
            uint32_t ch = *begin_;
            uint32_t ch2 = *(begin_ + 1);
            ch = ((ch - uni_sur_high_start) << half_shift)
                 + (ch2 - uni_sur_low_start) + half_base;
            return ch;
        }
        else 
        {
            return *begin_;
        }
    }

    template <class CharT = typename std::iterator_traits<Iterator>::value_type>
    typename std::enable_if<sizeof(CharT) == sizeof(uint32_t),uint32_t>::type 
    get_codepoint()
    {
        if (length_ == 0)
        {
            throw std::invalid_argument("Invalid sequence");
        }
        return *begin_;
    }

    template <class CharT = typename std::iterator_traits<Iterator>::value_type>
    typename std::enable_if<sizeof(CharT) == sizeof(uint8_t)>::type 
    next() 
    {
        begin_ += length_;
        if (begin_ != last_)
        {
            size_t length = trailing_bytes_for_utf8[static_cast<uint8_t>(*begin_)] + 1;
            if (length > (size_t)(last_ - begin_))
            {
                err_cd_ = uni_errc::source_exhausted;
            }
            else if ((err_cd_ = is_legal_utf8(begin_, length)) != uni_errc::ok)
            {
            }
            else
            {
                length_ = length;
            }
        }
    }

    template <class CharT = typename std::iterator_traits<Iterator>::value_type>
    typename std::enable_if<sizeof(CharT) == sizeof(uint16_t)>::type 
    next() 
    {
        begin_ += length_;
        if (begin_ != last_)
        {
            if (begin_ != last_)
            {

                Iterator it = begin_;

                uint32_t ch = *it++;
                /* If we have a surrogate pair, validate to uint32_t it. */
                if (ch >= uni_sur_high_start && ch <= uni_sur_high_end) 
                {
                    /* If the 16 bits following the high surrogate are in the it buffer... */
                    if (it < last_) {
                        uint32_t ch2 = *it;
                        /* If it's a low surrogate, */
                        if (ch2 >= uni_sur_low_start && ch2 <= uni_sur_low_end) 
                        {
                            ++it;
                            length_ = 2;
                        } 
                        else 
                        {
                            err_cd_ = uni_errc::unpaired_high_surrogate;
                        }
                    } 
                    else 
                    { 
                        // We don't have the 16 bits following the high surrogate.
                        err_cd_ = uni_errc::source_exhausted;
                    }
                } 
                else if (ch >= uni_sur_low_start && ch <= uni_sur_low_end) 
                {
                    /* leading low surrogate */
                    err_cd_ = uni_errc::source_illegal;
                }
                else
                {
                    length_ = 1;
                }
            }
        }
    }

    template <class CharT = typename std::iterator_traits<Iterator>::value_type>
    typename std::enable_if<sizeof(CharT) == sizeof(uint32_t)>::type 
    next() 
    {
        begin_ += length_;
        length_ = 1;
    }
};

// utf8

template <class Iterator>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<Iterator>::value_type>::value 
                              && sizeof(typename std::iterator_traits<Iterator>::value_type) == sizeof(uint8_t), 
                              uni_errc >::type
is_legal_utf8(Iterator first, size_t length) 
{
    uint8_t a;
    Iterator srcptr = first+length;
    switch (length) {
    default: return uni_errc::over_long_utf8_sequence;
        /* Everything else falls through when "true"... */
    case 4: if (((a = (*--srcptr))& 0xC0) != 0x80) 
        return uni_errc::expected_continuation_byte;
    case 3: if (((a = (*--srcptr))& 0xC0) != 0x80) 
        return uni_errc::expected_continuation_byte;
    case 2: if (((a = (*--srcptr))& 0xC0) != 0x80) 
        return uni_errc::expected_continuation_byte;

        switch (static_cast<uint8_t>(*first)) 
        {
            /* no fall-through in this inner switch */
            case 0xE0: if (a < 0xA0) return uni_errc::source_illegal; break;
            case 0xED: if (a > 0x9F) return uni_errc::source_illegal; break;
            case 0xF0: if (a < 0x90) return uni_errc::source_illegal; break;
            case 0xF4: if (a > 0x8F) return uni_errc::source_illegal; break;
            default:   if (a < 0x80) return uni_errc::source_illegal;
        }

    case 1: if (static_cast<uint8_t>(*first) >= 0x80 && static_cast<uint8_t>(*first) < 0xC2) 
        return uni_errc::source_illegal;
    }
    if (static_cast<uint8_t>(*first) > 0xF4) 
        return uni_errc::source_illegal;

    return uni_errc::ok;
}

template <class...> using void_t = void;

template <class, class, class = void>
struct is_output_iterator : std::false_type {};

template <class I, class E>
struct is_output_iterator<I, E, void_t<
    typename std::iterator_traits<I>::iterator_category,
    decltype(*std::declval<I>() = std::declval<E>())>> : std::true_type {};

template<class OutputIt, class CharT, class Enable = void>
struct is_compatible_output_iterator : std::false_type {};

template<class OutputIt, class CharT>
struct is_compatible_output_iterator<OutputIt,CharT,
    typename std::enable_if<(is_output_iterator<OutputIt,CharT>::value
                             && std::is_void<typename std::iterator_traits<OutputIt>::value_type>::value
                             && std::is_integral<typename OutputIt::container_type::value_type>::value 
                             && !std::is_void<typename OutputIt::container_type::value_type>::value
                             && sizeof(typename OutputIt::container_type::value_type) == sizeof(CharT))>::type
> : std::true_type {};

template<class OutputIt, class CharT>
struct is_compatible_output_iterator<OutputIt,CharT,
    typename std::enable_if<is_output_iterator<OutputIt,CharT>::value
                            && std::is_integral<typename std::iterator_traits<OutputIt>::value_type>::value 
                            && sizeof(typename std::iterator_traits<OutputIt>::value_type) == sizeof(CharT)>::type
> : std::true_type {};

template<class OutputIt, class CharT>
struct is_compatible_output_iterator<OutputIt,CharT,
    typename std::enable_if<(is_output_iterator<OutputIt,CharT>::value
                             && std::is_void<typename std::iterator_traits<OutputIt>::value_type>::value
                             && sizeof(typename OutputIt::char_type) == sizeof(CharT))>::type
> : std::true_type {};

// convert

template <class InputIt,class OutputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint8_t)
                               && is_compatible_output_iterator<OutputIt,uint8_t>::value,std::pair<uni_errc,InputIt>>::type 
convert(InputIt first, InputIt last, OutputIt target, conv_flags flags=conv_flags::strict) 
{
    (void)flags;

    uni_errc  result = uni_errc::ok;
    while (first != last) 
    {
        size_t length = trailing_bytes_for_utf8[static_cast<uint8_t>(*first)] + 1;
        if (length > (size_t)(last - first))
        {
            return std::make_pair(uni_errc::source_exhausted,first);
        }
        if ((result=is_legal_utf8(first, length)) != uni_errc::ok)
        {
            return std::make_pair(result,first);
        }

        switch (length) {
            case 4: *target++ = (static_cast<uint8_t>(*first++));
            case 3: *target++ = (static_cast<uint8_t>(*first++));
            case 2: *target++ = (static_cast<uint8_t>(*first++));
            case 1: *target++ = (static_cast<uint8_t>(*first++));
        }
    }
    return std::make_pair(result,first);
}

template <class InputIt,class OutputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint8_t)
                               && is_compatible_output_iterator<OutputIt,uint16_t>::value,std::pair<uni_errc,InputIt>>::type 
convert(InputIt first, InputIt last, 
        OutputIt target, 
        conv_flags flags = conv_flags::strict) 
{
    uni_errc  result = uni_errc::ok;

    while (first != last) 
    {
        uint32_t ch = 0;
        unsigned short extra_bytes_to_read = trailing_bytes_for_utf8[static_cast<uint8_t>(*first)];
        if (extra_bytes_to_read >= last - first) 
        {
            result = uni_errc::source_exhausted; 
            break;
        }
        /* Do this check whether lenient or strict */
        if ((result=is_legal_utf8(first, extra_bytes_to_read+1)) != uni_errc::ok)
        {
            break;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        switch (extra_bytes_to_read) {
            case 5: ch += static_cast<uint8_t>(*first++); ch <<= 6; /* remember, illegal UTF-8 */
            case 4: ch += static_cast<uint8_t>(*first++); ch <<= 6; /* remember, illegal UTF-8 */
            case 3: ch += static_cast<uint8_t>(*first++); ch <<= 6;
            case 2: ch += static_cast<uint8_t>(*first++); ch <<= 6;
            case 1: ch += static_cast<uint8_t>(*first++); ch <<= 6;
            case 0: ch += static_cast<uint8_t>(*first++);
        }
        ch -= offsets_from_utf8[extra_bytes_to_read];

        if (ch <= uni_max_bmp) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= uni_sur_high_start && ch <= uni_sur_low_end ) {
                if (flags == conv_flags::strict) {
                    first -= (extra_bytes_to_read+1); /* return to the illegal value itself */
                    result = uni_errc::source_illegal;
                    break;
                } else {
                    *target++ = (uni_replacement_char);
                }
            } else {
                *target++ = ((uint16_t)ch); /* normal case */
            }
        } else if (ch > uni_max_utf16) {
            if (flags == conv_flags::strict) {
                result = uni_errc::source_illegal;
                first -= (extra_bytes_to_read+1); /* return to the start */
                break; /* Bail out; shouldn't continue */
            } else {
                *target++ = (uni_replacement_char);
            }
        } else {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            ch -= half_base;
            *target++ = ((uint16_t)((ch >> half_shift) + uni_sur_high_start));
            *target++ = ((uint16_t)((ch & half_mask) + uni_sur_low_start));
        }
    }
    return std::make_pair(result,first);
}

template <class InputIt,class OutputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint8_t)
                               && is_compatible_output_iterator<OutputIt,uint32_t>::value,std::pair<uni_errc,InputIt>>::type 
convert(InputIt first, InputIt last, 
                 OutputIt target, 
                 conv_flags flags = conv_flags::strict) 
{
    uni_errc  result = uni_errc::ok;

    while (first < last) 
    {
        uint32_t ch = 0;
        unsigned short extra_bytes_to_read = trailing_bytes_for_utf8[static_cast<uint8_t>(*first)];
        if (extra_bytes_to_read >= last - first) 
        {
            result = uni_errc::source_exhausted; 
            break;
        }
        /* Do this check whether lenient or strict */
        if ((result=is_legal_utf8(first, extra_bytes_to_read+1)) != uni_errc::ok) {
            break;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        switch (extra_bytes_to_read) {
            case 5: ch += static_cast<uint8_t>(*first++); ch <<= 6;
            case 4: ch += static_cast<uint8_t>(*first++); ch <<= 6;
            case 3: ch += static_cast<uint8_t>(*first++); ch <<= 6;
            case 2: ch += static_cast<uint8_t>(*first++); ch <<= 6;
            case 1: ch += static_cast<uint8_t>(*first++); ch <<= 6;
            case 0: ch += static_cast<uint8_t>(*first++);
        }
        ch -= offsets_from_utf8[extra_bytes_to_read];

        if (ch <= uni_max_legal_utf32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            if (ch >= uni_sur_high_start && ch <= uni_sur_low_end ) {
                if (flags == conv_flags::strict) {
                    first -= (extra_bytes_to_read+1); /* return to the illegal value itself */
                    result = uni_errc::source_illegal;
                    break;
                } else {
                    *target++ = (uni_replacement_char);
                }
            } else {
                *target++ = (ch);
            }
        } else { /* i.e., ch > uni_max_legal_utf32 */
            result = uni_errc::source_illegal;
            *target++ = (uni_replacement_char);
        }
    }
    return std::make_pair(result,first);
}

// utf16

template <class InputIt,class OutputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint16_t)
                               && is_compatible_output_iterator<OutputIt,uint8_t>::value,std::pair<uni_errc,InputIt>>::type 
convert(InputIt first, InputIt last, 
                 OutputIt target, 
                 conv_flags flags = conv_flags::strict) {
    uni_errc  result = uni_errc::ok;
    while (first < last) {
        unsigned short bytes_to_write = 0;
        const uint32_t byteMask = 0xBF;
        const uint32_t byteMark = 0x80; 
        uint32_t ch = *first++;
        /* If we have a surrogate pair, convert to uint32_t first. */
        if (ch >= uni_sur_high_start && ch <= uni_sur_high_end) {
            /* If the 16 bits following the high surrogate are in the first buffer... */
            if (first < last) {
                uint32_t ch2 = *first;
                /* If it's a low surrogate, convert to uint32_t. */
                if (ch2 >= uni_sur_low_start && ch2 <= uni_sur_low_end) {
                    ch = ((ch - uni_sur_high_start) << half_shift)
                        + (ch2 - uni_sur_low_start) + half_base;
                    ++first;
                } else if (flags == conv_flags::strict) { /* it's an unpaired high surrogate */
                    --first; /* return to the illegal value itself */
                    result = uni_errc::unpaired_high_surrogate;
                    break;
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                --first; /* return to the high surrogate */
                result = uni_errc::source_exhausted;
                break;
            }
        } else if (flags == conv_flags::strict) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= uni_sur_low_start && ch <= uni_sur_low_end) {
                --first; /* return to the illegal value itself */
                result = uni_errc::source_illegal;
                break;
            }
        }
        /* Figure out how many bytes the result will require */
        if (ch < (uint32_t)0x80) {      
            bytes_to_write = 1;
        } else if (ch < (uint32_t)0x800) {     
            bytes_to_write = 2;
        } else if (ch < (uint32_t)0x10000) {   
            bytes_to_write = 3;
        } else if (ch < (uint32_t)0x110000) {  
            bytes_to_write = 4;
        } else {                            
            bytes_to_write = 3;
            ch = uni_replacement_char;
        }
        
        uint8_t byte1 = 0;
        uint8_t byte2 = 0;
        uint8_t byte3 = 0;
        uint8_t byte4 = 0;

        switch (bytes_to_write) { // note: everything falls through
            case 4: byte4 = (uint8_t)((ch | byteMark) & byteMask); ch >>= 6;
            case 3: byte3 = (uint8_t)((ch | byteMark) & byteMask); ch >>= 6;
            case 2: byte2 = (uint8_t)((ch | byteMark) & byteMask); ch >>= 6;
            case 1: byte1 = (uint8_t)(ch | first_byte_mark[bytes_to_write]);
        }
        switch (bytes_to_write) 
        {
        case 4: 
            *target++ = (byte1);
            *target++ = (byte2);
            *target++ = (byte3);
            *target++ = (byte4);
            break;
        case 3: 
            *target++ = (byte1);
            *target++ = (byte2);
            *target++ = (byte3);
            break;
        case 2: 
            *target++ = (byte1);
            *target++ = (byte2);
            break;
        case 1: 
            *target++ = (byte1);
            break;
        }
    }
    return std::make_pair(result,first);
}

template <class InputIt,class OutputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint16_t)
                               && is_compatible_output_iterator<OutputIt,uint16_t>::value,std::pair<uni_errc,InputIt>>::type 
convert(InputIt first, InputIt last, 
        OutputIt target, 
        conv_flags flags = conv_flags::strict) 
{
    uni_errc  result = uni_errc::ok;

    while (first != last) 
    {
        uint32_t ch = *first++;
        /* If we have a surrogate pair, convert to uint32_t first. */
        if (ch >= uni_sur_high_start && ch <= uni_sur_high_end) 
        {
            /* If the 16 bits following the high surrogate are in the first buffer... */
            if (first < last) {
                uint32_t ch2 = *first;
                /* If it's a low surrogate, */
                if (ch2 >= uni_sur_low_start && ch2 <= uni_sur_low_end) {
                    *target++ = ((uint16_t)ch);
                    *target++ = ((uint16_t)ch2);
                    ++first;
                } else if (flags == conv_flags::strict) { /* it's an unpaired high surrogate */
                    --first; /* return to the illegal value itself */
                    result = uni_errc::unpaired_high_surrogate;
                    break;
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                --first; /* return to the high surrogate */
                result = uni_errc::source_exhausted;
                break;
            }
        } else if (ch >= uni_sur_low_start && ch <= uni_sur_low_end) 
        {
            // illegal leading low surrogate
            if (flags == conv_flags::strict) {
                --first; /* return to the illegal value itself */
                result = uni_errc::source_illegal;
                break;
            }
            else
            {
                *target++ = ((uint16_t)ch);
            }
        }
        else
        {
            *target++ = ((uint16_t)ch);
        }
    }
    return std::make_pair(result,first);
}

template <class InputIt,class OutputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint16_t)
                               && is_compatible_output_iterator<OutputIt,uint32_t>::value,std::pair<uni_errc,InputIt>>::type 
convert(InputIt first, InputIt last, 
                 OutputIt target, 
                 conv_flags flags = conv_flags::strict) 
{
    uni_errc  result = uni_errc::ok;

    while (first != last) 
    {
        uint32_t ch = *first++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= uni_sur_high_start && ch <= uni_sur_high_end) {
            /* If the 16 bits following the high surrogate are in the first buffer... */
            if (first < last) {
                uint32_t ch2 = *first;
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= uni_sur_low_start && ch2 <= uni_sur_low_end ) {
                    ch = ((ch - uni_sur_high_start) << half_shift)
                        + (ch2 - uni_sur_low_start) + half_base;
                    ++first;
                } else if (flags == conv_flags::strict) { /* it's an unpaired high surrogate */
                    --first; /* return to the illegal value itself */
                    result = uni_errc::source_illegal;
                    break;
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                --first; /* return to the high surrogate */
                result = uni_errc::source_exhausted;
                break;
            }
        } else if (flags == conv_flags::strict) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= uni_sur_low_start && ch <= uni_sur_low_end ) {
                --first; /* return to the illegal value itself */
                result = uni_errc::source_illegal;
                break;
            }
        }
        *target++ = (ch);
    }
    return std::make_pair(result,first);
}

// utf32

template <class InputIt,class OutputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint32_t)
                               && is_compatible_output_iterator<OutputIt,uint8_t>::value,std::pair<uni_errc,InputIt>>::type 
convert(InputIt first, InputIt last, 
        OutputIt target, 
        conv_flags flags = conv_flags::strict) 
{
    uni_errc  result = uni_errc::ok;
    while (first < last) {
        unsigned short bytes_to_write = 0;
        const uint32_t byteMask = 0xBF;
        const uint32_t byteMark = 0x80; 
        uint32_t ch = *first++;
        if (flags == conv_flags::strict ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= uni_sur_high_start && ch <= uni_sur_low_end) {
                --first; /* return to the illegal value itself */
                result = uni_errc::illegal_surrogate_value;
                break;
            }
        }
        /*
         * Figure out how many bytes the result will require. Turn any
         * illegally large UTF32 things (> Plane 17) into replacement chars.
         */
        if (ch < (uint32_t)0x80) {      bytes_to_write = 1;
        } else if (ch < (uint32_t)0x800) {     bytes_to_write = 2;
        } else if (ch < (uint32_t)0x10000) {   bytes_to_write = 3;
        } else if (ch <= uni_max_legal_utf32) {  bytes_to_write = 4;
        } else {                            
            bytes_to_write = 3;
            ch = uni_replacement_char;
            result = uni_errc::source_illegal;
        }

        uint8_t byte1 = 0;
        uint8_t byte2 = 0;
        uint8_t byte3 = 0;
        uint8_t byte4 = 0;

        switch (bytes_to_write) { // note: everything falls through
        case 4: byte4 = (uint8_t)((ch | byteMark) & byteMask); ch >>= 6;
        case 3: byte3 = (uint8_t)((ch | byteMark) & byteMask); ch >>= 6;
        case 2: byte2 = (uint8_t)((ch | byteMark) & byteMask); ch >>= 6;
        case 1: byte1 = (uint8_t) (ch | first_byte_mark[bytes_to_write]);
        }

        switch (bytes_to_write) 
        {
        case 4: 
            *target++ = (byte1);
            *target++ = (byte2);
            *target++ = (byte3);
            *target++ = (byte4);
            break;
        case 3: 
            *target++ = (byte1);
            *target++ = (byte2);
            *target++ = (byte3);
            break;
        case 2: 
            *target++ = (byte1);
            *target++ = (byte2);
            break;
        case 1: 
            *target++ = (byte1);
            break;
        }
    }
    return std::make_pair(result,first);
}

template <class InputIt,class OutputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint32_t)
                               && is_compatible_output_iterator<OutputIt,uint16_t>::value,std::pair<uni_errc,InputIt>>::type 
convert(InputIt first, InputIt last, 
                 OutputIt target, 
                 conv_flags flags = conv_flags::strict) 
{
    uni_errc  result = uni_errc::ok;

    while (first != last) 
    {
        uint32_t ch = *first++;
        if (ch <= uni_max_bmp) { /* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
            if (ch >= uni_sur_high_start && ch <= uni_sur_low_end ) {
                if (flags == conv_flags::strict) {
                    --first; /* return to the illegal value itself */
                    result = uni_errc::source_illegal;
                    break;
                } else {
                    *target++ = (uni_replacement_char);
                }
            } else {
                *target++ = ((uint16_t)ch); /* normal case */
            }
        } else if (ch > uni_max_legal_utf32) {
            if (flags == conv_flags::strict) {
                result = uni_errc::source_illegal;
            } else {
                *target++ = (uni_replacement_char);
            }
        } else {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            ch -= half_base;
            *target++ = ((uint16_t)((ch >> half_shift) + uni_sur_high_start));
            *target++ = ((uint16_t)((ch & half_mask) + uni_sur_low_start));
        }
    }
    return std::make_pair(result,first);
}

template <class InputIt,class OutputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint32_t)
                               && is_compatible_output_iterator<OutputIt,uint32_t>::value,std::pair<uni_errc,InputIt>>::type 
convert(InputIt first, InputIt last, 
                 OutputIt target, 
                 conv_flags flags = conv_flags::strict) 
{
    uni_errc  result = uni_errc::ok;

    while (first != last) 
    {
        uint32_t ch = *first++;
        if (flags == conv_flags::strict ) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= uni_sur_high_start && ch <= uni_sur_low_end) {
                --first; /* return to the illegal value itself */
                result = uni_errc::illegal_surrogate_value;
                break;
            }
        }
        if (ch <= uni_max_legal_utf32)
        {
            *target++ = (ch);
        }
        else
        {
            *target++ = (uni_replacement_char);
            result = uni_errc::source_illegal;
        }
    }
    return std::make_pair(result,first);
}

// validate

template <class InputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint8_t)
                               ,std::pair<uni_errc,InputIt>>::type 
validate(InputIt first, InputIt last) 
{
    uni_errc  result = uni_errc::ok;
    while (first != last) 
    {
        size_t length = trailing_bytes_for_utf8[static_cast<uint8_t>(*first)] + 1;
        if (length > (size_t)(last - first))
        {
            return std::make_pair(uni_errc::source_exhausted,first);
        }
        if ((result=is_legal_utf8(first, length)) != uni_errc::ok)
        {
            return std::make_pair(result,first);
        }
        first += length;
    }
    return std::make_pair(result,first);
}

// utf16

template <class InputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint16_t)
                               ,std::pair<uni_errc,InputIt>>::type 
validate(InputIt first, InputIt last) 
{
    uni_errc  result = uni_errc::ok;

    while (first != last) 
    {
        uint32_t ch = *first++;
        /* If we have a surrogate pair, validate to uint32_t first. */
        if (ch >= uni_sur_high_start && ch <= uni_sur_high_end) 
        {
            /* If the 16 bits following the high surrogate are in the first buffer... */
            if (first < last) {
                uint32_t ch2 = *first;
                /* If it's a low surrogate, */
                if (ch2 >= uni_sur_low_start && ch2 <= uni_sur_low_end) {
                    ++first;
                } else {
                    --first; /* return to the illegal value itself */
                    result = uni_errc::unpaired_high_surrogate;
                    break;
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                --first; /* return to the high surrogate */
                result = uni_errc::source_exhausted;
                break;
            }
        } else if (ch >= uni_sur_low_start && ch <= uni_sur_low_end) 
        {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            --first; /* return to the illegal value itself */
            result = uni_errc::source_illegal;
            break;
        }
    }
    return std::make_pair(result,first);
}


// utf32


template <class InputIt>
static typename std::enable_if<std::is_integral<typename std::iterator_traits<InputIt>::value_type>::value && sizeof(typename std::iterator_traits<InputIt>::value_type) == sizeof(uint32_t)
                               ,std::pair<uni_errc,InputIt>>::type 
validate(InputIt first, InputIt last) 
{
    uni_errc  result = uni_errc::ok;

    while (first != last) 
    {
        uint32_t ch = *first++;
        /* UTF-16 surrogate values are illegal in UTF-32 */
        if (ch >= uni_sur_high_start && ch <= uni_sur_low_end) {
            --first; /* return to the illegal value itself */
            result = uni_errc::illegal_surrogate_value;
            break;
        }
        if (!(ch <= uni_max_legal_utf32))
        {
            result = uni_errc::source_illegal;
        }
    }
    return std::make_pair(result,first);
}
// unicode_traits

template <class CharT, class Enable=void>
struct unicode_traits
{
};

template <class CharT>
struct unicode_traits<CharT,
                      typename std::enable_if<std::is_integral<CharT>::value &&
                      sizeof(CharT)==sizeof(uint8_t)>::type> 
{
    static size_t utf_length(const CharT*, size_t length)
    {
        return length;
    }

    static std::pair<const CharT*,size_t> sequence_at(const CharT* it, 
                                                      const CharT* end,
                                                      size_t index)
    {
        const CharT* p = it;
        size_t count = 0;

        while (p < end && count < index)
        {
            uint8_t ch = *p;
            size_t length = trailing_bytes_for_utf8[ch] + 1;
            p += length;
            ++count;
        }

        if (p < end)
        {
            uint8_t ch = *p;
            size_t len = trailing_bytes_for_utf8[ch] + 1;
            return (p+len <= end) ? std::make_pair(p,len) : std::make_pair(it,static_cast<size_t>(0));
        }
        else
        {
            return std::make_pair(it,static_cast<size_t>(0));
        }
    }

    static size_t codepoint_count(const CharT* it, 
                                  const CharT* end)
    {
        size_t count = 0;
        const CharT* p = it;
        while (p < end)
        {
            size_t length = trailing_bytes_for_utf8[(uint8_t)(*p)] + 1;
            p += length;
            ++count;
        }
        return count;
    }

    /*
     * Indicates whether a sequence of bytes is legal UTF-8.
     * This must be called with the length pre-determined by the first byte.
     * If not calling this from ConvertUTF8to*, then the length can be set by:
     *  length = trailing_bytes_for_utf8[*source]+1;
     * and the sequence is illegal right away if there aren't that many bytes
     * available.
     * If presented with a length > 4, this returns false.  The Unicode
     * definition of UTF-8 goes up to 4-byte sequences.
     */

    template <class UTF8>
    static typename std::enable_if<std::is_integral<UTF8>::value && sizeof(UTF8) == sizeof(uint8_t), uni_errc >::type
    is_legal(const UTF8 *source, size_t length) 
    {
        uint8_t a;
        const UTF8 *srcptr = source+length;
        switch (length) {
        default: return uni_errc::over_long_utf8_sequence;
            /* Everything else falls through when "true"... */
        case 4: if (((a = (*--srcptr))& 0xC0) != 0x80) 
            return uni_errc::expected_continuation_byte;
        case 3: if (((a = (*--srcptr))& 0xC0) != 0x80) 
            return uni_errc::expected_continuation_byte;
        case 2: if (((a = (*--srcptr))& 0xC0) != 0x80) 
            return uni_errc::expected_continuation_byte;

            switch (static_cast<uint8_t>(*source)) 
            {
                /* no fall-through in this inner switch */
                case 0xE0: if (a < 0xA0) return uni_errc::source_illegal; break;
                case 0xED: if (a > 0x9F) return uni_errc::source_illegal; break;
                case 0xF0: if (a < 0x90) return uni_errc::source_illegal; break;
                case 0xF4: if (a > 0x8F) return uni_errc::source_illegal; break;
                default:   if (a < 0x80) return uni_errc::source_illegal;
            }

        case 1: if (static_cast<uint8_t>(*source) >= 0x80 && static_cast<uint8_t>(*source) < 0xC2) 
            return uni_errc::source_illegal;
        }
        if (static_cast<uint8_t>(*source) > 0xF4) 
            return uni_errc::source_illegal;

        return uni_errc::ok;
    }

    template <class UTF32>
    static typename std::enable_if<std::is_integral<UTF32>::value && sizeof(UTF32) == sizeof(uint32_t),uni_errc >::type 
    next_codepoint(const CharT* source_begin, const CharT* source_end, 
                   UTF32* target, const CharT** source_stop,
                   conv_flags flags = conv_flags::strict) 
    {
        uni_errc  result = uni_errc::ok;

        const CharT* source = source_begin;

        unsigned short extra_bytes_to_read = trailing_bytes_for_utf8[(uint8_t)(*source)];
        if (extra_bytes_to_read >= source_end - source) 
        {
            result = uni_errc::source_exhausted;
            *source_stop = source;
            return result;
        }
        /* Do this check whether lenient or strict */
        if ((result=is_legal(source, extra_bytes_to_read+1)) != uni_errc::ok)
        {
            *source_stop = source;
            return result;
        }
        /*
         * The cases all fall through. See "Note A" below.
         */
        UTF32 ch = 0;
        switch (extra_bytes_to_read) {
            case 5: ch += static_cast<uint8_t>(*source++); ch <<= 6;
            case 4: ch += static_cast<uint8_t>(*source++); ch <<= 6;
            case 3: ch += static_cast<uint8_t>(*source++); ch <<= 6;
            case 2: ch += static_cast<uint8_t>(*source++); ch <<= 6;
            case 1: ch += static_cast<uint8_t>(*source++); ch <<= 6;
            case 0: ch += static_cast<uint8_t>(*source++);
        }
        ch -= offsets_from_utf8[extra_bytes_to_read];

        if (ch <= uni_max_legal_utf32) {
            /*
             * UTF-16 surrogate values are illegal in UTF-32, and anything
             * over Plane 17 (> 0x10FFFF) is illegal.
             */
            if (ch >= uni_sur_high_start && ch <= uni_sur_low_end) {
                if (flags == conv_flags::strict) {
                    source -= (extra_bytes_to_read+1); /* return to the illegal value itself */
                    result = uni_errc::illegal_surrogate_value;
                } else {
                    *target = uni_replacement_char;
                }
            } else {
                *target = ch;
            }
        } else { /* i.e., ch > uni_max_legal_utf32 */
            result = uni_errc::source_illegal;
            *target = uni_replacement_char;
        }
        *source_stop = source;
        return result;
    }

    static size_t detect_bom(const CharT* it, size_t length)
    {
        size_t count = 0;
        if (length >= 3)
        {
            uint32_t bom = static_cast<uint32_t>(static_cast<uint8_t>(it[0]) |
                                                (static_cast<uint8_t>(it[1]) << 8) |
                                                (static_cast<uint8_t>(it[2]) << 16));
            if ((bom & 0xFFFFFF) == 0xBFBBEF)  
                count += 3;
        }
        return count;
    }

    template <class STraits,class SAllocator>
    static void append_codepoint_to_string(uint32_t cp, std::basic_string<CharT,STraits,SAllocator>& s)
    {
        auto result = convert(&cp,&cp+1,std::back_inserter(s));
/*
        if (cp <= 0x7f)
        {
            s.push_back(static_cast<CharT>(cp));
        }
        else if (cp <= 0x7FF)
        {
            s.push_back(static_cast<CharT>(0xC0 | (0x1f & (cp >> 6))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & cp)));
        }
        else if (cp <= 0xFFFF)
        {
            s.push_back(0xE0 | static_cast<CharT>((0xf & (cp >> 12))));
            s.push_back(0x80 | static_cast<CharT>((0x3f & (cp >> 6))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & cp)));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<CharT>(0xF0 | (0x7 & (cp >> 18))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & (cp >> 12))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & (cp >> 6))));
            s.push_back(static_cast<CharT>(0x80 | (0x3f & cp)));
        }
*/
    }
};

template <class CharT>
struct unicode_traits<CharT,
                      typename std::enable_if<std::is_integral<CharT>::value &&
                      sizeof(CharT)==sizeof(uint16_t)>::type> 
{
    static size_t utf_length(const CharT *source, size_t length)
    {
        const CharT* source_end = source + length;

        size_t count = 0;
        for (const CharT* p = source; p < source_end; ++p)
        {
            uint32_t ch = *p;
            if (ch < (uint32_t)0x80) {      
                ++count;
            } else if (ch < (uint32_t)0x800) {     
                count += 2;
            } else if (ch < (uint32_t)0x10000) {   
                count += 3;
            } else if (ch < (uint32_t)0x110000) {  
                count += 4;
            } else {                            
                count += 3;
            }
        }
        return count;
    }

    static std::pair<const CharT*,size_t> sequence_at(const CharT* it, 
                                                      const CharT* end,
                                                      size_t index)
    {
        const CharT* p = it;
        size_t count = 0;

        while (p < end && count < index)
        {
            uint8_t ch = *p;
            size_t length = (ch >= uni_sur_high_start && ch <= uni_sur_high_end) ? 2 : 1; 
            p += length;
            ++count;
        }

        if (p < end)
        {
            uint8_t ch = *p;
            size_t len = (ch >= uni_sur_high_start && ch <= uni_sur_high_end) ? 2 : 1; 
            return (p+len <= end) ? std::make_pair(p,len) : std::make_pair(it,static_cast<size_t>(0));
        }
        else
        {
            return std::make_pair(it,static_cast<size_t>(0));
        }
    }

    static size_t codepoint_count(const CharT* it, 
                                  const CharT* end)
    {
        size_t count = 0;
        const CharT* p = it;
        while (p < end)
        {
            uint8_t ch = *p;
            size_t length = (ch >= uni_sur_high_start && ch <= uni_sur_high_end) ? 2 : 1; 
            p += length;
            ++count;
        }
        return count;
    }

    template <class UTF32>
    static typename std::enable_if<std::is_integral<UTF32>::value && sizeof(UTF32) == sizeof(uint32_t),uni_errc >::type 
    next_codepoint(const CharT* source_begin, const CharT* source_end, 
                   UTF32* target, const CharT** source_stop,
                   conv_flags flags = conv_flags::strict) 
    {
        uni_errc  result = uni_errc::ok;
        const CharT* source = source_begin;
        uint32_t ch, ch2;

        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= uni_sur_high_start && ch <= uni_sur_high_end) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            if (source < source_end) {
                ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= uni_sur_low_start && ch2 <= uni_sur_low_end) {
                    ch = ((ch - uni_sur_high_start) << half_shift)
                        + (ch2 - uni_sur_low_start) + half_base;
                    ++source;
                } else if (flags == conv_flags::strict) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    result = uni_errc::source_illegal;
                    *source_stop = source;
                    return result;
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                result = uni_errc::unpaired_high_surrogate;
            }
        } else if (flags == conv_flags::strict) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= uni_sur_low_start && ch <= uni_sur_low_end) {
                --source; /* return to the illegal value itself */
                result = uni_errc::source_illegal;
            }
        }
        *target = ch;

        *source_stop = source;
        return result;
    }

    static size_t detect_bom(const CharT* it, size_t length)
    {
        size_t count = 0;
        if (length >= 1)
        {
            uint32_t bom = static_cast<uint32_t>(it[0]);
            if ((bom & 0xFFFF) == 0xFFFE)      
                ++count;
            else if ((bom & 0xFFFF) == 0xFEFF)      
                ++count;
        }
        return count;
    }

    template <class STraits,class SAllocator>
    static void append_codepoint_to_string(uint32_t cp, std::basic_string<CharT,STraits,SAllocator>& s)
    {
        auto result = convert(&cp,&cp+1,std::back_inserter(s));
/*
        if (cp <= 0xFFFF)
        {
            s.push_back(static_cast<CharT>(cp));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<CharT>((cp >> 10) + uni_sur_high_start - (0x10000 >> 10)));
            s.push_back(static_cast<CharT>((cp & 0x3ff) + uni_sur_low_start));
        }
*/
    }
};

template <class CharT>
struct unicode_traits<CharT,
                        typename std::enable_if<std::is_integral<CharT>::value &&
                        sizeof(CharT)==sizeof(uint32_t)>::type> 
{
    static size_t utf_length(const CharT *source, size_t length)
    {
        const CharT* source_end = source + length;

        size_t count = 0;
        for (const CharT* p = source; p < source_end; ++p)
        {
            uint32_t ch = *p;
            if (ch < (uint32_t)0x80) {      
                ++count;
            } else if (ch < (uint32_t)0x800) {     
                count += 2;
            } else if (ch < (uint32_t)0x10000) {   
                count += 3;
            } else if (ch <= uni_max_legal_utf32) {  
                count += 4;
            } else {                            
                count += 3;
            }
        }
        return count;
    }

    static std::pair<const CharT*,size_t> sequence_at(const CharT* it, 
                                                      const CharT* end,
                                                      size_t index)
    {
        const CharT* p = it;
        size_t count = 0;

        while (p < end && count < index)
        {
            ++p;
            ++count;
        }

        return (p < end) ? std::make_pair(p,1) : std::make_pair(it,static_cast<size_t>(0));
    }

    static size_t codepoint_count(const CharT* it, 
                                  const CharT* end)
    {
        return (size_t)(end-it);
    }

    template <class UTF32>
    static typename std::enable_if<std::is_integral<UTF32>::value && sizeof(UTF32) == sizeof(uint32_t),uni_errc >::type 
    next_codepoint(const CharT* source_begin, const CharT*, 
                   UTF32* target, const CharT** source_stop,
                   conv_flags = conv_flags::strict) 
    {
        const CharT* source = source_begin;
        *target = *source++;
        *source_stop = source;

        return uni_errc::ok;
    }

    static size_t detect_bom(const CharT* it, size_t length)
    {
        size_t count = 0;
        if (length >= 1)
        {
            uint32_t bom = static_cast<uint32_t>(it[0]);
            if (bom == 0xFFFE0000)                  
                ++count;
            else if (bom == 0x0000FEFF)             
                ++count;
        }
        return count;
    }

    template <class STraits,class SAllocator>
    static void append_codepoint_to_string(uint32_t cp, std::basic_string<CharT,STraits,SAllocator>& s)
    {
        auto result = convert(&cp,&cp+1,std::back_inserter(s));
/*
        if (cp <= 0xFFFF)
        {
            s.push_back(static_cast<CharT>(cp));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<CharT>(cp));
        }
*/
    }
};

}

namespace std {
    template<>
    struct is_error_code_enum<unicons::uni_errc> : public true_type
    {
    };
}

#endif

