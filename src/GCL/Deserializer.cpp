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
	constexpr bool LineStartsWithIndent( std::string_view line, int indent_level )
	{
		for( size_t i = 0; i < indent_level; ++i )
		{
			if( line[ i ] != '\t' )
				return false;
		}

		return true;
	}

	Deserializer::Deserializer( const std::filesystem::path& path )
	{
		if( !std::filesystem::exists( path ) )
		{
			std::cerr << "GCL::Serializer failed: '" << path << "' does not exist.\n";
			return;
		}

		if( int fd; POSIX_CALL( _wsopen_s( &fd, path.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYNO, 0 ) ) )
		{
			file_size_ = static_cast< size_t >( _lseek( fd, 0, SEEK_END ) );
			_lseek( fd, 0, SEEK_SET );

			file_buf_ = ( char* )malloc( file_size_ );

			for( int bytes_read = 0;
			     bytes_read < file_size_;
			     bytes_read += _read( fd, file_buf_, static_cast< uint32_t >( file_size_ ) )
			);

			_close( fd );
		}
	}

	Deserializer::~Deserializer( void )
	{
		free( file_buf_ );
	}

	void Deserializer::Objects( ObjectCallback callback, void* user )
	{
		std::string_view unparsed( file_buf_, file_size_ );

		while( !unparsed.empty() )
		{
			size_t           line_end = unparsed.find( '\n' );
			std::string_view line     = unparsed.substr( 0, line_end );

			ParseLine( line, 0, &unparsed, callback, user );
		}
	}

	bool Deserializer::IsOpen( void ) const
	{
		return ( ( file_buf_ != nullptr ) && ( file_size_ > 0 ) );
	}

	bool Deserializer::ParseLine( std::string_view line, int indent_level, std::string_view* unparsed, ObjectCallback callback, void* user )
	{
		if( !LineStartsWithIndent( line, indent_level ) )
			return false;

		*unparsed = unparsed->substr( line.size() + ( line.size() < unparsed->size() ) );

//////////////////////////////////////////////////////////////////////////

		std::string_view unindented_line = line.substr( indent_level );
		size_t           colon_index     = unindented_line.find_first_of( ':' );

		if( colon_index == std::string_view::npos )
		{
			Object* parent_object = static_cast< Object* >( user );

			parent_object->AddArrayItem( unindented_line );
		}
		else if( ( colon_index + 1 ) < unindented_line.size() )
		{
			Object object( unindented_line.substr( 0, colon_index ) );
			object.SetString( unindented_line.substr( colon_index + 1 ) );

			callback( std::move( object ), user );
		}
		else
		{
			Object object             = Object( unindented_line.substr( 0, colon_index ) );
			auto   add_child_callback = []( Object child, void* parent ) { static_cast< Object* >( parent )->AddChild( std::move( child ) ); };

			while( !unparsed->empty() && ParseLine( unparsed->substr( 0, unparsed->find( '\n' ) ), indent_level + 1, unparsed, add_child_callback, &object ) );

			callback( std::move( object ), user );
		}

		return true;
	}
}
