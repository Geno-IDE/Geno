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
#include "GUI/Widgets/StatusBar.h"
#include "GUI/Modals/DiscordRPCSettingsModal.h"
#include "GUI/Platform/Linux/X11WindowDrag.h"
#include "GUI/Platform/Linux/X11WindowResize.h"
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
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <X11/Xlib.h>
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

#if defined( __APPLE__ )
		ImGui::SetCursorPosX( 67.0f + m_Height * ( 1.0f - IconSizeRatio ) * 0.5f );
#else
		ImGui::SetCursorPosX( m_Height * ( 1.0f - IconSizeRatio ) * 0.5f );
#endif
		ImGui::SetCursorPosY( m_Height * ( 1.0f - IconSizeRatio ) * 0.5f );
		ImGui::Image( m_IconTexture.GetID(), ImVec2( m_Height, m_Height ) * IconSizeRatio );
		ImGui::SetCursorPosY( 0.0f );

		if( ImGui::BeginMenu( "File" ) )
		{
			if( ImGui::BeginMenu( "Open" ) )
			{
				if( ImGui::MenuItem( "Open Workspace", "Ctrl+O", false, true ) ) ActionFileOpenWorkspace();

				if( ImGui::BeginMenu( "Open Recent" ) )
				{
					for( int i = 0; i < static_cast< int >( MainWindow::Instance().GetRecentWorkspaces().size() ); i++ )
					{
						auto& rPath = MainWindow::Instance().GetRecentWorkspaces()[ i ];

						if( ImGui::MenuItem( rPath.string().c_str(), "", false, true ) ) ActionFileOpenRecentWorkspace( rPath );
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if( ImGui::MenuItem( "New Workspace", "Ctrl+N", false, true ) ) ActionFileNewWorkspace();
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

			ImGui::MenuItem( "Find Files in Workspace", "Alt+J", &ShowFindInWorkspaceWindow );

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

#if !defined( __APPLE__ )
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
				{
					if( Application::Instance().CurrentWorkspace()->m_AppProcess )
						Application::Instance().CurrentWorkspace()->m_AppProcess->Kill();

					exit( 0 );
				}

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
		}
#endif

#if defined( __linux__ )
		{
			GLFWwindow* pWindow          = MainWindow::Instance().GetWindow();
			Display*    pX11Display      = glfwGetX11Display();
			Window      X11Window        = glfwGetX11Window( pWindow );
			ImVec2      CursorPos        = ImGui::GetMousePos();
			int         WindowSize[ 2 ]  = { 0, 0 };
			int         Border           = 8;

			glfwGetWindowSize( pWindow, &WindowSize[ 0 ], &WindowSize[ 1 ] );

			XWindowAttributes WindowAttributes;
			XGetWindowAttributes( pX11Display, X11Window, &WindowAttributes );

			int    X, Y;
			Window JunkWindow;
			XTranslateCoordinates( pX11Display, X11Window, WindowAttributes.root, -WindowAttributes.border_width, -WindowAttributes.border_width, &X, &Y, &JunkWindow );

			ImRect WindowRect;
			WindowRect.Min = ImVec2( static_cast< float >( Y ), static_cast< float >( Y ) );
			WindowRect.Max = ImVec2( static_cast< float >( WindowSize[ 0 ] ), static_cast< float >( WindowSize[ 1 ] ) );

			if( ImGui::IsMousePosValid( &CursorPos ) && !ImGui::IsAnyItemHovered() )
			{
				const bool Left   = ( CursorPos.x <= ( X                   + Border ) );
				const bool Right  = ( CursorPos.x >= ( X + WindowSize[ 0 ] - Border ) );
				const bool Top    = ( CursorPos.y <= ( Y                   + Border ) );
				const bool Bottom = ( CursorPos.y >= ( Y + WindowSize[ 1 ] - Border ) );

				// Handle resize grabs unless the window is maximized
				switch( ( ( Left << 0 ) | ( Right << 1 ) | ( Top << 2 ) | ( Bottom << 3 ) ) * !( glfwGetWindowAttrib( pWindow, GLFW_MAXIMIZED ) == GLFW_TRUE ) )
				{
					/* _NET_WM_MOVERESIZE_SIZE_LEFT        */ case  1: { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeEW   ); if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) ResizeWindow( pWindow, 7 ); } break;
					/* _NET_WM_MOVERESIZE_SIZE_RIGHT       */ case  2: { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeEW   ); if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) ResizeWindow( pWindow, 3 ); } break;
					/* _NET_WM_MOVERESIZE_SIZE_TOP         */ case  4: { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS   ); if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) ResizeWindow( pWindow, 1 ); } break;
					/* _NET_WM_MOVERESIZE_SIZE_BOTTOM      */ case  8: { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS   ); if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) ResizeWindow( pWindow, 5 ); } break;
					/* _NET_WM_MOVERESIZE_SIZE_TOPLEFT     */ case  5: { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) ResizeWindow( pWindow, 0 ); } break;
					/* _NET_WM_MOVERESIZE_SIZE_TOPRIGHT    */ case  6: { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) ResizeWindow( pWindow, 2 ); } break;
					/* _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT  */ case  9: { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) ResizeWindow( pWindow, 6 ); } break;
					/* _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT */ case 10: { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) ResizeWindow( pWindow, 4 ); } break;

					default:
					{
						if( CursorPos.y < ( Y + m_Height ) )
						{
							if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
								MainWindow::Instance().Maximize();
							else if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
								DragWindow( pWindow );
						}
					}
					break;
				}
			}
		}
