/*
 * Copyright (c) 2021 Sebastian Kylander https://gaztin.com/
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

#include "Common/Process.h"

#include "Common/Platform/Win32/Win32Error.h"
#include "Common/Platform/Win32/Win32ProcessInfo.h"

#include <chrono>
#include <codecvt>
#include <thread>

#include <io.h>

#if defined( _WIN32 )
#include <Windows.h>
#endif // _WIN32

//////////////////////////////////////////////////////////////////////////

Process::Process( std::wstring CommandLine )
	: m_CommandLine( std::move( CommandLine ) )
	, m_ExitCode   ( Run() )
{
} // Process

//////////////////////////////////////////////////////////////////////////

int Process::Run( void )
{

#if defined( _WIN32 )

	STARTUPINFO      StartupInfo = { };
	int              FileIn      = _fileno( stdin );
	int              FileOut     = _fileno( stdout );
	int              FileErr     = _fileno( stderr );
	Win32ProcessInfo ProcessInfo;

	StartupInfo.cb          = sizeof( STARTUPINFO );
	StartupInfo.wShowWindow = SW_HIDE;
	StartupInfo.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	StartupInfo.hStdInput   = ( ( FileIn  > 0 ) ? ( HANDLE )_get_osfhandle( FileIn )  : GetStdHandle( STD_INPUT_HANDLE ) );
	StartupInfo.hStdOutput  = ( ( FileOut > 0 ) ? ( HANDLE )_get_osfhandle( FileOut ) : GetStdHandle( STD_OUTPUT_HANDLE ) );
	StartupInfo.hStdError   = ( ( FileErr > 0 ) ? ( HANDLE )_get_osfhandle( FileErr ) : GetStdHandle( STD_ERROR_HANDLE ) );

	static std::wstring_convert< std::codecvt_utf8< wchar_t > > ConvertUTF8;
	const std::string CommandLineUTF8 = ConvertUTF8.to_bytes( m_CommandLine.data(), m_CommandLine.data() + m_CommandLine.size() );

	if( WIN32_CALL( CreateProcessW( nullptr, const_cast< LPWSTR >( m_CommandLine.data() ), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &StartupInfo, &ProcessInfo ) ) )
	{
		BOOL  Result;
		DWORD ExitCode;

		while( WIN32_CALL( Result = GetExitCodeProcess( ProcessInfo->hProcess, &ExitCode ) ) && ExitCode == STILL_ACTIVE )
			Sleep( 1 );

		return Result ? static_cast< int >( ExitCode ) : -1;
	}

#endif // _WIN32

	return -1;

} // Run
