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

#include "TitleBar.h"

#include "Application.h"
#include "Auxiliary/STBAux.h"
#include "Compilers/ICompiler.h"
#include "GUI/MainWindow.h"
#include "GUI/Modals/NewItemModal.h"
#include "GUI/Modals/OpenFileModal.h"
#include "GUI/Widgets/OutputWindow.h"
#include "GUI/Widgets/TextEdit.h"
#include "GUI/Widgets/WorkspaceOutliner.h"
#include "GUI/Modals/DiscordRPCSettingsModal.h"
#include "Discord/DiscordRPC.h"

#include <Common/LocalAppData.h>
#include <Common/Process.h>

#include <GLFW/glfw3.h>
#include <imgui_internal.h>

#include <functional>
#include <imgui.h>
#include <iostream>
#include <numeric>
#include <string>

#if defined( __linux__ )
#include "GUI/Platform/Liunx/X11/glfwX11.h"
#endif // __linux__

//////////////////////////////////////////////////////////////////////////

TitleBar::TitleBar( void )
	: m_IconTexture( STBAux::LoadImageTexture( "Icons/Geno.png" ) )
{

} // TitleBar

//////////////////////////////////////////////////////////////////////////

void TitleBar::Draw( void )
{
	const bool WorkspaceActive = Application::Instance().CurrentWorkspace() != nullptr;

	ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 10.0f, 10.0f ) );

	if( ImGui::BeginMainMenuBar() )
	{
		constexpr float IconSizeRatio = 0.85f;

		m_Height = ImGui::GetWindowHeight();

		ImGui::SetCursorPosX( m_Height * ( 1.0f - IconSizeRatio ) * 0.5f );
		ImGui::SetCursorPosY( m_Height * ( 1.0f - IconSizeRatio ) * 0.5f );
		ImGui::Image( m_IconTexture.GetID(), ImVec2( m_Height, m_Height ) * IconSizeRatio );
		ImGui::SetCursorPosY( 0.0f );

		if( ImGui::BeginMenu( "File" ) )
		{
			if( ImGui::MenuItem( "New Workspace", "Ctrl+N", false, true ) ) ActionFileNewWorkspace();
			if( ImGui::MenuItem( "Open Workspace", "Ctrl+O", false, true ) ) ActionFileOpenWorkspace();
			if( ImGui::MenuItem( "Close Workspace", "Ctrl+W", false, WorkspaceActive ) ) ActionFileCloseWorkspace();

			ImGui::Separator();

			if( ImGui::MenuItem( "Exit", "Alt+E" ) ) exit( 0 );

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Build", WorkspaceActive ) )
		{
			if( ImGui::MenuItem( "Build And Run", "F5" ) ) ActionBuildBuildAndRun();
			if( ImGui::MenuItem( "Build", "F7" ) ) ActionBuildBuild();

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "View" ) )
		{
			ImGui::MenuItem( "Text Edit", "Alt+T", &ShowTextEdit );
			ImGui::MenuItem( "Workspace", "Alt+W", &ShowWorkspaceOutliner );
			ImGui::MenuItem( "Output", "Alt+O", &ShowOutputWindow );

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Help" ) )
		{
			if( ImGui::MenuItem( "Demo" ) ) ShowDemoWindow ^= 1;
			if( ImGui::MenuItem( "About" ) ) ShowAboutWindow ^= 1;

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Extensions" ) )
		{
			if( ImGui::MenuItem( "GenoDiscord" ) ) ActionExtShowGenoDiscord();

			ImGui::EndMenu();
		}

		if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
		{
			ImGui::SetCursorPosX( ImGui::GetCursorPosX() + 10.0f );
			ImGui::Separator();
			ImGui::SetCursorPosX( ImGui::GetCursorPosX() + 10.0f );

			for( BuildMatrix::Column& rColumn : pWorkspace->m_BuildMatrix.m_Columns )
			{
				AddBuildMatrixColumn( rColumn );
			}
		}

		// System buttons
		{
			const float  ButtonSize = ImGui::GetFrameHeight();
			const float  IconMargin = ButtonSize * 0.33f;
			ImRect       ButtonRect = ImRect( ImGui::GetWindowPos() + ImVec2( ImGui::GetWindowWidth() - ButtonSize, 0.0f ), ImGui::GetWindowPos() + ImGui::GetWindowSize() );
			ImDrawList*  pDrawList  = ImGui::GetWindowDrawList();

			// Exit button
			{
				bool Hovered = false;
				bool Held    = false;
				bool Pressed = ImGui::ButtonBehavior( ButtonRect, ImGui::GetID( "EXIT" ), &Hovered, &Held, 0 );

				if( Hovered )
				{
					const ImU32 Color = ImGui::GetColorU32( Held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered );
					pDrawList->AddRectFilled( ButtonRect.Min, ButtonRect.Max, Color );
				}

				// Render the cross
				{
					const ImU32 Color = ImGui::GetColorU32( ImGuiCol_Text );
					pDrawList->AddLine( ButtonRect.Min + ImVec2( IconMargin, IconMargin ),                         ButtonRect.Max - ImVec2( IconMargin, IconMargin ),                         Color, 1.0f );
					pDrawList->AddLine( ButtonRect.Min + ImVec2( ButtonRect.GetWidth() - IconMargin, IconMargin ), ButtonRect.Max - ImVec2( ButtonRect.GetWidth() - IconMargin, IconMargin ), Color, 1.0f );
				}

				if( Pressed )
					exit( 0 );

				ButtonRect.Min.x -= ButtonSize;
				ButtonRect.Max.x -= ButtonSize;
			}

			// Maximize button
			{
				bool Hovered = false;
				bool Held    = false;
				bool Pressed = ImGui::ButtonBehavior( ButtonRect, ImGui::GetID( "MAXIMIZE" ), &Hovered, &Held, 0 );

				if( Hovered )
				{
					const ImU32 Color = ImGui::GetColorU32( Held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered );
					pDrawList->AddRectFilled( ButtonRect.Min, ButtonRect.Max, Color );
				}

				// Render the box
				{
					const ImU32 Color = ImGui::GetColorU32( ImGuiCol_Text );
					pDrawList->AddRect( ButtonRect.Min + ImVec2( IconMargin, IconMargin ), ButtonRect.Max - ImVec2( IconMargin, IconMargin ), Color, 0.0f, 0, 1.0f );
				}

				if( Pressed )
					MainWindow::Instance().Maximize();

				ButtonRect.Min.x -= ButtonSize;
				ButtonRect.Max.x -= ButtonSize;
			}

			// Minimize button
			{
				bool Hovered = false;
				bool Held    = false;
				bool Pressed = ImGui::ButtonBehavior( ButtonRect, ImGui::GetID( "MINIMIZE" ), &Hovered, &Held, 0 );

				if( Hovered )
				{
					const ImU32 Color = ImGui::GetColorU32( Held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered );
					pDrawList->AddRectFilled( ButtonRect.Min, ButtonRect.Max, Color );
				}

				// Render the Line
				{
					const ImU32 Color = ImGui::GetColorU32( ImGuiCol_Text );
					pDrawList->AddLine( ButtonRect.Min + ImVec2( IconMargin, ButtonRect.GetHeight() / 2 ), ButtonRect.Max - ImVec2( IconMargin, ButtonRect.GetHeight() / 2 ), Color, 1.0f );
				}

				if( Pressed )
					MainWindow::Instance().Minimize();

				ButtonRect.Min.x -= ButtonSize;
				ButtonRect.Max.x -= ButtonSize;
			}

		#if defined ( __linux__ )
			// X11 Window Move
			{
				GLFWwindow* pWindow          = MainWindow::Instance().GetWindow();
				Display* pX11Display      = glfwGetX11Display();
				Window      X11Window        = glfwGetX11Window( pWindow );
				ImVec2      CursorPos        = ImGui::GetMousePos();
				int         WindowSize[ 2 ]  ={ 0, 0 };
				int         Border           = 5;

				glfwGetWindowSize( pWindow, &WindowSize[ 0 ], &WindowSize[ 1 ] );

				int rx, ry;

				XWindowAttributes win_attributes;

				Window junkwin;

				XGetWindowAttributes( pX11Display, X11Window, &win_attributes );

				// Get Top left
				XTranslateCoordinates( pX11Display, X11Window, win_attributes.root, -win_attributes.border_width, -win_attributes.border_width, &rx, &ry, &junkwin );
				Rect WindowRect;
				WindowRect.x1 = rx;
				WindowRect.y1 = ry;
				WindowRect.x2 = WindowSize[ 0 ];
				WindowRect.y2 = WindowSize[ 1 ];

				if( ImGui::IsMousePosValid( &CursorPos ) )
				{
					if( CursorPos.y < ( WindowRect.Top() + Border ) )
					{
						if( CursorPos.x < ( WindowRect.Left() + Border ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); glfwX11ResizeWindow( pWindow, 5 ); }
						else if( CursorPos.x >= ( WindowRect.Right() - Border ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); glfwX11ResizeWindow( pWindow, 6 ); }
						else { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS );     glfwX11ResizeWindow( pWindow, 2 ); }
					}
					else if( CursorPos.y >= ( WindowRect.Bottom() - Border ) )
					{
						if( CursorPos.x < ( WindowRect.Left() + Border ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); glfwX11ResizeWindow( pWindow, 7 ); }
						else if( CursorPos.x >= ( WindowRect.Right() - Border ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); glfwX11ResizeWindow( pWindow, 8 ); }
						else { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS );     glfwX11ResizeWindow( pWindow, 4 ); }
					}
					else if( CursorPos.x < ( WindowRect.Left() + Border ) )
					{
						ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeEW );
						glfwX11ResizeWindow( pWindow, 1 );
					}
					else if( CursorPos.x >= ( WindowRect.Right() - Border ) )
					{
						ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeEW );
						glfwX11ResizeWindow( pWindow, 3 );
					}
					else
					{
						// Drag the menu bar to move the window
						if( !ImGui::IsAnyItemHovered() && ( CursorPos.y < ( m_Height ) ) )
							glfwDragWindow( pWindow );
					}
				}
			}
		#endif
		}

		ImGui::EndMainMenuBar();
	}

	ImGui::PopStyleVar();

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
		if( ImGui::IsKeyPressed( GLFW_KEY_T ) ) ShowTextEdit ^= 1;
		if( ImGui::IsKeyPressed( GLFW_KEY_W ) ) ShowWorkspaceOutliner ^= 1;
		if( ImGui::IsKeyPressed( GLFW_KEY_O ) ) ShowOutputWindow ^= 1;
	}
	else
	{
		if( ImGui::IsKeyPressed( GLFW_KEY_F5 ) ) ActionBuildBuildAndRun();
		if( ImGui::IsKeyPressed( GLFW_KEY_F7 ) ) ActionBuildBuild();
	}

} // Draw

