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

	Table::Table( Table&& other )
		: key( other.key )
	{
		other.key = std::string_view();
		value.swap( other.value );
	}

	Table& Table::operator=( Table&& other )
	{
		key = other.key;
		value.swap( other.value );

		other.key = std::string_view();

		return *this;
	}

	Deserializer::Deserializer( const std::filesystem::path& path, ValueCallback value_callback, TableCallback table_callback, void* user )
		: value_callback_( value_callback )
		, table_callback_( table_callback )
		, user_          ( user )
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
			if( value_callback_ )
				value_callback_( unindented_line, user_ );
		}
		else if( ( colon_index + 1 ) < unindented_line.size() )
		{
			/*
			Key:Value
			*/
			Table table;
			table.key = unindented_line.substr( 0, colon_index );
			table.value.emplace< Value >( unindented_line.substr( colon_index + 1 ) );

			if( table_callback_ )
				table_callback_( std::move( table ), user_ );
		}
		else
		{
			/*
			Table:
				Item1:Foo1
				Item2:Bar1

			or

			Table:
				Item1
				Item2
			*/
			ValueCallback old_value_callback = value_callback_;
			TableCallback old_table_callback = table_callback_;
			void*         old_user           = user_;
			Table         table;

			table.key       = unindented_line.substr( 0, colon_index );
			value_callback_ = AddValueToTableCallback;
			table_callback_ = AddTableToTableCallback;
			user_           = &table;

			while( !unparsed_.empty() && ParseLine( unparsed_.substr( 0, unparsed_.find( '\n' ) ), indent_level + 1 ) );
			
			value_callback_ = old_value_callback;
			table_callback_ = old_table_callback;
			user_           = old_user;

			if( table_callback_ )
				table_callback_( std::move( table ), user_ );
		}

		return true;
	}

	void Deserializer::AddValueToTableCallback( Value value, void* user )
	{
		Table* parent_table     = ( Table* )user;
		Array& underlying_array = parent_table->value.index() == 0 ? parent_table->value.emplace< Array >() : std::get< Array >( parent_table->value );

		underlying_array.push_back( value );
	}

	void Deserializer::AddTableToTableCallback( Table table, void* user )
	{
		Table*       parent_table            = ( Table* )user;
		TableVector& underlying_table_vector = parent_table->value.index() == 0 ? parent_table->value.emplace< TableVector >() : std::get< TableVector >( parent_table->value );

		underlying_table_vector.emplace_back( std::move( table ) );
	}
}
