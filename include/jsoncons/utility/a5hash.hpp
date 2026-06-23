/**
 * @file a5hash.h
 *
 * @version 5.25
 *
 * @brief The header file for the "a5hash" 64-bit hash function, "a5hash32"
 * 32-bit hash function, "a5hash128" 128-bit hash function, and "a5rand"
 * 64-bit PRNG.
 *
 * The source code is written in ISO C99, with full C++ compliance enabled
 * conditionally and automatically when compiled with a C++ compiler.
 *
 * Description is available at https://github.com/avaneev/a5hash
 *
 * Email: aleksey.vaneev@gmail.com or info@voxengo.com
 *
 * LICENSE:
 *
 * Copyright (c) 2025 Aleksey Vaneev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef JSONCONS_UTILITY_A5HASH_HPP
#define JSONCONS_UTILITY_A5HASH_HPP

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <cstring>
#include <cstdint>

#define A5HASH_VER_STR "5.25" ///< A5HASH source code version string.

/**
 * @def A5HASH_NS_CUSTOM
 * @brief If this macro is defined externally, all symbols will be placed into
 * the C++ namespace specified by this macro and will not be exported to the
 * global namespace. WARNING: if the macro's value is empty, the symbols will
 * be placed into the global namespace anyway.
 */

/**
 * @def A5HASH_U64_C( x )
 * @brief Macro that defines a numeric value as unsigned 64-bit value.
 *
 * @param x Value.
 */

/**
 * @def A5HASH_NOEX
 * @brief Macro that defines the "noexcept" function specifier for C++
 * environment.
 */

/**
 * @def A5HASH_NULL
 * @brief Macro that defines "nullptr" value, for C++ guidelines compliance.
 */

/**
 * @def A5HASH_NS
 * @brief Macro that defines the actual implementation namespace in C++
 * environment, with export of relevant symbols to the global namespace
 * (if @ref A5HASH_NS_CUSTOM is undefined).
 */


#define A5HASH_U64_C( x ) UINT64_C( x )
#define A5HASH_NOEX noexcept
#define A5HASH_NULL nullptr

#if defined( _MSC_VER )
	#include <intrin.h>
#endif // defined( _MSC_VER )

#define A5HASH_VAL10 A5HASH_U64_C( 0xAAAAAAAAAAAAAAAA ) ///< `10` bit-pairs.
#define A5HASH_VAL01 A5HASH_U64_C( 0x5555555555555555 ) ///< `01` bit-pairs.

/**
 * @def A5HASH_ICC_GCC
 * @brief Macro that denotes the use of ICC classic compiler with GCC-style
 * built-in functions.
 */

#if defined( __INTEL_COMPILER ) && __INTEL_COMPILER >= 1300 && \
	!defined( _MSC_VER )

	#define A5HASH_ICC_GCC

#endif // ICC check

/**
 * @def A5HASH_GCC_BUILTINS
 * @brief Macro that denotes availability of GCC-style built-in functions.
 */

#if defined( __GNUC__ ) || defined( __clang__ ) || \
	defined( __IBMC__ ) || defined( __IBMCPP__ ) || defined( A5HASH_ICC_GCC )

	#define A5HASH_GCC_BUILTINS

#endif // GCC built-ins check

/**
 * @def A5HASH_BMI2
 * @brief Macro that denotes availability of the `mulx` intrinsic
 * (MSVC-compatible compilers only).
 */

#if defined( _MSC_VER )
	#if defined( __BMI2__ ) || ( !defined( A5HASH_GCC_BUILTINS ) && \
		defined( _M_AMD64 ) && defined( __AVX2__ ) && \
		( defined( __INTEL_COMPILER ) || _MSC_VER >= 1900 ))

		#include <immintrin.h>
		#define A5HASH_BMI2

	#else // BMI2

		#include <intrin.h>

	#endif // BMI2
#endif // defined( _MSC_VER )

/**
 * @def A5HASH_STATIC
 * @brief Macro that defines a function as "static".
 */

#if ( defined( __cplusplus ) && __cplusplus >= 201703L ) || \
	( defined( __STDC_VERSION__ ) && __STDC_VERSION__ >= 202311L )

	#define A5HASH_STATIC [[maybe_unused]] static

#elif defined( A5HASH_GCC_BUILTINS )

	#define A5HASH_STATIC static __attribute__((unused))

#else // defined( A5HASH_GCC_BUILTINS )

	#define A5HASH_STATIC static

