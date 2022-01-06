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

#include "JSONSerializer.h"

#include <fstream>

JSONSerializer::JSONSerializer( const std::filesystem::path& rFile )
	: m_File( rFile )
{
	m_Writer = new rapidjson::PrettyWriter< rapidjson::StringBuffer >( m_Buffer );
	m_Writer->StartObject();

} // JSONSerializer

//////////////////////////////////////////////////////////////////////////

JSONSerializer::~JSONSerializer( void )
{
	m_Writer->EndObject();
	std::ofstream File( m_File, std::ios::out );
	File << m_Buffer.GetString();
	File.close();
	delete m_Writer;

} // ~JSONSerializer

//////////////////////////////////////////////////////////////////////////

void JSONSerializer::Object( const std::string& rName, const std::function< void( void ) >& rFunc )
{
	m_Writer->String( rName.c_str() );
	m_Writer->StartObject();
	rFunc();
	m_Writer->EndObject();

} // Object

//////////////////////////////////////////////////////////////////////////

void JSONSerializer::Null( const std::string& rName )
{
	m_Writer->String( rName.c_str() );
	m_Writer->Null();
} // Null
