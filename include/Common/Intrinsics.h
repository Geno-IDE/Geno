/*
 * Copyright (c) 2020 Sebastian Kylander https://gaztin.com/
 *
 * This software is provided 'as-is', without any express or implied warranty. In no event will
 * the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the
 *    original software. If you use this software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as
 *    being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#pragma once
#include <string_view>

namespace Intrinsics
{
	inline void DebugBreak( void )
	{
	#if defined( _MSC_VER )
		__debugbreak();
	#elif defined( __clang__ ) // _MSC_VER
		__builtin_debugtrap();
	#endif // __clang__
	}

	constexpr std::string_view TargetMachine( void )
	{
	#if defined( _M_IX86 ) || defined( __i386__ )
		return "x86";
	#elif defined( _M_AMD64 ) || defined( __amd64__ ) // _M_IX86 || __i386__
		return "AMD64";
	#elif defined( _M_ARM ) || defined( __arm__ ) // _M_AMD64 || __amd64__
		return "ARM";
	#elif defined( _M_ARM64  ) || defined( __aarch64__ ) // _M_ARM || __arm__
		return "ARM64";
	#endif // _M_ARM64 || __aarch64__
	}
}
