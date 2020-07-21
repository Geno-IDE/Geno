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

#include <string>
#include <iostream>

#include <GLFW/glfw3.h>
#include <imgui.h>

void MainMenuBar::Show( void )
{
	if( ImGui::BeginMainMenuBar() )
	{
		height_ = ImGui::GetWindowHeight();

		if( ImGui::BeginMenu( "File" ) )
		{
			if( ImGui::MenuItem( "New", "Ctrl+N" ) )  ActionFileNew();
			if( ImGui::MenuItem( "Open", "Ctrl+O" ) ) ActionFileOpen();

			ImGui::Separator();

			if( ImGui::MenuItem( "Settings", "Alt+S" ) ) ActionFileSettings();

			ImGui::Separator();

			if( ImGui::MenuItem( "Exit", "Alt+E" ) ) ActionFileExit();

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Build" ) )
		{
			if( ImGui::MenuItem( "Build", "F7" ) ) ActionBuildBuild();

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Help" ) )
		{
			if( ImGui::MenuItem( "Demo" ) )  ActionHelpDemo();
			if( ImGui::MenuItem( "About" ) ) ActionHelpAbout();

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if( show_demo_window_ )
	{
		ImGui::ShowDemoWindow( &show_demo_window_ );
	}

	if( show_about_window_ )
	{
		ImGui::ShowAboutWindow( &show_about_window_ );
	}

	if( show_settings_ )
	{
		SettingsWindow::Get().Show( &show_settings_ );
	}

	// Keybinds

	if( ImGui::IsKeyDown( GLFW_KEY_LEFT_SHIFT ) || ImGui::IsKeyDown( GLFW_KEY_RIGHT_SHIFT ) )
	{
	}
	else if( ImGui::IsKeyDown( GLFW_KEY_LEFT_CONTROL ) || ImGui::IsKeyDown( GLFW_KEY_RIGHT_CONTROL ) )
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_N ) ) ActionFileNew();
		if( ImGui::IsKeyPressed( GLFW_KEY_O ) ) ActionFileOpen();
	}
	else if( ImGui::IsKeyDown( GLFW_KEY_LEFT_ALT ) || ImGui::IsKeyDown( GLFW_KEY_RIGHT_ALT ) )
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_S ) ) ActionFileSettings();
		if( ImGui::IsKeyPressed( GLFW_KEY_E ) ) ActionFileExit();
	}
	else
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_F7 ) ) ActionBuildBuild();
	}
}

MainMenuBar& MainMenuBar::Get( void )
{
	static MainMenuBar main_menu_bar;
	return main_menu_bar;
}

void MainMenuBar::ActionFileNew( void )
{
	std::cout << "New\n";
}

void MainMenuBar::ActionFileOpen( void )
{
	std::cout << "Open\n";
}

void MainMenuBar::ActionFileSettings( void )
{
	show_settings_ = true;
}

void MainMenuBar::ActionFileExit( void )
{
	exit( 0 );
}

void MainMenuBar::ActionBuildBuild( void )
{
	std::cout << "Build\n";
}

void MainMenuBar::ActionHelpDemo( void )
{
	show_demo_window_ = true;
}

void MainMenuBar::ActionHelpAbout( void )
{
	show_about_window_ = true;
}
