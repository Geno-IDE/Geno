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

#include "Compilers/ICompiler.h"
#include "GUI/Modals/NewItemModal.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/Modals/SettingsModal.h"
#include "GUI/Widgets/OutputWidget.h"
#include "GUI/Widgets/TextEditWidget.h"
#include "GUI/Widgets/WorkspaceWidget.h"
#include "GUI/Application.h"
#include "GUI/MainWindow.h"
#include "Misc/Settings.h"

#include <functional>
#include <iostream>
#include <numeric>
#include <string>

#include <Common/LocalAppData.h>
#include <Common/STLExtras.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

void MainMenuBar::Show( void )
{
	// Initialize windows before user requests it be shown
	OutputWidget::Instance();
	TextEditWidget::Instance();
	WorkspaceWidget::Instance();

	if( ImGui::BeginMainMenuBar() )
	{
		height_ = ImGui::GetWindowHeight();

		if( ImGui::BeginMenu( "File" ) )
		{
			if( ImGui::MenuItem( "New Workspace", "Ctrl+N" ) )   ActionFileNewWorkspace();
			if( ImGui::MenuItem( "Open Workspace", "Ctrl+O" ) )  ActionFileOpenWorkspace();
			if( ImGui::MenuItem( "Close Workspace", "Ctrl+W" ) ) ActionFileCloseWorkspace();

			ImGui::Separator();

			if( ImGui::MenuItem( "Exit", "Alt+E" ) ) ActionFileExit();

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Build" ) )
		{
			if( ImGui::MenuItem( "Build", "F7" ) ) ActionBuildBuild();

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "View" ) )
		{
			if( ImGui::MenuItem( "Text Edit", "Alt+T" ) ) ActionViewTextEdit();
			if( ImGui::MenuItem( "Workspace", "Alt+W" ) ) ActionViewWorkspace();
			if( ImGui::MenuItem( "Settings", "Alt+S" ) )  ActionViewSettings();
			if( ImGui::MenuItem( "Output", "Alt+O" ) )    ActionViewOutput();

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Help" ) )
		{
			if( ImGui::MenuItem( "Demo" ) )  ActionHelpDemo();
			if( ImGui::MenuItem( "About" ) ) ActionHelpAbout();

			ImGui::EndMenu();
		}

		if( Workspace* workspace = Application::Instance().CurrentWorkspace() )
		{
			ImGui::Separator();

			for( auto& column : workspace->build_matrix_.columns_ )
			{
				AddBuildMatrixColumn( column );
			}
		}

		ImGui::EndMainMenuBar();
	}

	// Keybinds

	if( ImGui::IsKeyDown( GLFW_KEY_LEFT_SHIFT ) || ImGui::IsKeyDown( GLFW_KEY_RIGHT_SHIFT ) )
	{
	}
	else if( ImGui::IsKeyDown( GLFW_KEY_LEFT_CONTROL ) || ImGui::IsKeyDown( GLFW_KEY_RIGHT_CONTROL ) )
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_N ) ) ActionFileNewWorkspace();
		if( ImGui::IsKeyPressed( GLFW_KEY_O ) ) ActionFileOpenWorkspace();
		if( ImGui::IsKeyPressed( GLFW_KEY_W ) ) ActionFileCloseWorkspace();
	}
	else if( ImGui::IsKeyDown( GLFW_KEY_LEFT_ALT ) || ImGui::IsKeyDown( GLFW_KEY_RIGHT_ALT ) )
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_E ) ) ActionFileExit();
		if( ImGui::IsKeyPressed( GLFW_KEY_T ) ) ActionViewTextEdit();
		if( ImGui::IsKeyPressed( GLFW_KEY_W ) ) ActionViewWorkspace();
		if( ImGui::IsKeyPressed( GLFW_KEY_S ) ) ActionViewSettings();
		if( ImGui::IsKeyPressed( GLFW_KEY_O ) ) ActionViewOutput();
	}
	else
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_F7 ) ) ActionBuildBuild();
	}

	// Show windows

	if( show_demo_window_ )
	{
		ImGui::ShowDemoWindow( &show_demo_window_ );
	}

	if( show_about_window_ )
	{
		ImGui::ShowAboutWindow( &show_about_window_ );
	}

	if( show_text_edit_ )
	{
		TextEditWidget::Instance().Show( &show_text_edit_ );
	}

	if( show_workspace_ )
	{
		WorkspaceWidget::Instance().Show( &show_workspace_ );
	}

	if( show_output_ )
	{
		OutputWidget::Instance().Show( &show_output_ );
	}
}

void MainMenuBar::ActionFileNewWorkspace( void )
{
	NewItemModal::Instance().Request( "New Workspace Location", this,
		[]( std::string_view name, std::filesystem::path location, void* /*user*/ )
		{
			Application::Instance().NewWorkspace( location, name );
		}
	);
}

void MainMenuBar::ActionFileOpenWorkspace( void )
{
	OpenFileModal::Instance().RequestFile( "Open Workspace", this,
		[]( const std::filesystem::path& path, void* /*user*/ )
		{
			Application::Instance().LoadWorkspace( path );
		}
	);
}

void MainMenuBar::ActionFileCloseWorkspace( void )
{
	Application::Instance().CloseWorkspace();
}

void MainMenuBar::ActionFileExit( void )
{
	exit( 0 );
}

void MainMenuBar::ActionBuildBuild( void )
{
	if( Workspace* workspace = Application::Instance().CurrentWorkspace() )
	{
		OutputWidget::Instance().ClearCapture();

		std::cout << "Building " << workspace->name_ << "..\n";

		workspace->Build();
	}
}

void MainMenuBar::ActionViewTextEdit( void )
{
	show_text_edit_ ^= 1;
}

void MainMenuBar::ActionViewWorkspace( void )
{
	show_workspace_ ^= 1;
}

void MainMenuBar::ActionViewSettings( void )
{
	Settings& settings = Settings::Instance();

	SettingsModal::Instance().Show( &settings.object_ );
}

void MainMenuBar::ActionViewOutput( void )
{
	show_output_ ^= 1;
}

void MainMenuBar::ActionHelpDemo( void )
{
	show_demo_window_ ^= 1;
}

void MainMenuBar::ActionHelpAbout( void )
{
	show_about_window_ ^= 1;
}

void MainMenuBar::AddBuildMatrixColumn( BuildMatrix::Column& column )
{
	ImGui::Spacing();
	ImGui::Text( "%s:", column.name.c_str() );

	const std::string label = "##" + column.name;

	// Add combo for this column
	ImGui::SetNextItemWidth( 100.0f );
	if( ImGui::BeginCombo( label.c_str(), column.current_configuration.c_str() ) )
	{
		for( auto& cfg : column.configurations )
		{
			if( ImGui::Selectable( cfg.name.c_str() ) )
				column.current_configuration = cfg.name;
		}

		ImGui::EndCombo();
	}

	// Add combos for exclusive categories for the selected configuration
	for( auto& cfg : column.configurations )
	{
		if( cfg.name == column.current_configuration )
		{
			for( auto& exclusive : cfg.exclusive_columns )
				AddBuildMatrixColumn( exclusive );

			break;
		}
	}
}
