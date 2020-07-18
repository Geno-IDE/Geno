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

#include <array>

#include <imgui.h>

void SettingsWindow::Show( bool* p_open )
{
	if( ImGui::Begin( "Settings", p_open, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		constexpr float list_width   = 120.f;
		constexpr char* list_items[] = { "Compiler", "Theme" };

		if( ImGui::BeginChildFrame( 1, ImVec2( list_width, ImGui::GetWindowHeight() ), ImGuiWindowFlags_ChildWindow ) )
		{
			if( ImGui::ListBox( "", &current_panel_item_, list_items, ( int )std::size( list_items ) ) )
			{
			}

		}
		ImGui::EndChildFrame();

		ImGui::SameLine();

		if( ImGui::BeginChildFrame( 2, ImVec2( ImGui::GetWindowWidth() - list_width, ImGui::GetWindowHeight() ), ImGuiWindowFlags_ChildWindow ) )
		{
			switch( current_panel_item_ )
			{
				case 0:
				{
					char llvm_path_buf[ 256 ] = { };

					ImGui::TextUnformatted( "LLVM Path:" );
					ImGui::InputText( "LLVM", llvm_path_buf, std::size( llvm_path_buf ) );

					llvm_path_ = llvm_path_buf;

				} break;

				case 1:
				{
					ImGui::TextUnformatted( "Theme:" );
					ImGui::Combo( "Thame:", &current_theme_, "Default\0Light\0Dark\0" );

				} break;
			}

		}
		ImGui::EndChildFrame();
	}
	ImGui::End();
}

SettingsWindow& SettingsWindow::Get( void )
{
	static SettingsWindow settings_window;
	return settings_window;
}
