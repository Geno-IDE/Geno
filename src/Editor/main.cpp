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

#include "Core/Compiler.h"
#include "Core/Console.h"
#include "Editor/Widgets/Window.h"

#include <cstdlib>
#include <thread>

#include <Windows.h>

static Alv::Menu SetupRootMenu( void )
{
	Alv::Menu menu;

	Alv::Menu file_menu;
	file_menu.AddItem( Alv::MenuItem( L"Open" ) );
	file_menu.AddItem( Alv::MenuItem( L"Save" ) );

	Alv::MenuItem file_item( L"File" );
	menu.AddItem( std::move( file_item ) );

	Alv::Menu build_menu;
	build_menu.AddItem( Alv::MenuItem( L"Build" ) );
	build_menu.AddItem( Alv::MenuItem( L"Rebuild" ) );
	build_menu.AddItem( Alv::MenuItem( L"Clean" ) );

	Alv::MenuItem build_item( L"Build" );
	menu.AddItem( std::move( build_item ) );

	return menu;
}

int WINAPI WinMain( HINSTANCE /*instance*/, HINSTANCE /*prev_instance*/, LPSTR /*cmd_line*/, int /*show_cmd*/ )
{
	Alv::Console               console;
	Alv::Compiler              compiler( CFG_LLVM_LOCATION );
	std::vector< Alv::Window > windows;
	windows.resize( 3 );

	Alv::Menu menu = SetupRootMenu();
	windows[ 0 ].SetMenu( menu );

	for( auto& window : windows )
		window.Show();

	while( !windows.empty() )
	{
		for( auto it = windows.begin(); it != windows.end(); )
		{
			it->PollEvents();

			if( it->IsOpen() ) ++it;
			else               it = windows.erase( it );
		}
	}

	return 0;
}
