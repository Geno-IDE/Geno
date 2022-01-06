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
#include "Components/Workspace.h"
#include "Discord/DiscordRPC.h"
#include "GUI/MainWindow.h"
#include "GUI/Modals//MessageModal.h"
#include "GUI/Modals/BuildMatrixModal.h"
#include "GUI/Modals/NewItemModal.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/Modals/ProjectSettingsModal.h"
#include "GUI/Widgets/TextEdit.h"
#include "GUI/Widgets/TitleBar.h"
#include "WidgetCommands/OutlinerCommands.h"

#include <fstream>
#include <sstream>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <stb_image.h>

//////////////////////////////////////////////////////////////////////////

WorkspaceOutliner::WorkspaceOutliner( void )
	: IWidget                ( std::filesystem::current_path() / "WorkspaceOutliner.json" )
	, m_IconTextureWorkspace ( STBAux::LoadImageTexture( "Icons/Workspace.png" ) )
	, m_IconTextureProject   ( STBAux::LoadImageTexture( "Icons/Project.png" ) )
	, m_IconTextureFileFilter( STBAux::LoadImageTexture( "Icons/FileFilterColored.png" ) )
	, m_IconTextureSourceFile( STBAux::LoadImageTexture( "Icons/SourceFile.png" ) )
{
	if( std::filesystem::exists( m_JsonFile ) )
	{
		rapidjson::Document Doc;

		std::ifstream     jsonFile( m_JsonFile, std::ios::in );
		std::stringstream Content;
		Content << jsonFile.rdbuf();
		jsonFile.close();
		Doc.Parse( Content.str().c_str() );

		for( auto It = Doc.MemberBegin(); It < Doc.MemberEnd(); ++It )
		{
			ReadSettings( It );
		}
	}

} // WorkspaceOutliner

//////////////////////////////////////////////////////////////////////////

WorkspaceOutliner::~WorkspaceOutliner( void )
{
	JSONSerializer Serializer( m_JsonFile );
	WriteSettings( Serializer );

} // ~WorkspaceOutliner

//////////////////////////////////////////////////////////////////////////

static void UpdateSelectedNode( INode*& rSelectedNode, INode* pNode, uint32_t ID )
{
	for( INode*& rNode : pNode->m_pChildren )
	{
		if( rNode->m_Id == ID )
		{
			rSelectedNode = rNode;
			return;
		}
		else
			UpdateSelectedNode( rSelectedNode, rNode, ID );
	}

} // UpdateSelectedNode

//////////////////////////////////////////////////////////////////////////

