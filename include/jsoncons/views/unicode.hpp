#ifndef JSONCONS_UTILITY_UNICODE_HPP
#define JSONCONS_UTILITY_UNICODE_HPP

#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { namespace utility {

    /*
     Each unicode code point is encoded as 1 to 4 bytes in UTF-8 encoding,
     we use 4-byte mask and pattern value to validate UTF-8 byte sequence,
     this requires the input data to have 4-byte zero padding.
     ---------------------------------------------------
     1 byte
     unicode range [U+0000, U+007F]
     unicode min   [.......0]
     unicode max   [.1111111]
     bit pattern   [0.......]
     ---------------------------------------------------
     2 byte
     unicode range [U+0080, U+07FF]
     unicode min   [......10 ..000000]
     unicode max   [...11111 ..111111]
     bit require   [...xxxx. ........] (1E 00)
     bit mask      [xxx..... xx......] (E0 C0)
     bit pattern   [110..... 10......] (C0 80)
     ---------------------------------------------------
     3 byte
     unicode range [U+0800, U+FFFF]
     unicode min   [........ ..100000 ..000000]
     unicode max   [....1111 ..111111 ..111111]
     bit require   [....xxxx ..x..... ........] (0F 20 00)
     bit mask      [xxxx.... xx...... xx......] (F0 C0 C0)
     bit pattern   [1110.... 10...... 10......] (E0 80 80)
     ---------------------------------------------------
     3 byte invalid (reserved for surrogate halves)
     unicode range [U+D800, U+DFFF]
     unicode min   [....1101 ..100000 ..000000]
     unicode max   [....1101 ..111111 ..111111]
     bit mask      [....xxxx ..x..... ........] (0F 20 00)
     bit pattern   [....1101 ..1..... ........] (0D 20 00)
     ---------------------------------------------------
     4 byte
     unicode range [U+10000, U+10FFFF]
     unicode min   [........ ...10000 ..000000 ..000000]
     unicode max   [.....100 ..001111 ..111111 ..111111]
     bit require   [.....xxx ..xx.... ........ ........] (07 30 00 00)
     bit mask      [xxxxx... xx...... xx...... xx......] (F8 C0 C0 C0)
     bit pattern   [11110... 10...... 10...... 10......] (F0 80 80 80)
     ---------------------------------------------------
     */
    
    // Extract mask from little endian encoded integer
    inline constexpr uint32_t b1_mask = (0x00 <<0) | (0x00<<8) | (0x00<<16) | ((uint32_t)0x80<<24);
    inline constexpr uint32_t b1_patt = (0x00 <<0) | (0x00<<8) | (0x00<<16) | ((uint32_t)0x00<<24);
    inline constexpr uint32_t b2_mask = (0x00 <<0) | (0x00<<8) | (0xC0<<16) | ((uint32_t)0xE0<<24);
    inline constexpr uint32_t b2_patt = (0x00 <<0) | (0x00<<8) | (0x80<<16) | ((uint32_t)0xC0<<24);
    inline constexpr uint32_t b2_requ = (0x00 <<0) | (0x00<<8) | (0x00<<16) | ((uint32_t)0x1E<<24);
    inline constexpr uint32_t b3_mask = (0x00 <<0) | (0xC0<<8) | (0xC0<<16) | ((uint32_t)0xF0<<24);
    inline constexpr uint32_t b3_patt = (0x00 <<0) | (0x80<<8) | (0x80<<16) | ((uint32_t)0xE0<<24);
    inline constexpr uint32_t b3_requ = (0x00 <<0) | (0x00<<8) | (0x20<<16) | ((uint32_t)0x0F<<24);
    inline constexpr uint32_t b3_erro = (0x00 <<0) | (0x00<<8) | (0x20<<16) | ((uint32_t)0x0D<<24);
    inline constexpr uint32_t b4_mask = (0x00 <<0) | (0xC0<<8) | (0xC0<<16) | ((uint32_t)0xF8<<24);
    inline constexpr uint32_t b4_patt = (0x00 <<0) | (0x80<<8) | (0x80<<16) | ((uint32_t)0xF0<<24);
    inline constexpr uint32_t b4_requ = (0x00 <<0) | (0x00<<8) | (0x30<<16) | ((uint32_t)0x07<<24);
    inline constexpr uint32_t b4_err0 = (0x00 <<0) | (0x00<<8) | (0x00<<16) | ((uint32_t)0x04<<24);
    inline constexpr uint32_t b4_err1 = (0x00 <<0) | (0x00<<8) | (0x30<<16) | ((uint32_t)0x03<<24);
    
JSONCONS_FORCE_INLINE constexpr bool is_valid_seq_1(std::uint32_t uni) 
{
    return ((uni & b1_mask) == b1_patt);
}

JSONCONS_FORCE_INLINE constexpr bool is_valid_seq_2(std::uint32_t uni) 
{
    return ((uni & b2_mask) == b2_patt) && ((uni & b2_requ));
}
    
JSONCONS_FORCE_INLINE constexpr bool is_valid_seq_3(std::uint32_t uni) 
{
    uint32_t tmp;
    return ((uni & b3_mask) == b3_patt) && ((tmp = (uni & b3_requ))) && ((tmp != b3_erro));
}
    
JSONCONS_FORCE_INLINE constexpr bool is_valid_seq_4(std::uint32_t uni) 
{
    uint32_t tmp;
    return ((uni & b4_mask) == b4_patt) && ((tmp = (uni & b4_requ))) && ((tmp & b4_err0) == 0 || (tmp & b4_err1) == 0);
}

} // namespace utility
} // namespace jsoncons

#endif // JSONCONS_JSON_ELEMENT_HPP
