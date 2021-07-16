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

#include "GUI/MainWindow.h"
#include "Auxiliary/ImGuiAux.h"
#include "Auxiliary/STBAux.h"

#include <Common/LocalAppData.h>

#include <fstream>
#include <functional>
#include <sstream>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

//////////////////////////////////////////////////////////////////////////

OpenFileModal::OpenFileModal( void )
	: m_IconFolder( STBAux::LoadImageTexture( "Icons/Folder.png" ) )
	, m_IconFile  ( STBAux::LoadImageTexture( "Icons/File.png" ) )
	, m_IconSearch( STBAux::LoadImageTexture( "Icons/Search.png" ) )
{

#if defined( _WIN32 )

	m_DrivesBufferSize = GetLogicalDriveStringsA( 0, nullptr );
	m_DrivesBuffer      = std::unique_ptr< char[] >( new char[ m_DrivesBufferSize ] );
	m_DrivesBufferSize = GetLogicalDriveStringsA( static_cast< DWORD >( m_DrivesBufferSize ), &m_DrivesBuffer[ 0 ] );

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
		
		if(pFileFilters)
		{
			m_Title += " - ";
			for(const auto& rFileFilter : std::filesystem::path(pFileFilters))
			{
				m_FileFilters[rFileFilter.string()] = true;
				m_Title += rFileFilter.string();
			}
		}
	}

} // RequestFile

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::OnClose( void )
{
	m_Callback = { };
	m_FileFilters.clear();
	m_SelectedFile.clear();

} // OnClose

//////////////////////////////////////////////////////////////////////////

/*void OpenFileModal::UpdateDerived( void )
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
								std::error_code ErrorCode;

								if( std::filesystem::create_directory( rNewPath, ErrorCode ) ) m_EditingPath.clear();
								else                                                           m_ChangeEditFocus = true;
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

			std::error_code                     ErrorCode;
			std::filesystem::directory_iterator RootDirectoryIterator( m_CurrentDirectory, std::filesystem::directory_options::skip_permission_denied, ErrorCode );

			if( ErrorCode )
			{
				const std::string ErrorMessage    = ErrorCode.message();
				const ImVec2      WindowSize      = ImGui::GetWindowSize();
				const ImVec2      WrappedTextSize = ImGui::CalcTextSize( ErrorMessage.c_str(), ErrorMessage.c_str() + ErrorMessage.size(), false, WindowSize.x );
				const ImVec2      TextPos         = ( WindowSize - WrappedTextSize ) / 2;

				ImGui::SetCursorPos( TextPos );
				ImGui::PushTextWrapPos( WindowSize.x );
				ImGui::TextWrapped( "%s", ErrorMessage.c_str() );
				ImGui::PopTextWrapPos();
			}
			else
			{
				std::vector< std::filesystem::path > DirectoryPaths;
				std::vector< std::filesystem::path > FilePaths;

				// Go up one directory
				if( m_CurrentDirectory.has_parent_path() )
				{
					if( ImGui::Selectable( ".." ) )
						m_CurrentDirectory = m_CurrentDirectory.parent_path();
				}

				for( const std::filesystem::directory_entry& rEntry : RootDirectoryIterator )
				{
					/**/ //if( rEntry.is_directory() )    DirectoryPaths.push_back( rEntry );
					//else if( rEntry.is_regular_file() ) FilePaths.push_back( rEntry );
				/*}

				for( const std::filesystem::path& rDirectoryEntry : DirectoryPaths )
				{
					const std::string FileName = rDirectoryEntry.filename().string();

					if( m_DirectoryRequested )
					{
						bool Selected = m_SelectedPath == rDirectoryEntry;

						if( ImGui::Selectable( FileName.c_str(), &Selected, ImGuiSelectableFlags_AllowDoubleClick ) )
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
						std::string FileName = rFilePath.filename().string();
						bool        Selected = rFilePath == m_SelectedPath;

						if( ImGui::Selectable( FileName.c_str(), &Selected, ImGuiSelectableFlags_AllowDoubleClick ) )
						{
							m_SelectedPath = rFilePath;

							// Open file immediately if file was double-clicked
							if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
							{
								if( m_Callback )
									m_Callback( m_SelectedPath, m_pUser );

								Close();
							}
						}
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

} // UpdateDerived*/

//////////////////////////////////////////////////////////////////////////

