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
#if defined( _WIN32 )

#include "Common/Process.h"

#include "Common/Platform/Win32/Win32Error.h"
#include "Common/Platform/Win32/Win32ProcessInfo.h"

#include <chrono>
#include <codecvt>
#include <thread>

#include <io.h>

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////

static int Run( const std::wstring& rCommandLine, HANDLE StdIn, HANDLE StdOut, HANDLE StdErr )
{
	STARTUPINFOW StartupInfo = { };
	StartupInfo.cb          = sizeof( STARTUPINFO );
	StartupInfo.wShowWindow = SW_HIDE;
	StartupInfo.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	StartupInfo.hStdInput   = StdIn;
	StartupInfo.hStdOutput  = StdOut;
	StartupInfo.hStdError   = StdErr;

	Win32ProcessInfo ProcessInfo;
	if( WIN32_CALL( CreateProcessW( nullptr, const_cast< LPWSTR >( rCommandLine.data() ), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &StartupInfo, &ProcessInfo ) ) )
	{
		BOOL  Result;
		DWORD ExitCode;

		while( WIN32_CALL( Result = GetExitCodeProcess( ProcessInfo->hProcess, &ExitCode ) ) && ExitCode == STILL_ACTIVE )
			Sleep( 1 );

		return Result ? static_cast< int >( ExitCode ) : -1;
	}

	return -1;

} // Run

//////////////////////////////////////////////////////////////////////////

int Process::ResultOf( const std::wstring& rCommandLine )
{
	HANDLE StdOut = ( HANDLE )_get_osfhandle( _fileno( stdout ) );
	HANDLE StdIn  = ( HANDLE )_get_osfhandle( _fileno( stdin ) );
	HANDLE StdErr = ( HANDLE )_get_osfhandle( _fileno( stderr ) );

	return Run( rCommandLine, StdIn, StdOut, StdErr );

} // ResultOf

//////////////////////////////////////////////////////////////////////////

std::wstring Process::OutputOf( const std::wstring& rCommandLine, int& rResult )
{
	HANDLE Read;
	HANDLE Write;

	SECURITY_ATTRIBUTES SecurityAttributes;
	SecurityAttributes.nLength              = sizeof( SECURITY_ATTRIBUTES );
	SecurityAttributes.bInheritHandle       = TRUE;
	SecurityAttributes.lpSecurityDescriptor = nullptr;

	if( CreatePipe( &Read, &Write, &SecurityAttributes, 0 ) )
	{
		std::wstring Output;
		std::string  AnsiBuffer;

		rResult = Run( rCommandLine, nullptr, Write, Write );

		DWORD BytesAvailable;
		if( PeekNamedPipe( Read, nullptr, 0, nullptr, &BytesAvailable, nullptr ) && BytesAvailable )
		{
			AnsiBuffer.resize( BytesAvailable );
			Output    .resize( BytesAvailable );

			ReadFile( Read, AnsiBuffer.data(), BytesAvailable, nullptr, nullptr );
			MultiByteToWideChar( CP_ACP, 0, AnsiBuffer.c_str(), BytesAvailable, Output.data(), BytesAvailable );
		}

		CloseHandle( Write );
		CloseHandle( Read );

		return Output;
	}

	return std::wstring();

} // OutputOf

//////////////////////////////////////////////////////////////////////////

std::wstring Process::OutputOf( const std::wstring& rCommandLine )
{
	int Result;
	return OutputOf( rCommandLine, Result );

} // OutputOf

#endif
