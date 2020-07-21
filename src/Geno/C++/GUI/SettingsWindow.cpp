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

#include "SettingsWindow.h"

#include "GUI/MainWindow.h"

#include <array>

#include <imgui.h>
#include <imgui_internal.h>

void SettingsWindow::Show( bool* p_open )
{
	if( ImGui::Begin( "Settings", p_open ) )
	{
		constexpr float       list_width   = 120.f;
		constexpr const char* list_items[] = { "Compiler", "Theme" };

		MainWindow::Instance().PushHorizontalLayout();

		if( ImGui::BeginChild( 1, ImVec2( list_width, 0.f ) ) )
		{
			for( size_t i = 0; i < std::size( list_items ); ++i )
			{
				if( ImGui::Selectable( list_items[ i ], current_panel_item_ == ( int )i ) )
				{
					current_panel_item_ = ( int )i;
				}
			}
		}
		ImGui::EndChild();

		if( ImGui::BeginChild( 2 ) )
		{
			switch( current_panel_item_ )
			{
				case 0:
				{
					char llvm_path_buf[ 256 ] = { };

					for( size_t i = 0; i < llvm_path_.native().size(); ++i )
					{
						llvm_path_buf[ i ] = ( char )llvm_path_.native().at( i );
					}

					if( ImGui::InputText( "LLVM Path", &llvm_path_buf[ 0 ], std::size( llvm_path_buf ) ) )
					{
						llvm_path_ = llvm_path_buf;
					}

				} break;

				case 1:
				{
					if( ImGui::Combo( "Theme", &current_theme_, "Classic\0Light\0Dark\0" ) )
					{
						switch( current_theme_ )
						{
							case 0: { ImGui::StyleColorsClassic(); } break;
							case 1: { ImGui::StyleColorsLight();   } break;
							case 2: { ImGui::StyleColorsDark();    } break;
						}
					}

				} break;
			}

		}
		ImGui::EndChild();

		MainWindow::Instance().PopHorizontalLayout();
	}
	ImGui::End();
}

SettingsWindow& SettingsWindow::Instance( void )
{
	static SettingsWindow instance;
	return instance;
}