//////////////////////////////////////////////////////////////////////////

void TitleBar::ActionFileNewWorkspace( void )
{
	NewItemModal::Instance().Show( "New Workspace", ".gwks", {}, []( const std::string& rName, const std::filesystem::path& rLocation )
		{ Application::Instance().NewWorkspace( rLocation, rName ); } );

} // ActionFileNewWorkspace

//////////////////////////////////////////////////////////////////////////

void TitleBar::ActionExtShowGenoDiscord( void )
{
	DiscordRPCSettingsModal::Instance().Show();

} // ActionExtShowGenoDiscord

//////////////////////////////////////////////////////////////////////////

void TitleBar::ActionFileOpenWorkspace( void )
{
	OpenFileModal::Instance().Show( "Open Workspace", "*.gwks", []( const std::filesystem::path& rFile )
	{
		Application::Instance().LoadWorkspace( rFile );
	} );

} // ActionFileOpenWorkspace

//////////////////////////////////////////////////////////////////////////

void TitleBar::ActionFileCloseWorkspace( void )
{
	Application::Instance().CloseWorkspace();

} // ActionFileCloseWorkspace

//////////////////////////////////////////////////////////////////////////

void TitleBar::ActionBuildBuildAndRun( void )
{
	if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
	{
		MainWindow::Instance().pOutputWindow->ClearCapture();

		pWorkspace->Events.BuildFinished += []( Workspace& /*rWorkspace*/, std::filesystem::path OutputFile, bool /*Success*/ )
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

void TitleBar::ActionBuildBuild( void )
{
	if( Workspace* pWorkspace = Application::Instance().CurrentWorkspace() )
	{
		MainWindow::Instance().pOutputWindow->ClearCapture();

		pWorkspace->Build();
	}

} // ActionBuildBuild

//////////////////////////////////////////////////////////////////////////

void TitleBar::AddBuildMatrixColumn( BuildMatrix::Column& rColumn )
{
	ImGui::Spacing();
	ImGui::Text( "%s:", rColumn.Name.c_str() );

	const std::string Label = "##" + rColumn.Name;

	// Add combo for this column
	ImGui::SetNextItemWidth( 100.0f );
	ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 1, 1 ) );
	ImGui::SetCursorPosY( 10.0f );
	if( ImGui::BeginCombo( Label.c_str(), rColumn.CurrentConfiguration.c_str() ) )
	{
		for( auto& [ rName, rConfiguration ] : rColumn.Configurations )
		{
			if( ImGui::Selectable( rName.c_str() ) )
				rColumn.CurrentConfiguration = rName;
		}

		ImGui::EndCombo();
	}
	ImGui::SetCursorPosY( 0.0f );
	ImGui::PopStyleVar();

} // AddBuildMatrixColumn
