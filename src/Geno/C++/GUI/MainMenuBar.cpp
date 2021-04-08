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
#include <Common/Process.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::Show( void )
{
	// Initialize windows before user requests it be shown
	OutputWidget   ::Instance();
	TextEditWidget ::Instance();
	WorkspaceWidget::Instance();

	if( ImGui::BeginMainMenuBar() )
	{
		m_Height = ImGui::GetWindowHeight();

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
			if( ImGui::MenuItem( "Build And Run", "F5" ) ) ActionBuildBuildAndRun();
			if( ImGui::MenuItem( "Build", "F7" ) )         ActionBuildBuild();

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
		if( ImGui::IsKeyPressed( GLFW_KEY_E ) ) ActionFileExit();
		if( ImGui::IsKeyPressed( GLFW_KEY_T ) ) ActionViewTextEdit();
		if( ImGui::IsKeyPressed( GLFW_KEY_W ) ) ActionViewWorkspace();
		if( ImGui::IsKeyPressed( GLFW_KEY_S ) ) ActionViewSettings();
		if( ImGui::IsKeyPressed( GLFW_KEY_O ) ) ActionViewOutput();
	}
	else
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_F5 ) ) ActionBuildBuildAndRun();
		if( ImGui::IsKeyPressed( GLFW_KEY_F7 ) ) ActionBuildBuild();
	}

	// Show widgets
	if( m_ShowDemoWindow  ) ImGui::ShowDemoWindow(            &m_ShowDemoWindow );
	if( m_ShowAboutWindow ) ImGui::ShowAboutWindow(           &m_ShowAboutWindow );
	if( m_ShowTextEdit    ) TextEditWidget ::Instance().Show( &m_ShowTextEdit );
	if( m_ShowWorkspace   ) WorkspaceWidget::Instance().Show( &m_ShowWorkspace );
	if( m_ShowOutput      ) OutputWidget   ::Instance().Show( &m_ShowOutput );

} // Show

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::OnDragDrop( const Drop& rDrop, int X, int Y )
{
	if( m_ShowTextEdit )
		TextEditWidget::Instance().OnDragDrop( rDrop, X, Y );

} // OnDragDrop

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

void MainMenuBar::ActionFileExit( void )
{
	exit( 0 );

} // ActionFileExit

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionBuildBuildAndRun( void )
{
	if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
	{
		OutputWidget::Instance().ClearCapture();

		*pWorkspace ^= [ this ]( const WorkspaceBuildFinished& rEvent )
		{
			const std::string OutputString = rEvent.Output.string();

			std::cout << "=== Running " << OutputString << "===\n";
			Process process( rEvent.Output.wstring() );
			std::cout << "=== " << OutputString << " finished with exit code " << process.ExitCode() << " ===\n";
		};

		pWorkspace->Build();
	}

} // ActionBuildBuildAndRun

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionBuildBuild( void )
{
	if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
	{
		OutputWidget::Instance().ClearCapture();

		pWorkspace->Build();
	}

} // ActionBuildBuild

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionViewTextEdit( void )
{
	m_ShowTextEdit ^= 1;

} // ActionViewTextEdit

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionViewWorkspace( void )
{
	m_ShowWorkspace ^= 1;

} // ActionViewWorkspace

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionViewSettings( void )
{
	Settings& rSettings = Settings::Instance();

	SettingsModal::Instance().Show( &rSettings.m_Object );

} // ActionViewSettings

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionViewOutput( void )
{
	m_ShowOutput ^= 1;

} // ActionViewOutput

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionHelpDemo( void )
{
	m_ShowDemoWindow ^= 1;

} // ActionHelpDemo

//////////////////////////////////////////////////////////////////////////

void MainMenuBar::ActionHelpAbout( void )
{
	m_ShowAboutWindow ^= 1;

} // ActionHelpAbout

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
		for( BuildMatrix::NamedConfiguration& rConfiguration : rColumn.Configurations )
		{
			if( ImGui::Selectable( rConfiguration.Name.c_str() ) )
				rColumn.CurrentConfiguration = rConfiguration.Name;
		}

		ImGui::EndCombo();
	}

	// Add combos for exclusive categories for the selected configuration
	for( BuildMatrix::NamedConfiguration& rConfiguration : rColumn.Configurations )
	{
		if( rConfiguration.Name == rColumn.CurrentConfiguration )
		{
			for( BuildMatrix::Column& rExclusiveColumn : rConfiguration.ExclusiveColumns )
				AddBuildMatrixColumn( rExclusiveColumn );

			break;
		}
	}

} // AddBuildMatrixColumn
