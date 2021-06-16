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
#include "Compilers/CompilerGCC.h"
#include "Compilers/CompilerMSVC.h"
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

	const bool ShowSidebar = !m_SelectedColumn.empty() && !m_SelectedConfiguration.empty();

	ImGuiID ID = 0;

	if( ImGuiAux::BeginChildHorizontal( ++ID, ImVec2( ShowSidebar ? -200.0f : 0.0f, -20.0f ), false, ImGuiWindowFlags_HorizontalScrollbar ) )
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
					for( const auto&[ rName, rConfiguration ] : rColumn.Configurations )
					{
						ImGuiWindow* pWindow   = ImGui::GetCurrentWindow();
						const ImVec2 TextSize  = ImGui::CalcTextSize( rName.c_str() );
						const ImVec2 CursorPos = pWindow->DC.CursorPos;
						const bool   Selected  = rColumn.Name == m_SelectedColumn && rName == m_SelectedConfiguration;

						ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::GetStyleColorVec4( ImGuiCol_ChildBg ) + ( Selected ? ImVec4( 0.5f, 0.5f, 0.5f, 0.5f ) : ImVec4( 0, 0, 0, 0 ) ) );

						if( ImGui::BeginChild( ++ID, ImVec2( 0, TextSize.y ) ) )
						{
							if( ImGui::IsWindowHovered() )
							{
								ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
								ImGui::RenderArrow( pWindow->DrawList, CursorPos + ImVec2( 0.0f,                                   0.0f ), 0xFFFFFFFF, ImGuiDir_Right );
								ImGui::RenderArrow( pWindow->DrawList, CursorPos + ImVec2( pWindow->Size.x - ImGui::GetFontSize(), 0.0f ), 0xFFFFFFFF, ImGuiDir_Left );

								if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
								{
									m_SelectedColumn        = rColumn.Name;
									m_SelectedConfiguration = rName;
								}
							}

							ImGui::SetCursorPosX( ( pWindow->Size.x - TextSize.x ) * 0.5f );
							ImGui::Text( rName.c_str() );

						} ImGui::EndChild();

						ImGui::PopStyleColor();
					}

				} ImGui::EndChild();

			} ImGui::EndChild();

			ImGui::Separator();
		}
		
	} ImGui::EndChild();

	if( ShowSidebar )
	{
		ImGui::SameLine();
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10, 10 ) );
		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.2f, 0.2f, 0.2f, 0.4f ) );
		if( ImGui::BeginChild( ++ID, ImVec2( 200, -20 ), true ) )
		{
			const char* pCompilerNames[] = { "None", "MSVC", "GCC" };
			auto        Column           = std::find_if( pWorkspace->m_BuildMatrix.m_Columns.begin(), pWorkspace->m_BuildMatrix.m_Columns.end(), [ this ]( const BuildMatrix::Column& rColumn ) { return rColumn.Name == m_SelectedColumn; } );
			auto        Configuration    = Column->Configurations.find( m_SelectedConfiguration );
			int         Index            = Configuration->second.m_Compiler ? static_cast< int >( std::distance( std::begin( pCompilerNames ), std::find_if( std::begin( pCompilerNames ), std::end( pCompilerNames ), [ &Configuration ]( const char* pName ) { return pName == Configuration->second.m_Compiler->GetName(); } ) ) ) : 0;

			ImGui::TextUnformatted( "Compiler" );

			if( ImGui::Combo( "##COMPILER", &Index, pCompilerNames, static_cast< int >( std::size( pCompilerNames ) ) ) )
			{
				switch( Index )
				{
					case 0: { Configuration->second.m_Compiler.reset();                              } break;
					case 1: { Configuration->second.m_Compiler = std::make_shared< CompilerMSVC >(); } break;
					case 2: { Configuration->second.m_Compiler = std::make_shared< CompilerGCC  >(); } break;
				}

				pWorkspace->Serialize();
			}

		} ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}

	ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.3f, 0.3f, 0.3f, 0.4f ) );
	if( ImGui::BeginChild( ++ID ) )
	{
		if( ImGui::Button( "Save & Close" ) )
		{
			Close();
		}

	} ImGui::EndChild();
	ImGui::PopStyleColor();

} // UpdateDerived
