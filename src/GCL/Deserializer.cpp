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

//////////////////////////////////////////////////////////////////////////

constexpr bool LineStartsWithIndent( std::string_view Line, int IndentLevel )
{
	for( int i = 0; i < IndentLevel; ++i )
	{
		if( Line[ i ] != '\t' )
			return false;
	}

	return true;

} // LineStartsWithIndent

//////////////////////////////////////////////////////////////////////////

GCL::Deserializer::Deserializer( const std::filesystem::path& rPath )
{
	if( !std::filesystem::exists( rPath ) )
	{
		std::cerr << "GCL::Serializer failed: '" << rPath << "' does not exist.\n";
		return;
	}

	if( int FileDescriptor; POSIX_CALL( _wsopen_s( &FileDescriptor, rPath.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYNO, 0 ) ) )
	{
		m_FileSize = static_cast< size_t >( _lseek( FileDescriptor, 0, SEEK_END ) );
		_lseek( FileDescriptor, 0, SEEK_SET );

		m_pFileBuffer = ( char* )malloc( m_FileSize );

		for( size_t BytesRead = 0; BytesRead < m_FileSize; BytesRead += _read( FileDescriptor, m_pFileBuffer, static_cast< uint32_t >( m_FileSize ) ) );

		_close( FileDescriptor );
	}

} // Deserializer

//////////////////////////////////////////////////////////////////////////

GCL::Deserializer::~Deserializer( void )
{
	free( m_pFileBuffer );

} // ~Deserializer

//////////////////////////////////////////////////////////////////////////

void GCL::Deserializer::Objects( void* pUser, ObjectCallback Callback )
{
	std::string_view Unparsed( m_pFileBuffer, m_FileSize );

	while( !Unparsed.empty() )
	{
		size_t           LineEnd = Unparsed.find( '\n' );
		std::string_view Line    = Unparsed.substr( 0, LineEnd );

		ParseLine( Line, 0, &Unparsed, Callback, pUser );
	}

} // Objects

//////////////////////////////////////////////////////////////////////////

bool GCL::Deserializer::IsOpen( void ) const
{
	return ( ( m_pFileBuffer != nullptr ) && ( m_FileSize > 0 ) );

} // IsOpen

//////////////////////////////////////////////////////////////////////////

bool GCL::Deserializer::ParseLine( std::string_view Line, int IndentLevel, std::string_view* pUnparsed, ObjectCallback Callback, void* pUser )
{
	if( !LineStartsWithIndent( Line, IndentLevel ) )
		return false;

	*pUnparsed = pUnparsed->substr( Line.size() + ( Line.size() < pUnparsed->size() ) );

//////////////////////////////////////////////////////////////////////////

	std::string_view UnindentedLine = Line.substr( IndentLevel );
	size_t           ColonIndex     = UnindentedLine.find_first_of( ':' );

	if( ColonIndex == std::string_view::npos ) // No colon found
	{
		Object* pParentObject = static_cast< Object* >( pUser );
		Object  Child( UnindentedLine );

		pParentObject->AddChild( std::move( Child ) );
	}
	else if( ( ColonIndex + 1 ) < UnindentedLine.size() ) // Something comes after the colon
	{
		Object Object( UnindentedLine.substr( 0, ColonIndex ) );
		Object.SetString( UnindentedLine.substr( ColonIndex + 1 ) );

		Callback( std::move( Object ), pUser );
	}
	else // Colon is at the end of the line, signifying the start of a table
	{
		std::string_view Name               = UnindentedLine.substr( 0, ColonIndex );
		Object           Object( Name, std::in_place_type< Object::TableType > );
		auto             AddChildCallback = []( GCL::Object Child, void* pParent ) { static_cast< GCL::Object* >( pParent )->AddChild( std::move( Child ) ); };

		// Parse remaining lines recursively until the indent level changes
		while( !pUnparsed->empty() && ParseLine( pUnparsed->substr( 0, pUnparsed->find( '\n' ) ), IndentLevel + 1, pUnparsed, AddChildCallback, &Object ) );

		Callback( std::move( Object ), pUser );
	}

	return true;

} // ParseLine