void OpenFileModal::UpdateDerived( void )
{
	if(ImGui::BeginChild(1, ImVec2(0, -44)))
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.25f, 0.25f, 0.25f, 0.7f));
		if(ImGui::BeginChild(2, ImVec2(150, 0))) // Side Panel
		{
			
			if(ImGui::CollapsingHeader("Volumes", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick))
			{
				if(ImGui::BeginTable("##Volumes", 1, ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("##Volume", ImGuiTableColumnFlags_NoHide);

					auto VolumeFunc = [this](const char* pLabel)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						if(ImGui::Selectable(pLabel))
							m_CurrentPath = std::filesystem::path(pLabel);
					};

#if defined( __linux__ )

	VolumeFunc("/");
	VolumeFunc("/home");

#endif // __linux__

					ImGui::EndTable();
				}
			}

		} ImGui::EndChild(); // Side Panel
		ImGui::PopStyleColor();

		ImGui::SameLine();

		if(ImGui::BeginChild(3, ImVec2(0, 0))) // Main Panel
		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.07f, 0.07f, 0.07f, 0.7f));
			if(ImGui::BeginChild(4, ImVec2(0, 35))) // Browse Panel
			{			
				if(ImGui::BeginChild(5, ImVec2(ImGui::GetContentRegionAvailWidth() - 50.0f, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) //Browse With Buttons
				{
					if(!m_SearchEnabled)
					{
						for(const auto& rSubPath : m_CurrentPath)
						{
							bool IsActiveFolder = rSubPath == m_CurrentPath.stem();

							ImGuiAux::ButtonData ButtonData;

							if(!IsActiveFolder)
								ButtonData.ColorText = ImGui::GetStyle().Colors[ImGuiCol_Text];

							const char* pName = rSubPath.c_str();
							float ButtonX = ImGui::CalcTextSize(pName).x * 1.18f;

							ButtonData.Size = ImVec2(ButtonX, ImGui::GetContentRegionAvail().y);
							ButtonData.Rounding = 6.0f;

							ImGuiAux::Button(pName, ButtonData, [this, &rSubPath, &pName](){
								std::string ActivePath = m_CurrentPath.string();
								ActivePath = ActivePath.substr(0, ActivePath.find(rSubPath.string())) + pName;
								m_CurrentPath = std::filesystem::path(ActivePath);
							});

							ImGui::SameLine();
						}
					}
					else
					{	
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
						ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f - 12.0f);

						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.2f);
						ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 5.0f));
						ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
						ImGui::InputTextWithHint("##SearchFile", "Search", &m_SearchResult);
						ImGui::PopStyleVar(3);
						ImGui::PopStyleColor();

						if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
						{
							m_SearchEnabled = false;
							m_SearchResult.clear();
						}
					}

				} ImGui::EndChild(); //Browse With Buttons

				ImGui::SameLine();
				ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
				ImGui::SameLine();
				
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ChildBg]);
				ImVec2 Size = ImVec2(ImGui::GetFontSize() * m_IconSearch.GetAspectRatio() * 2.5f, ImGui::GetFontSize() * 2.1f);
				if(ImGui::ImageButton((ImTextureID)m_IconSearch.GetID(), Size))
				{
					m_SearchEnabled = true;
				}
				ImGui::PopStyleColor();
				
			} ImGui::EndChild(); // Browse Panel
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 0.7f));
			if(ImGui::BeginChild(6, ImVec2(0, 0))) // Content Browser
			{
				if(ImGui::BeginTable("##FileBrowserPanel", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_NoBordersInBodyUntilResize))
				{
					ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
					ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Size").x * 2.0f);
					ImGui::TableHeadersRow();

					for(const auto& rCurrentPathIt : std::filesystem::directory_iterator(m_CurrentPath))
					{
						std::filesystem::path Path = rCurrentPathIt.path();
						bool IsHiddenFile = Path.filename().string()[0] == '.';
						
						if(!IsHiddenFile)
						{
							if(!m_FileFilters.empty())
							{
								if(!m_FileFilters["*" + Path.extension().string()] && !std::filesystem::is_directory(Path))
									continue;
							}

							if(m_SearchEnabled && !m_SearchResult.empty())
							{
								if(!strstr(Path.filename().c_str(), m_SearchResult.c_str()))
								{
									continue;
								}
							}

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);

							if(std::filesystem::is_directory(Path))
							{
								if(ImGuiAux::PushTreeWithIcon(Path.filename().c_str(), m_IconFolder, false, false))
								{
									if(ImGui::IsItemClicked())
									{
										m_CurrentPath = Path;
									}

									ImGui::TreePop();
								}
							}
							else
							{
								if(ImGuiAux::PushTreeWithIcon(Path.filename().c_str(), m_IconFile, false, false))
								{
									if(ImGui::IsItemClicked())
									{
										m_SelectedFile = Path;
									}

									ImGui::TreePop();
								}

								ImGui::TableSetColumnIndex(1);
								float Bytes = (float)std::filesystem::file_size(Path);
								float Kb = Bytes / 1024.0f;
								float Mb = Kb / 1024.0f;
								float Gb = Mb / 1024.0f;

								if(Kb < 1.0f)
									ImGui::Text("%.1f Bytes", Bytes);
								else if(Mb < 1.0f)
									ImGui::Text("%.1f Kb", Kb);
								else if(Gb < 1.0f)
									ImGui::Text("%.1f Mb", Mb);
								else
									ImGui::Text("%.1f Gb", Gb);
							}
						}
					}

					ImGui::EndTable();
				}
			}ImGui::EndChild(); // Content Browser
			ImGui::PopStyleColor();

		} ImGui::EndChild(); // Main Panel

	} ImGui::EndChild();

	ImGuiAux::ButtonData ButtonData;

	std::string OpenButton = m_SelectedFile.empty() ? "Open" : "Open - " + m_SelectedFile.filename().string();
	float OpenButtonSize = m_SelectedFile.empty() ? 70.0f : ImGui::CalcTextSize(OpenButton.c_str()).x;
	ButtonData.Size = ImVec2(OpenButtonSize + 20, 30);

	ImGuiAux::Button(OpenButton.c_str(), ButtonData, [this](){
		if(!m_SelectedFile.empty())
		{
			m_Callback(m_SelectedFile);
			Close();
		}
	});

	ImGui::SameLine();

	ButtonData.Size = ImVec2(70, 30);

	ImGuiAux::Button("Cancel", ButtonData, [this](){
		Close();
	});
} // Update Derived

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
