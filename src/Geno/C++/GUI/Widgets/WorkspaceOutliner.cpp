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

#include "Application.h"
#include "Auxiliary/ImGuiAux.h"
#include "Auxiliary/STBAux.h"
#include "Components/Project.h"
#include "GUI/MainWindow.h"
#include "GUI/Modals//MessageModal.h"
#include "GUI/Modals/BuildMatrixModal.h"
#include "GUI/Modals/FileFilterSettingsModal.h"
#include "GUI/Modals/NewItemModal.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/Modals/ProjectSettingsModal.h"
#include "GUI/Widgets/MainMenuBar.h"
#include "GUI/Widgets/TextEdit.h"
#include "WidgetCommands/WOC.h"

#include <GLFW/glfw3.h>
#include <fstream>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <stb_image.h>

//////////////////////////////////////////////////////////////////////////

WorkspaceOutliner::WorkspaceOutliner( void )
	: m_IconTextureWorkspace ( STBAux::LoadImageTexture( "Icons/Workspace.png" ) )
	, m_IconTextureProject   ( STBAux::LoadImageTexture( "Icons/Project.png" ) )
	, m_IconTextureFileFilter( STBAux::LoadImageTexture( "Icons/FileFilterColored.png" ) )
	, m_IconTextureSourceFile( STBAux::LoadImageTexture( "Icons/SourceFile.png" ) )
{
} // WorkspaceOutliner

//////////////////////////////////////////////////////////////////////////

