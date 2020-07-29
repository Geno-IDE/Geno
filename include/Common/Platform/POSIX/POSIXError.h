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
#include <optional>
#include <string_view>

extern bool _POSIXHandleErrno( errno_t err, std::string_view function, std::string_view file, int line );

inline bool _POSIXHandleResult( errno_t result, std::string_view function, std::string_view file, int line )
{
	return _POSIXHandleErrno( result, function, file, line );
}

template< typename T >
inline std::optional< T > _POSIXHandleResultPassthrough( T result, std::string_view function, std::string_view file, int line )
{
	if( _POSIXHandleErrno( errno, function, file, line ) )
		return std::make_optional< T >( result );
	else
		return std::nullopt;
}

#define POSIX_CALL( X )             ( ::_POSIXHandleResult( ( X ), #X, __FILE__, __LINE__ ) )
#define POSIX_CALL_PASSTHROUGH( X ) ( errno = 0, ::_POSIXHandleResultPassthrough( ( X ), #X, __FILE__, __LINE__ ) )