#endif // __linux__

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
		if( ImGui::IsKeyPressed( GLFW_KEY_J ) ) ShowFindInWorkspaceWindow ^= 1;
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
		auto& RecentWorkspaces = MainWindow::Instance().GetRecentWorkspaces();

		if( RecentWorkspaces.size() < 5 )
			RecentWorkspaces.insert( MainWindow::Instance().GetRecentWorkspaces().begin(), rFile );
		else
		{
			RecentWorkspaces.pop_back();
			RecentWorkspaces.insert( MainWindow::Instance().GetRecentWorkspaces().begin(), rFile );
		}


		Application::Instance().LoadWorkspace( rFile );
	} );

} // ActionFileOpenWorkspace

//////////////////////////////////////////////////////////////////////////

void TitleBar::ActionFileOpenRecentWorkspace( std::filesystem::path Path )
{
	Application::Instance().CloseWorkspace();
	Application::Instance().LoadWorkspace( Path );

	// Find the same value and put that remove and then put it at the front of the vector.
	auto& RecentWorkspaces = MainWindow::Instance().GetRecentWorkspaces();

	if( std::find( RecentWorkspaces.begin(), RecentWorkspaces.end(), Path ) != RecentWorkspaces.end() )
	{
		for( auto it = RecentWorkspaces.begin(); it != RecentWorkspaces.end(); )
		{
			if( *it == Path )
				it = RecentWorkspaces.erase( it );
			else
				++it;
		}
	}

	RecentWorkspaces.insert( RecentWorkspaces.begin(), Path );

} // ActionFileOpenRecentWorkspace

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

		// Save all open files before building
		if( MainWindow::Instance().pTextEdit )
		{
			TextEdit& rTextEdit = *MainWindow::Instance().pTextEdit;

			for( TextEdit::File& rFile : rTextEdit.Files )
				rTextEdit.SaveFile( rFile );
		}

		pWorkspace->Events.BuildFinished += []( Workspace& rWorkspace, std::filesystem::path OutputFile, bool /*Success*/ )
		{
			StatusBar::Instance().SetColor( StatusBar::Color::ORANGE );

			const std::string OutputString = OutputFile.string();
			const std::wstring OutputWString = OutputFile.wstring();

			std::cout << "=== Running " << OutputString << "===\n";

			rWorkspace.m_AppProcess = std::make_unique<Process>( OutputWString );

			const int ExitCode = rWorkspace.m_AppProcess->ResultOf();
			std::cout << "=== " << OutputString << " finished with exit code " << ExitCode << " ===\n";

			StatusBar::Instance().SetColor( StatusBar::Color::RED );
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

		// Save all open files before building
		if( MainWindow::Instance().pTextEdit )
		{
			TextEdit& rTextEdit = *MainWindow::Instance().pTextEdit;

			for( TextEdit::File& rFile : rTextEdit.Files )
				rTextEdit.SaveFile( rFile );
		}

		pWorkspace->Build();
	}

} // ActionBuildBuild

//////////////////////////////////////////////////////////////////////////

void TitleBar::AddBuildMatrixColumn( BuildMatrix::Column& rColumn )
{
	ImGui::Spacing();
	ImGui::Text( "%s:", rColumn.Name.c_str() );

	const std::string Label         = "##" + rColumn.Name;
	const char*       pPreviewValue = ( rColumn.CurrentConfiguration >= 0 && rColumn.CurrentConfiguration < static_cast< int32_t >( rColumn.Configurations.size() ) ) ? rColumn.Configurations[ rColumn.CurrentConfiguration ].first.c_str() : "";

	// Add combo for this column
	ImGui::SetNextItemWidth( 100.0f );
	ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 1, 1 ) );
	ImGui::SetCursorPosY( 10.0f );
	if( ImGui::BeginCombo( Label.c_str(), pPreviewValue ) )
	{
		for( size_t i = 0; i < rColumn.Configurations.size(); ++i )
		{
			if( ImGui::Selectable( rColumn.Configurations[ i ].first.c_str() ) )
				rColumn.CurrentConfiguration = static_cast< int32_t >( i );
		}

		ImGui::EndCombo();
	}
	ImGui::SetCursorPosY( 0.0f );
	ImGui::PopStyleVar();

} // AddBuildMatrixColumn
