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
#include "Platform/Windows/Win32Error.h"
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
	int              fd_in        = _fileno( stdin );
	int              fd_out       = _fileno( stdout );
	int              fd_err       = _fileno( stderr );
	Win32ProcessInfo process_info;
	DWORD            exit_code;

	startup_info.cb          = sizeof( STARTUPINFO );
	startup_info.wShowWindow = SW_HIDE;
	startup_info.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startup_info.hStdInput   = ( ( fd_in  > 0 ) ? ( HANDLE )_get_osfhandle( fd_in )  : GetStdHandle( STD_INPUT_HANDLE ) );
	startup_info.hStdOutput  = ( ( fd_out > 0 ) ? ( HANDLE )_get_osfhandle( fd_out ) : GetStdHandle( STD_OUTPUT_HANDLE ) );
	startup_info.hStdError   = ( ( fd_err > 0 ) ? ( HANDLE )_get_osfhandle( fd_err ) : GetStdHandle( STD_ERROR_HANDLE ) );

	if( !WIN32_CALL( CreateProcessW( NULL, &command_line[ 0 ], NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info ) ) )
		return false;

	do
	{
		if( !WIN32_CALL( GetExitCodeProcess( process_info->hProcess, &exit_code ) ) )
			return false;

		Sleep( 1 );

	} while( exit_code == STILL_ACTIVE );

	if( exit_code == 0 )
		std::cout << "Build successful!\n";

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
