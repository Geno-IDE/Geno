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

#include "GCL/Deserializer.h"

#include "Common/Platform/POSIX/POSIXError.h"

#include <cstdio>
#include <cstring>
#include <iostream>

#include <fcntl.h>

#if defined( _WIN32 )
#include <io.h>
#define open _wopen
#elif defined( __unix__ ) || defined( __APPLE__ ) // _WIN32
#include <unistd.h>
#define O_BINARY 0
#endif // __unix__ || __APPLE__

//////////////////////////////////////////////////////////////////////////

void GCL::GetMembersFromObject( std::stringstream& rBuffer, std::vector< GCL::Member >& rMembers )
{
	std::string Line;
	int         ObjectIndex = -1;

	while( getline( rBuffer, Line, '\n' ) )
	{
		Line += "\n"; // Add \n for checking if the Line Has Object Or Member

		// If Line Have \t Than We Add That Line To Object Member Value
		if( Line.find( "\t" ) == std::string::npos )
		{
			GCL::Member Member;

			Member.Key = Line.substr( 0, Line.find( ":" ) );
			if( Line.find( ": \n" ) == std::string::npos ) // Member
			{
				Line.resize( Line.size() - 1 ); // Removes \n From Line
				Member.Value = Line.substr( Line.find( ":" ) + 2, Line.size() );
			}
			else // Object Member
			{
				// Set ObjectIndex To Get The ObjectValue In It
				ObjectIndex = ( int )rMembers.size();
			}

			rMembers.push_back( Member );
		}
		else // Lines Containing \t (Are Members Of Object)
		{
			// Remove \t From Beginning And Add That Line To Object Value
			rMembers[ ObjectIndex ].Value += std::string( Line.begin() + 1, Line.end() );
		}
	}

} // GetMembersFromObject

//////////////////////////////////////////////////////////////////////////

GCL::Deserializer::Deserializer( const std::filesystem::path& rPath )
{
	if( !std::filesystem::exists( rPath ) )
	{
		std::cerr << "GCL::Serializer failed: '" << rPath << "' does not exist.\n";
		return;
	}

	if( int FileDescriptor; ( FileDescriptor = open( rPath.c_str(), O_RDONLY | O_BINARY ) ) != 0 )
	{
		m_FileSize = static_cast< size_t >( lseek( FileDescriptor, 0, SEEK_END ) );
		lseek( FileDescriptor, 0, SEEK_SET );

		m_pFileBuffer = static_cast< char* >( malloc( m_FileSize ) );

		read( FileDescriptor, m_pFileBuffer, static_cast< uint32_t >( m_FileSize ) );
		close( FileDescriptor );
	}

} // Deserializer

//////////////////////////////////////////////////////////////////////////

GCL::Deserializer::~Deserializer( void )
{
	if( m_pFileBuffer )
		free( m_pFileBuffer );

} // ~Deserializer

//////////////////////////////////////////////////////////////////////////

bool GCL::Deserializer::IsOpen( void ) const
{
	return ( ( m_pFileBuffer != nullptr ) && ( m_FileSize > 0 ) );

} // IsOpen

//////////////////////////////////////////////////////////////////////////

std::vector< GCL::Member >& GCL::Deserializer::GetMembers()
{
	if( m_Members.empty() )
	{
		std::stringstream Buffer( m_pFileBuffer );
		GetMembersFromObject( Buffer, m_Members );
	}

	return m_Members;

} // GetMembers
