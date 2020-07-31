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
	constexpr std::string_view indent_string = "\t";

	constexpr bool LineStartsWithIndent( std::string_view line, int indent_level )
	{
		for( size_t i = 0; i < ( indent_string.size() * indent_level ); i += indent_string.size() )
		{
			if( line.substr( i, indent_string.size() ) != indent_string )
				return false;
		}

		return true;
	}

	Deserializer::Deserializer( const std::filesystem::path& path, ObjectCallback object_callback, void* user )
		: object_callback_( object_callback )
		, user_           ( user )
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

			unparsed_ = std::string_view( buf, file_size );

			while( !unparsed_.empty() )
			{
				size_t           line_end = unparsed_.find( '\n' );
				std::string_view line     = unparsed_.substr( 0, line_end );

				ParseLine( line, 0 );
			}

			free( buf );
			_close( fd );
		}
	}

	bool Deserializer::ParseLine( std::string_view line, int indent_level )
	{
		if( !LineStartsWithIndent( line, indent_level ) )
			return false;

		unparsed_ = unparsed_.substr( line.size() + ( line.size() < unparsed_.size() ) );

//////////////////////////////////////////////////////////////////////////

		std::string_view unindented_line = line.substr( indent_level * indent_string.size() );
		size_t           colon_index     = unindented_line.find_first_of( ':' );

		if( colon_index == std::string_view::npos )
		{
			Object* parent_object = static_cast< Object* >( user_ );

			parent_object->AddArrayItem( unindented_line );
		}
		else if( ( colon_index + 1 ) < unindented_line.size() )
		{
			Object object( unindented_line.substr( 0, colon_index ) );
			object.SetString( unindented_line.substr( colon_index + 1 ) );

			if( object_callback_ )
				object_callback_( std::move( object ), user_ );
		}
		else
		{
			ObjectCallback old_object_callback = object_callback_;
			void*          old_user            = user_;
			Object         object              = Object( unindented_line.substr( 0, colon_index ) );

			object_callback_ = []( Object child, void* parent ) { static_cast< Object* >( parent )->AddChild( std::move( child ) ); };
			user_            = &object;

			while( !unparsed_.empty() && ParseLine( unparsed_.substr( 0, unparsed_.find( '\n' ) ), indent_level + 1 ) );

			object_callback_ = old_object_callback;
			user_            = old_user;

			if( object_callback_ )
				object_callback_( std::move( object ), user_ );
		}

		return true;
	}
}
