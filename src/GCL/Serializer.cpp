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

#include "GCL/Serializer.h"

#include "GCL/Object.h"

#include "Common/Platform/POSIX/POSIXError.h"

#include <iostream>

#include <sys/stat.h>
#include <fcntl.h>

#if defined( _WIN32 )
#include <corecrt_io.h>
#define open  _wopen
#elif defined( __unix__ ) || defined( __APPLE__ ) // _WIN32
#include <unistd.h>
#define O_BINARY 0
#endif // __unix__ || __APPLE__

//////////////////////////////////////////////////////////////////////////

GCL::Serializer::Serializer( const std::filesystem::path& rPath )
{
	m_FileDescriptor = open( rPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE );

} // Serializer

//////////////////////////////////////////////////////////////////////////

GCL::Serializer::~Serializer( void )
{
	if( m_FileDescriptor >= 0 )
		close( m_FileDescriptor );

} // ~Serializer

//////////////////////////////////////////////////////////////////////////

void GCL::Serializer::WriteObject( const Object& rObject, int IndentLevel )
{
	const std::string_view Name = rObject.Name();

	for( int i = 0; i < IndentLevel; ++i )
		write( m_FileDescriptor, "\t", 1 );

	write( m_FileDescriptor, Name.data(), static_cast< uint32_t >( Name.size() ) );

	if( rObject.IsNull() )
	{
		write( m_FileDescriptor, "\n", 1 );
	}
	else if( rObject.IsString() )
	{
		const Object::StringType& string = rObject.String();

		write( m_FileDescriptor, ":", 1 );
		write( m_FileDescriptor, string.data(), static_cast< uint32_t >( string.size() ) );
		write( m_FileDescriptor, "\n", 1 );
	}
	else if( rObject.IsTable() )
	{
		const Object::TableType& table = rObject.Table();

		write( m_FileDescriptor, ":", 1 );
		write( m_FileDescriptor, "\n", 1 );

		for( const Object& child : table )
			WriteObject( child, IndentLevel + 1 );
	}

} // WriteObject

//////////////////////////////////////////////////////////////////////////

bool GCL::Serializer::IsOpen( void ) const
{
	return ( m_FileDescriptor >= 0 );

} // IsOpen
