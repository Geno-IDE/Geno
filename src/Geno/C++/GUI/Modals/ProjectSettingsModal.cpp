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

#include "ProjectSettingsModal.h"

#include "Components/Project.h"
#include "Components/Workspace.h"
#include "GUI/Application.h"
#include "GUI/MainWindow.h"

#include <array>

#include <imgui.h>

enum Category
{
	CategoryGeneral,
	NumCategories
};

static constexpr const char* CategoryString( Category category )
{
	switch( category )
	{
		case CategoryGeneral: return "General";
		default:              return nullptr;
	}
}

void ProjectSettingsModal::Show( std::string project )
{
	if( Open() )
	{
		current_category_ = -1;
		edited_project_   = std::move( project );
	}
}

void ProjectSettingsModal::UpdateDerived( void )
{
	Workspace* workspace = Application::Instance().CurrentWorkspace();
	if( !workspace )
	{
		ImGui::TextUnformatted( "No active workspace" );
		return;
	}

	Project* project = workspace->ProjectByName( edited_project_ );
	if( !project )
	{
		ImGui::Text( "No project found by the name '%s'", edited_project_.c_str() );
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
				case CategoryGeneral:
				{
					const std::array kind_names   = { "Application", "Static Library", "Dynamic Library" };
					int              current_item = static_cast< int >( project->kind_ ) - 1;

					ImGui::TextUnformatted( "Kind" );

					ImGui::SetNextItemWidth( -5.0f );
					if( ImGui::Combo( "##Kind", &current_item, kind_names.data(), static_cast< int >( kind_names.size() ) ) )
					{
						project->kind_ = static_cast< ProjectKind >( current_item + 1 );
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

void ProjectSettingsModal::OnClose( void )
{
	current_category_ = -1;
	edited_project_.clear();
}
