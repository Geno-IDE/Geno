#include "Debugging/Console.h"
#include "GUI/MainMenuBar.h"
#include "GUI/MainWindow.h"
#include "GUI/TextEditor.h"
#include "ThirdParty/GLEW.h"

int main( int /*argc*/, char* /*argv*/[] )
{

#if defined( DEBUG )
	Console console;
#endif // DEBUG

	MainWindow::Instance().Init();

	TextEditor text_editor;

	while( MainWindow::Instance().BeginFrame() )
	{
		text_editor.Show();

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
