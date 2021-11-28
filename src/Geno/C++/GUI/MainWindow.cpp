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

#include "MainWindow.h"

#include "Application.h"
#include "Common/LocalAppData.h"
#include "GUI/Modals/IModal.h"
#include "GUI/Platform/Win32/Win32DropTarget.h"
#include "GUI/PrimaryMonitor.h"
#include "GUI/Widgets/TitleBar.h"
#include "GUI/Widgets/OutputWindow.h"
#include "GUI/Widgets/TextEdit.h"
#include "GUI/Widgets/WorkspaceOutliner.h"
#include "GUI/Widgets/StatusBar.h"
#include "GUI/Widgets/FindInWorkspace.h"
#include "GUI/Styles.h"

#include <iostream>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <imgui_internal.h>

#if defined( _WIN32 )
#include <dwmapi.h>
#endif // _WIN32

#if defined( _WIN32 )
#include "win32-resource.h"
#endif // _WIN32

#if defined( __APPLE__ )
#include "GUI/Platform/macOS/macOSContentView.h"
#endif // __APPLE__

//////////////////////////////////////////////////////////////////////////

static std::string IniFilename;

//////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow( void )
{
	glfwSetErrorCallback( GLFWErrorCB );

	if( glfwInit() == GLFW_FALSE )
		return;

	PrimaryMonitor& rMonitor = PrimaryMonitor::Instance();
	m_Width                  = 3 * rMonitor.Width()  / 4;
	m_Height                 = 3 * rMonitor.Height() / 4;

	glfwWindowHint( GLFW_DECORATED, GLFW_FALSE );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	
#if defined( __APPLE__ )
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
#endif // __APPLE__

	if( ( m_pWindow = glfwCreateWindow( m_Width, m_Height, "Geno", nullptr, nullptr ) ) == nullptr )
		return;

	glfwSetWindowUserPointer( m_pWindow, this );
	glfwMakeContextCurrent( m_pWindow );
	glfwSetWindowPos( m_pWindow, rMonitor.X() + ( rMonitor.Width() - m_Width ) / 2, rMonitor.Y() + ( rMonitor.Height() - m_Height ) / 2 );
	glfwSetWindowSizeCallback( m_pWindow, GLFWSizeCB );
	glfwSwapInterval( 1 );

	// Initialize GLEW after creating our graphics context
	glewExperimental = GL_TRUE;
	if( GLenum Result = glewInit(); Result != GLEW_OK )
	{
		std::cerr << "glewInit failed: " << ( const char* )glewGetErrorString( Result ) << "\n";
		return;
	}

#if defined( _WIN32 )

	// Set window icon
	HWND      WindowHandle    = glfwGetWin32Window( m_pWindow );
	HINSTANCE Instance        = GetModuleHandle( nullptr );
	HICON     IconHandleLarge = LoadIcon( Instance, MAKEINTRESOURCE( IDI_ICON_LARGE ) );
	HICON     IconHandleSmall = LoadIcon( Instance, MAKEINTRESOURCE( IDI_ICON_SMALL ) );

	SendMessage( WindowHandle, WM_SETICON, ICON_BIG,   ( LPARAM )IconHandleLarge );
	SendMessage( WindowHandle, WM_SETICON, ICON_SMALL, ( LPARAM )IconHandleSmall );

	// Add some necessary window styles manually.
	// First of all, fix the fact that GLFW doesn't set WS_THICKFRAME or WS_MAXIMIZEBOX for non-decorated windows (Required for resizability).
	// Secondly, add WS_CAPTION so that we get a smooth animation when we minimize and maximize the window.
	SetWindowLong( WindowHandle, GWL_STYLE, GetWindowLong( WindowHandle, GWL_STYLE ) | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX );

	// Fix missing drop shadow
	MARGINS ShadowMargins;
	ShadowMargins = { 1, 1, 1, 1 };
	DwmExtendFrameIntoClientArea( WindowHandle, &ShadowMargins );

	// Override window procedure with custom one to allow native window moving behavior without a title bar
	SetWindowLongPtr( WindowHandle, GWLP_USERDATA, ( LONG_PTR )this );
	m_DefaultWindowProc = ( WNDPROC )SetWindowLongPtr( WindowHandle, GWLP_WNDPROC, ( LONG_PTR )CustomWindowProc );

	// Create drop target
	m_pDropTarget = new Win32DropTarget();

#elif defined( __APPLE__ ) // _WIN32

	_GLFWwindow* pGLFWWindow = reinterpret_cast< _GLFWwindow* >( m_pWindow );
	[ [ [ WindowDelegate alloc ] initWithMainWindow:this:pGLFWWindow ] setGLFWVariables ];
	[ [ [ ContentView alloc ]    initWithMainWindow:this:pGLFWWindow ] setGLFWVariables ];

#endif // __APPLE__

	m_IniPath       = LocalAppData::Instance().Path() / L"imgui.ini";
	m_pImGuiContext = ImGui::CreateContext();
	IniFilename     = m_IniPath.string();

	// Set default style
	Styles::Dark();

	// Configure interface
	ImGuiIO& rIO                     = ImGui::GetIO();
	rIO.IniFilename                  = IniFilename.c_str();
	rIO.ConfigFlags                 |= ImGuiConfigFlags_DockingEnable;
	rIO.ConfigFlags                 |= ImGuiConfigFlags_ViewportsEnable;
	rIO.ConfigViewportsNoTaskBarIcon = true;

	// Load custom fonts
	const std::filesystem::path FontsDir = Application::Instance().GetDataDir() / L"Fonts";
	m_pFontSans = rIO.Fonts->AddFontFromFileTTF( ( FontsDir / L"LieraSans" / L"LieraSans-Regular.ttf" ).string().c_str(), 15.0f );
	m_pFontMono = rIO.Fonts->AddFontFromFileTTF( ( FontsDir / L"LVC-Mono"  / L"LVCMono.otf"           ).string().c_str(), 15.0f );
	rIO.Fonts->Build();

	// Set up custom settings handler
	ImGuiSettingsHandler IniHandler;
	IniHandler.UserData   = this;
	IniHandler.TypeName   = "Geno Widgets";
	IniHandler.TypeHash   = ImHashStr( IniHandler.TypeName );
	IniHandler.ReadOpenFn = ImGuiSettingsReadOpenCB;
	IniHandler.ReadLineFn = ImGuiSettingsReadLineCB;
	IniHandler.WriteAllFn = ImGuiSettingsWriteAllCB;
	m_pImGuiContext->SettingsHandlers.push_back( IniHandler );

	ImGui_ImplGlfw_InitForOpenGL( m_pWindow, true );
	ImGui_ImplOpenGL3_Init( "#version 330 core" );

	// Create widgets
	pTitleBar          = new TitleBar();
	pWorkspaceOutliner = new WorkspaceOutliner();
	pTextEdit          = new TextEdit();
	pOutputWindow      = new OutputWindow();
	pFindInWorkspace   = new FindInWorkspace();

} // MainWindow

