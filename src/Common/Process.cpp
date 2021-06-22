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

#include "Common/Aliases.h"
#include "Common/Platform/Win32/Win32Error.h"
#include "Common/Platform/Win32/Win32ProcessInfo.h"

#include <chrono>
#include <codecvt>
#include <locale>
#include <thread>

#include <fcntl.h>

#if defined( _WIN32 )
#include <Windows.h>
#include <corecrt_io.h>
#define fdopen _fdopen
#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32
#include <sys/wait.h>
#include <unistd.h>
#endif // __linux__ || __APPLE__

//////////////////////////////////////////////////////////////////////////

#if defined( _WIN32 )
using ProcessID = HANDLE;
#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32
using ProcessID = pid_t;
#endif // __linux__ || __APPLE__

//////////////////////////////////////////////////////////////////////////

static ProcessID StartProcess( const std::wstring_view CommandLine, FILE* pOutputStream )
{

#if defined( _WIN32 )

	STARTUPINFOW StartupInfo = { };
	StartupInfo.cb           = sizeof( STARTUPINFO );
	StartupInfo.wShowWindow  = SW_HIDE;
	StartupInfo.dwFlags      = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	StartupInfo.hStdOutput   = reinterpret_cast< HANDLE >( _get_osfhandle( fileno( pOutputStream ) ) );
	StartupInfo.hStdError    = reinterpret_cast< HANDLE >( _get_osfhandle( fileno( pOutputStream ) ) );

	PROCESS_INFORMATION ProcessInfo;
	WIN32_CALL( CreateProcessW( nullptr, const_cast< LPWSTR >( CommandLine.data() ), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &StartupInfo, &ProcessInfo ) );
	CloseHandle( ProcessInfo.hThread );

	return ProcessInfo.hProcess;

#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32

	ProcessID PID = fork();

	if( !PID ) // The child
	{
		// Take control of output
		dup2( fileno( pOutputStream ), 1 );
		dup2( fileno( pOutputStream ), 2 );

		execl( "/bin/sh", "/bin/sh", "-c", UTF8Converter().to_bytes( CommandLine.data(), CommandLine.data() + CommandLine.size() ).c_str(), NULL );

		exit( EXIT_FAILURE );
	}

	return PID;

#endif // __linux__ || __APPLE__

} // StartProcess

//////////////////////////////////////////////////////////////////////////

static int WaitProcess( ProcessID PID )
{

#if defined( _WIN32 )

	BOOL  Result;
	DWORD ExitCode;

	while( WIN32_CALL( Result = GetExitCodeProcess( PID, &ExitCode ) ) && ExitCode == STILL_ACTIVE )
		Sleep( 1 );

	CloseHandle( PID );

	return Result ? static_cast< int >( ExitCode ) : -1;

#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32

	int status;
	waitpid( PID, &status, 0 );

	return status;

#endif // __linux__ || __APPLE__

} // WaitProcess

//////////////////////////////////////////////////////////////////////////

int Process::ResultOf( const std::wstring_view CommandLine )
{
	ProcessID pid = StartProcess( CommandLine, stdout );

	return WaitProcess( pid );

} // ResultOf

//////////////////////////////////////////////////////////////////////////

std::wstring Process::OutputOf( const std::wstring_view CommandLine, int& rResult )
{

#if defined( _WIN32 )

	HANDLE              Read;
	HANDLE              Write;
	SECURITY_ATTRIBUTES SecurityAttributes  = { };
	SecurityAttributes.nLength              = sizeof( SECURITY_ATTRIBUTES );
	SecurityAttributes.bInheritHandle       = TRUE;
	SecurityAttributes.lpSecurityDescriptor = nullptr;

	if( CreatePipe( &Read, &Write, &SecurityAttributes, 0 ) )
	{
		std::wstring Output;
		std::string  AnsiBuffer;
		FILE*        pProcOutputHandle = fdopen( _open_osfhandle( reinterpret_cast< intptr_t >( Write ), _O_APPEND ), "w" );
		ProcessID    PID               = StartProcess( CommandLine, pProcOutputHandle );
		rResult                        = WaitProcess( PID );

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

#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32

	int FileDescriptors[ 2 ];
	pipe( FileDescriptors );
	fcntl( FileDescriptors[ 0 ], F_SETFL, O_NONBLOCK ); // Don't want to block on read

	FILE*     pStream = fdopen( FileDescriptors[ 1 ], "w" );
	ProcessID PID     = StartProcess( CommandLine, pStream );
	rResult           = WaitProcess( PID );

	char        Buffer[ 1024 ];
	ssize_t     Length;
	std::string Output;

	while( ( Length = read( FileDescriptors[ 0 ], Buffer, std::size( Buffer ) ) ) > 0 )
	{
		Output.append( Buffer, Length );
	}

	fclose( pStream );
	close( FileDescriptors[ 0 ] );

	return UTF8Converter().from_bytes( Output );

#endif // __linux__ || __APPLE__

} // OutputOf

//////////////////////////////////////////////////////////////////////////

std::wstring Process::OutputOf( const std::wstring_view CommandLine )
{
	int Result;

	return OutputOf( CommandLine, Result );

} // OutputOf
