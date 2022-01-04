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

#include "FindInWorkspace.h"
#include "Application.h"
#include "Components/Project.h"
#include "Components/Workspace.h"
#include "GUI/PrimaryMonitor.h"
#include "GUI/MainWindow.h"
#include "TextEdit.h"
#include "TitleBar.h"
#include "Auxiliary/ImGuiAux.h"

#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <regex>

//////////////////////////////////////////////////////////////////////////

bool CheckExtension( const std::string& rExtension )
{
	return rExtension == ".cpp" || rExtension == ".hpp" || rExtension == ".cxx" || rExtension == ".h" || rExtension == ".c";

} // CheckExtension

//////////////////////////////////////////////////////////////////////////

void FindInWorkspace::Show( bool* pOpen )
{
	if( pOpen == 0 /* false */ || Application::Instance().CurrentWorkspace() == nullptr )
		return;

	//////////////////////////////////////////////////////////////////////////

	m_WorkspacePath = "";
	m_pWorkspace = Application::Instance().CurrentWorkspace();
	m_WorkspacePath = m_pWorkspace->m_Location.string();

	ImGuiIO& rIo = ImGui::GetIO();

	ImGuiWindowFlags Flags = ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin( "Find Files in Workspace", pOpen, Flags );

	ImGui::SetWindowSize( ImVec2( 900, 400 ), ImGuiCond_FirstUseEver );
	ImGui::SetWindowPos( ImVec2( rIo.DisplaySize.x * 0.5f, rIo.DisplaySize.y * 0.5f ), ImGuiCond_FirstUseEver );

	ImGuiTableFlags TableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX | ImGuiTableFlags_NoBordersInBody;

	if( ImGui::BeginTable( "##FileTable", 3, TableFlags, ImVec2( ImGui::GetWindowSize().x - 4, ImGui::GetWindowSize().y * 0.85f ) ) )
	{
		ImGui::TableSetupColumn( "File" );
		ImGui::TableSetupColumn( "Path" );
		ImGui::TableSetupColumn( "Last Modified" );
		ImGui::TableHeadersRow();

		int TableRow = 0;

		std::vector< std::vector< std::filesystem::path > > SrcPaths;

		for( INode*& rNode : m_pWorkspace->m_pChildren )
		{
			Project* pProject = ( Project* )rNode;
			SrcPaths.push_back( pProject->FindSourceFolders() );
		}

		for ( auto& rProjectSrcPaths : SrcPaths )
		{
			for ( auto& rFile : rProjectSrcPaths )
			{
				if( std::filesystem::exists( rFile ) )
				{
					for( auto& rEntry : std::filesystem::recursive_directory_iterator( rFile ) )
					{
						std::filesystem::path Path = rEntry;
						std::string Filename  = Path.filename().string();
						std::string Filepath  = Path.string().c_str();
						std::string Extension = Path.extension().string();

						if( Path.has_extension() && CheckExtension( Extension ) && m_TextFilter.PassFilter( Filename.c_str() ) )
						{
							TableRow++;

							ImGui::TableNextRow();

							for( int Column = 0; Column < 3; Column++ )
							{
								if( Column == 0 )
								{
									ImGui::TableSetColumnIndex( Column );

									if( ImGui::Selectable( Filename.c_str() ) )
									{
										auto& ShowTextEdit = MainWindow::Instance().pTitleBar->ShowTextEdit;
										if( !ShowTextEdit )
										{
											ShowTextEdit = true;
											MainWindow::Instance().pTextEdit->Show( &ShowTextEdit );
										}

										MainWindow::Instance().pTextEdit->AddFile( Path );

										auto& ShowFindInWrks = MainWindow::Instance().pTitleBar->ShowFindInWorkspaceWindow;
										ShowFindInWrks = false;
										pOpen = 0 /* false */;
									}

									if( ImGui::IsItemHovered() )
									{
										ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
									}
								}
								else if( Column == 1 )
								{
									ImGui::TableSetColumnIndex( Column );
									ImGui::Text( "%s", Filepath.c_str() );
								}
								else
								{
									ImGui::TableSetColumnIndex( Column );

									struct stat TimeResult;

									if( stat( Path.string().c_str(), &TimeResult ) == 0 )
									{
										auto ModTime = TimeResult.st_mtime;

										std::stringstream SS;
										SS << std::put_time( std::localtime( &ModTime ), "%d/%m/%Y" );
										SS << std::put_time( std::localtime( &ModTime ), "  %T" );

										ImGui::Text( "%s", SS.str().c_str() );
									}

								}
							}
						}
					}
				}
			}
		}

		ImGui::EndTable();
	}


	ImGui::Text( "Search for files..." );
	ImGui::SameLine();
	m_TextFilter.Draw( "##search" );

	if( ImGui::IsWindowFocused( ImGuiFocusedFlags_RootAndChildWindows ) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked( 0 ) )
		ImGui::SetKeyboardFocusHere( 0 );

	ImGui::End();

} // Show

//////////////////////////////////////////////////////////////////////////
