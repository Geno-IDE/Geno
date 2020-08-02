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

#include "WorkspaceWidget.h"

#include "Components/Project.h"
#include "GUI/Widgets/TextEditWidget.h"
#include "GUI/Application.h"

#include <Common/LocalAppData.h>

#include <fstream>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void WorkspaceWidget::Show( bool* p_open )
{
	if( ImGui::Begin( "Workspace", p_open ) )
	{
		if( Workspace* workspace = Application::Instance().CurrentWorkspace() )
		{
			const std::string workspace_id_string    = workspace->name_ + "##WKS_" + workspace->name_;
			bool              workspace_item_hovered = false;
			Project*          project_hovered        = nullptr;

			if( ImGui::TreeNode( workspace_id_string.c_str() ) )
			{
				workspace_item_hovered = ImGui::IsItemHovered();

				for( Project& prj : workspace->projects_ )
				{
					const std::string project_id_string = prj.name_ + "##PRJ_" + prj.name_;

					if( ImGui::TreeNode( project_id_string.c_str() ) )
					{
						if( ImGui::IsItemHovered() )
							project_hovered = &prj;

						for( std::filesystem::path& file : prj.files_ )
						{
							std::filesystem::path relative_file_path = *workspace / file;
							std::string           file_string        = relative_file_path.string();

							if( ImGui::Selectable( file_string.c_str() ) )
							{
								TextEditWidget::Instance().AddFile( file );
							}
						}

						ImGui::TreePop();
					}
					else
					{
						if( ImGui::IsItemHovered() )
							project_hovered = &prj;
					}
				}

				ImGui::TreePop();
			}
			else
			{
				workspace_item_hovered = ImGui::IsItemHovered();
			}

//////////////////////////////////////////////////////////////////////////

			// ImGUI seeds the popup IDs by the ID of the last item on the stack, which would be the context menu below
			bool open_workspace_rename_popup = false;
			bool open_new_project_popup      = false;
			bool open_project_rename_popup   = false;
			bool open_new_file_popup         = false;

			if( ImGui::IsMouseReleased( ImGuiMouseButton_Right ) )
			{
				if( workspace_item_hovered )
				{
					ImGui::OpenPopup( "WorkspaceContextMenu", ImGuiPopupFlags_MouseButtonRight );
				}
				else if( project_hovered )
				{
					ImGui::OpenPopup( "ProjectContextMenu", ImGuiPopupFlags_MouseButtonRight );

					selected_project_ = project_hovered->name_;
				}
			}

			if( ImGui::BeginPopup( "WorkspaceContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )      open_workspace_rename_popup = true;
				if( ImGui::MenuItem( "New Project" ) ) open_new_project_popup      = true;

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopup( "ProjectContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )   open_project_rename_popup = true;
				if( ImGui::MenuItem( "New File" ) ) open_new_file_popup       = true;

				ImGui::EndPopup();
			}

			if( open_workspace_rename_popup )    ImGui::OpenPopup( "Rename Workspace" );
			else if( open_new_project_popup )    ImGui::OpenPopup( "New Project" );
			else if( open_project_rename_popup ) ImGui::OpenPopup( "Rename Project" );
			else if( open_new_file_popup )       ImGui::OpenPopup( "New File" );

			if( ImGui::BeginPopupModal( "Rename Workspace" ) )
			{
				ImGui::InputTextWithHint( "##Name", "New Name", &popup_text_ );

				if( ImGui::Button( "Rename", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();

					std::filesystem::remove( workspace->location_ );
					workspace->location_.remove_filename();
					workspace->location_.append( popup_text_ + ".gwks");
					workspace->name_ = std::move( popup_text_ );
					workspace->Serialize();
				}

				ImGui::SameLine();
				if( ImGui::Button( "Cancel", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopupModal( "New Project" ) )
			{
				ImGui::InputTextWithHint( "##Name", "Project Name", &popup_text_ );

				if( ImGui::Button( "Create", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();

					Project& prj = workspace->projects_.emplace_back( LocalAppData::Instance() / popup_text_ + ".gprj" );
					prj.name_    = std::move( popup_text_ );
				}

				ImGui::SameLine();
				if( ImGui::Button( "Cancel", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopupModal( "Rename Project" ) )
			{
				ImGui::InputTextWithHint( "##Name", "New Name", &popup_text_ );

				if( ImGui::Button( "Rename", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();

					if( Project* prj = workspace->ProjectByName( selected_project_ ) )
						prj->name_ = std::move( popup_text_ );

					popup_text_.clear();
					selected_project_.clear();
				}

				ImGui::SameLine();
				if( ImGui::Button( "Cancel", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopupModal( "New File" ) )
			{
				ImGui::InputTextWithHint( "##Name", "File Name", &popup_text_ );

				if( ImGui::Button( "Create", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();

					if( Project* prj = workspace->ProjectByName( selected_project_ ) )
					{
						std::filesystem::path file_path = *prj / std::move( popup_text_ );
						std::ofstream         ofs       = std::ofstream( file_path, std::ios::binary | std::ios::trunc );

						if( ofs.is_open() )
						{
							prj->files_.emplace_back( std::move( file_path ) );
						}
					}

					popup_text_.clear();
					selected_project_.clear();
				}

				ImGui::SameLine();
				if( ImGui::Button( "Cancel", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();
}
