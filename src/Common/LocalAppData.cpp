/*
 * Copyright (c) 2021 Sebastian Kylander https://gaztin.com/
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

#include "Common/LocalAppData.h"

#include <array>
#include <cstdlib>
#include <cstring>

#if defined( _WIN32 )
#include <ShlObj.h>
#elif defined( __linux__ ) // _WIN32
#include <sys/stat.h>
#endif // __linux__

//////////////////////////////////////////////////////////////////////////

LocalAppData::LocalAppData( void )
{

#if defined( _WIN32 )

	wchar_t Buffer[ FILENAME_MAX + 1 ] = { };

	PWSTR LocalAppDataFolderPath;
	if( SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, nullptr, &LocalAppDataFolderPath ) != S_OK )
		return;

	if( wcscat_s( Buffer, std::size( Buffer ), LocalAppDataFolderPath ) != 0 )
		return;

	if( wcscat_s( Buffer, std::size( Buffer ), L"\\Geno" ) != 0 )
		return;

	if( !CreateDirectoryW( Buffer, nullptr ) && GetLastError() != ERROR_ALREADY_EXISTS )
		return;

	m_Path.assign( std::begin( Buffer ), std::begin( Buffer ) + wcsnlen( Buffer, std::size( Buffer ) ) );
	m_Path = m_Path.lexically_normal();

#elif defined( __linux__ ) // _WIN32

	char Buffer[ FILENAME_MAX + 1 ] = { };

	if( const char* pDataHome = getenv( "XDG_DATA_HOME" ); pDataHome != nullptr )
	{
		strcpy( Buffer, pDataHome );
	}
	else if( const char* pDataDirs = getenv( "XDG_DATA_DIRS" ); pDataHome != nullptr )
	{
		if( const char* pColon = strchr( pDataDirs, ':' ); pColon != nullptr )
		{
			strncpy( Buffer, pDataDirs, pColon - pDataDirs - 1 );
		}
		else
		{
			strcpy( Buffer, pDataDirs );
		}
	}
	else
	{
		return;
	}

	strcat( Buffer, "/geno" );

	if( mkdir( Buffer, 0777 ) != 0 )
		return;

	m_Path.assign( std::begin( Buffer ), std::begin( Buffer ) + strnlen( Buffer, std::size( Buffer ) ) );
	m_Path = m_Path.lexically_normal();

#endif // __linux__

} // LocalAppData

//////////////////////////////////////////////////////////////////////////

std::filesystem::path LocalAppData::operator/( PathStringView RelativePath ) const
{
	return ( m_Path / RelativePath ).lexically_normal();

} // operator/