void WorkspaceOutliner::Show( bool* pOpen )
{
	ImGui::SetNextWindowSize( ImVec2( 350, 196 * 4 ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( "Workspace", pOpen ) )
	{
		if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
		{
			const std::string WorkspaceIDString         = pWorkspace->m_Name + "##WKS_" + pWorkspace->m_Name;
			bool              ShowWorkspaceContextMenu  = false;
			bool              ShowProjectContextMenu    = false;
			bool              ShowFileFilterContextMenu = false;
			bool              ShowFileContextMenu       = false;
			static bool       RenameWorkspace           = false;
			static bool       RenameProject             = false;
			static bool       RenameFileFilter          = false;
			static bool       RenameFile                = false;
			static bool       ForceFocusRename          = false;

			ImGui::SetNextItemOpen( true, m_ExpandWorkspaceNode ? ImGuiCond_Always : ImGuiCond_Appearing );
			m_ExpandWorkspaceNode = false;

			if( ImGui::IsKeyPressed( GLFW_KEY_LEFT_CONTROL ) && ImGui::IsKeyPressed( GLFW_KEY_Z ) )
			{
				m_UndoCommandStack.UndoCommand( m_RedoCommandStack );
			}

			if( ImGui::IsKeyPressed( GLFW_KEY_LEFT_CONTROL ) )
			{
				if( ( ImGui::IsKeyPressed( GLFW_KEY_LEFT_SHIFT ) && ImGui::IsKeyPressed( GLFW_KEY_Z ) ) || ImGui::IsKeyPressed( GLFW_KEY_Y ) )
				{
					m_RedoCommandStack.RedoCommand( m_UndoCommandStack );
				}
			}

			auto RenameWorkspaceFunc = [ & ]()
			{
				if( ForceFocusRename )
				{
					ImGui::SetKeyboardFocusHere();
					ForceFocusRename = false;
				}

				ImGuiAux::RenameTree( m_RenameText, RenameWorkspace, [ & ]()
					{
						if( m_RenameText != pWorkspace->m_Name )
						{
							m_UndoCommandStack.DoCommand( new WOC::RenameItemCommand( WOC::ItemType_Workspace, pWorkspace->m_Name, m_RenameText ) );
						}

						return true;
					} );
			};

			auto RenameProjectFunc = [ & ]()
			{
				if( ForceFocusRename )
				{
					ImGui::SetKeyboardFocusHere();
					ForceFocusRename = false;
				}

				ImGuiAux::RenameTree( m_RenameText, RenameProject, [ & ]()
					{
						if( m_RenameText == m_SelectedProjectName )
							return true;

						if( pWorkspace->ProjectByName( m_RenameText ) )
						{
							m_RenameText = m_SelectedProjectName;
							return false;
						}

						m_UndoCommandStack.DoCommand( new WOC::RenameItemCommand( WOC::ItemType_Project, m_SelectedProjectName, m_RenameText ) );

						return true;
					} );
			};

			auto RenameFilterFunc = [ & ]()
			{
				if( ForceFocusRename )
				{
					ImGui::SetKeyboardFocusHere();
					ForceFocusRename = false;
				}

				ImGuiAux::RenameTree( m_RenameText, RenameFileFilter, [ & ]()
					{
						if( m_RenameText == m_SelectedFileFilterName )
							return true;

						Project* pProject = pWorkspace->ProjectByName( m_SelectedProjectName );

						if( pProject->FileFilterByName( m_RenameText ) )
						{
							m_RenameText = m_SelectedFileFilterName.string();
							return false;
						}

						m_UndoCommandStack.DoCommand( new WOC::RenameItemCommand( WOC::ItemType_FileFilter, m_SelectedFileFilterName, m_RenameText, m_SelectedProjectName ) );
						return true;
					} );
			};

			auto PushFilterFunc = [ & ]( Project& rProject, FileFilter& rFileFilter, std::string_view Name ) -> bool
			{
				const std::string FilterIDString = std::string( Name ) + "##FILTER_" + std::string( Name );
				bool              ToRenameFilter = RenameFileFilter && rFileFilter.Name == m_SelectedFileFilterName;

				if( ImGuiAux::PushTreeWithIcon( FilterIDString.c_str(), m_IconTextureFileFilter, ToRenameFilter ) )
				{
					if( ToRenameFilter )
						RenameFilterFunc();

					if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
					{
						ShowFileFilterContextMenu = true;
						m_SelectedProjectName     = rProject.m_Name;
						m_SelectedFileFilterName  = rFileFilter.Name;
					}

					return true;
				}
				else
				{
					if( ToRenameFilter )
						RenameFilterFunc();

					if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
					{
						ShowFileFilterContextMenu = true;
						m_SelectedProjectName     = rProject.m_Name;
						m_SelectedFileFilterName  = rFileFilter.Name;
					}

					return false;
				}
			};

			auto PushFilterFilesFunc = [ & ]( Project& rProject, FileFilter& rFileFilter )
			{
				for( std::filesystem::path& rFile : rFileFilter.Files )
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
						if( ForceFocusRename )
						{
							ImGui::SetKeyboardFocusHere();
							ForceFocusRename = false;
						}

						ImGuiAux::RenameTree( m_RenameText, RenameFile, [ & ]()
							{
								if( m_RenameText == m_SelectedFile.filename().string() )
									return true;

								Project*    pProject    = pWorkspace->ProjectByName( m_SelectedProjectName );
								FileFilter* pFileFilter = pProject->FileFilterByName( m_SelectedFileFilterName );

								if( std::filesystem::exists( pProject->m_Location / pFileFilter->Path / m_RenameText ) )
								{
									m_RenameText = m_SelectedFile.filename().string();
									return false;
								}

								m_UndoCommandStack.DoCommand( new WOC::RenameItemCommand( WOC::ItemType_File, m_SelectedFile, pProject->m_Location / pFileFilter->Path / m_RenameText, m_SelectedProjectName, m_SelectedFileFilterName ) );
								return true;
							} );
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
						ShowFileContextMenu      = true;
						m_SelectedFileFilterName = rFileFilter.Name;
						m_SelectedFile           = rFile;
						m_SelectedProjectName    = rProject.m_Name;
					}

					if( FileTreeOpened )
					{
						ImGui::TreePop();
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

						std::vector< std::pair< FileFilter*, bool > > PreviousFileFilters;

						for( FileFilter& rFileFilter : rProject.m_FileFilters )
						{
							if( !rFileFilter.Name.empty() )
							{
								if( PreviousFileFilters.empty() )
								{
									const std::string FilterName = rFileFilter.Name.string();
									PreviousFileFilters.push_back( { &rFileFilter, PushFilterFunc( rProject, rFileFilter, FilterName ) } );
								}
								else
								{
									std::string RelativeNameString = std::filesystem::relative( rFileFilter.Name, PreviousFileFilters.back().first->Name ).string();
									std::replace( RelativeNameString.begin(), RelativeNameString.end(), static_cast< char >( std::filesystem::path::preferred_separator ), '/' );

									while( RelativeNameString.find( "../" ) < RelativeNameString.size() )
									{
										std::pair< FileFilter*, bool >& FileFilter = PreviousFileFilters.back();
										if( FileFilter.second )
										{
											PushFilterFilesFunc( rProject, *FileFilter.first );
											ImGui::TreePop();
										}
										PreviousFileFilters.pop_back();

										if( PreviousFileFilters.empty() )
										{
											RelativeNameString = rFileFilter.Name.string();
										}
										else
										{
											RelativeNameString = std::filesystem::relative( rFileFilter.Name, PreviousFileFilters.back().first->Name ).string();
											std::replace( RelativeNameString.begin(), RelativeNameString.end(), static_cast< char >( std::filesystem::path::preferred_separator ), '/' );
										}
									}

									if( PreviousFileFilters.empty() || PreviousFileFilters.back().second )
									{
										PreviousFileFilters.push_back( { &rFileFilter, PushFilterFunc( rProject, rFileFilter, RelativeNameString ) } );
									}
								}
							}
							else
							{
								while( !PreviousFileFilters.empty() )
								{
									std::pair< FileFilter*, bool >& FileFilter = PreviousFileFilters.back();
									if( FileFilter.second )
									{
										PushFilterFilesFunc( rProject, *FileFilter.first );
										ImGui::TreePop();
									}
									PreviousFileFilters.pop_back();
								}

								PushFilterFilesFunc( rProject, rFileFilter );
							}
						}

						while( !PreviousFileFilters.empty() )
						{
							std::pair< FileFilter*, bool >& FileFilter = PreviousFileFilters.back();
							if( FileFilter.second )
							{
								PushFilterFilesFunc( rProject, *FileFilter.first );
								ImGui::TreePop();
							}
							PreviousFileFilters.pop_back();
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
			else if( ShowFileFilterContextMenu )
				ImGui::OpenPopup( "FileFilterContextMenu" );
			else if( ShowFileContextMenu )
				ImGui::OpenPopup( "FileContextMenu" );

			if( ImGui::BeginPopup( "WorkspaceContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					RenameWorkspace          = RenameProject || RenameFileFilter || RenameFile ? false : true;
					ShowWorkspaceContextMenu = false;

					if( RenameWorkspace )
					{
						m_RenameText     = pWorkspace->m_Name;
						ForceFocusRename = true;
					}
				}
				if( ImGui::MenuItem( "Add Project" ) )
				{
					OpenFileModal::Instance().Show( "Add Project", "*.gprj", [ this ]( const std::filesystem::path& rPath )
						{ m_UndoCommandStack.DoCommand( new WOC::AddItemCommand( WOC::ItemType_Project, rPath ) ); } );
				}
				if( ImGui::MenuItem( "New Project" ) )
				{
					NewItemModal::Instance().Show( "New Project", ".gprj", pWorkspace->m_Location, [ this ]( const std::string& rName, const std::filesystem::path& rLocation )
						{
							// Automatically expand tree if adding an item for the first time
							m_ExpandWorkspaceNode = true;
							m_UndoCommandStack.DoCommand( new WOC::NewItemCommand( WOC::ItemType_Project, rLocation, rName ) );
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
					RenameProject          = RenameWorkspace || RenameFileFilter || RenameFile ? false : true;
					ShowProjectContextMenu = false;

					if( RenameProject )
					{
						m_RenameText     = m_SelectedProjectName;
						ForceFocusRename = true;
					}
				}

				if( ImGui::MenuItem( "New File Filter" ) )
				{
					Project* pProject = pWorkspace->ProjectByName( m_SelectedProjectName );

					size_t Count = 0;
					for( FileFilter& rFileFilter : pProject->m_FileFilters )
					{
						if( rFileFilter.Name == "File Filter" + std::to_string( Count ) )
						{
							Count++;
						}
					}

					std::string FilterName = "File Filter" + std::to_string( Count );
					m_UndoCommandStack.DoCommand( new WOC::NewItemCommand( WOC::ItemType_FileFilter, {}, FilterName, m_SelectedProjectName ) );
					RenameFileFilter         = true;
					ForceFocusRename         = true;
					m_RenameText             = FilterName;
					m_SelectedFileFilterName = FilterName;

					ShowProjectContextMenu = false;
				}

				if( ImGui::MenuItem( "New File" ) )
				{
					Project* pProject = pWorkspace->ProjectByName( m_SelectedProjectName );

					NewItemModal::Instance().Show( "New File", nullptr, pProject->m_Location, [ this ]( const std::string& rName, const std::filesystem::path& rLocation )
						{
							m_UndoCommandStack.DoCommand( new WOC::NewItemCommand( WOC::ItemType_File, rLocation, rName, m_SelectedProjectName, "" ) );
							m_ProjectNodeToBeExpanded = m_SelectedProjectName;
						} );

					ShowProjectContextMenu = false;
				}

				if( ImGui::MenuItem( "Add File" ) )
				{
					OpenFileModal::Instance().Show( "Add File", nullptr, [ this ]( const std::filesystem::path& rFile )
						{
							m_UndoCommandStack.DoCommand( new WOC::AddItemCommand( WOC::ItemType_File, rFile, m_SelectedProjectName, "" ) );
							m_ProjectNodeToBeExpanded = m_SelectedProjectName;
						} );
					ShowProjectContextMenu = false;
				}

				if( ImGui::MenuItem( "Remove" ) )
				{
					Project*                    pProject = pWorkspace->ProjectByName( m_SelectedProjectName );
					const std::filesystem::path Path     = ( pProject->m_Location / pProject->m_Name ).replace_extension( Project::EXTENSION );
					m_UndoCommandStack.DoCommand( new WOC::RemoveItemCommand( WOC::ItemType_Project, Path ) );
				}

				ImGui::Separator();

				if( ImGui::MenuItem( "Settings" ) )
				{
					ProjectSettingsModal::Instance().Show( m_SelectedProjectName );
					ShowProjectContextMenu = false;
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopup( "FileFilterContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					RenameFileFilter          = RenameWorkspace || RenameProject || RenameFile ? false : true;
					ShowFileFilterContextMenu = false;

					if( RenameFileFilter )
					{
						m_RenameText     = m_SelectedFileFilterName.string();
						ForceFocusRename = true;
					}
				}

				if( ImGui::MenuItem( "Remove" ) )
				{
					if( Project* pSelectedProject = pWorkspace->ProjectByName( m_SelectedProjectName ) )
					{
						m_UndoCommandStack.DoCommand( new WOC::RemoveItemCommand( WOC::ItemType_FileFilter, m_SelectedFileFilterName, m_SelectedProjectName ) );
						pSelectedProject->RemoveFileFilter( m_SelectedFileFilterName );
					}

					m_SelectedFileFilterName.clear();
					ShowFileFilterContextMenu = false;
				}

				if( ImGui::MenuItem( "New File Filter" ) )
				{
					Project*    pProject    = pWorkspace->ProjectByName( m_SelectedProjectName );
					FileFilter* pFileFilter = pProject->FileFilterByName( m_SelectedFileFilterName );

					size_t Count = 0;
					for( FileFilter& rFileFilter : pProject->m_FileFilters )
					{
						if( rFileFilter.Name == pFileFilter->Name.string() + "/File Filter" + std::to_string( Count ) )
						{
							Count++;
						}
					}

					std::string FileFilterName = pFileFilter->Name.string() + "/File Filter" + std::to_string( Count );
					m_UndoCommandStack.DoCommand( new WOC::NewItemCommand( WOC::ItemType_FileFilter, {}, FileFilterName, m_SelectedProjectName ) );
					RenameFileFilter         = true;
					ForceFocusRename         = true;
					m_RenameText             = FileFilterName;
					m_SelectedFileFilterName = FileFilterName;

					ShowFileFilterContextMenu = false;
				}

				if( ImGui::MenuItem( "New File" ) )
				{
					Project*    pProject    = pWorkspace->ProjectByName( m_SelectedProjectName );
					FileFilter* pFileFilter = pProject->FileFilterByName( m_SelectedFileFilterName );

					NewItemModal::Instance().Show( "New File", nullptr, std::filesystem::canonical( pProject->m_Location / pFileFilter->Path ), [ this ]( const std::string& rName, const std::filesystem::path& rLocation )
						{
							m_UndoCommandStack.DoCommand( new WOC::NewItemCommand( WOC::ItemType_File, rLocation, rName, m_SelectedProjectName, m_SelectedFileFilterName ) );
							m_ProjectNodeToBeExpanded = m_SelectedProjectName;
						} );
					ShowFileFilterContextMenu = false;
				}

				if( ImGui::MenuItem( "Add File" ) )
				{
					OpenFileModal::Instance().Show( "Add File", nullptr, [ this ]( const std::filesystem::path& rPath )
						{
							m_UndoCommandStack.DoCommand( new WOC::AddItemCommand( WOC::ItemType_File, rPath, m_SelectedProjectName, m_SelectedFileFilterName ) );
							m_ProjectNodeToBeExpanded = m_SelectedProjectName;
						} );
					ShowFileFilterContextMenu = false;
				}

				ImGui::Separator();

				if( ImGui::MenuItem( "Settings" ) )
				{
					FileFilterSettingsModal::Instance().Show( m_SelectedProjectName, m_SelectedFileFilterName );
					ShowFileFilterContextMenu = false;
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopup( "FileContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					RenameFile          = RenameWorkspace || RenameProject || RenameFileFilter ? false : true;
					ShowFileContextMenu = false;

					if( RenameFile )
					{
						m_RenameText     = m_SelectedFile.filename().string();
						ForceFocusRename = true;
					}
				}

				if( ImGui::MenuItem( "Remove" ) )
				{
					std::string Message = "Are you sure you want to remove '" + m_SelectedFile.filename().string() + "'";

					MessageModal::Instance().ShowMessage( Message, "Remove", [ & ]()
						{ m_UndoCommandStack.DoCommand( new WOC::RemoveItemCommand( WOC::ItemType_File, m_SelectedFile, m_SelectedProjectName, m_SelectedFileFilterName ) ); } );

					ShowFileContextMenu = false;
				}

				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();

} // Show
