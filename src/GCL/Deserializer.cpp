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

#include "GCL/Deserializer.h"

#include "Common/Platform/POSIX/POSIXError.h"

#include <iostream>

#include <fcntl.h>
#include <io.h>
#include <string.h>

namespace GCL
{
	Deserializer::Deserializer( const std::filesystem::path& path, ValueCallback value_callback, void* user )
	{
		if( !std::filesystem::exists( path ) )
		{
			std::cerr << "GCL::Serializer failed: '" << path << "' does not exist.\n";
			return;
		}

		if( int fd; POSIX_CALL( _wsopen_s( &fd, path.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYNO, 0 ) ) )
		{
			long file_size = _lseek( fd, 0, SEEK_END );
			_lseek( fd, 0, SEEK_SET );

			char* buf = ( char* )malloc( file_size );
			for( int bytes_read = 0;
			     bytes_read < file_size;
			     bytes_read += _read( fd, buf, file_size )
			);

			char* end = buf + file_size;
			for( char* kw_begin = buf, *kw_end = buf; kw_begin != end; )
			{
				kw_end = ( char* )memchr( kw_begin, '\n', end - kw_begin );

				const size_t           line_size    = ( kw_end - kw_begin );
				const std::string_view line         = std::string_view( kw_begin, line_size );
				const size_t           colon_offset = line.find( ':' );
				const std::string_view key          = line.substr( 0, colon_offset );
				const std::string_view value        = line.substr( colon_offset + 1 );

				if( value.empty() )
				{
					std::vector< KeyedValues > values;

					for( ;; )
					{
						char* item_begin = kw_end + 1;
						char* item_end   = ( char* )memchr( item_begin, '\n', end - item_begin );

						if( *( item_begin++ ) == '\t' )
						{
							const size_t           item_line_size    = ( item_end - item_begin );
							const std::string_view item_line         = std::string_view( item_begin, item_line_size );
							const size_t           item_colon_offset = item_line.find( ':' );
							const std::string_view item_key          = item_line.substr( 0, item_colon_offset );
							const std::string_view item_value        = item_line.substr( item_colon_offset + 1 );
							const KeyedValues      item_keyed_value  = KeyedValues( item_value );

							values.emplace_back( item_key, &item_keyed_value, &item_keyed_value );

							kw_begin = item_begin;
							kw_end   = item_end;
						}
						else
						{
							value_callback( KeyedValues( key, &values.front(), &values.back() ), user );
							break;
						}
					}
				}
				else
				{
					const KeyedValues keyed_value( value );

					value_callback( KeyedValues( key, &keyed_value, &keyed_value ), user );
				}

				kw_begin = kw_end + ( kw_end != end );
			}

			free( buf );
			_close( fd );
		}
	}
}
