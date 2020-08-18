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

#include "SettingsModal.h"

#include "Compilers/ICompiler.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/MainWindow.h"
#include "Misc/Settings.h"

#include <array>
#include <fstream>

#include <Common/LocalAppData.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include "SettingsModal.h"

enum Category
{
	CategoryCompiler,
	CategoryTheme,
	NumCategories
};

constexpr const char* CategoryString( Category category )
{
	switch( category )
	{
		case CategoryCompiler: return "Compiler";
		case CategoryTheme:    return "Theme";

		case NumCategories:
		default:               return nullptr;
	}
}

void SettingsModal::Show( void )
{
	if( Open() )
	{
	}
}

void SettingsModal::UpdateDerived( void )
{
	constexpr float list_width = 120.f;

	MainWindow::Instance().PushHorizontalLayout();

	ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::GetStyleColorVec4( ImGuiCol_FrameBg ) );
	if( ImGui::BeginChild( 1, ImVec2( list_width, 0.f ) ) )
	{
		for( int i = 0; i < NumCategories; ++i )
		{
			Category category = static_cast< Category >( i );

			if( ImGui::Selectable( CategoryString( category ), current_category_ == category ) )
			{
				current_category_ = category;
			}
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();

	if( ImGui::BeginChild( 2 ) )
	{
		Settings& settings = Settings::Instance();

		switch( current_category_ )
		{
			case CategoryCompiler:
			{

			#if defined( _WIN32 )

				std::string mingw_path_buf = settings.mingw_path_.string();

				ImGui::TextUnformatted( "MinGW Path" );

				ImGui::SetNextItemWidth( -60.0f );
				if( ImGui::InputText( "##MinGW Path", &mingw_path_buf ) )
				{
					settings.mingw_path_ = mingw_path_buf;
				}

				ImGui::SameLine();
				ImGui::SetNextItemWidth( 50.0f );
				if( ImGui::Button( "Browse" ) )
				{
					OpenFileModal::Instance().RequestDirectory( "Locate MinGW Directory", this,
						[]( const std::filesystem::path& path, void* /*user*/ )
						{
							Settings::Instance().mingw_path_ = path;
						}
					);
				}

			#endif // _WIN32

			} break;

			case CategoryTheme:
			{
				const std::array theme_names   = { "Classic", "Light", "Dark" };
				auto             current_theme = std::find( theme_names.begin(), theme_names.end(), settings.theme_ );
				int              current_item  = ( current_theme == theme_names.end() ) ? -1 : static_cast< int >( std::distance( theme_names.begin(), current_theme ) );

				ImGui::TextUnformatted( "Theme" );

				ImGui::SetNextItemWidth( -5.0f );
				if( ImGui::Combo( "##Theme", &current_item, theme_names.data(), static_cast< int >( theme_names.size() ) ) )
				{
					settings.theme_ = theme_names[ current_item ];
					settings.UpdateTheme();
				}

			} break;
		}
	}
	ImGui::EndChild();

	MainWindow::Instance().PopHorizontalLayout();
}
