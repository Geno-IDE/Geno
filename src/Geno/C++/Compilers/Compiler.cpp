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

#include "Compiler.h"

#include "Core/LocalAppData.h"
#include "Platform/Windows/Win32ProcessInfo.h"

#include <iostream>

#if defined( _WIN32 )
#include <Windows.h>
#endif // _WIN32

Compiler::Compiler( void )
{
}

bool Compiler::Compile( std::wstring_view cpp )
{
	if( !std::filesystem::exists( cpp ) )
		return false;

//////////////////////////////////////////////////////////////////////////

	Args args;
	args.input  = cpp;
	args.output = cpp;
	args.output.replace_extension( "exe" );

//////////////////////////////////////////////////////////////////////////

#if defined( _WIN32 )

	std::wstring     command_line = MakeCommandLine( args );
	path_string      cd           = LocalAppData::Instance().Path();
	STARTUPINFO      startup_info = { };
	Win32ProcessInfo process_info;
	DWORD            exit_code;

	startup_info.cb = sizeof( STARTUPINFO );

	if( !CreateProcessW( NULL, &command_line[ 0 ], NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info ) )
	{
		wchar_t buf[ 256 ];
		FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buf, std::size( buf ), NULL );
		std::wcerr << L"CreateProcessW failed: " << buf;
		return false;
	}

	do
	{
		if( !GetExitCodeProcess( process_info->hProcess, &exit_code ) )
		{
			wchar_t buf[ 256 ];
			FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buf, std::size( buf ), NULL );
			std::wcerr << L"GetExitCodeProcess failed: " << buf;
			return false;
		}

		Sleep( 1 );

	} while( exit_code == STILL_ACTIVE );

	if( exit_code == 0 )
	{
		std::cout << "Build successful!\n";
	}

	return ( exit_code == 0 );

#else // _WIN32

#error Invoke compiler

	return false;

#endif // else

}

void Compiler::SetLLVMPath( path_view llvm_path )
{
	path_ = llvm_path;
}

Compiler& Compiler::Instance( void )
{
	static Compiler instance;
	return instance;
}

std::wstring Compiler::MakeCommandLine( const Args& args ) const
{
	std::wstring string;
	string.reserve( 128 );

	// GCC. Must be added first.
	string += ( path_ / L"bin" / L"g++" ) += L" ";

	// Output file. Must be added last
	string += L" -o \"" + args.output.wstring() + L"\"";

	// Input file. Must be added last
	string += L" \"" + args.input.wstring() +  L"\"";

	return string;
}