#endif // defined( A5HASH_GCC_BUILTINS )

/**
 * @def A5HASH_INLINE
 * @brief Macro that defines a function as inlinable at the compiler's
 * discretion.
 */

#define A5HASH_INLINE A5HASH_STATIC inline

/**
 * @def A5HASH_INLINE_F
 * @brief Macro to force code inlining.
 */

#if defined( __LP64__ ) || defined( _LP64 ) || \
	!( SIZE_MAX <= 0xFFFFFFFFU ) || ( defined( UINTPTR_MAX ) && \
	!( UINTPTR_MAX <= 0xFFFFFFFFU )) || defined( __x86_64__ ) || \
	defined( __aarch64__ ) || defined( _M_AMD64 ) || defined( _M_ARM64 )

	#if defined( A5HASH_GCC_BUILTINS )

		#define A5HASH_INLINE_F A5HASH_INLINE __attribute__((always_inline))

	#elif defined( _MSC_VER )

		#define A5HASH_INLINE_F A5HASH_STATIC __forceinline

	#endif // defined( _MSC_VER )

#endif // 64-bit platform check

#if !defined( A5HASH_INLINE_F )
	#define A5HASH_INLINE_F A5HASH_INLINE
#endif // !defined( A5HASH_INLINE_F )

using std :: memcpy;
using std :: size_t;

using std :: uint32_t;
using std :: uint64_t;
using uint8_t = unsigned char; ///< For C++ type aliasing compliance.

/**
 * @{
 * @brief Load unsigned value of the specific bit size from memory.
 *
 * @param p Load address.
 */

A5HASH_INLINE_F uint32_t a5hash_lu32( const uint8_t* const p ) A5HASH_NOEX
{
	uint32_t v;
	memcpy( &v, p, 4 );

	return( v );
}

A5HASH_INLINE_F uint64_t a5hash_lu64( const uint8_t* const p ) A5HASH_NOEX
{
	uint64_t v;
	memcpy( &v, p, 8 );

	return( v );
}

/** @} */

/**
 * @brief 64-bit by 64-bit unsigned multiplication producing a 128-bit result.
 *
 * @param u Multiplier 1.
 * @param v Multiplier 2.
 * @param[out] rl The lower half of the 128-bit result.
 * @param[out] rh The upper half of the 128-bit result.
 */

A5HASH_INLINE_F void a5hash_umul128( const uint64_t u, const uint64_t v,
	uint64_t* const rl, uint64_t* const rh ) A5HASH_NOEX
{
#if defined( A5HASH_BMI2 )

	*rl = _mulx_u64( u, v, rh );

#elif defined( _MSC_VER ) && ( defined( _M_ARM64 ) || defined( _M_ARM64EC ) || \
	( defined( __INTEL_COMPILER ) && defined( _M_AMD64 )))

	*rl = u * v;
	*rh = __umulh( u, v );

#elif defined( _MSC_VER ) && ( defined( _M_AMD64 ) || defined( _M_IA64 ))

	*rl = _umul128( u, v, rh );

#elif defined( __SIZEOF_INT128__ ) || \
	( defined( A5HASH_ICC_GCC ) && defined( __x86_64__ ))

	__uint128_t r = u;
	r *= v;

	*rl = (uint64_t) r;
	*rh = (uint64_t) ( r >> 64 );

#elif ( defined( __IBMC__ ) || defined( __IBMCPP__ )) && defined( __LP64__ )

	*rl = u * v;
	*rh = __mulhdu( u, v );

#else // defined( __IBMC__ )

	// _umul128() code for 32-bit systems, adapted from Hacker's Delight,
	// Henry S. Warren, Jr.

	*rl = u * v;

	const uint32_t u0 = (uint32_t) u;
	const uint32_t v0 = (uint32_t) v;
	const uint64_t w0 = (uint64_t) u0 * v0;
	const uint32_t u1 = (uint32_t) ( u >> 32 );
	const uint32_t v1 = (uint32_t) ( v >> 32 );
	const uint64_t t = (uint64_t) u1 * v0 + (uint32_t) ( w0 >> 32 );
	const uint64_t w1 = (uint64_t) u0 * v1 + (uint32_t) t;

	*rh = (uint64_t) u1 * v1 + (uint32_t) ( w1 >> 32 ) +
		(uint32_t) ( t >> 32 );

#endif // defined( __IBMC__ )
}

