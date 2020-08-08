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

#if defined( _WIN32 )
#include <Windows.h>
#endif // _WIN32

OpenFileModal::OpenFileModal( void )
{
#if defined( _WIN32 )

	drives_buffer_size_ = GetLogicalDriveStringsA( 0, nullptr );
	drives_buffer_      = std::unique_ptr< char[] >( new char[ drives_buffer_size_ ] );
	drives_buffer_size_ = GetLogicalDriveStringsA( static_cast< DWORD >( drives_buffer_size_ ), &drives_buffer_[ 0 ] );

#endif // _WIN32

	current_directory_ = RootDirectory();
}

void OpenFileModal::Present( void* user, Callback callback )
{
	if( user_ || callback_ )
		return;

	user_     = user;
	callback_ = callback;
	open_     = true;
}

void OpenFileModal::Update( void )
{
	if( open_ && !ImGui::IsPopupOpen( "OpenFile" ) )
	{
		ImGui::OpenPopup( "OpenFile" );
	}

	if( ImGui::BeginPopupModal( "OpenFile" ) )
	{
		if( ImGui::BeginChild( 1 , ImVec2( 0, -24 ) ) )
		{
			MainWindow::Instance().PushHorizontalLayout();

	#if defined( _WIN32 )

			if( ImGui::BeginChild( 2, ImVec2( 100, 0 ) ) )
			{
				size_t i = 0;
				for( char* it = &drives_buffer_[ 0 ]; it < &drives_buffer_[ drives_buffer_size_ ]; it += ( strlen( it ) + 1 ), ++i )
				{
					bool selected = ( current_drive_index_ == i );

					if( ImGui::Selectable( it, &selected ) )
					{
						current_drive_index_ = i;
						current_directory_   = RootDirectory();
					}
				}
			}
			ImGui::EndChild();

	#endif // _WIN32

			if( ImGui::BeginChild( 3, ImVec2( 0, 0 ), false, ImGuiWindowFlags_MenuBar ) )
			{
				ImGui::PushStyleColor( ImGuiCol_MenuBarBg, ImVec4( 0, 0, 0, 1 ) );

				if( ImGui::BeginMenuBar() )
				{
					if( ImGui::Button( "New Folder" ) )
					{
						editing_path_           = current_directory_ / "New Folder";
						editing_path_is_folder_ = true;
						change_edit_focus_      = true;
					}

					if( ImGui::Button( "New File" ) )
					{
						editing_path_           = current_directory_ / "New File.txt";
						editing_path_is_folder_ = false;
						change_edit_focus_      = true;
					}

					if( !editing_path_.empty() )
					{
						std::string filename = editing_path_.filename().string();

						if( change_edit_focus_ )
						{
							ImGui::SetKeyboardFocusHere();

							if( ImGuiInputTextState* state = ImGui::GetInputTextState( ImGui::GetID( "##NewFileName" ) ) )
							{
								state->ID = 0;
							}

							change_edit_focus_ = false;
						}

						if( ImGui::InputText( "##NewFileName", &filename, ImGuiInputTextFlags_EnterReturnsTrue ) )
						{
							std::filesystem::path& new_path = editing_path_.replace_filename( filename );

							if( std::filesystem::exists( new_path ) )
							{
								change_edit_focus_ = true;
							}
							else
							{
								if( editing_path_is_folder_ )
								{
									if( std::filesystem::create_directory( new_path ) ) editing_path_.clear();
									else                                                change_edit_focus_ = true;
								}
								else
								{
									std::ofstream ofs( new_path );

									if( ofs.is_open() ) editing_path_.clear();
									else                change_edit_focus_ = true;
								}
							}
						}
						else if( ImGui::IsItemDeactivated() )
						{
							editing_path_.clear();
							change_edit_focus_ = false;
						}
					}

					ImGui::EndMenuBar();
				}

				ImGui::PopStyleColor();

//////////////////////////////////////////////////////////////////////////

				std::filesystem::directory_iterator  root_directory_iterator( current_directory_, std::filesystem::directory_options::skip_permission_denied );
				std::filesystem::path                parent_directory = current_directory_.parent_path();
				std::vector< std::filesystem::path > directory_paths;
				std::vector< std::filesystem::path > file_paths;

				if( current_directory_ != parent_directory )
				{
					if( ImGui::Selectable( ".." ) )
					{
						current_directory_ = parent_directory;
					}
				}

				for( const std::filesystem::directory_entry& entry : root_directory_iterator )
				{
					/**/ if( entry.is_directory() )    directory_paths.push_back( entry );
					else if( entry.is_regular_file() ) file_paths.push_back( entry );
				}

				for( const std::filesystem::path& directory_entry : directory_paths )
				{
					std::string filename = directory_entry.filename().string();

					if( ImGui::Selectable( filename.c_str() ) )
					{
						current_directory_ = directory_entry;
					}
				}

				ImGui::Separator();

				for( const std::filesystem::path& file_path : file_paths )
				{
					std::string filename = file_path.filename().string();
					bool        selected = file_path == selected_file_;

					if( ImGui::Selectable( filename.c_str(), &selected ) )
					{
						selected_file_ = file_path;
					}
				}
			}
			ImGui::EndChild();

			MainWindow::Instance().PopHorizontalLayout();
		}
		ImGui::EndChild();

		if( ImGui::BeginChild( 4 , ImVec2( 0, 20 ) ) )
		{
			bool disable_ok_button = selected_file_.empty() || !std::filesystem::exists( selected_file_ );

			if( disable_ok_button )
			{
				ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
			}

			if( ImGui::Button( "OK", ImVec2( 80, 0 ) ) )
			{
				if( callback_ )
					callback_( selected_file_, user_ );

				Close();
			}

			if( disable_ok_button )
			{
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}

			ImGui::SameLine();
			if( ImGui::Button( "Cancel", ImVec2( 80, 0 ) ) )
			{
				Close();
			}

			std::string selected_file_string = selected_file_.string();
			ImGui::SameLine();
			ImGui::Text( selected_file_string.c_str() );
		}
		ImGui::EndChild();

		ImGui::EndPopup();
	}
}

void OpenFileModal::Close( void )
{
	callback_ = nullptr;
	user_     = nullptr;
	open_     = false;

	ImGui::CloseCurrentPopup();
}

std::filesystem::path OpenFileModal::RootDirectory( void )
{
#if defined( _WIN32 )

	char* drive = &drives_buffer_[ 0 ];
	char* end   = &drives_buffer_[ drives_buffer_size_ ];

	for( size_t i = 0; i < current_drive_index_ && drive < end; drive += ( strlen( drive ) + 1 ), ++i );

	return std::filesystem::path( drive, drive + strlen( drive ) );

#else // _WIN32

	return std::filesystem::path( "/" );

#endif // else
}
