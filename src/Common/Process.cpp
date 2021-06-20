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

#include <fcntl.h>
#if defined( _WIN32 )
#include <Windows.h>
#include <corecrt_io.h>
#define fdopen _fdopen
#else
#include <sys/wait.h>
#include <unistd.h>
#endif // _WIN32

#include <chrono>
#include <codecvt>
#include <locale>
#include <thread>

#if defined(_WIN32)
#include <Windows.h>
typedef HANDLE ProcessID;
#else
typedef pid_t ProcessID;
#endif

//////////////////////////////////////////////////////////////////////////

static ProcessID StartProcess( const std::wstring rCommandLine, FILE* OutputStream )
{
#if defined( _WIN32 )
	STARTUPINFOW StartupInfo = {};
	StartupInfo.cb           = sizeof( STARTUPINFO );
	StartupInfo.wShowWindow  = SW_HIDE;
	StartupInfo.dwFlags      = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	StartupInfo.hStdOutput   = reinterpret_cast< HANDLE >( _get_osfhandle( fileno( OutputStream ) ) );
	StartupInfo.hStdError    = reinterpret_cast< HANDLE >( _get_osfhandle( fileno( OutputStream ) ) );

	Win32ProcessInfo ProcessInfo;
	WIN32_CALL( CreateProcessW( nullptr, const_cast< LPWSTR >( rCommandLine.data() ), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &StartupInfo, &ProcessInfo ) );
	return ProcessInfo->hProcess;

#else
	ProcessID pid = fork();

	if( !pid ) // The child
	{
		// Take control of output
		dup2( fileno( OutputStream ), 1 );
		dup2( fileno( OutputStream ), 2 );

		execl( "/bin/sh", "/bin/sh", "-c", std::wstring_convert< std::codecvt_utf8< wchar_t > >().to_bytes( rCommandLine ).c_str(), NULL );

		exit( EXIT_FAILURE );
	}

	return pid;
#endif
}

static int WaitProcess( ProcessID pid )
{
#if defined( _WIN32 )
	BOOL  Result;
	DWORD ExitCode;

	while( WIN32_CALL( Result = GetExitCodeProcess( pid, &ExitCode ) ) && ExitCode == STILL_ACTIVE )
		Sleep( 1 );

	return Result ? static_cast< int >( ExitCode ) : -1;
#else
	int status;
	waitpid( pid, &status, 0 );
	return status;
#endif
}

//////////////////////////////////////////////////////////////////////////

int Process::ResultOf( const std::wstring& rCommandLine )
{
	ProcessID pid = StartProcess( rCommandLine, stdout );
	return WaitProcess( pid );
} // ResultOf

//////////////////////////////////////////////////////////////////////////

std::wstring Process::OutputOf( const std::wstring& rCommandLine, int& rResult )
{
#if defined( _WIN32 )

	HANDLE              Read;
	HANDLE              Write;

	SECURITY_ATTRIBUTES SecurityAttributes;
	SecurityAttributes.nLength              = sizeof( SECURITY_ATTRIBUTES );
	SecurityAttributes.bInheritHandle       = TRUE;
	SecurityAttributes.lpSecurityDescriptor = nullptr;

	if( CreatePipe( &Read, &Write, &SecurityAttributes, 0 ) )
	{
		std::wstring Output;
		std::string  AnsiBuffer;

		FILE* ProcOutputHandle = fdopen( _open_osfhandle( reinterpret_cast< long > ( Write ), _O_APPEND ), "w" );
		ProcessID pid = StartProcess( rCommandLine, ProcOutputHandle );
		rResult = WaitProcess( pid );
		fclose(ProcOutputHandle);

		DWORD BytesAvailable;
		if( PeekNamedPipe( Read, nullptr, 0, nullptr, &BytesAvailable, nullptr ) && BytesAvailable )
		{
			AnsiBuffer.resize( BytesAvailable );
			Output.resize( BytesAvailable );

			ReadFile( Read, AnsiBuffer.data(), BytesAvailable, nullptr, nullptr );
			MultiByteToWideChar( CP_ACP, 0, AnsiBuffer.c_str(), BytesAvailable, Output.data(), BytesAvailable );
		}

		CloseHandle( Write );
		CloseHandle( Read );

		return Output;
	}
#else
	constexpr size_t READ_CHUNK_LEN = 1024; // TODO: This is kind of arbitrary?

	int              fds [ 2 ];
	pipe( fds );
	fcntl( fds [ 0 ], F_SETFL, O_NONBLOCK ); // Don't want to block on read
	FILE* stream = fdopen( fds [ 1 ], "w" );
	ProcessID   pid    = StartProcess( rCommandLine, stream );
	rResult      = WaitProcess( pid );

	std::string output{};
	char        tmp [ READ_CHUNK_LEN + 1 ];
	ssize_t     length = 0;
	while( ( length = read( fds [ 0 ], tmp, READ_CHUNK_LEN ) ) > 0 )
	{
		tmp [ length ] = '\0';
		output += tmp;
	}

	fclose( stream );
	close( fds [ 0 ] );

	return std::wstring_convert< std::codecvt_utf8< wchar_t > >().from_bytes( output );
#endif
} // OutputOf

//////////////////////////////////////////////////////////////////////////

std::wstring Process::OutputOf( const std::wstring& rCommandLine )
{
	int Result;
	return OutputOf( rCommandLine, Result );

} // OutputOf