/**
 * @brief A5HASH 64-bit hash function.
 *
 * Produces and returns a 64-bit hash value (digest) of the specified message,
 * string, or binary data block. Designed for string/small key data hash-map
 * and hash-table uses.
 *
 * @param Msg0 The message to produce a hash from. The alignment of this
 * pointer is unimportant. It is valid to pass 0 when `MsgLen` equals 0.
 * @param MsgLen Message length, in bytes, can be zero.
 * @param UseSeed An optional value to use instead of the default seed (0).
 * This value can have any number of significant bits and any statistical
 * quality.
 * @return 64-bit hash of the input data.
 */

A5HASH_INLINE_F uint64_t a5hash( const void* const Msg0, size_t MsgLen,
	const uint64_t UseSeed ) A5HASH_NOEX
{
	const uint8_t* Msg = (const uint8_t*) Msg0;

	uint64_t val01 = A5HASH_VAL01;
	uint64_t val10 = A5HASH_VAL10;

	// The seeds are initialized to mantissa bits of PI.

	uint64_t Seed1 = A5HASH_U64_C( 0x243F6A8885A308D3 ) ^ MsgLen;
	uint64_t Seed2 = A5HASH_U64_C( 0x452821E638D01377 ) ^ MsgLen;

	a5hash_umul128( Seed2 ^ ( UseSeed & val10 ),
		Seed1 ^ ( UseSeed & val01 ), &Seed1, &Seed2 );

	if( MsgLen > 16 )
	{
		val01 ^= Seed1;
		val10 ^= Seed2;

		do
		{
			a5hash_umul128( (uint64_t) a5hash_lu32( Msg ) << 32 ^
				a5hash_lu32( Msg + 4 ) ^ Seed1,
				(uint64_t) a5hash_lu32( Msg + 8 ) << 32 ^
				a5hash_lu32( Msg + 12 ) ^ Seed2, &Seed1, &Seed2 );

			MsgLen -= 16;
			Msg += 16;

			Seed1 += val01;
			Seed2 += val10;

		} while( MsgLen > 16 );
	}

	if( MsgLen == 0 )
	{
		goto _fin;
	}

	if( MsgLen > 3 )
	{
		const uint8_t* Msg4;
		size_t mo;

		Msg4 = Msg + MsgLen - 4;
		mo = MsgLen >> 3;

		Seed1 ^= (uint64_t) a5hash_lu32( Msg ) << 32 | a5hash_lu32( Msg4 );

		Seed2 ^= (uint64_t) a5hash_lu32( Msg + mo * 4 ) << 32 |
			a5hash_lu32( Msg4 - mo * 4 );

	_fin:
		a5hash_umul128( Seed1, Seed2, &Seed1, &Seed2 );

		a5hash_umul128( val01 ^ Seed1, Seed2, &Seed1, &Seed2 );

		return( Seed1 ^ Seed2 );
	}
	else
	{
		Seed1 ^= Msg[ 0 ];

		if( --MsgLen != 0 )
		{
			Seed1 ^= (uint64_t) Msg[ 1 ] << 8;

			if( --MsgLen != 0 )
			{
				Seed1 ^= (uint64_t) Msg[ 2 ] << 16;
			}
		}

		goto _fin;
	}
}

/**
 * @brief 32-bit by 32-bit unsigned multiplication producing a 64-bit result.
 *
 * @param u Multiplier 1.
 * @param v Multiplier 2.
 * @param[out] rl The lower half of the 64-bit result.
 * @param[out] rh The upper half of the 64-bit result.
 */

A5HASH_INLINE_F void a5hash_umul64( const uint32_t u, const uint32_t v,
	uint32_t* const rl, uint32_t* const rh ) A5HASH_NOEX
{
	const uint64_t r = (uint64_t) u * v;

	*rl = (uint32_t) r;
	*rh = (uint32_t) ( r >> 32 );
}

#undef A5HASH_NS_CUSTOM
#undef A5HASH_U64_C
#undef A5HASH_NOEX
#undef A5HASH_NULL
#undef A5HASH_VAL10
#undef A5HASH_VAL01
#undef A5HASH_ICC_GCC
#undef A5HASH_GCC_BUILTINS
#undef A5HASH_BMI2
#undef A5HASH_STATIC
#undef A5HASH_INLINE
#undef A5HASH_INLINE_F

#endif // JSONCONS_UTILITY_A5HASH_HPP
