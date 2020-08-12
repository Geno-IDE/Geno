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

#include "GCL/Object.h"

#include <Common/Platform/POSIX/POSIXError.h>

#include <iostream>

#include <fcntl.h>
#include <io.h>
#include <string.h>

namespace GCL
{
	Serializer::Serializer( const std::filesystem::path& path )
	{
		constexpr int open_flags       = O_WRONLY | O_BINARY | O_TRUNC | O_CREAT;
		constexpr int share_flags      = SH_DENYNO;
		constexpr int permission_flags = S_IREAD | S_IWRITE;

	#if defined( _WIN32 )
		POSIX_CALL( _wsopen_s( &file_descriptor_, path.c_str(), open_flags, share_flags, permission_flags ) );
	#else // _WIN32
		POSIX_CALL( file_descriptor_ = open( path.c_str(), ofstream, share_flags, permission_flags ) );
	#endif // else
	}

	Serializer::~Serializer( void )
	{
		if( file_descriptor_ >= 0 )
		{
	#if defined( _WIN32 )
			_close( file_descriptor_ );
	#else // _WIN32
			close( file_descriptor_ );
	#endif // else
		}
	}

	void Serializer::WriteObject( const Object& object, int indent_level )
	{
		std::string_view key = object.Key();

		for( int i = 0; i < indent_level; ++i )
			_write( file_descriptor_, "\t", 1 );

		_write( file_descriptor_, key.data(), static_cast< uint32_t >( key.size() ) );
		_write( file_descriptor_, ":", 1 );

		if( object.IsString() )
		{
			/*
			String:Value
			*/
			Object::StringType string = object.String();

			_write( file_descriptor_, string.data(), static_cast< uint32_t >( string.size() ) );
			_write( file_descriptor_, "\n", 1 );
		}
		else if( object.IsArray() )
		{
			/*
			Array:
				Foo
				Bar
			*/
			const Object::ArrayType& array = object.Array();

			_write( file_descriptor_, "\n", 1 );

			for( std::string_view elem : array )
			{
				for( int i = 0; i < ( indent_level + 1 ); ++i )
					_write( file_descriptor_, "\t", 1 );

				_write( file_descriptor_, elem.data(), static_cast< uint32_t >( elem.size() ) );
				_write( file_descriptor_, "\n", 1 );
			}
		}
		else if( object.IsTable() )
		{
			/*
			Table:
				Key:Value
				Array:
					Foo
					Bar
			*/
			const Object::TableType& table = object.Table();

			_write( file_descriptor_, "\n", 1 );

			for( const Object& child : table )
				WriteObject( child, indent_level + 1 );
		}
	}

	bool Serializer::IsOpen( void ) const
	{
		return ( file_descriptor_ >= 0 );
	}
}
