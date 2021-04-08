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

#include "OpenFileModal.h"

#include "GUI/MainWindow.h"

#include <Common/LocalAppData.h>

#include <fstream>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

//////////////////////////////////////////////////////////////////////////

OpenFileModal::OpenFileModal( void )
{

#if defined( _WIN32 )

	m_DrivesBufferSize = GetLogicalDriveStringsA( 0, nullptr );
	m_DrivesBuffer      = std::unique_ptr< char[] >( new char[ m_DrivesBufferSize ] );
	m_DrivesBufferSize = GetLogicalDriveStringsA( static_cast< DWORD >( m_DrivesBufferSize ), &m_DrivesBuffer[ 0 ] );

#endif // _WIN32

	m_CurrentDirectory = RootDirectory();

} // OpenFileModal

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::SetCurrentDirectory( std::filesystem::path Directory )
{
	m_CurrentDirectory = std::move( Directory );

} // SetCurrentDirectory

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::RequestFile( std::string Title, void* pUser, Callback Callback )
{
	if( Open() )
	{
		// Make sure we don't have a pre-selected directory from last popup
		if( m_DirectoryRequested )
			m_SelectedPath.clear();

		m_pUser              = pUser;
		m_Callback           = Callback;
		m_DirectoryRequested = false;
		m_Title              = std::move( Title );
	}

} // RequestFile

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::RequestDirectory( std::string Title, void* pUser, Callback Callback )
{
	if( Open() )
	{
		// Make sure we don't have a pre-selected file from last popup
		if( !m_DirectoryRequested )
			m_SelectedPath = m_CurrentDirectory;

		m_pUser              = pUser;
		m_Callback           = Callback;
		m_DirectoryRequested = true;
		m_Title              = std::move( Title );
	}

} // RequestDirectory

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::OnClose( void )
{
	m_Callback = nullptr;
	m_pUser    = nullptr;

} // OnClose

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::UpdateDerived( void )
{
	if( ImGui::BeginChild( 1 , ImVec2( 0, -24 ) ) )
	{
		MainWindow::Instance().PushHorizontalLayout();

	#if defined( _WIN32 )

		if( ImGui::BeginChild( 2, ImVec2( 100, 0 ) ) )
		{
			size_t Index = 0;
			for( char* pIt = &m_DrivesBuffer[ 0 ]; pIt < &m_DrivesBuffer[ m_DrivesBufferSize ]; pIt += ( strlen( pIt ) + 1 ), ++Index )
			{
				bool selected = ( m_CurrentDriveIndex == Index );

				if( ImGui::Selectable( pIt, &selected ) )
				{
					m_CurrentDriveIndex = Index;
					m_CurrentDirectory   = RootDirectory();
				}
			}

		} ImGui::EndChild();

	#endif // _WIN32

		if( ImGui::BeginChild( 3, ImVec2( 0, 0 ), false, ImGuiWindowFlags_MenuBar ) )
		{
			ImGui::PushStyleColor( ImGuiCol_MenuBarBg, ImVec4( 0, 0, 0, 1 ) );

			if( ImGui::BeginMenuBar() )
			{
				if( ImGui::Button( "New Folder" ) )
				{
					m_EditingPath           = m_CurrentDirectory / "New Folder";
					m_EditingPathIsFolder = true;
					m_ChangeEditFocus      = true;
				}

				if( !m_DirectoryRequested && ImGui::Button( "New File" ) )
				{
					m_EditingPath           = m_CurrentDirectory / "New File.txt";
					m_EditingPathIsFolder = false;
					m_ChangeEditFocus      = true;
				}

				if( !m_EditingPath.empty() )
				{
					std::string FileName = m_EditingPath.filename().string();

					if( m_ChangeEditFocus )
					{
						ImGui::SetKeyboardFocusHere();

						if( ImGuiInputTextState* pState = ImGui::GetInputTextState( ImGui::GetID( "##NewFileName" ) ) )
						{
							pState->ID = 0;
						}

						m_ChangeEditFocus = false;
					}

					if( ImGui::InputText( "##NewFileName", &FileName, ImGuiInputTextFlags_EnterReturnsTrue ) )
					{
						const std::filesystem::path& rNewPath = m_EditingPath.replace_filename( FileName );

						if( std::filesystem::exists( rNewPath ) )
						{
							m_ChangeEditFocus = true;
						}
						else
						{
							if( m_EditingPathIsFolder )
							{
								if( std::filesystem::create_directory( rNewPath ) ) m_EditingPath.clear();
								else                                                m_ChangeEditFocus = true;
							}
							else
							{
								const std::ofstream OutputFileStream( rNewPath );

								if( OutputFileStream.is_open() ) m_EditingPath.clear();
								else                             m_ChangeEditFocus = true;
							}
						}
					}
					else if( ImGui::IsItemDeactivated() )
					{
						m_EditingPath.clear();
						m_ChangeEditFocus = false;
					}

				} ImGui::EndMenuBar();
			}
			ImGui::PopStyleColor();

//////////////////////////////////////////////////////////////////////////

			std::filesystem::directory_iterator  RootDirectoryIterator( m_CurrentDirectory, std::filesystem::directory_options::skip_permission_denied );
			std::filesystem::path                ParentDirectory = m_CurrentDirectory.parent_path();
			std::vector< std::filesystem::path > DirectoryPaths;
			std::vector< std::filesystem::path > FilePaths;

			if( m_CurrentDirectory != ParentDirectory )
			{
				if( ImGui::Selectable( ".." ) )
				{
					m_CurrentDirectory = ParentDirectory;
				}
			}

			for( const std::filesystem::directory_entry& rEntry : RootDirectoryIterator )
			{
				/**/ if( rEntry.is_directory() )    DirectoryPaths.push_back( rEntry );
				else if( rEntry.is_regular_file() ) FilePaths.push_back( rEntry );
			}

			for( const std::filesystem::path& rDirectoryEntry : DirectoryPaths )
			{
				const std::string FileName = rDirectoryEntry.filename().string();

				if( m_DirectoryRequested )
				{
					bool selected = m_SelectedPath == rDirectoryEntry;

					if( ImGui::Selectable( FileName.c_str(), &selected, ImGuiSelectableFlags_AllowDoubleClick ) )
					{
						const bool DoubleClicked = ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left );

						if( DoubleClicked ) m_CurrentDirectory = rDirectoryEntry;
						else                m_SelectedPath     = rDirectoryEntry;
					}
				}
				else
				{
					if( ImGui::Selectable( FileName.c_str() ) )
					{
						m_CurrentDirectory = rDirectoryEntry;
					}
				}
			}

			if( !m_DirectoryRequested )
			{
				ImGui::Separator();

				for( const std::filesystem::path& rFilePath : FilePaths )
				{
					std::string filename = rFilePath.filename().string();
					bool        selected = rFilePath == m_SelectedPath;

					if( ImGui::Selectable( filename.c_str(), &selected ) )
					{
						m_SelectedPath = rFilePath;
					}
				}
			}

		} ImGui::EndChild();

		MainWindow::Instance().PopHorizontalLayout();

	} ImGui::EndChild();

	if( ImGui::BeginChild( 4 , ImVec2( 0, 20 ) ) )
	{
		const bool DisableOkButton = m_SelectedPath.empty() || !std::filesystem::exists( m_SelectedPath );

		if( DisableOkButton )
		{
			ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
		}

		if( ImGui::Button( "OK", ImVec2( 80, 0 ) ) )
		{
			if( m_Callback )
				m_Callback( m_SelectedPath, m_pUser );

			Close();
		}

		if( DisableOkButton )
		{
			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
		}

		ImGui::SameLine();
		if( ImGui::Button( "Cancel", ImVec2( 80, 0 ) ) )
		{
			Close();
		}

		const std::string SelectedPathString = m_SelectedPath.string();

		ImGui::SameLine();
		ImGui::Text( SelectedPathString.c_str() );

	} ImGui::EndChild();

} // UpdateDerived

//////////////////////////////////////////////////////////////////////////

std::filesystem::path OpenFileModal::RootDirectory( void )
{

#if defined( _WIN32 )

	char* pDrive = &m_DrivesBuffer[ 0 ];
	char* pEnd   = &m_DrivesBuffer[ m_DrivesBufferSize ];

	for( size_t i = 0; i < m_CurrentDriveIndex && pDrive < pEnd; pDrive += ( strlen( pDrive ) + 1 ), ++i );

	return std::filesystem::path( pDrive, pDrive + strlen( pDrive ) );

#else // _WIN32

	return std::filesystem::path( "/" );

#endif // _WIN32

} // RootDirectory
