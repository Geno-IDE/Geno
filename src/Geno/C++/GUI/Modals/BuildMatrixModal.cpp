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

#include "BuildMatrixModal.h"

#include "Auxiliary/ImGuiAux.h"
#include "Application.h"

#include <imgui.h>
#include <imgui_internal.h>

//////////////////////////////////////////////////////////////////////////

void BuildMatrixModal::Show( void )
{
	Open();

} // Show

//////////////////////////////////////////////////////////////////////////

std::string BuildMatrixModal::PopupID( void )
{
	return "BUILD_MATRIX_MODAL";

} // PopupID

//////////////////////////////////////////////////////////////////////////

std::string BuildMatrixModal::Title( void )
{
	return "Configuring Build Matrix";

} // Title

//////////////////////////////////////////////////////////////////////////

void BuildMatrixModal::UpdateDerived( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	if( !pWorkspace )
	{
		ImGui::TextUnformatted( "No workspace open" );
		return;
	}

	ImGuiID ID = 0;

	if( ImGuiAux::BeginChildHorizontal( ++ID, ImVec2( -200, -20 ), ImGuiWindowFlags_HorizontalScrollbar ) )
	{
		for( const BuildMatrix::Column& rColumn : pWorkspace->m_BuildMatrix.m_Columns )
		{
			if( ImGui::BeginChild( ++ID, ImVec2( 128, 0 ), false, ImGuiWindowFlags_NoScrollbar ) )
			{
				if( ImGui::BeginChild( ++ID, ImVec2( 0, 20 ) ) )
				{
					ImGuiAux::TextCentered( rColumn.Name.c_str() );
					ImGui::Separator();

				} ImGui::EndChild();

				if( ImGui::BeginChild( ++ID ) )
				{
					for( const BuildMatrix::NamedConfiguration& rConfiguration : rColumn.Configurations )
					{
						const ImGuiWindow* pWindow   = ImGui::GetCurrentWindow();
						const ImVec2       TextSize  = ImGui::CalcTextSize( rConfiguration.Name.c_str() );
						const ImVec2       CursorPos = pWindow->DC.CursorPos;

						ImGui::SetCursorPosX( ( pWindow->Size.x - TextSize.x ) * 0.5f );
						ImGui::Text( rConfiguration.Name.c_str() );

						if( ImGui::IsItemHovered() )
						{
							ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
							ImGui::RenderArrow( pWindow->DrawList, CursorPos + ImVec2( 0.0f,                                   0.0f ), 0xFFFFFFFF, ImGuiDir_Right );
							ImGui::RenderArrow( pWindow->DrawList, CursorPos + ImVec2( pWindow->Size.x - ImGui::GetFontSize(), 0.0f ), 0xFFFFFFFF, ImGuiDir_Left );
						}
					}

				} ImGui::EndChild();

			} ImGui::EndChild();

			ImGui::Separator();
		}
		
	} ImGui::EndChild();

	ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 1, 0, 0, 1 ) );

	ImGui::SameLine();
	if( ImGuiAux::BeginChildHorizontal( ++ID, ImVec2( 200, -20 ) ) )
	{
	} ImGui::EndChild();

	ImGui::PopStyleColor();

	if( ImGui::BeginChild( ++ID ) )
	{
		if( ImGui::Button( "Save & Close" ) )
		{
			Close();
		}

	} ImGui::EndChild();

} // UpdateDerived
