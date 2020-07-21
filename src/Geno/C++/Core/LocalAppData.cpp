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

#include "LocalAppData.h"

#include <array>
#include <cstdlib>
#include <cstring>

#if defined( _WIN32 )
#include <ShlObj.h>
#elif defined( __linux__ ) // _WIN32
#include <sys/stat.h>
#endif // __linux__

LocalAppData::LocalAppData( void )
{
#if defined( _WIN32 )

	wchar_t buf[ FILENAME_MAX + 1 ] = { };

	PWSTR local_app_data_folder_path;
	if( SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, nullptr, &local_app_data_folder_path ) != S_OK )
		return;

	if( wcscat_s( buf, std::size( buf ), local_app_data_folder_path ) != 0 )
		return;

	if( wcscat_s( buf, std::size( buf ), L"\\Geno" ) != 0 )
		return;

	if( !CreateDirectoryW( buf, nullptr ) && GetLastError() != ERROR_ALREADY_EXISTS )
		return;

	path_.assign( std::begin( buf ), std::begin( buf ) + wcsnlen( buf, std::size( buf ) ) );
	path_.make_preferred();

#elif defined( __linux__ ) // _WIN32

	char buf[ FILENAME_MAX + 1 ] = { };

	if( const char* data_home = getenv( "XDG_DATA_HOME" ); data_home != nullptr )
	{
		strcpy( buf, data_home );
	}
	else if( const char* data_dirs = getenv( "XDG_DATA_DIRS" ); data_home != nullptr )
	{
		if( const char* colon = strchr( data_dirs, ':' ); colon != nullptr )
		{
			strncpy( buf, data_dirs, colon - data_dirs - 1 );
		}
		else
		{
			strcpy( buf, data_dirs );
		}
	}
	else
	{
		return;
	}

	strcat( buf, "/geno" );

	if( mkdir( buf, 0777 ) != 0 )
		return;

	path_.assign( std::begin( buf ), std::begin( buf ) + strnlen( buf, std::size( buf ) ) );
	path_.make_preferred();

#else // __linux__

#error Implement LocalAppData

#endif // else
}

std::string LocalAppData::operator/( std::string_view relative_path ) const
{
	std::filesystem::path abs_path = ( path_ / relative_path );
	abs_path.make_preferred();

	return abs_path.string();
}

std::wstring LocalAppData::operator/( std::wstring_view relative_path ) const
{
	std::filesystem::path abs_path = ( path_ / relative_path );
	abs_path.make_preferred();

	return abs_path.wstring();
}

LocalAppData& LocalAppData::Instance( void )
{
	static LocalAppData instance;
	return instance;
}
