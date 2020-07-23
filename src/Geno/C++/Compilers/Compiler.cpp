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

#include <io.h>

#if defined( _WIN32 )
#include <Windows.h>
#endif // _WIN32

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

	startup_info.cb          = sizeof( STARTUPINFO );
	startup_info.wShowWindow = SW_HIDE;
	startup_info.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
	startup_info.hStdOutput  = GetStdHandle( STD_OUTPUT_HANDLE );
	startup_info.hStdError   = GetStdHandle( STD_ERROR_HANDLE );
	startup_info.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	if( int in = _fileno( stdin ); in > 0 )
		startup_info.hStdInput = ( HANDLE )_get_osfhandle( in );

	if( int out = _fileno( stdout ); out > 0 )
		startup_info.hStdOutput = ( HANDLE )_get_osfhandle( out );
	
	if( int err = _fileno( stderr ); err > 0 )
		startup_info.hStdError = ( HANDLE )_get_osfhandle( err );

	if( !CreateProcessW( NULL, &command_line[ 0 ], NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info ) )
	{
		char buf[ 256 ];
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buf, std::size( buf ), NULL );
		std::cerr << "CreateProcessW failed: " << buf;
		return false;
	}

	do
	{
		if( !GetExitCodeProcess( process_info->hProcess, &exit_code ) )
		{
			char buf[ 256 ];
			FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buf, std::size( buf ), NULL );
			std::cerr << "GetExitCodeProcess failed: " << buf;
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

void Compiler::SetPath( path_view path )
{
	path_ = path;
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
