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

#include <codecvt>
#include <future>
#include <iostream>

#include <io.h>

#if defined( _WIN32 )
#include <Windows.h>
#endif // _WIN32

bool Compiler::IsBuilding( void ) const
{
	return ( build_future_.valid() && build_future_.wait_until( std::chrono::steady_clock::now() ) != std::future_status::ready );
}

void Compiler::Compile( std::wstring_view cpp )
{
	if( !std::filesystem::exists( cpp ) )
	{
		wstring_convert_utf8 wstring_converter;
		std::string          cpp_utf8 = wstring_converter.to_bytes( cpp.data(), cpp.data() + cpp.size() );

		std::cerr << "Failed to compile " << cpp_utf8 << ". File does not exist.\n";
		return;
	}

	if( IsBuilding() )
	{
		wstring_convert_utf8 wstring_converter;
		std::string          cpp_utf8 = wstring_converter.to_bytes( cpp.data(), cpp.data() + cpp.size() );

		std::cerr << "Build already in progress (won't compile " << cpp_utf8 << ").\n";
		return;
	}

//////////////////////////////////////////////////////////////////////////

	Args args;
	args.input  = cpp;
	args.output = cpp;
	args.output.replace_extension( "exe" );

	build_future_ = std::async( &Compiler::AsyncCB, this, std::move( args ) );
}

void Compiler::SetPath( path_view path )
{
	std::scoped_lock lock( path_mutex_ );
	path_ = path;
}

Compiler& Compiler::Instance( void )
{
	static Compiler instance;
	return instance;
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
	DWORD            exit_code;

	startup_info.cb          = sizeof( STARTUPINFO );
	startup_info.wShowWindow = SW_HIDE;
	startup_info.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startup_info.hStdInput   = ( ( fd_in  > 0 ) ? ( HANDLE )_get_osfhandle( fd_in )  : GetStdHandle( STD_INPUT_HANDLE ) );
	startup_info.hStdOutput  = ( ( fd_out > 0 ) ? ( HANDLE )_get_osfhandle( fd_out ) : GetStdHandle( STD_OUTPUT_HANDLE ) );
	startup_info.hStdError   = ( ( fd_err > 0 ) ? ( HANDLE )_get_osfhandle( fd_err ) : GetStdHandle( STD_ERROR_HANDLE ) );

	if( !WIN32_CALL( CreateProcessW( NULL, &command_line[ 0 ], NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info ) ) )
		return;

	do
	{
		if( !WIN32_CALL( GetExitCodeProcess( process_info->hProcess, &exit_code ) ) )
			return;

		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

	} while( exit_code == STILL_ACTIVE );

	CompilerDone e;
	e.exit_code = ( int )exit_code;

	Publish( e );

#else // _WIN32

#error Can not invoke compiler

#endif // else
}
