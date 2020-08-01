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

#include "TextEditWidget.h"

#include "Common/LocalAppData.h"
#include "GUI/Application.h"
#include "GUI/MainMenuBar.h"
#include "GUI/MainWindow.h"

#include <fstream>
#include <iostream>

#include <imgui.h>
#include <imgui_internal.h>

void TextEditWidget::Show( bool* p_open )
{
	if( ImGui::Begin( "Text Edit", p_open ) )
	{
		const int tab_bar_flags = ( 0
			| ImGuiTabBarFlags_Reorderable
			| ImGuiTabBarFlags_FittingPolicyScroll
		);

		if( ImGui::BeginTabBar( "TextEditTabBar", tab_bar_flags ) )
		{
			Workspace& workspace = Application::Instance().CurrentWorkspace();

			if( workspace.IsOpen() )
			{
				for( File& file : files_ )
				{
					std::filesystem::path relative_path        = workspace.RelativePath( file.path );
					std::string           relative_path_string = relative_path.string();

					if( ImGui::BeginTabItem( relative_path_string.c_str(), &file.open ) )
					{
						const int input_text_flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize;

						if( ImGui::InputTextMultiline( "##TextEditor", &file.text[ 0 ], file.text.size() + 1, ImVec2( -0.01f, -0.01f ), input_text_flags, InputTextCB, &file ) )
						{
							std::ofstream ofs( file.path, std::ios::binary | std::ios::trunc );
							ofs << file.text;
						}

						ImGui::EndTabItem();
					}
				}

				// Clear closed files from list
				for( auto it = files_.begin(); it != files_.end(); )
				{
					if( it->open ) it++;
					else           it = files_.erase( it );
				}
			}

			// #TODO: Add manually opened files here

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void TextEditWidget::AddFile( const std::filesystem::path& path )
{
	if( !std::filesystem::exists( path ) )
	{
		std::cerr << "Failed to add '" << path << "' to text-edit. File does not exist.\n";
		return;
	}

//////////////////////////////////////////////////////////////////////////

	std::ifstream ifs  = std::ifstream( path, std::ios::binary );
	std::string   text = std::string( ( std::istreambuf_iterator< char >( ifs ) ), std::istreambuf_iterator< char >() );

	for( File& file : files_ )
	{
		// Do not need to add file to vector if it already exists
		if( file.path == path )
		{
			// Update text in case file changed externally
			file.text = text;
			return;
		}
	}

	File file;
	file.path = path;
	file.text = text;

	files_.emplace_back( std::move( file ) );
}

int TextEditWidget::InputTextCB( ImGuiInputTextCallbackData* data )
{
	File* file = static_cast< File* >( data->UserData );

	if( data->EventFlag == ImGuiInputTextFlags_CallbackResize )
	{
		file->text.resize( data->BufTextLen );
		data->Buf = &file->text[ 0 ];
	}

	return 0;
}
