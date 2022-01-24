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
#include <functional>
#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

class JSONSerializer
{
public:

	 JSONSerializer( const std::filesystem::path& rFile );
	~JSONSerializer( void );

//////////////////////////////////////////////////////////////////////////

	void Object( const std::string& rName, const std::function< void( void ) >& rFunc );

	template< typename T >
	void Add( const std::string& rName, const T& rValue );

	void Null( const std::string& rName );

//////////////////////////////////////////////////////////////////////////

private:

	rapidjson::StringBuffer                             m_Buffer = {};
	rapidjson::PrettyWriter< rapidjson::StringBuffer >* m_Writer;

	std::filesystem::path m_File = {};

}; // JSONSerializer

//////////////////////////////////////////////////////////////////////////

template< typename T >
inline void JSONSerializer::Add( const std::string& rName, const T& rValue )
{
	std::string TempStr   = rName;
	T           TempValue = rValue;
	return;

} // Add< T >

//////////////////////////////////////////////////////////////////////////

template<>
inline void JSONSerializer::Add< std::string >( const std::string& rName, const std::string& rValue )
{
	m_Writer->String( rName.c_str() );
	m_Writer->String( rValue.c_str() );

} // Add< std::string >

//////////////////////////////////////////////////////////////////////////

template<>
inline void JSONSerializer::Add< bool >( const std::string& rName, const bool& rValue )
{
	m_Writer->String( rName.c_str() );
	m_Writer->Bool( rValue );

} // Add<bool>

//////////////////////////////////////////////////////////////////////////

template<>
inline void JSONSerializer::Add< int >( const std::string& rName, const int& rValue )
{
	m_Writer->String( rName.c_str() );
	m_Writer->Int( rValue );

} // Add< int >

//////////////////////////////////////////////////////////////////////////

template<>
inline void JSONSerializer::Add< double >( const std::string& rName, const double& rValue )
{
	m_Writer->String( rName.c_str() );
	m_Writer->Double( rValue );

} // Add< double >

//////////////////////////////////////////////////////////////////////////

template<>
inline void JSONSerializer::Add< uint32_t >( const std::string& rName, const uint32_t& rValue )
{
	m_Writer->String( rName.c_str() );
	m_Writer->Uint( rValue );

} // Add< unsigned int >

//////////////////////////////////////////////////////////////////////////

template<>
inline void JSONSerializer::Add< std::vector< int > >( const std::string& rName, const std::vector< int >& rArray )
{
	m_Writer->String( rName.c_str() );
	m_Writer->StartArray();
	for( const int& rValue : rArray )
		m_Writer->Int( rValue );
	m_Writer->EndArray();

} // Add< std::vector< int > >

//////////////////////////////////////////////////////////////////////////

template<>
inline void JSONSerializer::Add< std::vector< double > >( const std::string& rName, const std::vector< double >& rArray )
{
	m_Writer->String( rName.c_str() );
	m_Writer->StartArray();
	for( const double& rValue : rArray )
		m_Writer->Double( rValue );
	m_Writer->EndArray();

} // Add< std::vector< double > >

//////////////////////////////////////////////////////////////////////////

template<>
inline void JSONSerializer::Add< std::vector< uint32_t > >( const std::string& rName, const std::vector< uint32_t >& rArray )
{
	m_Writer->String( rName.c_str() );
	m_Writer->StartArray();
	for( const unsigned int& rValue : rArray )
		m_Writer->Uint( rValue );
	m_Writer->EndArray();

} // Add< std::vector< unsigned int > >

//////////////////////////////////////////////////////////////////////////

template<>
inline void JSONSerializer::Add< std::vector< std::string > >( const std::string& rName, const std::vector< std::string >& rArray )
{
	m_Writer->String( rName.c_str() );
	m_Writer->StartArray();
	for( const std::string& rValue : rArray )
		m_Writer->String( rValue.c_str() );
	m_Writer->EndArray();

} // Add< std::vector< std::string > >