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
#include "GUI/Modals/NewItemModal.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/Modals/ProjectSettingsModal.h"
#include "GUI/Modals/WorkspaceSettingsModal.h"
#include "GUI/Application.h"

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

			if( expand_workspace_node_ )
			{
				ImGui::SetNextItemOpen( true );
				expand_workspace_node_ = false;
			}

			if( ImGui::TreeNode( workspace_id_string.c_str() ) )
			{
				workspace_item_hovered = ImGui::IsItemHovered();

				for( Project& prj : workspace->projects_ )
				{
					const std::string project_id_string = prj.name_ + "##PRJ_" + prj.name_;

					if( project_node_to_be_expanded_ == prj.name_ )
					{
						ImGui::SetNextItemOpen( true );
						project_node_to_be_expanded_.clear();
					}

					if( ImGui::TreeNode( project_id_string.c_str() ) )
					{
						if( ImGui::IsItemHovered() )
							project_hovered = &prj;

						for( std::filesystem::path& file : prj.files_ )
						{
							std::string file_string = file.filename().string();

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
			bool open_project_rename_popup   = false;

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
				if( ImGui::MenuItem( "New Project" ) )
				{
					NewItemModal::Instance().RequestPath( "New Project", workspace->location_, this,
						[]( std::string name, std::filesystem::path location, void* user )
						{
							WorkspaceWidget* self = static_cast< WorkspaceWidget* >( user );

							if( Workspace* workspace = Application::Instance().CurrentWorkspace() )
							{
								// Automatically expand tree if adding an item for the first time
								self->expand_workspace_node_ = true;

								workspace->NewProject( std::move( location ), std::move( name ) );
							}
						}
					);
				}
				ImGui::Separator();
				if( ImGui::MenuItem( "Settings" ) )
				{
					WorkspaceSettingsModal::Instance().Show();
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopup( "ProjectContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
					open_project_rename_popup = true;

				if( ImGui::MenuItem( "New File" ) )
				{
					Project* prj = workspace->ProjectByName( selected_project_ );

					NewItemModal::Instance().RequestPath( "New File", prj->location_, this,
						[]( std::string name, std::filesystem::path location, void* user )
						{
							WorkspaceWidget* self = static_cast< WorkspaceWidget* >( user );

							if( Workspace* workspace = Application::Instance().CurrentWorkspace() )
							{
								if( Project* prj = workspace->ProjectByName( self->selected_project_ ) )
								{
									std::filesystem::path file_path = location / name;
									std::ofstream         ofs = std::ofstream( file_path, std::ios::binary | std::ios::trunc );

									if( ofs.is_open() )
									{
										prj->files_.emplace_back( std::move( file_path ) );
									}
								}
							}

							self->project_node_to_be_expanded_ = self->selected_project_;
							self->popup_text_.clear();
						}
					);
				}
				ImGui::Separator();
				if( ImGui::MenuItem( "Settings" ) )
				{
					ProjectSettingsModal::Instance().Show( selected_project_ );
				}

				ImGui::EndPopup();
			}

			if( open_workspace_rename_popup )    ImGui::OpenPopup( "Rename Workspace" );
			else if( open_project_rename_popup ) ImGui::OpenPopup( "Rename Project" );

			if( ImGui::BeginPopupModal( "Rename Workspace" ) )
			{
				ImGui::InputTextWithHint( "##Name", "New Name", &popup_text_ );

				if( ImGui::Button( "Rename", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();

					std::filesystem::path old_path = ( workspace->location_ / workspace->name_ ).replace_extension( Workspace::ext );

					if( std::filesystem::exists( old_path ) )
					{
						std::filesystem::path new_path = ( workspace->location_ / popup_text_ ).replace_extension( Workspace::ext );
						std::filesystem::rename( old_path, new_path );
					}

					workspace->name_ = std::move( popup_text_ );
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
					{
						std::filesystem::path old_path = ( prj->location_ / prj->name_ ).replace_extension( Project::ext );

						if( std::filesystem::exists( old_path ) )
						{
							std::filesystem::path new_path = ( prj->location_ / popup_text_ ).replace_extension( Project::ext );
							std::filesystem::rename( old_path, new_path );
						}

						prj->name_ = std::move( popup_text_ );
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
