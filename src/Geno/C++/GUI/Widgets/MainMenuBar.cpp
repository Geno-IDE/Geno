/*
 * Copyright (c) 2021 Sebastian Kylander https://gaztin.com/
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
#include "GUI/Widgets/OutputWindow.h"
#include "GUI/Widgets/TextEdit.h"
#include "GUI/Widgets/WorkspaceOutliner.h"
#include "GUI/MainWindow.h"
#include "Application.h"

#include <functional>
#include <iostream>
#include <numeric>
#include <string>

#include <Common/LocalAppData.h>
#include <Common/Process.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::Draw( void )
{
	const bool WorkspaceActive = Application::Instance().CurrentWorkspace() != nullptr;

	if( ImGui::BeginMainMenuBar() )
	{
		m_Height = ImGui::GetWindowHeight();

		if( ImGui::BeginMenu( "File" ) )
		{
			if( ImGui::MenuItem( "New Workspace",   "Ctrl+N", false, true            ) ) ActionFileNewWorkspace();
			if( ImGui::MenuItem( "Open Workspace",  "Ctrl+O", false, true            ) ) ActionFileOpenWorkspace();
			if( ImGui::MenuItem( "Close Workspace", "Ctrl+W", false, WorkspaceActive ) ) ActionFileCloseWorkspace();

			ImGui::Separator();

			if( ImGui::MenuItem( "Exit", "Alt+E" ) ) exit( 0 );

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Build", WorkspaceActive ) )
		{
			if( ImGui::MenuItem( "Build And Run", "F5" ) ) ActionBuildBuildAndRun();
			if( ImGui::MenuItem( "Build",         "F7" ) ) ActionBuildBuild();

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "View" ) )
		{
			ImGui::MenuItem( "Text Edit", "Alt+T", &ShowTextEdit );
			ImGui::MenuItem( "Workspace", "Alt+W", &ShowWorkspaceOutliner );
			ImGui::MenuItem( "Output",    "Alt+O", &ShowOutputWindow );

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Help" ) )
		{
			if( ImGui::MenuItem( "Demo" ) )  ShowDemoWindow  ^= 1;
			if( ImGui::MenuItem( "About" ) ) ShowAboutWindow ^= 1;

			ImGui::EndMenu();
		}

		if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
		{
			ImGui::Separator();

			for( BuildMatrix::Column& rColumn : pWorkspace->m_BuildMatrix.m_Columns )
			{
				AddBuildMatrixColumn( rColumn );
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
		if( ImGui::IsKeyPressed( GLFW_KEY_E ) ) exit( 0 );
		if( ImGui::IsKeyPressed( GLFW_KEY_T ) ) ShowTextEdit          ^= 1;
		if( ImGui::IsKeyPressed( GLFW_KEY_W ) ) ShowWorkspaceOutliner ^= 1;
		if( ImGui::IsKeyPressed( GLFW_KEY_O ) ) ShowOutputWindow      ^= 1;
	}
	else
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_F5 ) ) ActionBuildBuildAndRun();
		if( ImGui::IsKeyPressed( GLFW_KEY_F7 ) ) ActionBuildBuild();
	}

} // Draw

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionFileNewWorkspace( void )
{
	NewItemModal::Instance().RequestPath( "New Workspace Location", std::filesystem::current_path(), this,
		[]( std::string Name, std::filesystem::path Location, void* /*pUser*/ )
		{
			Application::Instance().NewWorkspace( Location, std::move( Name ) );
		}
	);

} // ActionFileNewWorkspace

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionFileOpenWorkspace( void )
{
	OpenFileModal::Instance().RequestFile( "Open Workspace", this,
		[]( const std::filesystem::path& rPath, void* /*pUser*/ )
		{
			Application::Instance().LoadWorkspace( rPath );
		}
	);

} // ActionFileOpenWorkspace

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionFileCloseWorkspace( void )
{
	Application::Instance().CloseWorkspace();

} // ActionFileCloseWorkspace

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionBuildBuildAndRun( void )
{
	if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
	{
		MainWindow::Instance().pOutputWindow->ClearCapture();

		pWorkspace->Events.BuildFinished += [ this ]( Workspace& /*rWorkspace*/, std::filesystem::path OutputFile, bool /*Success*/ )
		{
			const std::string OutputString = OutputFile.string();

			std::cout << "=== Running " << OutputString << "===\n";

			const int ExitCode = Process::ResultOf( OutputFile.wstring() );
			std::cout << "=== " << OutputString << " finished with exit code " << ExitCode << " ===\n";
		};

		pWorkspace->Build();
	}

} // ActionBuildBuildAndRun

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionBuildBuild( void )
{
	if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
	{
		MainWindow::Instance().pOutputWindow->ClearCapture();

		pWorkspace->Build();
	}

} // ActionBuildBuild

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::AddBuildMatrixColumn( BuildMatrix::Column& rColumn )
{
	ImGui::Spacing();
	ImGui::Text( "%s:", rColumn.Name.c_str() );

	const std::string Label = "##" + rColumn.Name;

	// Add combo for this column
	ImGui::SetNextItemWidth( 100.0f );
	if( ImGui::BeginCombo( Label.c_str(), rColumn.CurrentConfiguration.c_str() ) )
	{
		for( auto&[ rName, rConfiguration ] : rColumn.Configurations )
		{
			if( ImGui::Selectable( rName.c_str() ) )
				rColumn.CurrentConfiguration = rName;
		}

		ImGui::EndCombo();
	}

} // AddBuildMatrixColumn
