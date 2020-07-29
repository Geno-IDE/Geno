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

#include "Common/Platform/POSIX/POSIXError.h"

#include <iostream>

#include <string.h>

bool _POSIXHandleErrno( errno_t err, std::string_view function, std::string_view file, int line )
{
	if( errno != 0 )
	{
	#if defined( _WIN32 )

		auto function_name = function.substr( 0, function.find_first_of( "(", 0 ) );
		char buf[ 256 ]    = { };

		strerror_s( buf, std::size( buf ), err );

		std::cerr << file << "(L" << line << ") " << function_name << " failed: " << buf << "\n";

	#else // _WIN32

		std::cerr << file << "(L" << line << ") " << function_name << " failed: " << strerror( err ) << "\n";

	#endif // else

		return false;
	}

	return true;
}
