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

#include "FileFilterSettingsModal.h"

#include "GUI/Modals/OpenFileModal.h"
#include "Components/Project.h"
#include "Components/Workspace.h"
#include "GUI/MainWindow.h"
#include "Application.h"

#include <array>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

//////////////////////////////////////////////////////////////////////////

void FileFilterSettingsModal::Show( std::string Project, std::filesystem::path FileFilter )
{
	if( Open() )
	{
		m_EditedProject    = std::move( Project );
		m_EditedFileFilter = std::move( FileFilter );
	}
} // Show

//////////////////////////////////////////////////////////////////////////

void FileFilterSettingsModal::UpdateDerived( void )
{
	Workspace* pWorkspace = Application::Instance().CurrentWorkspace();
	if( !pWorkspace )
	{
		ImGui::TextUnformatted( "No active workspace" );
		return;
	}

	Project* pProject = pWorkspace->ProjectByName( m_EditedProject );
	if( !pProject )
	{
		ImGui::Text( "No project found by the name '%s'", m_EditedProject.c_str() );
		return;
	}

	FileFilter* pFileFilter = pProject->FileFilterByName( m_EditedFileFilter );
	if( !pFileFilter )
	{
		std::string EditedFileFilterString = m_EditedFileFilter.string();
		ImGui::Text( "No file filter found by the name '%s'", EditedFileFilterString.c_str() );
		return;
	}

	if( ImGui::BeginChild( 1, ImVec2( 0, -30 ) ) )
	{
		MainWindow::Instance().PushHorizontalLayout();

		if( ImGui::BeginChild( 2 ) )
		{
			if( ImGui::BeginTable( "##PATH_TABLE", 3 ) )
			{
				ImGui::TableSetupColumn( "##PATH_TABLE_1", ImGuiTableColumnFlags_WidthFixed );
				ImGui::TableSetupColumn( "##PATH_TABLE_2", ImGuiTableColumnFlags_WidthStretch );
				ImGui::TableSetupColumn( "##PATH_TABLE_3", ImGuiTableColumnFlags_WidthFixed );
				ImGui::TableNextColumn();
				ImGui::TextUnformatted( "Path" );
				ImGui::TableNextColumn();
				std::filesystem::path FileFilterPath = std::filesystem::canonical(pProject->m_Location / pFileFilter->Path);
				std::string FileFilterPathString = FileFilterPath.string();
				ImGui::SetNextItemWidth( -FLT_MIN );
				ImGui::InputText( "##PATH", &FileFilterPathString );
				ImGui::TableNextColumn();
				if( ImGui::Button( "Browse" ) )
				{
					OpenFileModal::Instance().SetCurrentDirectory( FileFilterPath );
					OpenFileModal::Instance().RequestDirectory( "File Filter Path", this, &FileFilterPathCallback );
				}
				else
				{
					pFileFilter->Path = std::filesystem::relative( FileFilterPathString, pProject->m_Location );
				}
				ImGui::EndTable();
			}
		} ImGui::EndChild();

		MainWindow::Instance().PopHorizontalLayout();
	} ImGui::EndChild();

	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + 4 );
	if( ImGui::Button( "Save & Close", ImVec2(100, 0) ) )
	{
		pProject->Serialize();
		Close();
	}
} // UpdateDerived

//////////////////////////////////////////////////////////////////////////

void FileFilterSettingsModal::OnClose( void )
{
	m_EditedProject.clear();
	m_EditedFileFilter.clear();
} // OnClose

//////////////////////////////////////////////////////////////////////////

void FileFilterSettingsModal::FileFilterPathCallback( const std::filesystem::path& rPath, void* pUser )
{
	FileFilterSettingsModal* pSelf      = static_cast< FileFilterSettingsModal* >( pUser );
	Workspace*               pWorkspace = Application::Instance().CurrentWorkspace();
	if( !pWorkspace )
	{
		return;
	}

	Project* pProject = pWorkspace->ProjectByName( pSelf->m_EditedProject );
	if ( !pProject )
	{
		return;
	}

	FileFilter* pFileFilter = pProject->FileFilterByName( pSelf->m_EditedFileFilter );
	if( !pFileFilter )
	{
		return;
	}

	pFileFilter->Path = std::filesystem::relative( rPath, pProject->m_Location );
}

//////////////////////////////////////////////////////////////////////////
