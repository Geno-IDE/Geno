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

#include "OpenFileModal.h"

#include "Auxiliary/ImGuiAux.h"
#include "Auxiliary/STBAux.h"
#include "GUI/MainWindow.h"

#include <Common/LocalAppData.h>
#include <fstream>
#include <functional>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <sstream>

//////////////////////////////////////////////////////////////////////////

OpenFileModal::OpenFileModal( void )
	: m_IconFolder( STBAux::LoadImageTexture( "Icons/Folder.png" ) )
	, m_IconFile( STBAux::LoadImageTexture( "Icons/File.png" ) )
	, m_IconSearch( STBAux::LoadImageTexture( "Icons/Search.png" ) )
{
#if defined( __linux__ )

	m_Volumes.push_back( "/" );
	m_Volumes.push_back( "/home" );

#endif // __linux__

#if defined( _WIN32 )

	DWORD DrivesSize    = GetLogicalDriveStringsA( 0, NULL );
	LPSTR pDrivesBuffer = new char[ DrivesSize ];
	GetLogicalDriveStringsA( DrivesSize, pDrivesBuffer );
	LPSTR pDrive = pDrivesBuffer;
	while( *pDrive != '\0' )
	{
		m_Volumes.push_back( pDrive );
		pDrive += strlen( pDrive ) + 1;
	}
	delete[] pDrivesBuffer;

#endif // _WIN32

	m_CurrentPath = std::filesystem::current_path();
} // OpenFileModal

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::SetCurrentDirectory( std::filesystem::path Directory )
{
	m_CurrentPath = std::move( Directory );

} // SetCurrentDirectory

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::Show( std::string Title, const char* pFileFilters, Callback Callback )
{
	if( Open() )
	{
		m_Callback = Callback;
		m_Title    = std::move( Title );

		if( pFileFilters )
		{
			m_Title += " - ";
			for( const auto& rFileFilter : std::filesystem::path( pFileFilters ) )
			{
				m_FileFilters[ rFileFilter.string() ] = true;
				m_Title += rFileFilter.string();
			}
		}
	}

} // RequestFile

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::OnClose( void )
{
	m_Callback = {};
	m_FileFilters.clear();
	m_SelectedFile.clear();
	m_Volumes.clear();

} // OnClose

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::UpdateDerived( void )
{
	if( ImGui::BeginChild( 1, ImVec2( 0, -44 ) ) )
	{
		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.25f, 0.25f, 0.25f, 0.7f ) );
		if( ImGui::BeginChild( 2, ImVec2( 150, 0 ) ) ) // Side Panel
		{
			if( ImGui::CollapsingHeader( "Volumes", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick ) )
			{
				if( ImGui::BeginTable( "##Volumes", 1, ImGuiTableFlags_RowBg ) )
				{
					ImGui::TableSetupColumn( "##Volume", ImGuiTableColumnFlags_NoHide );

					for( const auto& rVolume : m_Volumes )
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						if( ImGui::Selectable( rVolume.c_str() ) )
						{
							SetCurrentDirectory( std::filesystem::path( rVolume ) );
						}
					}

					ImGui::EndTable();
				}
			}
		}
		ImGui::EndChild(); // Side Panel
		ImGui::PopStyleColor();

		ImGui::SameLine();

		if( ImGui::BeginChild( 3, ImVec2( 0, 0 ) ) ) // Main Panel
		{
			ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.07f, 0.07f, 0.07f, 0.7f ) );
			if( ImGui::BeginChild( 4, ImVec2( 0, 35 ) ) ) // Browse Panel
			{
				if( ImGui::BeginChild( 5, ImVec2( ImGui::GetContentRegionAvailWidth() - 50.0f, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar ) ) //Browse With Buttons
				{
					if( !m_SearchEnabled )
					{
						for( const auto& rSubPath : m_CurrentPath )
						{
							bool IsActiveFolder = rSubPath == m_CurrentPath.stem();

							ImGuiAux::ButtonData ButtonData;

							if( !IsActiveFolder )
								ButtonData.ColorText = ImGui::GetStyle().Colors[ ImGuiCol_Text ];

							std::string Name    = rSubPath.string();
							float       ButtonX = ImGui::CalcTextSize( Name.c_str() ).x * 1.18f;

#if defined( _WIN32 )

							if( Name == "\\" )
								continue;
							else if( Name.find( ':' ) != std::string::npos )
								Name += "\\";

#endif // _WIN32

							ButtonData.Size     = ImVec2( ButtonX, ImGui::GetContentRegionAvail().y );
							ButtonData.Rounding = 6.0f;

							bool Break = false;

							ImGuiAux::Button( Name.c_str(), ButtonData, [ this, &rSubPath, &Name, &Break ]()
								{
									std::string ActivePath = m_CurrentPath.string();
									ActivePath             = ActivePath.substr( 0, ActivePath.find( rSubPath.string() ) ) + Name;
									SetCurrentDirectory( std::filesystem::path( ActivePath ) );
									Break = true;
								} );

							if( Break )
								break;

							ImGui::SameLine();
						}
					}
					else
					{
						ImGui::SetNextItemWidth( ImGui::GetContentRegionAvailWidth() );
						ImGui::SetCursorPosY( ImGui::GetWindowHeight() * 0.5f - 12.0f );

						ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 4.0f );
						ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 1.2f );
						ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 8.0f, 5.0f ) );
						ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0.2f, 0.6f, 0.8f, 1.0f ) );
						ImGui::InputTextWithHint( "##SearchFile", "Search", &m_SearchResult );
						ImGui::PopStyleVar( 3 );
						ImGui::PopStyleColor();

						if( ImGui::IsKeyPressed( ImGui::GetKeyIndex( ImGuiKey_UpArrow ) ) )
						{
							m_SearchEnabled = false;
							m_SearchResult.clear();
						}
					}
				}
				ImGui::EndChild(); //Browse With Buttons

				ImGui::SameLine();
				ImGui::SeparatorEx( ImGuiSeparatorFlags_Vertical );
				ImGui::SameLine();

				ImGui::PushStyleColor( ImGuiCol_Button, ImGui::GetStyle().Colors[ ImGuiCol_ChildBg ] );
				ImVec2 Size = ImVec2( ImGui::GetFontSize() * m_IconSearch.GetAspectRatio() * 2.5f, ImGui::GetFontSize() * 2.1f );
				if( ImGui::ImageButton( ( ImTextureID )m_IconSearch.GetID(), Size ) )
				{
					m_SearchEnabled = true;
				}
				ImGui::PopStyleColor();
			}
			ImGui::EndChild(); // Browse Panel
			ImGui::PopStyleColor();

			ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.15f, 0.15f, 0.15f, 0.7f ) );
			if( ImGui::BeginChild( 6, ImVec2( 0, 0 ) ) ) // Content Browser
			{
				if( ImGui::BeginTable( "##FileBrowserPanel", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_NoBordersInBodyUntilResize ) )
				{
					ImGui::TableSetupColumn( "Name", ImGuiTableColumnFlags_NoHide );
					ImGui::TableSetupColumn( "Size", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize( "Size" ).x * 2.0f );
					ImGui::TableHeadersRow();

					for( const auto& rCurrentPathIt : std::filesystem::directory_iterator( m_CurrentPath ) )
					{
						std::filesystem::path Path         = rCurrentPathIt.path();
						bool                  IsHiddenFile = Path.filename().string()[ 0 ] == '.';

						if( !IsHiddenFile )
						{
							std::error_code ErrorCode = {};
							if( !m_FileFilters.empty() )
							{
								if( m_FileFilters.find( "*" + Path.extension().string() ) == m_FileFilters.end() && !std::filesystem::is_directory( Path, ErrorCode ) )
									continue;
							}

							if( m_SearchEnabled && !m_SearchResult.empty() )
							{
								if( !strstr( Path.filename().string().c_str(), m_SearchResult.c_str() ) )
								{
									continue;
								}
							}

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex( 0 );
							ErrorCode = {};
							if( std::filesystem::is_directory( Path, ErrorCode ) )
							{
								if( ImGuiAux::PushTreeWithIcon( Path.filename().string().c_str(), m_IconFolder, false, false ) )
								{
									if( ImGui::IsItemClicked() )
									{
										m_CurrentPath = Path;
									}

									ImGui::TreePop();
								}
							}
							else
							{
								if( ImGuiAux::PushTreeWithIcon( Path.filename().string().c_str(), m_IconFile, false, false ) )
								{
									if( ImGui::IsItemClicked() )
									{
										m_SelectedFile = Path;
									}

									ImGui::TreePop();
								}

								ImGui::TableSetColumnIndex( 1 );
								float Bytes = ( float )std::filesystem::file_size( Path );
								float Kb    = Bytes / 1024.0f;
								float Mb    = Kb / 1024.0f;
								float Gb    = Mb / 1024.0f;

								if( Kb < 1.0f )
									ImGui::Text( "%.1f Bytes", Bytes );
								else if( Mb < 1.0f )
									ImGui::Text( "%.1f Kb", Kb );
								else if( Gb < 1.0f )
									ImGui::Text( "%.1f Mb", Mb );
								else
									ImGui::Text( "%.1f Gb", Gb );
							}
						}
					}

					ImGui::EndTable();
				}
			}
			ImGui::EndChild(); // Content Browser
			ImGui::PopStyleColor();
		}
		ImGui::EndChild(); // Main Panel
	}
	ImGui::EndChild();

	ImGuiAux::ButtonData ButtonData;

	std::string OpenButton     = m_SelectedFile.empty() ? "Open" : "Open - " + m_SelectedFile.filename().string();
	float       OpenButtonSize = m_SelectedFile.empty() ? 70.0f : ImGui::CalcTextSize( OpenButton.c_str() ).x;
	ButtonData.Size            = ImVec2( OpenButtonSize + 20, 30 );

	ImGuiAux::Button( OpenButton.c_str(), ButtonData, [ this ]()
		{
			if( !m_SelectedFile.empty() )
			{
				m_Callback( m_SelectedFile );
				Close();
			}
		} );

	ImGui::SameLine();

	ButtonData.Size = ImVec2( 70, 30 );

	ImGuiAux::Button( "Cancel", ButtonData, [ this ]()
		{ Close(); } );
} // Update Derived
