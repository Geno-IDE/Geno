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
#include <rapidjson/prettywriter.h>
#include <string>
#include <vector>

class jsonSerializer
{
public:

	 jsonSerializer( const std::filesystem::path& rFile );
	~jsonSerializer( void );

//////////////////////////////////////////////////////////////////////////

	void Object( const std::string& rName, const std::function< void( void ) >& rFunc );

	template< typename T >
	void Add( const std::string& rName, const T& rValue )
	{
		static_assert( false );

	} // Add<T>

	template<>
	void Add( const std::string& rName, const std::string& rValue )
	{
		m_Writer->String( rName.c_str() );
		m_Writer->String( rValue.c_str() );

	} // Add<std::string>

	template<>
	void Add( const std::string& rName, const bool& rValue )
	{
		m_Writer->String( rName.c_str() );
		m_Writer->Bool( rValue );

	} // Add<bool>

	template<>
	void Add( const std::string& rName, const int& rValue )
	{
		m_Writer->String( rName.c_str() );
		m_Writer->Int( rValue );

	} // Add<int>

	template<>
	void Add( const std::string& rName, const double& rValue )
	{
		m_Writer->String( rName.c_str() );
		m_Writer->Double( rValue );

	} // Add<double>

	template<>
	void Add( const std::string& rName, const unsigned int& rValue )
	{
		m_Writer->String( rName.c_str() );
		m_Writer->Uint( rValue );

	} // Add<unsigned int>

	template<>
	void Add( const std::string& rName, const std::vector< int >& rArray )
	{
		m_Writer->String( rName.c_str() );
		m_Writer->StartArray();
		for( const int& rValue : rArray )
			m_Writer->Int( rValue );
		m_Writer->EndArray();

	} // Add<std::vector<int>>

	template<>
	void Add( const std::string& rName, const std::vector< double >& rArray )
	{
		m_Writer->String( rName.c_str() );
		m_Writer->StartArray();
		for( const double& rValue : rArray )
			m_Writer->Double( rValue );
		m_Writer->EndArray();

	} // Add<std::vector<double>>

	template<>
	void Add( const std::string& rName, const std::vector< unsigned int >& rArray )
	{
		m_Writer->String( rName.c_str() );
		m_Writer->StartArray();
		for( const unsigned int& rValue : rArray )
			m_Writer->Uint( rValue );
		m_Writer->EndArray();

	} // Add<std::vector<unsigned int>>

	void Null( const std::string& rName );

//////////////////////////////////////////////////////////////////////////

private:

	rapidjson::StringBuffer                             m_Buffer = {};
	rapidjson::PrettyWriter< rapidjson::StringBuffer >* m_Writer;

	std::filesystem::path m_File = {};

}; // jsonSerializer
