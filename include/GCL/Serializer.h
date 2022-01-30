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

#pragma once

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace GCL
{
class Serializer
{
public:

	 Serializer( const std::filesystem::path& rPath );
	~Serializer( void );

//////////////////////////////////////////////////////////////////////////

	template< typename T >
	void Write( const std::string& rKey, const T& rValue );

	void StartObject( const std::string& rKey );
	void EndObject();
	void Null( const std::string& rKey );

//////////////////////////////////////////////////////////////////////////

	bool IsOpen( void ) const;

//////////////////////////////////////////////////////////////////////////

private:

//////////////////////////////////////////////////////////////////////////

	std::string GetIndent( void ) const;

//////////////////////////////////////////////////////////////////////////

	template< typename ArrayT >
	void WriteArray( const std::string& rKey, const std::vector< ArrayT >& rArray );

//////////////////////////////////////////////////////////////////////////

private:

	int m_IndentLevel    = 0;
	int m_FileDescriptor = -1;
	int m_ObjectsCount   = 0;

	std::stringstream m_Buffer;

}; // Serializer

//////////////////////////////////////////////////////////////////////////

template< typename ArrayT >
inline void Serializer::WriteArray( const std::string& rKey, const std::vector< ArrayT >& rArray )
{
	m_Buffer << GetIndent() + rKey + ": [";
	for( int i = 0; ( int )i < rArray.size(); ++i )
	{
		m_Buffer << rArray[ i ];
		if( i != rArray.size() - 1 )
			m_Buffer << ", ";
	}
	m_Buffer << "]\n";

} // WriteArray

//////////////////////////////////////////////////////////////////////////

template< typename T >
inline void Serializer::Write( const std::string& rKey, const T& rValue )
{
	m_Buffer << GetIndent() + rKey + ": " << rValue << std::endl;

} // Write

//////////////////////////////////////////////////////////////////////////

template<>
inline void Serializer::Write< bool >( const std::string& rKey, const bool& rValue )
{
	if( rValue )
		m_Buffer << GetIndent() + rKey + ": true" << std::endl;
	else
		m_Buffer << GetIndent() + rKey + ": false" << std::endl;

} // Write< bool >

//////////////////////////////////////////////////////////////////////////

template<>
inline void Serializer::Write< std::vector< int > >( const std::string& rKey, const std::vector< int >& rValue )
{
	WriteArray< int >( rKey, rValue );

} // Write< std::vector< int > >

//////////////////////////////////////////////////////////////////////////

template<>
inline void Serializer::Write< std::vector< float > >( const std::string& rKey, const std::vector< float >& rValue )
{
	WriteArray< float >( rKey, rValue );

} // Write< std::vector< float > >

//////////////////////////////////////////////////////////////////////////

template<>
inline void Serializer::Write< std::vector< std::string > >( const std::string& rKey, const std::vector< std::string >& rValue )
{
	WriteArray< std::string >( rKey, rValue );

} // Write< std::vector< std::string > >

//////////////////////////////////////////////////////////////////////////

} // namespace GCL
