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

#include "Common/Platform/POSIX/POSIXError.h"

#include <iostream>

#include <sys/stat.h>
#include <fcntl.h>

#if defined( _WIN32 )
#include <corecrt_io.h>
#define open _wopen
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
	{
		if( m_ObjectsCount > 0 )
			std::cerr << "[Serializer] -> Missing EndObject()\nObjectCount " << m_ObjectsCount << std::endl;
		else if( m_ObjectsCount < 0 )
			std::cerr << "[Serializer] -> Missing StartObject()\nObjectCount " << m_ObjectsCount << std::endl;
		else
			write( m_FileDescriptor, m_Buffer.str().data(), ( uint32_t )m_Buffer.str().size() );
		close( m_FileDescriptor );
	}

} // ~Serializer

//////////////////////////////////////////////////////////////////////////

void GCL::Serializer::StartObject( const std::string& rKey )
{
	m_ObjectsCount++;
	Write( rKey, "" );
	m_IndentLevel++;

} // StartObject

//////////////////////////////////////////////////////////////////////////

void GCL::Serializer::EndObject()
{
	m_ObjectsCount--;
	m_IndentLevel--;

} // EndObject

//////////////////////////////////////////////////////////////////////////

void GCL::Serializer::Null( const std::string& rKey )
{
	Write( rKey, "Null" );

} // Null

//////////////////////////////////////////////////////////////////////////

bool GCL::Serializer::IsOpen( void ) const
{
	return ( m_FileDescriptor >= 0 );

} // IsOpen

//////////////////////////////////////////////////////////////////////////

std::string GCL::Serializer::GetIndent( void ) const
{
	std::string Indent = "";
	for( int i = 0; i < m_IndentLevel; ++i )
		Indent += "\t";
	return Indent;

} // GetIndent
