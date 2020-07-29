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

#include "GCL/Serializer.h"

#include "Common/Platform/POSIX/POSIXError.h"

#include <iostream>

#include <fcntl.h>
#include <io.h>
#include <string.h>

namespace GCL
{
	Serializer::Serializer( const std::filesystem::path& path )
	{
		if( !std::filesystem::exists( path ) )
		{
			std::cerr << "GCL::Serializer failed: '" << path << "' does not exist.\n";
			return;
		}

	#if defined( _WIN32 )

//		if( int fd; POSIX_CALL( _wsopen_s( &fd, path.c_str(), _O_WRONLY | _O_BINARY | _O_TRUNC | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE ) ) )
//		{
//			constexpr std::string_view buf =
//R"(Name: MyWorkspace
//Matrix:
//	Platform: x86, x64
//	Configuration: Debug, Release
//Projects:
//	$(Root)/MyLibrary
//	$(Root)/MyApp
//)";
//			_write( fd, buf.data(), static_cast< uint32_t >( buf.size() ) );
//			_close( fd );
//		}

	#else // _WIN32

	#error Write

	#endif // else
	}
}
