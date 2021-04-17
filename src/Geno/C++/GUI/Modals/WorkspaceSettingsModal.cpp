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

#include "WorkspaceSettingsModal.h"

#include "Components/Workspace.h"
#include "GUI/Modals/NewItemModal.h"
#include "GUI/MainWindow.h"
#include "Application.h"

#include <imgui.h>

enum Category
{
	CategoryBuildMatrix,
	NumCategories
};

//////////////////////////////////////////////////////////////////////////

static constexpr const char* StringifyCategory( Category Category )
{
	switch( Category )
	{
		case CategoryBuildMatrix: return "Build Matrix";
		default:                  return nullptr;
	}

} // CategoryString

//////////////////////////////////////////////////////////////////////////

void WorkspaceSettingsModal::Show( void )
{
	if( Open() )
	{
		m_CurrentCategory = -1;
	}

} // Show

//////////////////////////////////////////////////////////////////////////

void WorkspaceSettingsModal::UpdateDerived( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	if( !pWorkspace )
	{
		ImGui::TextUnformatted( "No active workspace" );
		return;
	}

	if( ImGui::BeginChild( 1, ImVec2( 0, -30 ) ) )
	{
		MainWindow::Instance().PushHorizontalLayout();

		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::GetStyleColorVec4( ImGuiCol_FrameBg ) );
		if( ImGui::BeginChild( 1, ImVec2( 120, 0 ) ) )
		{
			for( int i = 0; i < NumCategories; ++i )
			{
				Category Category = static_cast< ::Category >( i );

				if( ImGui::Selectable( StringifyCategory( Category ), m_CurrentCategory == Category ) )
				{
					m_CurrentCategory = Category;
				}
			}

		} ImGui::EndChild();
		ImGui::PopStyleColor();

		if( ImGui::BeginChild( 2 ) )
		{
			switch( m_CurrentCategory )
			{
				case CategoryBuildMatrix:
				{
					// Show build matrix configurations
					for( BuildMatrix::Column& rColumn : pWorkspace->m_BuildMatrix.m_Columns )
						ShowConfigurationColumn( rColumn, std::string() );

					if( ImGui::SmallButton( "+##NewColumn" ) )
					{
						NewItemModal::Instance().RequestString( "New Column", nullptr,
							[]( std::string String, void* /*pUser*/ )
							{
								if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
									pWorkspace->m_BuildMatrix.NewColumn( std::move( String ) );
							}
						);
					}

				} break;
			}

		} ImGui::EndChild();

		MainWindow::Instance().PopHorizontalLayout();

	} ImGui::EndChild();

	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + 4 );
	if( ImGui::Button( "Close", ImVec2( 80, 0 ) ) )
	{
		Close();
	}

} // UpdateDerived

//////////////////////////////////////////////////////////////////////////

void WorkspaceSettingsModal::OnClose( void )
{
	m_CurrentCategory = -1;

} // OnClose

//////////////////////////////////////////////////////////////////////////

void WorkspaceSettingsModal::ShowConfigurationColumn( BuildMatrix::Column& rColumn, std::string IDPrefix )
{
	IDPrefix += rColumn.Name;
	IDPrefix += '_';

	ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 4.0f );
	ImGui::Text( "%s:", rColumn.Name.c_str() );
	ImGui::SameLine();
	if( ImGui::SmallButton( ( "+##NewConfiguration_" + IDPrefix + rColumn.Name ).c_str() ) )
	{
		NewItemModal::Instance().RequestString( "New Exclusive Configuration", &rColumn,
			[]( std::string String, void* pUser )
			{
				BuildMatrix::Column&             rColumn        = *static_cast< BuildMatrix::Column* >( pUser );
				BuildMatrix::NamedConfiguration& rConfiguration = rColumn.Configurations.emplace_back();
				rConfiguration.Name                             = std::move( String );
			}
		);
	}

	if( !rColumn.Configurations.empty() )
	{
		ImGui::Indent();

		for( BuildMatrix::NamedConfiguration& rConfiguration : rColumn.Configurations )
		{
			if( ImGui::SmallButton( ( rConfiguration.Name + "##" + IDPrefix + rConfiguration.Name ).c_str() ) )
			{
				NewItemModal::Instance().RequestString( "New Exclusive Category", &rConfiguration,
					[]( std::string string, void* user )
					{
						BuildMatrix::NamedConfiguration& rConfiguration   = *static_cast< BuildMatrix::NamedConfiguration* >( user );
						BuildMatrix::Column&             rExclusiveColumn = rConfiguration.ExclusiveColumns.emplace_back();
						rExclusiveColumn.Name                             = std::move( string );
					}
				);
			}

			if( !rConfiguration.ExclusiveColumns.empty() )
			{
				ImGui::Indent();

				for( BuildMatrix::Column& rExclusiveColumn : rConfiguration.ExclusiveColumns )
					ShowConfigurationColumn( rExclusiveColumn, IDPrefix );

				ImGui::Unindent();
			}
		}

		ImGui::Unindent();
	}

	ImGui::PopStyleVar();

} // ShowConfigurationColumn
