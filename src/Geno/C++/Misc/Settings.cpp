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

#include "Settings.h"

#include <Common/LocalAppData.h>
#include <GCL/Deserializer.h>
#include <GCL/Serializer.h>

#include <fstream>

#include <imgui.h>

//////////////////////////////////////////////////////////////////////////

Settings::Settings( void )
{
	Load();

} // Settings

//////////////////////////////////////////////////////////////////////////

Settings::~Settings( void )
{
	Save();

} // ~Settings

//////////////////////////////////////////////////////////////////////////

void Settings::Load( void )
{
	m_Object = GCL::Object( "Settings", std::in_place_type< GCL::Object::TableType > );

	GCL::Deserializer Deserializer( LocalAppData::Instance() / L"settings.gcl" );
	if( !Deserializer.IsOpen() )
		return;

	Deserializer.Objects( this,
		[]( GCL::Object Object, void* pUser )
		{
			Settings* pSelf = static_cast< Settings* >( pUser );

			pSelf->m_Object.AddChild( std::move( Object ) );
		}
	);

	UpdateTheme();

} // Load

//////////////////////////////////////////////////////////////////////////

void Settings::Save( void )
{
	GCL::Serializer Serializer( LocalAppData::Instance() / L"settings.gcl" );
	if( !Serializer.IsOpen() )
		return;

	for( const GCL::Object& rChild : m_Object.Table() )
		Serializer.WriteObject( rChild );

} // Save

//////////////////////////////////////////////////////////////////////////

void Settings::UpdateTheme( void )
{
	GCL::Object& rTheme = m_Object[ "Theme" ];

	/**/ if( rTheme == "Classic" ) ImGui::StyleColorsClassic();
	else if( rTheme == "Light" )   ImGui::StyleColorsLight();
	else if( rTheme == "Dark" )    ImGui::StyleColorsDark();

} // UpdateTheme
