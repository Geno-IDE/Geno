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
#include "Win32Error.h"

#include <iostream>

#include <comdef.h>

bool _Win32HandleResult( DWORD result, std::string_view function, std::string_view file, int line )
{
	if( result != S_OK )
	{
		constexpr DWORD buf_size      = 256;
		auto            function_name = function.substr( 0, function.find_first_of( "(", 0 ) );
		char            buf[ buf_size ];

		if( FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buf, buf_size, NULL ) == 0 )
			strcpy_s( buf, buf_size, "Unknown error" );

		std::cerr << file << "(L" << line << ") " << function_name << " failed: " << buf << "\n";

		return false;
	}

	return true;
}

bool _Win32HandleResult( HRESULT result, std::string_view function, std::string_view file, int line )
{
	if( result != S_OK )
	{
		_Win32HandleResult( ( DWORD )result, function, file, line );
		return false;
	}

	return true;
}

bool _Win32HandleResult( BOOL result, std::string_view function, std::string_view file, int line )
{
	if( result == FALSE )
	{
		_Win32HandleResult( GetLastError(), function, file, line );
		return false;
	}

	return true;
}

#endif // _WIN32
