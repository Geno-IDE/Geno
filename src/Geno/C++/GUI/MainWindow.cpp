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
#include "GUI/Widgets/MainMenuBar.h"
#include "GUI/Widgets/OutputWindow.h"
#include "GUI/Widgets/TextEdit.h"
#include "GUI/Widgets/WorkspaceOutliner.h"
#include "GUI/Styles.h"

#include <iostream>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <imgui_internal.h>

#if defined( _WIN32 )
#include "win32-resource.h"
#define sscanf sscanf_s
#endif // _WIN32

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

	// Fix the fact that GLFW doesn't set WS_THICKFRAME or WS_MAXIMIZEBOX for non-decorated windows (Required for resizability)
	SetWindowLong( WindowHandle, GWL_STYLE, GetWindowLong( WindowHandle, GWL_STYLE ) | WS_THICKFRAME | WS_MAXIMIZEBOX );

	// Override window procedure with custom one to allow native window moving behavior without a title bar
	SetWindowLongPtr( WindowHandle, GWLP_USERDATA, ( LONG_PTR )this );
	m_DefaultWindowProc = ( WNDPROC )SetWindowLongPtr( WindowHandle, GWLP_WNDPROC, ( LONG_PTR )CustomWindowProc );

	// Create drop target
	m_pDropTarget = new Win32DropTarget();

#endif // _WIN32

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
	pMenuBar           = new MainMenuBar();
	pWorkspaceOutliner = new WorkspaceOutliner();
	pTextEdit          = new TextEdit();
	pOutputWindow      = new OutputWindow();

} // MainWindow

//////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow( void )
{
	// Destroy widgets
	delete pOutputWindow;
	delete pTextEdit;
	delete pWorkspaceOutliner;
	delete pMenuBar;

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
	const ImVec4 Color = ImGui::GetStyleColorVec4( ImGuiCol_WindowBg );

	glClearColor( Color.x, Color.y, Color.z, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport( nullptr, ImGuiDockNodeFlags_NoWindowMenuButton );
	ImGui::PushFont( m_pFontSans );

	pMenuBar->Draw();

	if( pMenuBar->ShowDemoWindow        ) ImGui::ShowDemoWindow(    &pMenuBar->ShowDemoWindow );
	if( pMenuBar->ShowAboutWindow       ) ImGui::ShowAboutWindow(   &pMenuBar->ShowAboutWindow );
	if( pMenuBar->ShowWorkspaceOutliner ) pWorkspaceOutliner->Show( &pMenuBar->ShowWorkspaceOutliner );
	if( pMenuBar->ShowTextEdit          ) pTextEdit         ->Show( &pMenuBar->ShowTextEdit );
	if( pMenuBar->ShowOutputWindow      ) pOutputWindow     ->Show( &pMenuBar->ShowOutputWindow );

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
	MainWindow* pSelf = ( MainWindow* )pHandler->UserData;
	const char* pName = ( const char* )pEntry;
	int         Bool;

	if(      strcmp( pName, "Text Edit" ) == 0 ) { if( sscanf( pLine, "Active=%d", &Bool ) == 1 ) pSelf->pMenuBar->ShowTextEdit          = Bool; }
	else if( strcmp( pName, "Workspace" ) == 0 ) { if( sscanf( pLine, "Active=%d", &Bool ) == 1 ) pSelf->pMenuBar->ShowWorkspaceOutliner = Bool; }
	else if( strcmp( pName, "Output"    ) == 0 ) { if( sscanf( pLine, "Active=%d", &Bool ) == 1 ) pSelf->pMenuBar->ShowOutputWindow      = Bool; }

} // ImGuiSettingsReadLineCB

//////////////////////////////////////////////////////////////////////////

void MainWindow::ImGuiSettingsWriteAllCB( ImGuiContext* pContext, ImGuiSettingsHandler* pHandler, ImGuiTextBuffer* pOutBuffer )
{
	for( ImGuiWindow* pWindow : pContext->Windows )
	{
		if( pWindow->Active )
		{
			pOutBuffer->appendf( "[%s][%s]\n", pHandler->TypeName, pWindow->Name );
			pOutBuffer->appendf( "Active=%d\n", pWindow->Active );
			pOutBuffer->append( "\n" );
		}
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
				// Drag the menu bar to move the window
				if( !ImGui::IsAnyItemHovered() && ( MousePos.y < ( WindowRect.top + pSelf->pMenuBar->Height() ) ) )
					return HTCAPTION;
			}

		} break;

		case WM_NCCALCSIZE:
		{
			if( WParam == FALSE )
				return 0;

			LPNCCALCSIZE_PARAMS CalcSizeParams = ( LPNCCALCSIZE_PARAMS )LParam;

			// We need to modify the new rectangle for it to resize correctly after the user has snapped the window in some direction.
			InflateRect( &CalcSizeParams->rgrc[ 0 ], -1, -1 );
			InvalidateRect( Handle, &CalcSizeParams->rgrc[ 0 ], TRUE );

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
	}

	return CallWindowProc( pSelf->m_DefaultWindowProc, Handle, Msg, WParam, LParam );

} // CustomWindowProc

#endif // _WIN32
