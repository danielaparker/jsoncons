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

namespace jsoncons {

#define A5HASH_VER_STR "5.25" ///< A5HASH source code version string.

/**
 * @def A5HASH_U64_C( x )
 * @brief Macro that defines a numeric value as unsigned 64-bit value.
 *
 * @param x Value.
 */

#if defined( _MSC_VER )
	#include <intrin.h>
#endif // defined( _MSC_VER )

JSONCONS_INLINE_CONSTEXPR std::uint64_t a5hash_val10{0xAAAAAAAAAAAAAAAA}; ///< `10` bit-pairs.
JSONCONS_INLINE_CONSTEXPR std::uint64_t a5hash_val01{0x5555555555555555}; ///< `01` bit-pairs.

using uint8_t = unsigned char; ///< For C++ type aliasing compliance.

/**
 * @{
 * @brief Load unsigned value of the specific bit size from memory.
 *
 * @param p Load address.
 */

JSONCONS_A5HASH_INLINE_F std::uint32_t a5hash_lu32( const uint8_t* const p ) noexcept
{
	std::uint32_t v;
	std::memcpy( &v, p, 4 );

	return( v );
}

JSONCONS_A5HASH_INLINE_F std::uint64_t a5hash_lu64( const uint8_t* const p ) noexcept
{
	std::uint64_t v;
	std::memcpy( &v, p, 8 );

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

JSONCONS_A5HASH_INLINE_F void a5hash_umul128( const std::uint64_t u, const std::uint64_t v,
	std::uint64_t* const rl, std::uint64_t* const rh ) noexcept
{
#if defined( JSONCONS_A5HASH_BMI2 )

	*rl = _mulx_u64( u, v, rh );

#elif defined( _MSC_VER ) && ( defined( _M_ARM64 ) || defined( _M_ARM64EC ) || \
	( defined( __INTEL_COMPILER ) && defined( _M_AMD64 )))

	*rl = u * v;
	*rh = __umulh( u, v );

#elif defined( _MSC_VER ) && ( defined( _M_AMD64 ) || defined( _M_IA64 ))

	*rl = _umul128( u, v, rh );

#elif defined( __SIZEOF_INT128__ ) || \
	( defined( JSONCONS_A5HASH_ICC_GCC ) && defined( __x86_64__ ))

	__uint128_t r = u;
	r *= v;

	*rl = (std::uint64_t) r;
	*rh = (std::uint64_t) ( r >> 64 );

#elif ( defined( __IBMC__ ) || defined( __IBMCPP__ )) && defined( __LP64__ )

	*rl = u * v;
	*rh = __mulhdu( u, v );

#else // defined( __IBMC__ )

	// _umul128() code for 32-bit systems, adapted from Hacker's Delight,
	// Henry S. Warren, Jr.

	*rl = u * v;

	const std::uint32_t u0 = (std::uint32_t) u;
	const std::uint32_t v0 = (std::uint32_t) v;
	const std::uint64_t w0 = (std::uint64_t) u0 * v0;
	const std::uint32_t u1 = (std::uint32_t) ( u >> 32 );
	const std::uint32_t v1 = (std::uint32_t) ( v >> 32 );
	const std::uint64_t t = (std::uint64_t) u1 * v0 + (std::uint32_t) ( w0 >> 32 );
	const std::uint64_t w1 = (std::uint64_t) u0 * v1 + (std::uint32_t) t;

	*rh = (std::uint64_t) u1 * v1 + (std::uint32_t) ( w1 >> 32 ) +
		(std::uint32_t) ( t >> 32 );

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

JSONCONS_A5HASH_INLINE_F std::uint64_t a5hash( const void* const Msg0, std::size_t MsgLen,
	const std::uint64_t UseSeed ) noexcept
{
	const uint8_t* Msg = (const uint8_t*) Msg0;

	std::uint64_t val01 = a5hash_val01;
	std::uint64_t val10 = a5hash_val10;

	// The seeds are initialized to mantissa bits of PI.

	std::uint64_t Seed1 = uint64_t{0x243F6A8885A308D3} ^ MsgLen;
	std::uint64_t Seed2 = uint64_t{0x452821E638D01377} ^ MsgLen;

	a5hash_umul128( Seed2 ^ ( UseSeed & val10 ),
		Seed1 ^ ( UseSeed & val01 ), &Seed1, &Seed2 );

	if( MsgLen > 16 )
	{
		val01 ^= Seed1;
		val10 ^= Seed2;

		do
		{
			a5hash_umul128( (std::uint64_t) a5hash_lu32( Msg ) << 32 ^
				a5hash_lu32( Msg + 4 ) ^ Seed1,
				(std::uint64_t) a5hash_lu32( Msg + 8 ) << 32 ^
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
		std::size_t mo;

		Msg4 = Msg + MsgLen - 4;
		mo = MsgLen >> 3;

		Seed1 ^= (std::uint64_t) a5hash_lu32( Msg ) << 32 | a5hash_lu32( Msg4 );

		Seed2 ^= (std::uint64_t) a5hash_lu32( Msg + mo * 4 ) << 32 |
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
			Seed1 ^= (std::uint64_t) Msg[ 1 ] << 8;

			if( --MsgLen != 0 )
			{
				Seed1 ^= (std::uint64_t) Msg[ 2 ] << 16;
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

JSONCONS_A5HASH_INLINE_F void a5hash_umul64( const std::uint32_t u, const std::uint32_t v,
	std::uint32_t* const rl, std::uint32_t* const rh ) noexcept
{
	const std::uint64_t r = (std::uint64_t) u * v;

	*rl = (std::uint32_t) r;
	*rh = (std::uint32_t) ( r >> 32 );
}

} // namespace jsoncons

#endif // JSONCONS_UTILITY_A5HASH_HPP
