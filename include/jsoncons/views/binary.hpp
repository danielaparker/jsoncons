#ifndef JSONCONS2_UTILITY_BINARY_HPP
#define JSONCONS2_UTILITY_BINARY_HPP

#include <jsoncons/views/jsoncons_config.hpp>

// Macros used for loop unrolling and other purpose
#define repeat2(x)  { x x }
#define repeat3(x)  { x x x }
#define repeat4(x)  { x x x x }
#define repeat8(x)  { x x x x x x x x }
#define repeat16(x) { x x x x x x x x x x x x x x x x }

#define repeat2_incr(x)   { x(0)  x(1) }
#define repeat4_incr(x)   { x(0)  x(1)  x(2)  x(3) }
#define repeat8_incr(x)   { x(0)  x(1)  x(2)  x(3)  x(4)  x(5)  x(6)  x(7)  }
#define repeat16_incr(x)  { x(0)  x(1)  x(2)  x(3)  x(4)  x(5)  x(6)  x(7)  \
                            x(8)  x(9)  x(10) x(11) x(12) x(13) x(14) x(15) }

#define repeat_in_1_18(x) { x(1)  x(2)  x(3)  x(4)  x(5)  x(6)  x(7)  x(8)  \
                            x(9)  x(10) x(11) x(12) x(13) x(14) x(15) x(16) \
                            x(17) x(18) }

