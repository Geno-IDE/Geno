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

#include "Common/Drop.h"
#include "Common/LocalAppData.h"
#include "GUI/Application.h"
#include "GUI/MainMenuBar.h"
#include "GUI/MainWindow.h"

#include <fstream>
#include <iostream>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

const char* WINDOW_NAME = "Text Edit";

void TextEditWidget::Show( bool* p_open )
{
	ImGuiStyle& style    = ImGui::GetStyle();
	ImVec4      bg_color = style.Colors[ ImGuiCol_WindowBg ];

	// Use a brighter background color if the widget is being drag-hovered
	if( const Drop* drop = MainWindow::Instance().GetDraggedDrop() )
	{
		const float x = static_cast< float >( MainWindow::Instance().GetDragPosX() );
		const float y = static_cast< float >( MainWindow::Instance().GetDragPosY() );

		if( ImGuiWindow* window = ImGui::FindWindowByName( WINDOW_NAME ) )
		{
			if( window->Rect().Contains( ImVec2( x, y ) ) )
			{
				bg_color = bg_color + ImVec4( 0.1f, 0.1f, 0.1f, 0.1f );

				switch( drop->GetType() )
				{
					case Drop::TypeIndex::Bitmap:
					{
						const Drop::Bitmap& bitmap = drop->GetBitmap();

						dragged_bitmap_texture_.SetPixels( GL_RGBA8, bitmap.width, bitmap.height, GL_BGRA, bitmap.data.get() );

						// Adjust image size and ensure that no dimension is bigger than 200px
						constexpr float image_max_size = 200.0f;
						ImVec2          image_size;
						if( bitmap.width > bitmap.height ) image_size = ImVec2( image_max_size, image_max_size * ( bitmap.height / static_cast< float >( bitmap.width ) ) );
						else                               image_size = ImVec2( image_max_size * ( bitmap.width / static_cast< float >( bitmap.height ) ), image_max_size );

						ImGui::SetNextWindowPos( ImVec2( x, y ) );
						ImGui::BeginTooltip();
						ImGui::Image( dragged_bitmap_texture_.GetID(), image_size, ImVec2( 0, 1 ), ImVec2( 1, 0 ) );
						ImGui::EndTooltip();

					} break;

					case Drop::TypeIndex::Text:
					{
						const Drop::Text& text = drop->GetText();
						
						ImGui::SetNextWindowPos( ImVec2( x, y ) );
						ImGui::BeginTooltip();
						ImGui::Text( "%ws", text.c_str() );
						ImGui::EndTooltip();

					} break;

					case Drop::TypeIndex::Paths:
					{
						const Drop::Paths& paths = drop->GetPaths();
						
						ImGui::SetNextWindowPos( ImVec2( x, y ) );
						ImGui::BeginTooltip();

						for( const std::filesystem::path& path : paths )
							ImGui::BulletText( "%ws", path.c_str() );

						ImGui::EndTooltip();

					} break;
				}
			}
		}
	}

	ImGui::PushStyleColor( ImGuiCol_ChildBg, bg_color );

	if( ImGui::Begin( WINDOW_NAME, p_open ) )
	{
		const int tab_bar_flags = ( 0
			| ImGuiTabBarFlags_Reorderable
			| ImGuiTabBarFlags_FittingPolicyScroll
		);

		if( ImGui::BeginTabBar( "TextEditTabBar", tab_bar_flags ) )
		{
			for( File& file : files_ )
			{
				std::string file_string = file.path.filename().string();

				if( ImGui::BeginTabItem( file_string.c_str(), &file.open ) )
				{
					const int input_text_flags = ImGuiInputTextFlags_AllowTabInput;

					if( ImGui::InputTextMultiline( "##TextEditor", &file.text, ImVec2( -0.01f, -0.01f ), input_text_flags ) )
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

			ImGui::EndTabBar();
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();
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

void TextEditWidget::OnDragDrop( const Drop& drop, int x, int y )
{
	ImGuiWindow* window = ImGui::FindWindowByName( WINDOW_NAME );

	if( window && window->Rect().Contains( ImVec2( static_cast< float >( x ), static_cast< float >( y ) ) ) )
	{
		switch( drop.GetType() )
		{
			case Drop::TypeIndex::Paths:
			{
				const Drop::Paths& paths = drop.GetPaths();

				for( const std::filesystem::path& path : paths )
					AddFile( path );

			} break;
		}
	}
}
