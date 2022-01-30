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

struct Member
{
	std::string Key;
	std::string Value;

	template< typename T >
	T GetValue();

}; // Member

//////////////////////////////////////////////////////////////////////////

void GetMembersFromObject( std::stringstream& rBuffer, std::vector< Member >& rMembers );

//////////////////////////////////////////////////////////////////////////

class Deserializer
{
public:

	 Deserializer( const std::filesystem::path& rPath );
	~Deserializer();

//////////////////////////////////////////////////////////////////////////

	bool IsOpen( void ) const;

	std::vector< Member >& GetMembers();

//////////////////////////////////////////////////////////////////////////

private:

//////////////////////////////////////////////////////////////////////////

	std::vector< Member > m_Members;

//////////////////////////////////////////////////////////////////////////

	char*  m_pFileBuffer = nullptr;
	size_t m_FileSize    = 0;

}; // Deserializer

//////////////////////////////////////////////////////////////////////////

template< typename T >
inline T Member::GetValue()
{
	return T();

} // GetValue

//////////////////////////////////////////////////////////////////////////

template<>
inline int Member::GetValue()
{
	return stoi( Value );

} // GetValue< int >

//////////////////////////////////////////////////////////////////////////

template<>
inline bool Member::GetValue()
{
	return Value == "true" ? true : false;

} // GetValue< bool >

//////////////////////////////////////////////////////////////////////////

template<>
inline std::vector< std::string > Member::GetValue()
{
	std::stringstream          ValueStr( std::string( Value.begin() + 1, Value.end() - 1 ) ); // Remove [] from ArrayString
	std::vector< std::string > Array;
	while( ValueStr.good() )
	{
		getline( ValueStr, Array.emplace_back( "" ), ',' );
	}
	return Array;

} // GetValue< std::vector< std::string > >

//////////////////////////////////////////////////////////////////////////

template<>
inline std::vector< int > Member::GetValue()
{
	std::stringstream  ValueStr( std::string( Value.begin() + 1, Value.end() - 1 ) ); // Remove [] from ArrayString
	std::vector< int > Array;
	while( ValueStr.good() )
	{
		std::string T;
		getline( ValueStr, T, ',' );
		Array.push_back( stoi( T ) );
	}
	return Array;

} // GetValue< std::vector< int > >

//////////////////////////////////////////////////////////////////////////

template<>
inline std::vector< Member > Member::GetValue() // Returns Members In An Object
{
	std::stringstream     ValueStr( Value );
	std::vector< Member > Members;
	GetMembersFromObject( ValueStr, Members );

	return Members;

} // GetValue< std::vector< Member > >

} // namespace GCL
