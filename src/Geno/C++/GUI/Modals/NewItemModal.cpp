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

#include "NewItemModal.h"

#include "GUI/Modals/OpenFileModal.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

enum RequestType
{
	None = 0,
	RequestTypePath,
	RequestTypeString,

}; // RequestType

//////////////////////////////////////////////////////////////////////////

void NewItemModal::RequestPath( std::string Title, std::filesystem::path DefaultLocation, void* pUser, PathCallback Callback )
{
	if( Open() )
	{
		m_Title       = std::move( Title );
		m_Location    = std::move( DefaultLocation );
		m_Callback    = reinterpret_cast<void*> (Callback);
		m_pUser       = pUser;
		m_RequestType = RequestTypePath;
	}

} // RequestPath

//////////////////////////////////////////////////////////////////////////

void NewItemModal::RequestString( std::string Title, void* pUser, StringCallback Callback )
{
	if( Open() )
	{
		m_Title       = std::move( Title );
		m_Callback    = reinterpret_cast<void*> (Callback);
		m_pUser       = pUser;
		m_RequestType = RequestTypeString;
	}

} // RequestString

//////////////////////////////////////////////////////////////////////////

void NewItemModal::UpdateDerived( void )
{
	if( ImGui::BeginChild( 1, ImVec2( 0, -24 ) ) )
	{
		ImGui::TextUnformatted( "Name" );

		switch( m_RequestType )
		{
			case RequestTypePath:   { UpdateItem();   } break;
			case RequestTypeString: { UpdateString(); } break;
		}

	} ImGui::EndChild();

	const bool CanClickOK = !m_Name.empty() && !m_Location.empty();

	ImGui::PushItemFlag( ImGuiItemFlags_Disabled, !CanClickOK );
	ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * ( CanClickOK ? 1.0f : 0.5f ) );
	if( ImGui::Button( "OK", ImVec2( 80, 0 ) ) )
	{
		if( m_Callback )
		{
			switch( m_RequestType )
			{
				case RequestTypePath:   { ( reinterpret_cast<PathCallback> (m_Callback) )( std::move( m_Name ), std::move( m_Location ), m_pUser ); } break;
				case RequestTypeString: { ( reinterpret_cast<StringCallback> (m_Callback) )( std::move( m_Name ), m_pUser );                          } break;
				default:                { GENO_ASSERT( false ); /* Request type was corrupted */                                                } break;
			}
		}

		Close();
	}
	ImGui::PopStyleVar();
	ImGui::PopItemFlag();

	// Explain why OK button is not clickable when hovering it
	if( !CanClickOK && ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		if( m_Name    .empty() ) ImGui::BulletText( "Name not set" );
		if( m_Location.empty() ) ImGui::BulletText( "Location not set" );
		ImGui::EndTooltip();
	}

	ImGui::SameLine();
	if( ImGui::Button( "Cancel", ImVec2( 80, 0 ) ) )
	{
		Close();
	}

} // UpdateDerived

//////////////////////////////////////////////////////////////////////////

void NewItemModal::OnClose( void )
{
	m_Title   .clear();
	m_Name    .clear();
	m_Location.clear();

	m_Callback    = nullptr;
	m_pUser       = nullptr;
	m_RequestType = -1;

} // OnClose

//////////////////////////////////////////////////////////////////////////

void NewItemModal::UpdateItem( void )
{
	ImGui::SetNextItemWidth( -5.0f );
	ImGui::InputText( "##Name", &m_Name );

	ImGui::TextUnformatted( "Location" );

	std::string LocationBuffer = m_Location.string();

	ImGui::SetNextItemWidth( -60.0f );
	ImGui::InputText( "##Location", &LocationBuffer );

	m_Location.assign( std::move( LocationBuffer ) );

	ImGui::SameLine();
	if( ImGui::Button( "Browse" ) )
	{
		OpenFileModal::Instance().SetCurrentDirectory( m_Location );
		OpenFileModal::Instance().RequestDirectory( m_Title + " Location", this,
			[]( const std::filesystem::path& path, void* user )
			{
				NewItemModal* pSelf = static_cast< NewItemModal* >( user );

				pSelf->m_Location = path.lexically_normal();
			}
		);
	}

} // UpdateItem

//////////////////////////////////////////////////////////////////////////

void NewItemModal::UpdateString( void )
{
	ImGui::SetNextItemWidth( -5.0f );
	ImGui::InputText( "##Name", &m_Name );

} // UpdateString
