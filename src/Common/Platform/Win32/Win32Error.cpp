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

#if defined( _WIN32 )

#include "Common/Platform/Win32/Win32Error.h"

#include <iostream>

#include <comdef.h>

//////////////////////////////////////////////////////////////////////////

bool _Win32HandleResult( DWORD Result, std::string_view Function, std::string_view File, int Line )
{
	if( FAILED( Result ) )
	{
		constexpr DWORD        BUFFER_SIZE  = 256;
		const std::string_view FunctionName = Function.substr( 0, Function.find_first_of( "(", 0 ) );
		char                   Buffer[ BUFFER_SIZE ];

		if( FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, Result, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), Buffer, BUFFER_SIZE, NULL ) == 0 )
			strcpy_s( Buffer, BUFFER_SIZE, "Unknown error" );

		std::cerr << File << "(L" << Line << ") " << FunctionName << " failed: " << Buffer << "\n";

		return false;
	}

	return true;

} // _Win32HandleResult

//////////////////////////////////////////////////////////////////////////

bool _Win32HandleResult( HRESULT result, std::string_view Function, std::string_view file, int line )
{
	if( FAILED( result ) )
	{
		_Win32HandleResult( ( DWORD )result, Function, file, line );
		return false;
	}

	return true;

} // _Win32HandleResult

//////////////////////////////////////////////////////////////////////////

bool _Win32HandleResult( BOOL result, std::string_view Function, std::string_view file, int line )
{
	if( result == FALSE )
	{
		_Win32HandleResult( GetLastError(), Function, file, line );
		return false;
	}

	return true;

} // _Win32HandleResult

#endif // _WIN32
