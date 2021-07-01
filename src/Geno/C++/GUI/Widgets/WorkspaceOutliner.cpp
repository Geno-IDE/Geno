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
#include "Auxiliary/STBAux.h"
#include "Components/Project.h"
#include "GUI/Modals/BuildMatrixModal.h"
#include "GUI/Modals/NewItemModal.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/Modals/ProjectSettingsModal.h"
#include "GUI/Widgets/TextEdit.h"
#include "GUI/Widgets/MainMenuBar.h"
#include "GUI/MainWindow.h"
#include "Application.h"

#include <fstream>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <stb_image.h>

//////////////////////////////////////////////////////////////////////////

WorkspaceOutliner::WorkspaceOutliner( void )
	: m_IconTextureWorkspace ( STBAux::LoadImageTexture( "Icons/Workspace.png" ) )
	, m_IconTextureProject   ( STBAux::LoadImageTexture( "Icons/Project.png" ) )
	, m_IconTextureSourceFile( STBAux::LoadImageTexture( "Icons/SourceFile.png" ) )
{

} // WorkspaceOutliner

//////////////////////////////////////////////////////////////////////////

void WorkspaceOutliner::Show( bool* pOpen )
{
	ImGui::SetNextWindowSize( ImVec2( 350, 196*4 ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( "Workspace", pOpen ) )
	{
		if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
		{
			const std::string WorkspaceIDString        = pWorkspace->m_Name + "##WKS_" + pWorkspace->m_Name;
			bool              ShowWorkspaceContextMenu = false;
			bool              ShowProjectContextMenu   = false;
			bool              ShowFileContextMenu      = false;
			static bool       RenameWorkspace          = false;
			static bool       RenameProject            = false;
			static bool       RenameFile               = false;

			ImGui::SetNextItemOpen( true, m_ExpandWorkspaceNode ? ImGuiCond_Always : ImGuiCond_Appearing );
			m_ExpandWorkspaceNode = false;

			auto RenameWorkspaceFunc = [ & ]()
			{
				if( ImGuiAux::RenameTree( m_RenameText ) )
				{
					if( m_RenameText != pWorkspace->m_Name )
					{
						const std::filesystem::path OldPath = ( pWorkspace->m_Location / pWorkspace->m_Name ).replace_extension( Workspace::EXTENSION );

						if( std::filesystem::exists( OldPath ) )
						{
							const std::filesystem::path NewPath = ( pWorkspace->m_Location / m_RenameText ).replace_extension( Workspace::EXTENSION );
							std::filesystem::rename( OldPath, NewPath );
						}

						pWorkspace->m_Name = std::move( m_RenameText );
						pWorkspace->Serialize();
					}

					RenameWorkspace = false;
					m_RenameText.clear();
				}
			};

			auto RenameProjectFunc = [ & ]()
			{
				if( ImGuiAux::RenameTree( m_RenameText ) )
				{
					//Check For Project With Same Name If It Doesnt Exist Than Only Execute The If Block
					Project* pProject = pWorkspace->ProjectByName( m_RenameText );

					//Enter The If Block Only When There Is No Project With Same Name Or The SelectedProject Name == Changed Project Name
					if( !pProject || m_RenameText == m_SelectedProjectName )
					{
						pProject = pWorkspace->ProjectByName( m_SelectedProjectName );
						if( m_RenameText != m_SelectedProjectName ) //Rename Only If The Changed Name If Not Same
						{
							const std::filesystem::path OldPath = ( pProject->m_Location / pProject->m_Name ).replace_extension( Project::EXTENSION );

							if( std::filesystem::exists( OldPath ) )
							{
								const std::filesystem::path NewPath = ( pProject->m_Location / m_RenameText ).replace_extension( Project::EXTENSION );
								std::filesystem::rename( OldPath, NewPath );
							}

							pProject->m_Name = std::move( m_RenameText );
							pProject->Serialize();
							pWorkspace->Serialize();
						}

						RenameProject = false;
						m_SelectedProjectName.clear();
						m_RenameText.clear();
					}
					else
					{
						m_RenameText = m_SelectedProjectName;
					}
				}
			};

			if( ImGuiAux::PushTreeWithIcon( WorkspaceIDString.c_str(), m_IconTextureWorkspace, RenameWorkspace ) )
			{
				if( RenameWorkspace )
					RenameWorkspaceFunc();

				ShowWorkspaceContextMenu = ImGui::IsItemClicked( ImGuiMouseButton_Right );

				for( Project& rProject : pWorkspace->m_Projects )
				{
					const std::string ProjectIDString = rProject.m_Name + "##PRJ_" + rProject.m_Name;

					ImGui::SetNextItemOpen( true, m_ProjectNodeToBeExpanded == rProject.m_Name ? ImGuiCond_Always : ImGuiCond_Appearing );
					m_ProjectNodeToBeExpanded.clear();

					bool ToRenameProject = RenameProject && rProject.m_Name == m_SelectedProjectName;

					if( ImGuiAux::PushTreeWithIcon( ProjectIDString.c_str(), m_IconTextureProject, ToRenameProject ) )
					{
						if( ToRenameProject )
							RenameProjectFunc();

						if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
						{
							ShowProjectContextMenu = true;
							m_SelectedProjectName  = rProject.m_Name;
						}

						for( std::filesystem::path& rFile : rProject.m_Files )
						{
							const std::string FileString    = rFile.filename().string();
							bool              ToRenameFile  = RenameFile && rFile == m_SelectedFile;
							bool              ColorFileText = MainWindow::Instance().pTextEdit->GetActiveFilePath() == rFile;

							if( ColorFileText )
								ImGui::PushStyleColor( ImGuiCol_Text, { 0.2f, 0.6f, 0.8f, 1.0f } );

							const bool FileTreeOpened = ImGuiAux::PushTreeWithIcon( FileString.c_str(), m_IconTextureSourceFile, ToRenameFile, false );

							if( ColorFileText )
								ImGui::PopStyleColor();

							if( ToRenameFile )
							{
								if( ImGuiAux::RenameTree( m_RenameText ) )
								{
									Project* pProject            = pWorkspace->ProjectByName( m_SelectedProjectName );
									bool     FileExistsInProject = std::filesystem::exists( pProject->m_Location / m_RenameText );

									if( !FileExistsInProject || m_RenameText == m_SelectedFile.filename().string() )
									{
										if( m_RenameText != m_SelectedFile.filename().string() ) //Rename Only If The Changed Name If Not Same
										{
											const std::filesystem::path OldPath = m_SelectedFile;

											if( std::filesystem::exists( OldPath ) )
											{
												const std::filesystem::path NewPath = pProject->m_Location / m_RenameText;
												std::filesystem::rename( OldPath, NewPath );
											}

											rFile = pProject->m_Location / m_RenameText;
											pProject->Serialize();

											MainWindow::Instance().pTextEdit->ReplaceFile( m_SelectedFile, rFile );
										}

										m_SelectedFile.clear();
										m_RenameText.clear();
										RenameFile = false;
									}
									else
									{
										m_RenameText = m_SelectedFile.filename().string();
									}
								}
							}

							if( ImGui::IsItemHovered() )
							{
								ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
							}

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

							if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
							{
								ShowFileContextMenu   = true;
								m_SelectedFile        = rFile;
								m_SelectedProjectName = rProject.m_Name;
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
						if( ToRenameProject )
							RenameProjectFunc();

						if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
						{
							ShowProjectContextMenu = true;
							m_SelectedProjectName  = rProject.m_Name;
						}
					}
				}

				ImGui::TreePop();
			}
			else
			{
				if( RenameWorkspace )
					RenameWorkspaceFunc();

				ShowWorkspaceContextMenu = ImGui::IsItemClicked( ImGuiMouseButton_Right );
			}

			if( ShowWorkspaceContextMenu )
				ImGui::OpenPopup( "WorkspaceContextMenu" );
			else if( ShowProjectContextMenu )
				ImGui::OpenPopup( "ProjectContextMenu" );
			else if( ShowFileContextMenu )
				ImGui::OpenPopup( "FileContextMenu" );

			if( ImGui::BeginPopup( "WorkspaceContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					RenameWorkspace          = RenameProject || RenameFile ? false : true;
					ShowWorkspaceContextMenu = false;

					if( RenameWorkspace )
					{
						m_RenameText = pWorkspace->m_Name;
					}
				}
				if( ImGui::MenuItem( "New Project" ) )
				{
					NewItemModal::Instance().RequestPath( "New Project", pWorkspace->m_Location, this, []( std::string Name, std::filesystem::path Location, void* pUser )
						{
							WorkspaceOutliner* pSelf = static_cast< WorkspaceOutliner* >( pUser );

							if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
							{
								// Automatically expand tree if adding an item for the first time
								pSelf->m_ExpandWorkspaceNode = true;

								pWorkspace->NewProject( std::move( Location ), std::move( Name ) );
								pWorkspace->Serialize();
							}
						} );
					ShowWorkspaceContextMenu = false;
				}
				ImGui::Separator();
				if( ImGui::MenuItem( "Build Matrix" ) )
				{
					BuildMatrixModal::Instance().Show();
					ShowWorkspaceContextMenu = false;
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopup( "ProjectContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					RenameProject          = RenameWorkspace || RenameFile ? false : true;
					ShowProjectContextMenu = false;

					if( RenameProject )
					{
						m_RenameText = m_SelectedProjectName;
					}
				}

				if( ImGui::MenuItem( "New File" ) )
				{
					Project* pProject = pWorkspace->ProjectByName( m_SelectedProjectName );

					NewItemModal::Instance().RequestPath( "New File", pProject->m_Location, this, []( std::string Name, std::filesystem::path Location, void* pUser )
						{
							WorkspaceOutliner* pSelf = static_cast< WorkspaceOutliner* >( pUser );

							if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
							{
								if( Project* pProject = pWorkspace->ProjectByName( pSelf->m_SelectedProjectName ) )
								{
									std::filesystem::path FilePath = Location / Name;
									std::ofstream         OutputFileStream( FilePath, std::ios::binary | std::ios::trunc );

									if( OutputFileStream.is_open() )
									{
										pProject->m_Files.emplace_back( std::move( FilePath ) );
										pProject->Serialize();
									}
								}
							}

							pSelf->m_ProjectNodeToBeExpanded = pSelf->m_SelectedProjectName;
						} );
					ShowProjectContextMenu = false;
				}

				if( ImGui::MenuItem( "Add File" ) )
				{
					OpenFileModal::Instance().RequestFile( "Add File", this, []( const std::filesystem::path& rPath, void* pUser )
						{
							WorkspaceOutliner* pSelf = static_cast< WorkspaceOutliner* >( pUser );

							if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
							{
								Project* pProject = pWorkspace->ProjectByName( pSelf->m_SelectedProjectName );

								pProject->m_Files.push_back( rPath );
								pProject->Serialize();
							}

							pSelf->m_ProjectNodeToBeExpanded = pSelf->m_SelectedProjectName;
						} );
					ShowProjectContextMenu = false;
				}

				ImGui::Separator();

				if( ImGui::MenuItem( "Settings" ) )
				{
					ProjectSettingsModal::Instance().Show( m_SelectedProjectName );
					ShowProjectContextMenu = false;
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopup( "FileContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					RenameFile          = RenameWorkspace || RenameProject ? false : true;
					ShowFileContextMenu = false;

					if( RenameFile )
					{
						m_RenameText = m_SelectedFile.filename().string();
					}
				}

				if( ImGui::MenuItem( "Remove" ) )
				{
					if( Project* pSelectedProject = pWorkspace->ProjectByName( m_SelectedProjectName ) )
					{
						auto SelectedFileIt = std::find_if( pSelectedProject->m_Files.begin(), pSelectedProject->m_Files.end(), [ this ]( const std::filesystem::path& rPath )
							{ return rPath == m_SelectedFile; } );
						if( SelectedFileIt != pSelectedProject->m_Files.end() )
						{
							pSelectedProject->m_Files.erase( SelectedFileIt );
						}
					}

					m_SelectedFile.clear();
					ShowFileContextMenu = false;
				}

				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();

} // Show
