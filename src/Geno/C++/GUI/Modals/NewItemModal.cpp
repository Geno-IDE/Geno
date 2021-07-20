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

#include "Auxiliary/ImGuiAux.h"
#include "Auxiliary/STBAux.h"
#include "GUI/Modals/OpenFileModal.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

NewItemModal::NewItemModal( void )
	: m_IconFolder( STBAux::LoadImageTexture( "Icons/FolderColored.png" ) )
{
	m_MinSize = ImVec2( 360.0f, 115.0f );
	m_MaxSize = ImVec2( 530.0f, 115.0f );
} // NewItemModal

//////////////////////////////////////////////////////////////////////////

void NewItemModal::Show( const std::string Title, const char* pFilter, const std::filesystem::path& rLocation, Callback Callback )
{
	if( Open() )
	{
		m_Title    = Title;
		m_pFilter  = pFilter;
		m_Callback = Callback;

		if( m_pFilter )
		{
			m_Title += " - " + std::string( m_pFilter );
		}

		if( !rLocation.empty() )
			m_Directory = rLocation.string();
	}
} // Show

//////////////////////////////////////////////////////////////////////////

void NewItemModal::UpdateDerived( void )
{
	ImGui::TextUnformatted( "Name          " );
	ImGui::SameLine();

	ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 6.0f );

	ImGui::SetNextItemWidth( ImGui::GetContentRegionAvailWidth() );

	ImGui::InputText( "##FileName", &m_Name );

	ImGui::TextUnformatted( "Directory " );
	ImGui::SameLine();

	ImVec2 ButtonSize = ImVec2( ImGui::GetFontSize() * m_IconFolder.GetAspectRatio(), ImGui::GetFontSize() );

	ImGui::SetNextItemWidth( ImGui::GetContentRegionAvailWidth() - ButtonSize.x * 1.5f );

	ImGui::InputText( "##Directory", &m_Directory, ImGuiInputTextFlags_ReadOnly );

	ImGui::PopStyleVar();

	ImGui::SameLine();

	ImGui::PushStyleColor( ImGuiCol_Button, ImGui::GetStyle().Colors[ ImGuiCol_ChildBg ] );
	if( ImGui::ImageButton( ( ImTextureID )m_IconFolder.GetID(), ButtonSize ) )
	{
		OpenFileModal::Instance().Show( [ this ]( const std::filesystem::path& rFile )
			{ m_Directory = rFile.string(); } );
	}
	ImGui::PopStyleColor();

	bool DisableButton = m_Name.empty() || m_Directory.empty();

	if( DisableButton )
	{
		m_ButtonData.ColorText = ImGui::GetStyle().Colors[ ImGuiCol_Text ];
		m_ButtonData.Color     = ImVec4( 0.15f, 0.15f, 0.15f, 1.0f );
	}
	else
	{
		m_ButtonData = {};
	}

	m_ButtonData.Size = ImVec2( 70, 30 );

	ImGui::PushItemFlag( ImGuiItemFlags_Disabled, DisableButton );

	ImGuiAux::Button( "Create", m_ButtonData, [ this ]()
		{
			m_Callback( m_Name, m_Directory );
			Close();
		} );

	ImGui::PopItemFlag();

	// Explain why OK button is not clickable when hovering it
	if( DisableButton && ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		if( m_Name.empty() ) ImGui::BulletText( "Name not set" );
		if( m_Directory.empty() ) ImGui::BulletText( "Directory not set" );
		ImGui::EndTooltip();
	}

	ImGui::SameLine();

	m_ButtonData      = {};
	m_ButtonData.Size = ImVec2( 70, 30 );

	ImGuiAux::Button( "Cancel", m_ButtonData, [ this ]()
		{ Close(); } );

} // UpdateDerived

//////////////////////////////////////////////////////////////////////////

void NewItemModal::OnClose( void )
{
	m_Title.clear();
	m_Name.clear();
	m_Directory.clear();
	m_Callback = {};
	m_pFilter  = nullptr;
} // OnClose