//////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow( void )
{
	// Destroy widgets
	delete pOutputWindow;
	delete pTextEdit;
	delete pWorkspaceOutliner;
	delete pTitleBar;
	delete pFindInWorkspace;

#if defined( _WIN32 )

	// Destroy drop target
	delete m_pDropTarget;

#endif // _WIN32

	if( m_pImGuiContext )
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();

		ImGui::DestroyContext( m_pImGuiContext );
	}

	if( m_pWindow )
	{
		glfwDestroyWindow( m_pWindow );
	}

	glfwTerminate();

} // ~MainWindow

//////////////////////////////////////////////////////////////////////////

void MainWindow::MakeCurrent( void )
{
	glfwMakeContextCurrent( m_pWindow );

} // MakeCurrent

//////////////////////////////////////////////////////////////////////////

bool MainWindow::Update( void )
{
	glfwPollEvents();

	return !glfwWindowShouldClose( m_pWindow );

} // Update

//////////////////////////////////////////////////////////////////////////

void MainWindow::Render( void )
{
	if( b_Rendering ) return;
	b_Rendering = true;

	const ImVec4 Color = ImGui::GetStyleColorVec4( ImGuiCol_WindowBg );

	glClearColor( Color.x, Color.y, Color.z, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImGui::NewFrame();

	pViewport->WorkSize.y -= StatusBar::GetHeight();

	ImGui::DockSpaceOverViewport( pViewport );
	ImGui::PushFont( m_pFontSans );

	pTitleBar->Draw();

	if( pTitleBar->ShowDemoWindow                 ) ImGui::ShowDemoWindow(    &pTitleBar->ShowDemoWindow );
	if( pTitleBar->ShowAboutWindow                ) ImGui::ShowAboutWindow(   &pTitleBar->ShowAboutWindow );
	if( pTitleBar->ShowWorkspaceOutliner          ) pWorkspaceOutliner->Show( &pTitleBar->ShowWorkspaceOutliner );
	if( pTitleBar->ShowTextEdit                   ) pTextEdit         ->Show( &pTitleBar->ShowTextEdit );
	if( pTitleBar->ShowOutputWindow               ) pOutputWindow     ->Show( &pTitleBar->ShowOutputWindow );
	if( pTitleBar->ShowFindInWorkspaceWindow      ) pFindInWorkspace  ->Show( &pTitleBar->ShowFindInWorkspaceWindow );

	StatusBar::Instance().Show();

	// This will update all modals recursively
	if( !m_Modals.empty() )
		m_Modals.front()->Update();

	ImGui::PopFont();
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

	if( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent( m_pWindow );
	}

	glfwSwapBuffers( m_pWindow );

	b_Rendering = false;

} // Render

//////////////////////////////////////////////////////////////////////////

void MainWindow::PushModal( IModal* pModal )
{
	m_Modals.push_back( pModal );

} // PushModal

  //////////////////////////////////////////////////////////////////////////

void MainWindow::PopModal( void )
{
	m_Modals.pop_back();

} // PopModal

//////////////////////////////////////////////////////////////////////////

IModal* MainWindow::NextModal( IModal* pPrevious )
{
	if( auto Modal = std::find( m_Modals.begin(), m_Modals.end(), pPrevious ); Modal != m_Modals.end() )
	{
		if( ++Modal == m_Modals.end() )
			return nullptr;

		return *Modal;
	}

	return nullptr;

} // NextModal

//////////////////////////////////////////////////////////////////////////

void MainWindow::PushHorizontalLayout( void )
{
	if( m_LayoutStackCounter++ == 0 )
		m_pImGuiContext->CurrentWindow->DC.LayoutType = ImGuiLayoutType_Horizontal;

} // PushHorizontalLayout

//////////////////////////////////////////////////////////////////////////

void MainWindow::PopHorizontalLayout( void )
{
	if( --m_LayoutStackCounter == 0 )
		m_pImGuiContext->CurrentWindow->DC.LayoutType = ImGuiLayoutType_Vertical;

} // PopHorizontalLayout

//////////////////////////////////////////////////////////////////////////

void MainWindow::Minimize( void )
{
	const bool WasMinimized = ( glfwGetWindowAttrib( m_pWindow, GLFW_ICONIFIED ) == GLFW_TRUE );

	// Minimize the window, unless it was already minimized. In that case, restore it.
	if( !WasMinimized ) glfwIconifyWindow( m_pWindow );
	else                glfwRestoreWindow( m_pWindow );

} // Minimize

//////////////////////////////////////////////////////////////////////////

void MainWindow::Maximize( void )
{
	const bool WasMaximized = ( glfwGetWindowAttrib( m_pWindow, GLFW_MAXIMIZED ) == GLFW_TRUE );

	// Maximize the window, unless it was already maximized. In that case, restore it.
	if( !WasMaximized ) glfwMaximizeWindow( m_pWindow );
	else                glfwRestoreWindow( m_pWindow );

} // Maximize

//////////////////////////////////////////////////////////////////////////

void MainWindow::DragEnter( Drop Drop, int X, int Y )
{
	m_DraggedDrop = std::move( Drop );
	m_DragPosX   = X;
	m_DragPosY   = Y;

} // DragEnter

//////////////////////////////////////////////////////////////////////////

void MainWindow::DragOver( int X, int Y )
{
	m_DragPosX = X;
	m_DragPosY = Y;

} // DragOver

//////////////////////////////////////////////////////////////////////////

void MainWindow::DragLeave( void )
{
	m_DraggedDrop.reset();

} // DragLeave

//////////////////////////////////////////////////////////////////////////

void MainWindow::DragDrop( const Drop& rDrop, int X, int Y )
{
	m_DragPosX = X;
	m_DragPosY = Y;

	pTextEdit->OnDragDrop( rDrop, X, Y );

	m_DraggedDrop.reset();

} // DragDrop

//////////////////////////////////////////////////////////////////////////

void MainWindow::AddRecentWorkspace( const char* pPath )
{
	m_RecentWorkspaces.push_back( pPath );
} // AddRecentWorkspace

//////////////////////////////////////////////////////////////////////////

void MainWindow::GLFWErrorCB( int Error, const char* pDescription )
{
	std::cerr << "GLFW Error: (" << Error << ") " << pDescription << "\n";

} // GLFWErrorCB

//////////////////////////////////////////////////////////////////////////

void MainWindow::GLFWSizeCB( GLFWwindow* pWindow, int Width, int Height )
{
	MainWindow* pSelf = ( MainWindow* )glfwGetWindowUserPointer( pWindow );

	pSelf->m_Width    = Width;
	pSelf->m_Height   = Height;

} // GLFWSizeCB

//////////////////////////////////////////////////////////////////////////

void* MainWindow::ImGuiSettingsReadOpenCB( ImGuiContext* /*pContext*/, ImGuiSettingsHandler* /*pHandler*/, const char* pName )
{
	return ( void* )pName;

} // ImGuiSettingsReadOpenCB

//////////////////////////////////////////////////////////////////////////

void MainWindow::ImGuiSettingsReadLineCB( ImGuiContext* /*pContext*/, ImGuiSettingsHandler* pHandler, void* pEntry, const char* pLine )
{
	MainWindow*           pSelf = ( MainWindow* )pHandler->UserData;
	const char*           pName = ( const char* )pEntry;
	int                   Bool;

	if(      strcmp( pName, "Text Edit" ) == 0 ) { if( sscanf( pLine, "Active=%d", &Bool ) == 1 ) pSelf->pTitleBar->ShowTextEdit          = Bool; }
	else if( strcmp( pName, "Workspace" ) == 0 ) { if( sscanf( pLine, "Active=%d", &Bool ) == 1 ) pSelf->pTitleBar->ShowWorkspaceOutliner = Bool; }
	else if( strcmp( pName, "Output"    ) == 0 ) { if( sscanf( pLine, "Active=%d", &Bool ) == 1 ) pSelf->pTitleBar->ShowOutputWindow      = Bool; }

	// Load Recent Workspaces
	if( strncmp( pLine, "Path=", 5 ) == 0 ) { pSelf->AddRecentWorkspace( pLine + 5 ); }

} // ImGuiSettingsReadLineCB

//////////////////////////////////////////////////////////////////////////

void MainWindow::ImGuiSettingsWriteAllCB( ImGuiContext* pContext, ImGuiSettingsHandler* pHandler, ImGuiTextBuffer* pOutBuffer )
{
	for( ImGuiWindow* pWindow : pContext->Windows )
	{
		pOutBuffer->appendf( "[%s][%s]\n", pHandler->TypeName, pWindow->Name );
		pOutBuffer->appendf( "Active=%d\n", pWindow->WasActive ); // For some reason, Active is set to false while closing the application
		pOutBuffer->append( "\n" );
	}

	for( int I = static_cast< int >( MainWindow::Instance().GetRecentWorkspaces().size() ) - 1; I >= 0; I-- )
	{
		pOutBuffer->appendf( "[%s][%s]\n", pHandler->TypeName, "Recent Workspaces" );
		pOutBuffer->appendf( "Path=%s\n", MainWindow::Instance().GetRecentWorkspaces()[ I ].string().c_str() );
		pOutBuffer->append( "\n" );
	}

} // ImGuiSettingsWriteAllCB

//////////////////////////////////////////////////////////////////////////

#if defined( _WIN32 )

LRESULT MainWindow::CustomWindowProc( HWND Handle, UINT Msg, WPARAM WParam, LPARAM LParam )
{
	MainWindow* pSelf = ( MainWindow* )GetWindowLongPtr( Handle, GWLP_USERDATA );

	switch( Msg )
	{
		case WM_NCHITTEST:
		{
			POINT MousePos;
			RECT  WindowRect;

			GetCursorPos( &MousePos );
			GetWindowRect( Handle, &WindowRect );

			if( PtInRect( &WindowRect, MousePos ) )
			{
				const int BorderX = GetSystemMetrics( SM_CXFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER );
				const int BorderY = GetSystemMetrics( SM_CYFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER );

				if( MousePos.y < ( WindowRect.top + BorderY ) )
				{
					if(      MousePos.x <  ( WindowRect.left  + BorderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); return HTTOPLEFT;  }
					else if( MousePos.x >= ( WindowRect.right - BorderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); return HTTOPRIGHT; }
					else                                                    { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS );   return HTTOP;      }
				}
				else if( MousePos.y >= ( WindowRect.bottom - BorderY ) )
				{
					if(      MousePos.x <  ( WindowRect.left  + BorderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNESW ); return HTBOTTOMLEFT;  }
					else if( MousePos.x >= ( WindowRect.right - BorderX ) ) { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNWSE ); return HTBOTTOMRIGHT; }
					else                                                    { ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeNS );   return HTBOTTOM;      }
				}
				else if( MousePos.x < ( WindowRect.left + BorderX ) )
				{
					ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeEW );
					return HTLEFT;
				}
				else if( MousePos.x >= ( WindowRect.right - BorderX ) )
				{
					ImGui::SetMouseCursor( ImGuiMouseCursor_ResizeEW );
					return HTRIGHT;
				}
				else
				{
					// Drag the menu bar to move the window
					if( !ImGui::IsAnyItemHovered() && ( MousePos.y < ( WindowRect.top + pSelf->pTitleBar->Height() ) ) )
						return HTCAPTION;
				}
			}

		} break;

		case WM_NCCALCSIZE:
		{
			// Fix maximized windows for some reason accounting for border size
			if( WParam == TRUE )
			{
				WINDOWPLACEMENT WindowPlacement{ .length = sizeof( WINDOWPLACEMENT ) };

				if( GetWindowPlacement( Handle, &WindowPlacement ) && WindowPlacement.showCmd == SW_SHOWMAXIMIZED )
				{
					NCCALCSIZE_PARAMS& rParams = *reinterpret_cast< LPNCCALCSIZE_PARAMS >( LParam );
					const int          BorderX = GetSystemMetrics( SM_CXFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER );
					const int          BorderY = GetSystemMetrics( SM_CYFRAME ) + GetSystemMetrics( SM_CXPADDEDBORDER );

					rParams.rgrc[ 0 ].left   += BorderX;
					rParams.rgrc[ 0 ].top    += BorderY;
					rParams.rgrc[ 0 ].right  -= BorderX;
					rParams.rgrc[ 0 ].bottom -= BorderY;

					// Use the rectangle specified in rgrc[0] for the new client area
					return WVR_VALIDRECTS;
				}
			}

			// Preserve the old client area and align it with the upper-left corner of the new client area
			return 0;

		} break;

		case WM_ENTERSIZEMOVE:
		{
			SetTimer( Handle, 1, USER_TIMER_MINIMUM, NULL );

		} break;

		case WM_EXITSIZEMOVE:
		{
			KillTimer( Handle, 1 );

		} break;

		case WM_TIMER:
		{
			const UINT_PTR TimerID = ( UINT_PTR )WParam;

			if( TimerID == 1 )
			{
				pSelf->Render();
			}

		} break;

		case WM_SIZE:
		case WM_MOVE:
		{
			pSelf->Render();

		} break;
	}

	return CallWindowProc( pSelf->m_DefaultWindowProc, Handle, Msg, WParam, LParam );

} // CustomWindowProc

#endif // _WIN32
