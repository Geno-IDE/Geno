#include "Debugging/Console.h"
#include "GUI/MainMenuBar.h"
#include "GUI/MainWindow.h"
#include "ThirdParty/GLEW.h"

int main( int /*argc*/, char* /*argv*/[] )
{

#if defined( DEBUG )
	Console console;
#endif // DEBUG

	MainWindow::Get().Init();

	while( MainWindow::Get().BeginFrame() )
	{
		MainMenuBar::Get().Show();

		MainWindow::Get().EndFrame();
	}

	return 0;
}

#if defined( _WIN32 )

#include <Windows.h>

int WINAPI WinMain( HINSTANCE /*instance*/, HINSTANCE /*prev_instance*/, LPSTR /*cmd_line*/, int /*cmd_show*/ )
{
	return main( __argc, __argv );
}

#endif // _WIN32
