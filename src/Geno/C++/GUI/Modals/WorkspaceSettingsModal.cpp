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

#include "WorkspaceSettingsModal.h"

#include "Components/Workspace.h"
#include "GUI/Modals/NewItemModal.h"
#include "GUI/Application.h"
#include "GUI/MainWindow.h"

#include <imgui.h>

enum Category
{
	CategoryBuildMatrix,
	NumCategories
};

static constexpr const char* CategoryString( Category category )
{
	switch( category )
	{
		case CategoryBuildMatrix: return "Build Matrix";
		default:                  return nullptr;
	}
}

void WorkspaceSettingsModal::Show( void )
{
	if( Open() )
	{
		current_category_ = -1;
	}
}

void WorkspaceSettingsModal::UpdateDerived( void )
{
	Workspace* workspace = Application::Instance().CurrentWorkspace();
	if( !workspace )
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
				Category category = static_cast< Category >( i );

				if( ImGui::Selectable( CategoryString( category ), current_category_ == category ) )
				{
					current_category_ = category;
				}
			}
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();

		if( ImGui::BeginChild( 2 ) )
		{
			switch( current_category_ )
			{
				case CategoryBuildMatrix:
				{
					// Show build matrix configurations
					for( auto& column : workspace->build_matrix_.columns_ )
						ShowConfigurationColumn( column, std::string() );

					if( ImGui::SmallButton( "+##NewColumn" ) )
					{
						NewItemModal::Instance().RequestString( "New Column", nullptr,
							[]( std::string string, void* /*user*/ )
							{
								if( Workspace* workspace = Application::Instance().CurrentWorkspace() )
									workspace->build_matrix_.NewColumn( std::move( string ) );
							}
						);
					}

				} break;
			}
		}
		ImGui::EndChild();

		MainWindow::Instance().PopHorizontalLayout();
	}
	ImGui::EndChild();

	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + 4 );
	if( ImGui::Button( "Close", ImVec2( 80, 0 ) ) )
	{
		Close();
	}
}

void WorkspaceSettingsModal::OnClose( void )
{
	current_category_ = -1;
}

void WorkspaceSettingsModal::ShowConfigurationColumn( BuildMatrix::Column& column, std::string id_prefix )
{
	id_prefix += column.name;
	id_prefix += '_';

	ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 4.0f );
	ImGui::Text( "%s:", column.name.c_str() );
	ImGui::SameLine();
	if( ImGui::SmallButton( ( "+##NewConfiguration_" + id_prefix + column.name ).c_str() ) )
	{
		NewItemModal::Instance().RequestString( "New Exclusive Configuration", &column,
			[]( std::string string, void* user )
			{
				auto& column = *static_cast< BuildMatrix::Column* >( user );
				auto& cfg    = column.configurations.emplace_back();
				cfg.name     = std::move( string );
			}
		);
	}

	if( !column.configurations.empty() )
	{
		ImGui::Indent();

		for( auto& cfg : column.configurations )
		{
			if( ImGui::SmallButton( ( cfg.name + "##" + id_prefix + cfg.name ).c_str() ) )
			{
				NewItemModal::Instance().RequestString( "New Exclusive Category", &cfg,
					[]( std::string string, void* user )
					{
						auto& cfg       = *static_cast< BuildMatrix::NamedConfiguration* >( user );
						auto& exclusive = cfg.exclusive_columns.emplace_back();
						exclusive.name  = std::move( string );
					}
				);
			}

			if( !cfg.exclusive_columns.empty() )
			{
				ImGui::Indent();

				for( auto& exclusive : cfg.exclusive_columns )
					ShowConfigurationColumn( exclusive, id_prefix );

				ImGui::Unindent();
			}
		}

		ImGui::Unindent();
	}

	ImGui::PopStyleVar();
}
