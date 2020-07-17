#include "Debugging/Console.h"

#include <iostream>

#if defined( _WIN32 )
#include <Windows.h>
#include <GL/glew.h>
#endif // _WIN32

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static void GLFWErrorCB( int error, const char* description )
{
	std::cerr << "GLFW Error: (" << error << ") " << description << "\n";
}

int main( int /*argc*/, char* /*argv*/[] )
{
	Console console;

	glfwSetErrorCallback( GLFWErrorCB );

	if( !glfwInit() )
		return 1;

//////////////////////////////////////////////////////////////////////////

	int          monitor_count   = 0;
	GLFWmonitor* primary_monitor = nullptr;

	if( GLFWmonitor** monitors = glfwGetMonitors( &monitor_count ); monitors != nullptr )
	{
		if( monitor_count > 0 )
			primary_monitor = monitors[ 0 ];
	}

	if( primary_monitor == nullptr )
		return 1;


//////////////////////////////////////////////////////////////////////////

	int monitor_x, monitor_y, monitor_width, monitor_height;

	glfwGetMonitorWorkarea( primary_monitor, &monitor_x, &monitor_y, &monitor_width, &monitor_height );

//////////////////////////////////////////////////////////////////////////

	const int   width  = 3 * monitor_width  / 4;
	const int   height = 3 * monitor_height / 4;
	GLFWwindow* window;

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

	if( ( window = glfwCreateWindow( width, height, "Geno", nullptr, nullptr ) ) == nullptr )
		return 1;

	glfwSetWindowPos( window, monitor_x + ( monitor_width - width ) / 2, monitor_y + ( monitor_height - height ) / 2 );
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 );

	if( GLenum err = glewInit(); err != GLEW_OK )
	{
		std::cerr << "glewInit failed: " << ( const char* )glewGetErrorString( err ) << "\n";
		return 1;
	}

//////////////////////////////////////////////////////////////////////////

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL( window, true );
	ImGui_ImplOpenGL3_Init( "#version 130" );

//////////////////////////////////////////////////////////////////////////

	bool show_demo_window = true;

	while( !glfwWindowShouldClose( window ) )
	{
		glfwPollEvents();
		glClearColor( 0.25f, 0.25f, 0.25f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if( show_demo_window )
			ImGui::ShowDemoWindow( &show_demo_window );

		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

		glfwSwapBuffers( window );
	}

	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();

	ImGui::DestroyContext();

	glfwDestroyWindow( window );
	glfwTerminate();

	return 0;
}

#if defined( _WIN32 )

#include <Windows.h>

int WINAPI WinMain( HINSTANCE /*instance*/, HINSTANCE /*prev_instance*/, LPSTR /*cmd_line*/, int /*cmd_show*/ )
{
	return main( __argc, __argv );
}

#endif // _WIN32
