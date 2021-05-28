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

#include <Common/Platform/POSIX/POSIXError.h>

#include <iostream>

#include <fcntl.h>
#include <io.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////

GCL::Serializer::Serializer( const std::filesystem::path& rPath )
{
	constexpr int OPEN_FLAGS       = O_WRONLY | O_BINARY | O_TRUNC | O_CREAT;
	constexpr int SHARE_FLAGS      = SH_DENYNO;
	constexpr int PERMISSION_FLAGS = S_IREAD | S_IWRITE;

#if defined( _WIN32 )
	POSIX_CALL( _wsopen_s( &m_FileDescriptor, rPath.c_str(), OPEN_FLAGS, SHARE_FLAGS, PERMISSION_FLAGS ) );
#else // _WIN32
	POSIX_CALL( m_FileDescriptor = open( rPath.c_str(), ofstream, SHARE_FLAGS, PERMISSION_FLAGS ) );
#endif // _WIN32

} // Serializer

//////////////////////////////////////////////////////////////////////////

GCL::Serializer::~Serializer( void )
{
	if( m_FileDescriptor >= 0 )
	{

#if defined( _WIN32 )
		_close( m_FileDescriptor );
#else // _WIN32
		close( m_FileDescriptor );
#endif // else

	}

} // ~Serializer

//////////////////////////////////////////////////////////////////////////

void GCL::Serializer::WriteObject( const Object& rObject, int IndentLevel )
{
	const std::string_view Name = rObject.Name();

	for( int i = 0; i < IndentLevel; ++i )
		_write( m_FileDescriptor, "\t", 1 );

	_write( m_FileDescriptor, Name.data(), static_cast< uint32_t >( Name.size() ) );

	if( rObject.IsNull() )
	{
		_write( m_FileDescriptor, "\n", 1 );
	}
	else if( rObject.IsString() )
	{
		const Object::StringType& string = rObject.String();

		_write( m_FileDescriptor, ":", 1 );
		_write( m_FileDescriptor, string.data(), static_cast< uint32_t >( string.size() ) );
		_write( m_FileDescriptor, "\n", 1 );
	}
	else if( rObject.IsTable() )
	{
		const Object::TableType& table = rObject.Table();

		_write( m_FileDescriptor, ":", 1 );
		_write( m_FileDescriptor, "\n", 1 );

		for( const Object& child : table )
			WriteObject( child, IndentLevel + 1 );
	}

} // WriteObject

//////////////////////////////////////////////////////////////////////////

bool GCL::Serializer::IsOpen( void ) const
{
	return ( m_FileDescriptor >= 0 );

} // IsOpen
