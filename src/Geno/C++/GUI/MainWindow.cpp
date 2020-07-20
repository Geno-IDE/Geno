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

#include <array>
#include <cstring>

#if defined( _WIN32 )
#include <ShlObj.h>
#elif defined( __linux__ ) // _WIN32
#include <sys/stat.h>
#endif // __linux__

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

	glfwSetWindowUserPointer( window_, this );
	glfwMakeContextCurrent( window_ );
	glfwSetWindowPos( window_, monitor.X() + ( monitor.Width() - width_ ) / 2, monitor.Y() + ( monitor.Height() - height_ ) / 2 );
	glfwSetWindowSizeCallback( window_, GLFWSizeCB );
	glfwSwapInterval( 1 );
}

MainWindow::~MainWindow( void )
{
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
		im_gui_context_ = ImGui::CreateContext();

		// Disable settings saving by default
		ImGui::GetIO().IniFilename = nullptr;

		// #TODO: get user file on other platforms

#if defined( _WIN32 )

		static char filename[ FILENAME_MAX + 1 ] = { };

		do 
		{
			PWSTR buf;
			if( SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, nullptr, &buf ) != S_OK )
				break;

			size_t num_converted;
			if( wcstombs_s( &num_converted, filename, buf, MAX_PATH ) != 0 )
				break;

			if( strcat_s( filename, std::size( filename ), "\\Geno" ) != 0 )
				break;

			if( CreateDirectoryA( filename, nullptr ) || GetLastError() == ERROR_ALREADY_EXISTS )
			{
				if( strcat_s( filename, std::size( filename ), "\\imgui.ini" ) != 0 )
					break;

				ImGui::GetIO().IniFilename = filename;
			}

		} while( false );

#elif defined( __linux__ ) // _WIN32

		static char filename[ FILENAME_MAX + 1 ] = { };

		do 
		{
			if( const char* data_home = getenv( "XDG_DATA_HOME" ); data_home != nullptr )
			{
				strcpy( filename, data_home );
			}
			else if( const char* data_dirs = getenv( "XDG_DATA_DIRS" ); data_home != nullptr )
			{
				if( const char* colon = strchr( data_dirs, ':' ); colon != nullptr )
				{
					strncpy( filename, data_dirs, colon - data_dirs - 1 );
				}
				else
				{
					strcpy( filename, data_dirs );
				}
			}
			else
			{
				break;
			}

			strcat( filename, "/geno" );

			if( mkdir( filename, 0777 ) != 0 )
				break;

			strcat( filename, "/imgui.ini" );

			ImGui::GetIO().IniFilename = filename;

		} while( false );

#endif // __linux__

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

	const ImVec4 col = ImGui::GetStyleColorVec4( ImGuiCol_WindowBg );

	glfwPollEvents();
	glClearColor( col.x, col.y, col.z, 1.0f );
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

void MainWindow::GLFWSizeCB( GLFWwindow* window, int width, int height )
{
	MainWindow* self = ( MainWindow* )glfwGetWindowUserPointer( window );

	self->width_  = width;
	self->height_ = height;
}
