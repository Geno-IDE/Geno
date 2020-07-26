#include "GUI/Widgets/TextEditWidget.h"
#include "GUI/MainMenuBar.h"
#include "GUI/MainWindow.h"
#include "ThirdParty/GLEW.h"

int main( int /*argc*/, char* /*argv*/[] )
{
	MainWindow::Instance().Init();

	while( MainWindow::Instance().BeginFrame() )
	{
		MainMenuBar::Instance().Show();

		MainWindow::Instance().EndFrame();
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
