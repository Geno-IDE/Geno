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

#include "GUI/PrimaryMonitor.h"
#include "ThirdParty/GLEW.h"

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <imgui_internal.h>

MainWindow::MainWindow( void )
: window_        ( nullptr )
, im_gui_context_( nullptr )
, width_         ( 0 )
, height_        ( 0 )
{
	PrimaryMonitor& monitor = PrimaryMonitor::Get();

	width_  = 3 * monitor.Width()  / 4;
	height_ = 3 * monitor.Height() / 4;

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

	if( ( window_ = glfwCreateWindow( width_, height_, "Geno", nullptr, nullptr ) ) == nullptr )
		return;

	glfwMakeContextCurrent( window_ );
	glfwSetWindowPos( window_, monitor.X() + ( monitor.Width() - width_ ) / 2, monitor.Y() + ( monitor.Height() - height_ ) / 2 );
	glfwSwapInterval( 1 );
}

MainWindow::~MainWindow( void )
{
	if( im_gui_context_ )
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();

		ImGui::DestroyContext();
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
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		// Requires GLEW to be initialized
		GLEW::Get();

		ImGui_ImplGlfw_InitForOpenGL( window_, true );
		ImGui_ImplOpenGL3_Init( "#version 130" );
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

	glfwPollEvents();
	glClearColor( 0.25f, 0.25f, 0.25f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();

	return true;
}

void MainWindow::EndFrame( void )
{
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

	glfwSwapBuffers( window_ );
}

void MainWindow::PushHorizontalLayout( void )
{
	im_gui_context_->CurrentWindow->DC.LayoutType = ImGuiLayoutType_Horizontal;
}

void MainWindow::PopHorizontalLayout( void )
{
	im_gui_context_->CurrentWindow->DC.LayoutType = ImGuiLayoutType_Vertical;
}

MainWindow& MainWindow::Get( void )
{
	static MainWindow main_window;
	return main_window;
}
