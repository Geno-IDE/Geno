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

#include "Auxiliary/ImGuiAux.h"
#include "Components/Project.h"
#include "GUI/Modals/NewItemModal.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/Modals/ProjectSettingsModal.h"
#include "GUI/Modals/WorkspaceSettingsModal.h"
#include "GUI/Widgets/TextEdit.h"
#include "GUI/Widgets/MainMenuBar.h"
#include "GUI/MainWindow.h"
#include "Application.h"

#include <fstream>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <stb_image.h>

//////////////////////////////////////////////////////////////////////////

static void LoadTexture( const char* pFilePath, Texture2D& rTexture )
{
	int      Width;
	int      Height;
	stbi_uc* pData = stbi_load( pFilePath, &Width, &Height, nullptr, STBI_rgb_alpha );

	if( pData )
	{
		rTexture.SetPixels( GL_RGBA8, Width, Height, GL_RGBA, pData );
		free( pData );
	}

} // LoadTexture

//////////////////////////////////////////////////////////////////////////

WorkspaceOutliner::WorkspaceOutliner( void )
{
	LoadTexture( "Icons/Workspace.png",  m_IconTextureWorkspace );
	LoadTexture( "Icons/Project.png",    m_IconTextureProject );
	LoadTexture( "Icons/SourceFile.png", m_IconTextureSourceFile );

} // WorkspaceOutliner

//////////////////////////////////////////////////////////////////////////

void WorkspaceOutliner::Show( bool* pOpen )
{
	ImGui::SetNextWindowSize( ImVec2( 350, 196*4 ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( "Workspace", pOpen ) )
	{
		if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
		{
			const std::string WorkspaceIDString    = pWorkspace->m_Name + "##WKS_" + pWorkspace->m_Name;
			bool              WorkspaceItemHovered = false;
			const Project*    pHoveredProject      = nullptr;

			ImGui::SetNextItemOpen( true, m_ExpandWorkspaceNode ? ImGuiCond_Always : ImGuiCond_Appearing );
			m_ExpandWorkspaceNode = false;

			if( ImGuiAux::PushTreeWithIcon( WorkspaceIDString.c_str(), m_IconTextureWorkspace ) )
			{
				WorkspaceItemHovered = ImGui::IsItemHovered();

				for( const Project& rProject : pWorkspace->m_Projects )
				{
					const std::string ProjectIDString = rProject.m_Name + "##PRJ_" + rProject.m_Name;

					ImGui::SetNextItemOpen( true, m_ProjectNodeToBeExpanded == rProject.m_Name ? ImGuiCond_Always : ImGuiCond_Appearing );
					m_ProjectNodeToBeExpanded.clear();

					if( ImGuiAux::PushTreeWithIcon( ProjectIDString.c_str(), m_IconTextureProject ) )
					{
						if( ImGui::IsItemHovered() )
							pHoveredProject = &rProject;

						for( const std::filesystem::path& rFile : rProject.m_Files )
						{
							const std::string FileString = rFile.filename().string();

							const bool FileTreeOpened = ImGuiAux::PushTreeWithIcon( FileString.c_str(), m_IconTextureSourceFile, false );

							if( ImGui::IsItemHovered() )
							{
								ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );

								m_SelectedFile        = rFile;
								m_SelectedProjectName = rProject.m_Name;

								if( ImGui::IsItemClicked() )
								{
									auto& ShowTextEdit = MainWindow::Instance().pMenuBar->ShowTextEdit;
									if( !ShowTextEdit )
									{
										ShowTextEdit = true;
										MainWindow::Instance().pTextEdit->Show( &ShowTextEdit );
									}
									
									MainWindow::Instance().pTextEdit->AddFile( rFile );
								}
							}

							if( FileTreeOpened )
							{
								ImGui::TreePop();
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

			if( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
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
				else if( !m_SelectedFile.empty() )
				{
					ImGui::OpenPopup( "FileContextMenu", ImGuiPopupFlags_MouseButtonRight );
				}
			}

			if( ImGui::BeginPopup( "WorkspaceContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					OpenWorkspaceRenamePopup = true;
				}
				if( ImGui::MenuItem( "New Project" ) )
				{
					NewItemModal::Instance().RequestPath( "New Project", pWorkspace->m_Location, this,
						[]( std::string Name, std::filesystem::path Location, void* pUser )
						{
							WorkspaceOutliner* pSelf = static_cast< WorkspaceOutliner* >( pUser );

							if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
							{
								// Automatically expand tree if adding an item for the first time
								pSelf->m_ExpandWorkspaceNode = true;

								pWorkspace->NewProject( std::move( Location ), std::move( Name ) );
								pWorkspace->Serialize();
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
			else if( ImGui::BeginPopup( "FileContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Remove" ) )
				{
					if( Project* pSelectedProject = pWorkspace->ProjectByName( m_SelectedProjectName ) )
					{
						auto SelectedFileIt = std::find_if( pSelectedProject->m_Files.begin(), pSelectedProject->m_Files.end(), [ this ]( const std::filesystem::path& rPath ) { return rPath == m_SelectedFile; } );
						if( SelectedFileIt != pSelectedProject->m_Files.end() )
						{
							pSelectedProject->m_Files.erase( SelectedFileIt );
						}
					}

					m_SelectedFile.clear();
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
