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
#include <iostream>

#include <Common/LocalAppData.h>
#include <GCL/Object.h>

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

static constexpr const char* CategoryString( Category category )
{
	switch( category )
	{
		case CategoryCompiler: return "Compiler";
		case CategoryTheme:    return "Theme";
		default:               return nullptr;
	}
}

void SettingsModal::Show( GCL::Object* object )
{
	if( !object->IsTable() )
	{
		std::cerr << "SettingsModal error: Trying to edit non-table object\n";
		return;
	}

	if( object->Empty() )
	{
		std::cerr << "SettingsModal error: Trying to edit empty table\n";
		return;
	}

	if( Open() )
	{
		current_category_ = -1;
		edited_object_    = object;
	}
}

void SettingsModal::UpdateDerived( void )
{
	if( ImGui::BeginChild( 1, ImVec2( 0, -30 ) ) )
	{
		MainWindow::Instance().PushHorizontalLayout();

		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::GetStyleColorVec4( ImGuiCol_FrameBg ) );
		if( ImGui::BeginChild( 1, ImVec2( 120, 0 ) ) )
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

					GCL::Object& mingw_path     = settings.object_[ "MinGW-Path" ];
					std::string  mingw_path_buf = mingw_path.IsString() ? mingw_path.String() : std::string();

					ImGui::TextUnformatted( "MinGW Path" );

					ImGui::SetNextItemWidth( -60.0f );
					if( ImGui::InputText( "##MinGW Path", &mingw_path_buf ) )
					{
						mingw_path.SetString( mingw_path_buf );
					}

					ImGui::SameLine();
					ImGui::SetNextItemWidth( 50.0f );
					if( ImGui::Button( "Browse" ) )
					{
						OpenFileModal::Instance().RequestDirectory( "Locate MinGW Directory", this,
							[]( const std::filesystem::path& path, void* /*user*/ )
							{
								Settings& settings = Settings::Instance();

								settings.object_[ "MinGW-Path" ] = path.string();
							}
						);
					}

				#endif // _WIN32

				} break;

				case CategoryTheme:
				{
					const std::array theme_names   = { "Classic", "Light", "Dark" };
					GCL::Object&     theme         = settings.object_[ "Theme" ];
					auto             current_theme = theme.IsString() ? std::find( theme_names.begin(), theme_names.end(), theme.String() ) : theme_names.end();
					int              current_item  = ( current_theme == theme_names.end() ) ? -1 : static_cast< int >( std::distance( theme_names.begin(), current_theme ) );

					ImGui::TextUnformatted( "Theme" );

					ImGui::SetNextItemWidth( -5.0f );
					if( ImGui::Combo( "##Theme", &current_item, theme_names.data(), static_cast< int >( theme_names.size() ) ) )
					{
						theme = theme_names[ current_item ];
						settings.UpdateTheme();
					}

				} break;
			}
		}
		ImGui::EndChild();

		MainWindow::Instance().PopHorizontalLayout();
	}
	ImGui::EndChild();

	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + 4 );
	if( ImGui::Button( "Close", ImVec2( 80, 0 ) ) )
	{
		Close();
	}
}

void SettingsModal::OnClose( void )
{
	current_category_ = 0;
	edited_object_    = nullptr;
}
