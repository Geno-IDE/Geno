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
#include <iostream>

#if defined( _WIN32 )
#include <corecrt_io.h>
#define fdopen _fdopen
#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32
#include <sys/wait.h>
#include <sys/signal.h>
#include <signal.h>
#include <unistd.h>
#endif // __linux__ || __APPLE__

//////////////////////////////////////////////////////////////////////////

Process::Process( const std::wstring_view& rCommandLine )
{
	m_CommandLine = rCommandLine;

	m_ExitCode = 0;

	m_Pid = 0;
} // Process

//////////////////////////////////////////////////////////////////////////

Process::Process( const Process& rOther )
{
	m_CommandLine = rOther.m_CommandLine;
	m_ExitCode    = rOther.m_ExitCode;
	m_Pid         = rOther.m_Pid;
} // Process

//////////////////////////////////////////////////////////////////////////

Process::Process( Process&& rrOther ) noexcept
{
	m_CommandLine = std::exchange( rrOther.m_CommandLine, nullptr );
	m_ExitCode    = std::exchange( rrOther.m_ExitCode, 0 );
#if defined( _WIN32 )
	m_Pid         = std::exchange( rrOther.m_Pid, nullptr );
#elif defined( __linux__ ) || defined( __APPLE__ ) // WIN32
	m_Pid         = std::exchange( rrOther.m_Pid, 0 );
#endif // __linux__ || __APPLE__
} // Process

//////////////////////////////////////////////////////////////////////////

void Process::Start( FILE* pOutputStream )
{

#if defined( _WIN32 )

	STARTUPINFOW StartupInfo ={ };
	StartupInfo.cb           = sizeof( STARTUPINFO );
	StartupInfo.wShowWindow  = SW_HIDE;
	StartupInfo.dwFlags      = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	StartupInfo.hStdOutput   = reinterpret_cast< HANDLE >( _get_osfhandle( fileno( pOutputStream ) ) );
	StartupInfo.hStdError    = reinterpret_cast< HANDLE >( _get_osfhandle( fileno( pOutputStream ) ) );

	PROCESS_INFORMATION ProcessInfo;
	WIN32_CALL( CreateProcessW( nullptr, const_cast< LPWSTR >( m_CommandLine.data() ), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &StartupInfo, &ProcessInfo ) );
	CloseHandle( ProcessInfo.hThread );

	m_Pid = ProcessInfo.hProcess;

#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32

	ProcessID PID = fork();

	if( !PID ) // The child
	{
		// Take control of output
		dup2( fileno( pOutputStream ), 1 );
		dup2( fileno( pOutputStream ), 2 );

		execl( "/bin/sh", "/bin/sh", "-c", UTF8Converter().to_bytes( m_CommandLine.data(), m_CommandLine.data() + m_CommandLine.size() ).c_str(), NULL );

		exit( EXIT_FAILURE );
	}

	m_Pid = PID;

#endif // __linux__ || __APPLE__

} // Start

//////////////////////////////////////////////////////////////////////////

int Process::Wait( void )
{

#if defined( _WIN32 )

	BOOL  Result;
	DWORD ExitCode;

	while( WIN32_CALL( Result = GetExitCodeProcess( m_Pid, &ExitCode ) ) && ExitCode == STILL_ACTIVE )
		Sleep( 1 );

	CloseHandle( m_Pid );

	m_Pid = nullptr;
	m_ExitCode = ExitCode;

	return Result ? m_ExitCode : -1;

#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32

	waitpid( m_Pid, &m_ExitCode, 0 );

	return m_ExitCode;

#endif // __linux__ || __APPLE__

} // Wait

//////////////////////////////////////////////////////////////////////////

void Process::Kill( void )
{

#if defined( _WIN32 )

	//CloseHandle( m_Pid );
	TerminateProcess( m_Pid, 1 );

	m_Pid = nullptr;

#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32

	m_ExitCode = kill( m_Pid, SIGUSR1 );

	m_Pid = 0;

#endif // __linux__ || __APPLE__

} // Kill

//////////////////////////////////////////////////////////////////////////

int Process::ResultOf( void )
{
	Start( stdout );

	return Wait();

} // ResultOf

//////////////////////////////////////////////////////////////////////////

std::wstring Process::OutputOf( int& rResult )
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
		Start( pProcOutputHandle );
		rResult                        = Wait();

		DWORD BytesAvailable;
		if( PeekNamedPipe( Read, nullptr, 0, nullptr, &BytesAvailable, nullptr ) && BytesAvailable )
		{
			AnsiBuffer.resize( BytesAvailable );
			Output    .resize( BytesAvailable );

			ReadFile( Read, AnsiBuffer.data(), BytesAvailable, nullptr, nullptr );
			MultiByteToWideChar( CP_ACP, 0, AnsiBuffer.c_str(), BytesAvailable, Output.data(), BytesAvailable );
		}

		fclose( pProcOutputHandle );
		CloseHandle( Read );

		return Output;
	}

	return std::wstring();

#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32

	int FileDescriptors[ 2 ];
	pipe( FileDescriptors );
	fcntl( FileDescriptors[ 0 ], F_SETFL, O_NONBLOCK ); // Don't want to block on read

	FILE*     pStream = fdopen( FileDescriptors[ 1 ], "w" );
	Start( pStream );
	rResult           = Wait();

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

std::wstring Process::OutputOf( void )
{
	int Result;

	return OutputOf( Result );

} // OutputOf
