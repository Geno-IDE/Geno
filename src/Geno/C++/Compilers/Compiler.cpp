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

#include "Common/Platform/Windows/Win32Error.h"
#include "Common/Platform/Windows/Win32ProcessInfo.h"
#include "Common/LocalAppData.h"

#include <codecvt>
#include <future>
#include <iostream>

#include <io.h>

#if defined( _WIN32 )
#include <Windows.h>
#endif // _WIN32

void Compiler::Compile( const std::filesystem::path& path )
{
	if( !std::filesystem::exists( path ) )
	{
		std::cerr << "Failed to compile " << path.string() << ". File does not exist.\n";
		return;
	}

//////////////////////////////////////////////////////////////////////////

	Args args;
	args.input  = path;
	args.output = path;
	args.output.replace_extension( "exe" );

	std::future future = std::async( &Compiler::AsyncCB, this, std::move( args ) );

	build_futures_.emplace_back( std::move( future ) );
}

void Compiler::SetPath( path_view path )
{
	std::scoped_lock lock( path_mutex_ );
	path_ = path;
}

std::wstring Compiler::MakeCommandLine( const Args& args )
{
	std::wstring string;
	string.reserve( 128 );

	// GCC. Must be added first.
	{
		std::scoped_lock lock( path_mutex_ );
		string += ( path_ / L"bin" / L"g++" ) += L" ";
	}

	// Output file. Must be added last
	string += L" -o \"" + args.output.wstring() + L"\"";

	// Input file. Must be added last
	string += L" \"" + args.input.wstring() +  L"\"";

	return string;
}

void Compiler::AsyncCB( Args args )
{
#if defined( _WIN32 )

	std::wstring     command_line = MakeCommandLine( args );
	path_string      cd           = LocalAppData::Instance().Path();
	STARTUPINFO      startup_info = { };
	int              fd_in        = _fileno( stdin );
	int              fd_out       = _fileno( stdout );
	int              fd_err       = _fileno( stderr );
	Win32ProcessInfo process_info;

	startup_info.cb          = sizeof( STARTUPINFO );
	startup_info.wShowWindow = SW_HIDE;
	startup_info.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startup_info.hStdInput   = ( ( fd_in  > 0 ) ? ( HANDLE )_get_osfhandle( fd_in )  : GetStdHandle( STD_INPUT_HANDLE ) );
	startup_info.hStdOutput  = ( ( fd_out > 0 ) ? ( HANDLE )_get_osfhandle( fd_out ) : GetStdHandle( STD_OUTPUT_HANDLE ) );
	startup_info.hStdError   = ( ( fd_err > 0 ) ? ( HANDLE )_get_osfhandle( fd_err ) : GetStdHandle( STD_ERROR_HANDLE ) );

	if( !WIN32_CALL( CreateProcessW( nullptr, &command_line[ 0 ], nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startup_info, &process_info ) ) )
		return;

//////////////////////////////////////////////////////////////////////////

	BOOL  result;
	DWORD exit_code;

	while( WIN32_CALL( result = GetExitCodeProcess( process_info->hProcess, &exit_code ) ) && exit_code == STILL_ACTIVE )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	if( result )
	{
		CompilationDone e;
		e.path      = args.input;
		e.exit_code = ( int )exit_code;

		Publish( e );
	}

#else // _WIN32

#error Can not invoke compiler

#endif // else
}
