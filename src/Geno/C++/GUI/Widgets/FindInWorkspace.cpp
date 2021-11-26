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
#include "Components/Workspace.h"
#include "GUI/PrimaryMonitor.h"
#include "GUI/MainWindow.h"
#include "TextEdit.h"
#include "TitleBar.h"

#include "Auxiliary/ImGuiAux.h"

#include <filesystem>
#include <iostream>
#include <sstream>

//////////////////////////////////////////////////////////////////////////

FindInWorkspace::FindInWorkspace( void )
{
} // FindInWorkspace

//////////////////////////////////////////////////////////////////////////

FindInWorkspace::~FindInWorkspace( void )
{

} // ~FindInWorkspace

//////////////////////////////////////////////////////////////////////////

void FindInWorkspace::Show( bool* pOpen )
{
	if( !pOpen && !Application::Instance().CurrentWorkspace() )
		return;

	//////////////////////////////////////////////////////////////////////////

	m_WorkspacePath = Application::Instance().CurrentWorkspace()->m_Location.string();

	ImGuiIO& rIo = ImGui::GetIO();

	ImGuiWindowFlags Flags = ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin( "Find Files in Workspace", pOpen, Flags );

	ImGui::SetWindowSize( ImVec2( 900, 400 ), ImGuiCond_FirstUseEver );
	ImGui::SetWindowPos( ImVec2( rIo.DisplaySize.x * 0.5f, rIo.DisplaySize.y * 0.5f ), ImGuiCond_FirstUseEver );

	ImGuiTableFlags TableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollX | ImGuiTableFlags_NoBordersInBody;

	if( ImGui::BeginTable( "##FileTable", 3, TableFlags ) )
	{
		ImGui::TableSetupColumn( "File" );
		ImGui::TableSetupColumn( "Path" );
		ImGui::TableSetupColumn( "Last Modified" );
		ImGui::TableHeadersRow();

		int Row = 0;

		for( auto& rEntry : std::filesystem::recursive_directory_iterator( m_WorkspacePath ) )
		{
			std::filesystem::path Path = rEntry;
			std::string Filename  = Path.filename().string();
			std::string Filepath  = Path.string().c_str();
			std::string Extension = Path.extension().string();

			if( Path.has_extension() && Extension == ".cpp" || Extension == ".hpp" || Extension == ".cxx" || Extension == ".h" || Extension == ".c" )
			{
				Row++;

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
							pOpen = false;
						}

						if( ImGui::IsItemHovered() )
						{
							ImGui::SetMouseCursor( ImGuiMouseCursor_Hand );
						}
					}
					else if( Column == 1 )
					{
						ImGui::TableSetColumnIndex( Column );
						ImGui::Text( "%s", Filepath );
					}
					else
					{
						ImGui::TableSetColumnIndex( Column );

						struct stat TimeResult;

						if( stat( Path.string().c_str(), &TimeResult ) == 0 )
						{
							auto mod_time = TimeResult.st_mtime;

							std::stringstream SS;
							SS << std::put_time( std::localtime( &mod_time ), "%d/%m/%Y" );

							ImGui::Text( "%s", SS.str().c_str() );
						}

					}
				}
			}
		}
		ImGui::EndTable();
	}

	ImGui::End();
} // Show

//////////////////////////////////////////////////////////////////////////
