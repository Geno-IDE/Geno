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

#include "WorkspaceOutliner.h"

#include "Components/Project.h"
#include "GUI/Modals/NewItemModal.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/Modals/ProjectSettingsModal.h"
#include "GUI/Modals/WorkspaceSettingsModal.h"
#include "GUI/Widgets/TextEdit.h"
#include "GUI/MainWindow.h"
#include "Application.h"

#include <fstream>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

//////////////////////////////////////////////////////////////////////////

void WorkspaceOutliner::Show( bool* pOpen )
{
	if( ImGui::Begin( "Workspace", pOpen ) )
	{
		if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
		{
			const std::string WorkspaceIDString    = pWorkspace->m_Name + "##WKS_" + pWorkspace->m_Name;
			bool              WorkspaceItemHovered = false;
			const Project*    pHoveredProject      = nullptr;

			if( m_ExpandWorkspaceNode )
			{
				ImGui::SetNextItemOpen( true );
				m_ExpandWorkspaceNode = false;
			}

			if( ImGui::TreeNode( WorkspaceIDString.c_str() ) )
			{
				WorkspaceItemHovered = ImGui::IsItemHovered();

				for( const Project& rProject : pWorkspace->m_Projects )
				{
					const std::string ProjectIDString = rProject.m_Name + "##PRJ_" + rProject.m_Name;

					if( m_ProjectNodeToBeExpanded == rProject.m_Name )
					{
						ImGui::SetNextItemOpen( true );
						m_ProjectNodeToBeExpanded.clear();
					}

					if( ImGui::TreeNode( ProjectIDString.c_str() ) )
					{
						if( ImGui::IsItemHovered() )
							pHoveredProject = &rProject;

						for( const std::filesystem::path& rFile : rProject.m_Files )
						{
							const std::string FileString = rFile.filename().string();

							if( ImGui::Selectable( FileString.c_str() ) )
							{
								MainWindow::Instance().TextEdit.AddFile( rFile );
							}
						}

						ImGui::TreePop();
					}
					else
					{
						if( ImGui::IsItemHovered() )
							pHoveredProject = &rProject;
					}
				}

				ImGui::TreePop();
			}
			else
			{
				WorkspaceItemHovered = ImGui::IsItemHovered();
			}

			// ImGUI seeds the popup IDs by the ID of the last item on the stack, which would be the context menu below
			bool OpenWorkspaceRenamePopup = false;
			bool OpenProjectRenamePopup   = false;

			if( ImGui::IsMouseReleased( ImGuiMouseButton_Right ) )
			{
				if( WorkspaceItemHovered )
				{
					ImGui::OpenPopup( "WorkspaceContextMenu", ImGuiPopupFlags_MouseButtonRight );
				}
				else if( pHoveredProject )
				{
					ImGui::OpenPopup( "ProjectContextMenu", ImGuiPopupFlags_MouseButtonRight );

					m_SelectedProjectName = pHoveredProject->m_Name;
				}
			}

			if( ImGui::BeginPopup( "WorkspaceContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )      OpenWorkspaceRenamePopup = true;
				if( ImGui::MenuItem( "New Project" ) )
				{
					NewItemModal::Instance().RequestPath( "New Project", pWorkspace->m_Location, this,
						[]( std::string name, std::filesystem::path location, void* user )
						{
							WorkspaceOutliner* self = static_cast< WorkspaceOutliner* >( user );

							if( Workspace* workspace = Application::Instance().CurrentWorkspace() )
							{
								// Automatically expand tree if adding an item for the first time
								self->m_ExpandWorkspaceNode = true;

								workspace->NewProject( std::move( location ), std::move( name ) );
								workspace->Serialize();
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
					OpenProjectRenamePopup = true;

				if( ImGui::MenuItem( "New File" ) )
				{
					Project* pProject = pWorkspace->ProjectByName( m_SelectedProjectName );

					NewItemModal::Instance().RequestPath( "New File", pProject->m_Location, this,
						[]( std::string Name, std::filesystem::path Location, void* pUser )
						{
							WorkspaceOutliner* pSelf = static_cast< WorkspaceOutliner* >( pUser );

							if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
							{
								if( Project* pProject = pWorkspace->ProjectByName( pSelf->m_SelectedProjectName ) )
								{
									std::filesystem::path FilePath         = Location / Name;
									std::ofstream         OutputFileStream( FilePath, std::ios::binary | std::ios::trunc );

									if( OutputFileStream.is_open() )
									{
										pProject->m_Files.emplace_back( std::move( FilePath ) );
										pProject->Serialize();
									}
								}
							}

							pSelf->m_ProjectNodeToBeExpanded = pSelf->m_SelectedProjectName;
						}
					);
				}

				if( ImGui::MenuItem( "Add File" ) )
				{
					OpenFileModal::Instance().RequestFile( "Add File", this,
						[]( const std::filesystem::path& rPath, void* pUser )
						{
							WorkspaceOutliner* pSelf = static_cast< WorkspaceOutliner* >( pUser );

							if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
							{
								Project* pProject = pWorkspace->ProjectByName( pSelf->m_SelectedProjectName );

								pProject->m_Files.push_back( rPath );
								pProject->Serialize();
							}

							pSelf->m_ProjectNodeToBeExpanded = pSelf->m_SelectedProjectName;
						}
					);
				}

				ImGui::Separator();

				if( ImGui::MenuItem( "Settings" ) )
				{
					ProjectSettingsModal::Instance().Show( m_SelectedProjectName );
				}

				ImGui::EndPopup();
			}

			if( OpenWorkspaceRenamePopup )    ImGui::OpenPopup( "Rename Workspace" );
			else if( OpenProjectRenamePopup ) ImGui::OpenPopup( "Rename Project" );

			if( ImGui::BeginPopupModal( "Rename Workspace" ) )
			{
				ImGui::InputTextWithHint( "##Name", "New Name", &m_PopupText );

				if( ImGui::Button( "Rename", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();

					const std::filesystem::path OldPath = ( pWorkspace->m_Location / pWorkspace->m_Name ).replace_extension( Workspace::EXTENSION );

					if( std::filesystem::exists( OldPath ) )
					{
						const std::filesystem::path NewPath = ( pWorkspace->m_Location / m_PopupText ).replace_extension( Workspace::EXTENSION );
						std::filesystem::rename( OldPath, NewPath );
					}

					pWorkspace->m_Name = std::move( m_PopupText );
					pWorkspace->Serialize();
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
				ImGui::InputTextWithHint( "##Name", "New Name", &m_PopupText );

				if( ImGui::Button( "Rename", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();

					if( Project* pProject = pWorkspace->ProjectByName( m_SelectedProjectName ) )
					{
						const std::filesystem::path OldPath = ( pProject->m_Location / pProject->m_Name ).replace_extension( Project::EXTENSION );

						if( std::filesystem::exists( OldPath ) )
						{
							const std::filesystem::path NewPath = ( pProject->m_Location / m_PopupText ).replace_extension( Project::EXTENSION );
							std::filesystem::rename( OldPath, NewPath );
						}

						pProject->m_Name = std::move( m_PopupText );
						pProject->Serialize();
					}

					m_PopupText.clear();
					m_SelectedProjectName.clear();
				}

				ImGui::SameLine();
				if( ImGui::Button( "Cancel", ImVec2( 100, 0 ) ) )
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

	} ImGui::End();

} // Show