void WorkspaceOutliner::Show( bool* pOpen )
{
	ImGui::SetNextWindowSize( ImVec2( 350, 196 * 4 ), ImGuiCond_FirstUseEver );

	if( ImGui::Begin( "Workspace", pOpen ) )
	{
		if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
		{
			if( !m_pSelectedNode ) { m_pSelectedNode = pWorkspace; }

			const std::string WorkspaceIDString = pWorkspace->m_Name + "##WKS_" + pWorkspace->m_Name;
			static bool       ForceFocusRename  = false;

			if( m_Actions.size() == 0 )
			{
				auto Undo = [ this ]( void )
				{
					m_UndoCommandStack.UndoCommand( m_RedoCommandStack );
				};

				auto Redo = [ this ]( void )
				{
					m_RedoCommandStack.RedoCommand( m_UndoCommandStack );
				};

				auto RenameNode = [ & ]( void )
				{
					m_RenameNode     = true;
					m_RenameText     = m_pSelectedNode->m_Name;
					ForceFocusRename = true;
				};

				m_Actions = {
					{ "Undo", Undo },
					{ "Redo", Redo },
					{ "RenameNode", RenameNode }
				};
			}

			if( m_KeyBindings.size() == 0 )
			{
				m_KeyBindings = {
					{ { GLFW_KEY_LEFT_CONTROL, GLFW_KEY_Z }, "Undo" },
					{ { GLFW_KEY_LEFT_CONTROL, GLFW_KEY_Y }, "Redo" },
					{ { GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_Z }, "Redo" },
					{ { GLFW_KEY_LEFT_ALT, GLFW_KEY_R }, "RenameNode" }
				};
			}

			Observe();

			if( ImGui::IsKeyPressed( GLFW_KEY_UP ) )
			{
				for( int i = 1; i < ( int )m_NodesSequence.size(); ++i )
				{
					if( m_NodesSequence[ i ] == m_pSelectedNode->m_Id )
					{
						if( m_NodesSequence[ i - 1 ] == pWorkspace->m_Id )
							m_pSelectedNode = pWorkspace;
						else
						{
							UpdateSelectedNode( m_pSelectedNode, pWorkspace, m_NodesSequence[ i - 1 ] );
							break;
						}
					}
				}
			}

			if( ImGui::IsKeyPressed( GLFW_KEY_DOWN ) )
			{
				for( int i = 0; i < ( int )m_NodesSequence.size() - 1; ++i )
				{
					if( m_NodesSequence[ i ] == m_pSelectedNode->m_Id )
					{
						UpdateSelectedNode( m_pSelectedNode, pWorkspace, m_NodesSequence[ i + 1 ] );
						break;
					}
				}
			}

			if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_LeftArrow ) ) )
				m_pSelectedNode->m_ExpandNode = false;

			if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_RightArrow ) ) )
				m_pSelectedNode->m_ExpandNode = true;

			m_NodesSequence.clear();

			bool ToRenameWorkspace = pWorkspace->m_Name == m_pSelectedNode->m_Name && m_RenameNode;

			auto RenameWorkspaceFunc = [ & ]()
			{
				if( ForceFocusRename )
				{
					ImGui::SetKeyboardFocusHere();
					ForceFocusRename = false;
				}

				ImGuiAux::RenameTree( m_RenameText, m_RenameNode, [ this, &pWorkspace ]()
					{
						if( m_RenameText != pWorkspace->m_Name )
						{
							m_UndoCommandStack.DoCommand( new OutlinerCommands::RenameNodeCommand( m_RenameText, m_pSelectedNode ) );
						}

						return true;
					} );
			};

		    ImGui::SetNextItemOpen( pWorkspace->m_ExpandNode, pWorkspace->m_ExpandNode ? ImGuiCond_Always : ImGuiCond_None );

			if( ImGuiAux::PushTreeWithIcon( WorkspaceIDString.c_str(), m_IconTextureWorkspace, ToRenameWorkspace, m_pSelectedNode == pWorkspace, &pWorkspace->m_ExpandNode ) )
			{
				if( ToRenameWorkspace ) { RenameWorkspaceFunc(); }

				m_NodesSequence.push_back( pWorkspace->m_Id );

				if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
				{
					m_ShowNodeContextMenu = true;
					m_pSelectedNode       = pWorkspace;
				}
				else if( ImGui::IsItemClicked( ImGuiMouseButton_Left ) )
				{
					m_pSelectedNode = pWorkspace;
				}

				for( INode*& rProject : pWorkspace->m_pChildren )
				{
					if( !rProject ) { continue; }

					auto RenameProjectFunc = [ & ]()
					{
						if( ForceFocusRename )
						{
							ImGui::SetKeyboardFocusHere();
							ForceFocusRename = false;
						}

						ImGuiAux::RenameTree( m_RenameText, m_RenameNode, [ this, &pWorkspace ]()
							{
								if( m_RenameText == m_pSelectedNode->m_Name )
									return true;

								if( pWorkspace->ChildByName( m_RenameText ) )
								{
									m_RenameText = m_pSelectedNode->m_Name;
									return false;
								}

								m_UndoCommandStack.DoCommand( new OutlinerCommands::RenameNodeCommand( m_RenameText, m_pSelectedNode ) );

								return true;
							} );
					};

					bool ToRenameProject = rProject == m_pSelectedNode && m_RenameNode;

					const std::string ProjectIDString = rProject->m_Name + "##PRJ_" + rProject->m_Name;

					ImGui::SetNextItemOpen( rProject->m_ExpandNode, rProject->m_ExpandNode ? ImGuiCond_Always : ImGuiCond_None );

					if( ImGuiAux::PushTreeWithIcon( ProjectIDString.c_str(), m_IconTextureProject, ToRenameProject, m_pSelectedNode == rProject, &rProject->m_ExpandNode, rProject->m_pChildren.size() ) )
					{
						if( ToRenameProject ) { RenameProjectFunc(); }

						m_NodesSequence.push_back( rProject->m_Id );

						if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
						{
							m_ShowNodeContextMenu = true;
							m_pSelectedNode       = rProject;
						}
						else if( ImGui::IsItemClicked( ImGuiMouseButton_Left ) )
						{
							m_pSelectedNode = rProject;
						}

						auto RenameFileFilterFunc = [ & ]()
						{
							if( ForceFocusRename )
							{
								ImGui::SetKeyboardFocusHere();
								ForceFocusRename = false;
							}

							ImGuiAux::RenameTree( m_RenameText, m_RenameNode, [ & ]()
								{
									if( m_RenameText == m_pSelectedNode->m_Name )
										return true;

									if( rProject->ChildByName( m_RenameText ) )
									{
										m_RenameText = m_pSelectedNode->m_Name;
										return false;
									}

									m_UndoCommandStack.DoCommand( new OutlinerCommands::RenameNodeCommand( m_RenameText, m_pSelectedNode ) );
									return true;
								} );
						};

						auto FileFunc = [ this ]( INode*& rFile )
						{
							if( !rFile ) { return; }

							m_NodesSequence.push_back( rFile->m_Id );

							bool ToRenameFile  = m_RenameNode && rFile == m_pSelectedNode;
							bool ColorFileText = MainWindow::Instance().pTextEdit->GetActiveFilePath() == rFile->m_Location / rFile->m_Name && m_pSelectedNode != rFile;

							if( ColorFileText )
								ImGui::PushStyleColor( ImGuiCol_Text, { 0.2f, 0.6f, 0.8f, 1.0f } );

							const bool FileTreeOpened = ImGuiAux::PushTreeWithIcon( rFile->m_Name.c_str(), m_IconTextureSourceFile, ToRenameFile, m_pSelectedNode == rFile, &rFile->m_ExpandNode, false );

							if( ColorFileText )
								ImGui::PopStyleColor();

							if( ImGui::IsItemHovered() )
							{
								ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
							}

							if( ImGui::IsItemClicked() )
							{
								m_pSelectedNode    = rFile;
								auto& ShowTextEdit = MainWindow::Instance().pTitleBar->ShowTextEdit;
								if( !ShowTextEdit )
								{
									ShowTextEdit = true;
									MainWindow::Instance().pTextEdit->Show( &ShowTextEdit );
								}

								MainWindow::Instance().pTextEdit->AddFile( rFile->m_Location / rFile->m_Name );
							}

							if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
							{
								m_pSelectedNode       = rFile;
								m_ShowNodeContextMenu = true;
							}

							if( FileTreeOpened )
							{
								ImGui::TreePop();
							}

							if( ToRenameFile )
							{
								if( ForceFocusRename )
								{
									ImGui::SetKeyboardFocusHere();
									ForceFocusRename = false;
								}

								ImGuiAux::RenameTree( m_RenameText, m_RenameNode, [ & ]()
									{
										if( m_RenameText == m_pSelectedNode->m_Name )
											return true;

										if( rFile->m_pParent->ChildByName( m_RenameText ) )
										{
											m_RenameText = m_pSelectedNode->m_Name;
											return false;
										}

										m_UndoCommandStack.DoCommand( new OutlinerCommands::RenameNodeCommand( m_RenameText, m_pSelectedNode ) );
										return true;
									} );
							}
						};

						std::function< void( INode*& ) > FileFilterFunc = [ & ]( INode*& rNode )
						{
							if( !rNode ) { return; }

							bool ToRenameFileFilter = m_RenameNode && m_pSelectedNode == rNode;

							const std::string FileFilterIDString = rNode->m_Name + "##FILTER_" + rNode->m_Name;

							ImGui::SetNextItemOpen( rNode->m_ExpandNode, rNode->m_ExpandNode ? ImGuiCond_Always : ImGuiCond_None );

							if( ImGuiAux::PushTreeWithIcon( FileFilterIDString.c_str(), m_IconTextureFileFilter, ToRenameFileFilter, m_pSelectedNode == rNode, &rNode->m_ExpandNode, rNode->m_pChildren.size() ) )
							{
								if( ToRenameFileFilter ) { RenameFileFilterFunc(); }

								m_NodesSequence.push_back( rNode->m_Id );

								if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
								{
									m_ShowNodeContextMenu = true;
									m_pSelectedNode       = rNode;
								}
								else if( ImGui::IsItemClicked( ImGuiMouseButton_Left ) )
								{
									m_pSelectedNode = rNode;
								}

								for( INode*& rFileFilterChild : rNode->m_pChildren )
								{
									if( rFileFilterChild->m_Kind == NodeKind::FileFilter )
									{
										FileFilterFunc( rFileFilterChild );
									}
									else if( rFileFilterChild->m_Kind == NodeKind::File )
									{
										FileFunc( rFileFilterChild );
									}
								}

								ImGui::TreePop();
							}
							else
							{
								if( ToRenameFileFilter ) { RenameFileFilterFunc(); }

								m_NodesSequence.push_back( rNode->m_Id );

								if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
								{
									m_ShowNodeContextMenu = true;
									m_pSelectedNode       = rNode;
								}
								else if( ImGui::IsItemClicked( ImGuiMouseButton_Left ) )
								{
									m_pSelectedNode = rNode;
								}
							}
						};

						for( int i = 1; i < ( int )rProject->m_pChildren.size(); ++i )
						{
							FileFilterFunc( rProject->m_pChildren[ i ] );
						}

						// Draw The Default FileFilter Files
						for( INode*& rFile : rProject->m_pChildren[ 0 ]->m_pChildren )
							FileFunc( rFile );

						ImGui::TreePop();
					}
					else
					{
						if( ToRenameProject ) { RenameProjectFunc(); }

						m_NodesSequence.push_back( rProject->m_Id );

						if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
						{
							m_ShowNodeContextMenu = true;
							m_pSelectedNode       = rProject;
						}
						else if( ImGui::IsItemClicked( ImGuiMouseButton_Left ) )
						{
							m_pSelectedNode = rProject;
						}
					}
				}

				ImGui::TreePop();
			}
			else
			{
				if( ToRenameWorkspace ) { RenameWorkspaceFunc(); }

				m_NodesSequence.push_back( pWorkspace->m_Id );

				if( ImGui::IsItemClicked( ImGuiMouseButton_Right ) )
				{
					m_ShowNodeContextMenu = true;
					m_pSelectedNode       = pWorkspace;
				}
				else if( ImGui::IsItemClicked( ImGuiMouseButton_Left ) )
				{
					m_pSelectedNode = pWorkspace;
				}
			}

			if( m_ShowNodeContextMenu )
			{
				if( m_pSelectedNode->m_Kind == NodeKind::Workspace )
					ImGui::OpenPopup( "WorkspaceContextMenu" );
				else if( m_pSelectedNode->m_Kind == NodeKind::Project )
					ImGui::OpenPopup( "ProjectContextMenu" );
				else if( m_pSelectedNode->m_Kind == NodeKind::FileFilter )
					ImGui::OpenPopup( "FileFilterContextMenu" );
				else if( m_pSelectedNode->m_Kind == NodeKind::File )
					ImGui::OpenPopup( "FileContextMenu" );

				m_ShowNodeContextMenu = false;
			}

			if( ImGui::BeginPopup( "WorkspaceContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					m_RenameNode          = true;
					m_ShowNodeContextMenu = false;
					m_RenameText          = m_pSelectedNode->m_Name;
					ForceFocusRename      = true;

				}
				if( ImGui::MenuItem( "Add Project" ) )
				{
					OpenFileModal::Instance().Show( "Add Project", "*.gprj", [ this ]( const std::filesystem::path& rPath )
						{ m_UndoCommandStack.DoCommand( new OutlinerCommands::AddNodeCommand( NodeKind::Project, rPath.stem().string(), rPath.parent_path().string(), m_pSelectedNode ) ); } );

					m_ShowNodeContextMenu         = false;
					m_pSelectedNode->m_ExpandNode = true;
				}
				if( ImGui::MenuItem( "New Project" ) )
				{
					NewItemModal::Instance().Show( "New Project", ".gprj", pWorkspace->m_Location, [ this ]( const std::string& rName, const std::filesystem::path& rLocation )
						{ m_UndoCommandStack.DoCommand( new OutlinerCommands::NewNodeCommand( NodeKind::Project, rName, rLocation, m_pSelectedNode ) ); } );

					m_ShowNodeContextMenu         = false;
					m_pSelectedNode->m_ExpandNode = true;
				}

				ImGui::Separator();

				if( ImGui::MenuItem( "Build Matrix" ) )
				{
					BuildMatrixModal::Instance().Show();
					m_ShowNodeContextMenu = false;
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopup( "ProjectContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					m_RenameNode          = true;
					m_ShowNodeContextMenu = false;
					m_RenameText          = m_pSelectedNode->m_Name;
					ForceFocusRename      = true;
				}

				if( ImGui::MenuItem( "New File Filter" ) )
				{
					size_t Count = 0;
					for( INode*& rFileFilter : m_pSelectedNode->m_pChildren )
					{
						if( rFileFilter->m_Name == "File Filter " + std::to_string( Count ) )
							Count++;
					}

					std::string FilterName = "File Filter " + std::to_string( Count );
					m_UndoCommandStack.DoCommand( new OutlinerCommands::NewNodeCommand( NodeKind::FileFilter, FilterName, {}, m_pSelectedNode ) );
					m_RenameNode                  = true;
					ForceFocusRename              = true;
					m_RenameText                  = FilterName;
					m_pSelectedNode->m_ExpandNode = true;
					m_pSelectedNode               = m_pSelectedNode->ChildByName( FilterName );
					m_ShowNodeContextMenu         = false;
				}

				if( ImGui::MenuItem( "New File" ) )
				{
					NewItemModal::Instance().Show( "New File", nullptr, m_pSelectedNode->m_Location, [ this ]( const std::string& rName, const std::filesystem::path& rLocation )
						{ m_UndoCommandStack.DoCommand( new OutlinerCommands::NewNodeCommand( NodeKind::File, rName, rLocation, m_pSelectedNode->m_pChildren[ 0 ] ) ); } );

					m_ShowNodeContextMenu         = false;
					m_pSelectedNode->m_ExpandNode = true;
				}

				if( ImGui::MenuItem( "Add File" ) )
				{
					OpenFileModal::Instance().Show( "Add File", nullptr, [ this ]( const std::filesystem::path& rFile )
						{ m_UndoCommandStack.DoCommand( new OutlinerCommands::AddNodeCommand( NodeKind::File, rFile.filename().string(), rFile.parent_path(), m_pSelectedNode->m_pChildren[ 0 ] ) ); } );

					m_ShowNodeContextMenu         = false;
					m_pSelectedNode->m_ExpandNode = true;
				}

				if( ImGui::MenuItem( "Remove" ) )
				{
					m_UndoCommandStack.DoCommand( new OutlinerCommands::RemoveNodeCommand( m_pSelectedNode->m_Name, m_pSelectedNode->m_pParent ) );
					m_pSelectedNode       = m_pSelectedNode->m_pParent;
					m_ShowNodeContextMenu = false;
				}

				ImGui::Separator();

				if( ImGui::MenuItem( "Settings" ) )
				{
					ProjectSettingsModal::Instance().Show( m_pSelectedNode->m_Name );
					m_ShowNodeContextMenu = false;
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopup( "FileFilterContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					m_RenameNode          = true;
					m_ShowNodeContextMenu = false;
					m_RenameText          = m_pSelectedNode->m_Name;
					ForceFocusRename      = true;
				}

				if( ImGui::MenuItem( "Remove" ) )
				{
					m_UndoCommandStack.DoCommand( new OutlinerCommands::RemoveNodeCommand( m_pSelectedNode->m_Name, m_pSelectedNode->m_pParent ) );

					m_pSelectedNode       = m_pSelectedNode->m_pParent;
					m_ShowNodeContextMenu = false;
				}

				if( ImGui::MenuItem( "New File Filter" ) )
				{
					size_t Count = 0;
					for( INode*& rFileFilter : m_pSelectedNode->m_pChildren )
					{
						if( rFileFilter->m_Name == "File Filter " + std::to_string( Count ) )
						{
							Count++;
						}
					}

					std::string FileFilterName = "File Filter " + std::to_string( Count );
					m_UndoCommandStack.DoCommand( new OutlinerCommands::NewNodeCommand( NodeKind::FileFilter, FileFilterName, {}, m_pSelectedNode ) );
					m_RenameNode                  = true;
					ForceFocusRename              = true;
					m_RenameText                  = FileFilterName;
					m_pSelectedNode->m_ExpandNode = true;
					m_pSelectedNode               = m_pSelectedNode->ChildByName( FileFilterName );
					m_ShowNodeContextMenu         = false;
				}

				if( ImGui::MenuItem( "New File" ) )
				{
					NewItemModal::Instance().Show( "New File", nullptr, std::filesystem::canonical( m_pSelectedNode->m_Location ), [ this ]( const std::string& rName, const std::filesystem::path& rLocation )
						{ m_UndoCommandStack.DoCommand( new OutlinerCommands::NewNodeCommand( NodeKind::File, rName, rLocation, m_pSelectedNode ) ); } );

					m_ShowNodeContextMenu         = false;
					m_pSelectedNode->m_ExpandNode = true;
				}

				if( ImGui::MenuItem( "Add File" ) )
				{
					OpenFileModal::Instance().Show( "Add File", nullptr, [ this ]( const std::filesystem::path& rPath )
						{ m_UndoCommandStack.DoCommand( new OutlinerCommands::AddNodeCommand( NodeKind::File, rPath.filename().string(), rPath.parent_path(), m_pSelectedNode ) ); } );
					m_ShowNodeContextMenu         = false;
					m_pSelectedNode->m_ExpandNode = true;
				}

				ImGui::EndPopup();
			}
			else if( ImGui::BeginPopup( "FileContextMenu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
			{
				if( ImGui::MenuItem( "Rename" ) )
				{
					m_RenameNode          = true;
					m_ShowNodeContextMenu = false;
					m_RenameText          = m_pSelectedNode->m_Name;
					ForceFocusRename      = true;
				}

				if( ImGui::MenuItem( "Remove" ) )
				{
					std::string Message = "Are you sure you want to remove '" + m_pSelectedNode->m_Name + "'";

					MessageModal::Instance().ShowMessage( Message, "Remove", [ & ]()
						{ m_UndoCommandStack.DoCommand( new OutlinerCommands::RemoveNodeCommand( m_pSelectedNode->m_Name, m_pSelectedNode->m_pParent ) ); } );

					m_ShowNodeContextMenu = false;
				}

				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();

	if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
		DiscordRPC::Instance().m_Workspace = pWorkspace->m_Name;
	else
		DiscordRPC::Instance().m_Workspace = "No Workspace";

} // Show

//////////////////////////////////////////////////////////////////////////

void WorkspaceOutliner::WriteSettings( JSONSerializer& rSerializer )
{
	Workspace*            pWorkspace = Application::Instance().CurrentWorkspace();
	std::filesystem::path Workspace  = pWorkspace->m_Location / ( pWorkspace->m_Name + ".gwks" );
	rSerializer.Add( "Workspace", Workspace.string() );

	WriteKeyBindings( rSerializer );

} // WriteSettings

//////////////////////////////////////////////////////////////////////////

void WorkspaceOutliner::ReadSettings( const rapidjson::Value::ConstMemberIterator& rIt )
{
	const std::string MemberName = rIt->name.GetString();
	if( MemberName == "Workspace" )
		Application::Instance().LoadWorkspace( std::string( rIt->value.GetString() ) );
	else if( MemberName == "KeyBindings" )
		ReadKeyBindings( rIt );

} // ReadSettings
