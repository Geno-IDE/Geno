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

#include "MainWindow.h"

#include "Common/Platform/Win32/Win32DropTarget.h"
#include "Common/LocalAppData.h"
#include "GUI/MainMenuBar.h"
#include "GUI/PrimaryMonitor.h"
#include "Misc/Settings.h"
#include "ThirdParty/GLEW.h"

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <imgui_internal.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

MainWindow::MainWindow( void )
{
	PrimaryMonitor& monitor = PrimaryMonitor::Instance();

	width_  = 3 * monitor.Width()  / 4;
	height_ = 3 * monitor.Height() / 4;

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

	if( ( window_ = glfwCreateWindow( width_, height_, "Geno", nullptr, nullptr ) ) == nullptr )
		return;

	glfwSetWindowUserPointer( window_, this );
	glfwMakeContextCurrent( window_ );
	glfwSetWindowPos( window_, monitor.X() + ( monitor.Width() - width_ ) / 2, monitor.Y() + ( monitor.Height() - height_ ) / 2 );
	glfwSetWindowSizeCallback( window_, GLFWSizeCB );
	glfwSwapInterval( 1 );

#if defined( _WIN32 )

	// Create drop target
	drop_target_ = new Win32DropTarget( glfwGetWin32Window( window_ ) );

#endif // _WIN32

}

MainWindow::~MainWindow( void )
{

#if defined( _WIN32 )

	// Destroy drop target
	delete drop_target_;

#endif // _WIN32

	if( im_gui_context_ )
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();

		ImGui::DestroyContext( im_gui_context_ );
	}

	if( window_ )
	{
		glfwDestroyWindow( window_ );
	}
}

void MainWindow::Init( void )
{
	if( !im_gui_context_ )
	{
		ini_path_       = LocalAppData::Instance() / "imgui.ini";
		im_gui_context_ = ImGui::CreateContext();

		ImGui::GetIO().IniFilename  = ini_path_.c_str();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

		// Requires GLEW to be initialized
		GLEW::Instance();

		ImGui_ImplGlfw_InitForOpenGL( window_, true );
		ImGui_ImplOpenGL3_Init( "#version 130" );

		// Load settings
		Settings::Instance();
	}
}

void MainWindow::MakeCurrent( void )
{
	glfwMakeContextCurrent( window_ );
}

bool MainWindow::BeginFrame( void )
{
	if( glfwWindowShouldClose( window_ ) )
		return false;

	const ImVec4 col = ImGui::GetStyleColorVec4( ImGuiCol_WindowBg );

	glfwPollEvents();
	glClearColor( col.x, col.y, col.z, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	return true;
}

void MainWindow::EndFrame( void )
{
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

	if( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent( window_ );
	}

	glfwSwapBuffers( window_ );
}

void MainWindow::PushHorizontalLayout( void )
{
	if( layout_stack_counter_++ == 0 )
		im_gui_context_->CurrentWindow->DC.LayoutType = ImGuiLayoutType_Horizontal;
}

void MainWindow::PopHorizontalLayout( void )
{
	if( --layout_stack_counter_ == 0 )
		im_gui_context_->CurrentWindow->DC.LayoutType = ImGuiLayoutType_Vertical;
}

void MainWindow::GLFWSizeCB( GLFWwindow* window, int width, int height )
{
	MainWindow* self = ( MainWindow* )glfwGetWindowUserPointer( window );

	self->width_  = width;
	self->height_ = height;
}
