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

#include "MainMenuBar.h"

#include "GUI/MainWindow.h"
#include "GUI/SettingsWindow.h"

#include <iostream>

#include <imgui.h>

void MainMenuBar::Show( void )
{
	if( ImGui::BeginMainMenuBar() )
	{
		if( ImGui::BeginMenu( "File" ) )
		{
			if( ImGui::MenuItem( "New", "Ctrl+N" ) )
			{
				std::cout << "New\n";
			}

			if( ImGui::MenuItem( "Open", "Ctrl+O" ) )
			{
				std::cout << "Open\n";
			}

			ImGui::Separator();

			if( ImGui::MenuItem( "Settings" ) )
			{
				show_settings_ = true;
			}

			ImGui::Separator();

			if( ImGui::MenuItem( "Exit" ) )
			{
				exit( 0 );
			}

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Help" ) )
		{
			if( ImGui::MenuItem( "About" ) )
			{
				show_about_window_ = true;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if( show_settings_ )
	{
		SettingsWindow::Get().Show( &show_settings_ );
	}

	if( show_about_window_ )
	{
		ImGui::ShowAboutWindow( &show_about_window_ );
	}
}

MainMenuBar& MainMenuBar::Get( void )
{
	static MainMenuBar main_menu_bar;
	return main_menu_bar;
}