namespace jsoncons { namespace utility {

/* endian */
#if yyjson_has_include(<sys/types.h>)
#    include <sys/types.h> /* POSIX */
#endif
#if yyjson_has_include(<endian.h>)
#    include <endian.h> /* Linux */
#elif yyjson_has_include(<sys/endian.h>)
#    include <sys/endian.h> /* BSD, Android */
#elif yyjson_has_include(<machine/endian.h>)
#    include <machine/endian.h> /* BSD, Darwin */
#endif

#define YYJSON_BIG_ENDIAN       4321
#define YYJSON_LITTLE_ENDIAN    1234

#if defined(BYTE_ORDER) && BYTE_ORDER
#   if defined(BIG_ENDIAN) && (BYTE_ORDER == BIG_ENDIAN)
#       define YYJSON_ENDIAN YYJSON_BIG_ENDIAN
#   elif defined(LITTLE_ENDIAN) && (BYTE_ORDER == LITTLE_ENDIAN)
#       define YYJSON_ENDIAN YYJSON_LITTLE_ENDIAN
#   endif

#elif defined(__BYTE_ORDER) && __BYTE_ORDER
#   if defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)
#       define YYJSON_ENDIAN YYJSON_BIG_ENDIAN
#   elif defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)
#       define YYJSON_ENDIAN YYJSON_LITTLE_ENDIAN
#   endif

#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__
#   if defined(__ORDER_BIG_ENDIAN__) && \
        (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#       define YYJSON_ENDIAN YYJSON_BIG_ENDIAN
#   elif defined(__ORDER_LITTLE_ENDIAN__) && \
        (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#       define YYJSON_ENDIAN YYJSON_LITTLE_ENDIAN
#   endif

#elif (defined(__LITTLE_ENDIAN__) && __LITTLE_ENDIAN__ == 1) || \
    defined(__i386) || defined(__i386__) || \
    defined(_X86_) || defined(__X86__) || \
    defined(_M_IX86) || defined(__THW_INTEL__) || \
    defined(__x86_64) || defined(__x86_64__) || \
    defined(__amd64) || defined(__amd64__) || \
    defined(_M_AMD64) || defined(_M_X64) || \
    defined(_M_ARM) || defined(_M_ARM64) || \
    defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) || \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__) || \
    defined(__EMSCRIPTEN__) || defined(__wasm__) || \
    defined(__loongarch__)
#   define YYJSON_ENDIAN YYJSON_LITTLE_ENDIAN

#elif (defined(__BIG_ENDIAN__) && __BIG_ENDIAN__ == 1) || \
    defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || \
    defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__) || \
    defined(__or1k__) || defined(__OR1K__)
#   define YYJSON_ENDIAN YYJSON_BIG_ENDIAN

#else
#   define YYJSON_ENDIAN 0 /* unknown endian, detect at run-time */
#endif

/*
 This macro controls how yyjson handles unaligned memory accesses.
 
 By default, yyjson uses `memcpy()` for memory copying. This takes advantage of
 the compiler's automatic optimizations to generate unaligned memory access
 instructions when the target architecture supports it.
 
 However, for some older compilers or architectures where `memcpy()` isn't
 optimized well and may generate unnecessary function calls, consider defining
 this macro as 1. In such cases, yyjson switches to manual byte-by-byte access,
 potentially improving performance. An example of the generated assembly code on
 the ARM platform can be found here: https://godbolt.org/z/334jjhxPT
 
 As this flag has already been enabled for some common architectures in the
 following code, users typically don't need to manually specify it. If users are
 unsure about it, please review the generated assembly code or perform actual
 benchmark to make an informed decision.
 */
#ifndef YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS
#   if defined(__ia64) || defined(_IA64) || defined(__IA64__) ||  \
        defined(__ia64__) || defined(_M_IA64) || defined(__itanium__)
#       define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1 /* Itanium */
#   elif (defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && \
        (defined(__GNUC__) || defined(__clang__)) && \
        (!defined(__ARM_FEATURE_UNALIGNED) || !__ARM_FEATURE_UNALIGNED)
#       define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1 /* ARM */
#   elif defined(__sparc) || defined(__sparc__)
#       define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1 /* SPARC */
#   elif defined(__mips) || defined(__mips__) || defined(__MIPS__)
#       define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1 /* MIPS */
#   elif defined(__m68k__) || defined(M68000)
#       define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 1 /* M68K */
#   else
#       define YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS 0
#   endif
#endif

/** 16/32/64-bit vector */
struct v16 { char c[2]; };
struct v32 { char c[4]; };
struct v64 { char c[8]; };

/** 16/32/64-bit vector union */
union v16_uni { v16 v; std::uint16_t u; };
union v32_uni { v32 v; std::uint32_t u; };
union v64_uni { v64 v; std::uint64_t u; };



/*==============================================================================
 * Load/Store Utils
 *============================================================================*/

#if YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS

#define byte_move_idx(x) ((char *)dst)[x] = ((const char *)src)[x];

static inline void byte_copy_2(void *dst, const void *src) {
    repeat2_incr(byte_move_idx)
}

static inline void byte_copy_4(void *dst, const void *src) {
    repeat4_incr(byte_move_idx)
}

static inline void byte_copy_8(void *dst, const void *src) {
    repeat8_incr(byte_move_idx)
}

static inline void byte_copy_16(void *dst, const void *src) {
    repeat16_incr(byte_move_idx)
}

static inline void byte_move_2(void *dst, const void *src) {
    repeat2_incr(byte_move_idx)
}

static inline void byte_move_4(void *dst, const void *src) {
    repeat4_incr(byte_move_idx)
}

static inline void byte_move_8(void *dst, const void *src) {
    repeat8_incr(byte_move_idx)
}

static inline void byte_move_16(void *dst, const void *src) {
    repeat16_incr(byte_move_idx)
}

static inline bool byte_match_2(void *buf, const char *pat) {
    return
    ((char *)buf)[0] == ((const char *)pat)[0] &&
    ((char *)buf)[1] == ((const char *)pat)[1];
}

static inline bool byte_match_4(void *buf, const char *pat) {
    return
    ((char *)buf)[0] == ((const char *)pat)[0] &&
    ((char *)buf)[1] == ((const char *)pat)[1] &&
    ((char *)buf)[2] == ((const char *)pat)[2] &&
    ((char *)buf)[3] == ((const char *)pat)[3];
}

static inline uint16_t byte_load_2(const void *src) {
    v16_uni uni;
    uni.v.c[0] = ((const char *)src)[0];
    uni.v.c[1] = ((const char *)src)[1];
    return uni.u;
}

static inline uint32_t byte_load_3(const void *src) {
    v32_uni uni;
    uni.v.c[0] = ((const char *)src)[0];
    uni.v.c[1] = ((const char *)src)[1];
    uni.v.c[2] = ((const char *)src)[2];
    uni.v.c[3] = 0;
    return uni.u;
}

static inline uint32_t byte_load_4(const void *src) {
    v32_uni uni;
    uni.v.c[0] = ((const char *)src)[0];
    uni.v.c[1] = ((const char *)src)[1];
    uni.v.c[2] = ((const char *)src)[2];
    uni.v.c[3] = ((const char *)src)[3];
    return uni.u;
}

#undef byte_move_expr

#else

static inline void byte_copy_2(void *dst, const void *src) {
    memcpy(dst, src, 2);
}

static inline void byte_copy_4(void *dst, const void *src) {
    memcpy(dst, src, 4);
}

static inline void byte_copy_8(void *dst, const void *src) {
    memcpy(dst, src, 8);
}

static inline void byte_copy_16(void *dst, const void *src) {
    memcpy(dst, src, 16);
}

static inline void byte_move_2(void *dst, const void *src) {
    uint16_t tmp;
    memcpy(&tmp, src, 2);
    memcpy(dst, &tmp, 2);
}

static inline void byte_move_4(void *dst, const void *src) {
    uint32_t tmp;
    memcpy(&tmp, src, 4);
    memcpy(dst, &tmp, 4);
}

static inline void byte_move_8(void *dst, const void *src) {
    uint64_t tmp;
    memcpy(&tmp, src, 8);
    memcpy(dst, &tmp, 8);
}

static inline void byte_move_16(void *dst, const void *src) {
    char *pdst = (char *)dst;
    const char *psrc = (const char *)src;
    uint64_t tmp1, tmp2;
    memcpy(&tmp1, psrc, 8);
    memcpy(&tmp2, psrc + 8, 8);
    memcpy(pdst, &tmp1, 8);
    memcpy(pdst + 8, &tmp2, 8);
}

static inline bool byte_match_2(void *buf, const char *pat) {
    v16_uni u1, u2;
    memcpy(&u1, buf, 2);
    memcpy(&u2, pat, 2);
    return u1.u == u2.u;
}

static inline bool byte_match_4(void *buf, const char *pat) {
    v32_uni u1, u2;
    memcpy(&u1, buf, 4);
    memcpy(&u2, pat, 4);
    return u1.u == u2.u;
}

static inline uint16_t byte_load_2(const void *src) {
    v16_uni uni;
    memcpy(&uni, src, 2);
    return uni.u;
}

static inline uint32_t byte_load_3(const void *src) {
    v32_uni uni;
    memcpy(&uni, src, 2);
    uni.v.c[2] = ((const char *)src)[2];
    uni.v.c[3] = 0;
    return uni.u;
}

static inline uint32_t byte_load_4(const void *src) {
    v32_uni uni;
    memcpy(&uni, src, 4);
    return uni.u;
}

#endif


} // namespace utility
} // namespace jsoncons

#endif // JSONCONS2_UTILITY_BINARY_HPP
