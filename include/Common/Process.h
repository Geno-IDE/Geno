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

#pragma once

#include <string>
#include <string_view>

 //////////////////////////////////////////////////////////////////////////

#if defined( _WIN32 )
#include <Windows.h>
using ProcessID = HANDLE;
#elif defined( __linux__ ) || defined( __APPLE__ ) // _WIN32
using ProcessID = pid_t;
#endif // __linux__ || __APPLE__

//////////////////////////////////////////////////////////////////////////

class Process
{
public:
	 Process( void ) { }
	 Process( const std::wstring_view& rCommandLine );
	~Process( void ) { Kill(); }
	 Process( const Process& rOther );
	 Process( Process&& rrOther ) noexcept;

	 Process& operator=( const Process& rOther )
	 {
		 return *this = Process( rOther );
	 }

	 Process& operator=( const Process&& rrOther ) noexcept
	 {
		 m_CommandLine = rrOther.m_CommandLine;
		 m_ExitCode = rrOther.m_ExitCode;
		 m_Pid = rrOther.m_Pid;

		 return *this;
	 }

 #if defined( _WIN32 )
	 operator bool() { return m_Pid != nullptr; }
 #elif defined( __linux__ ) && defined( __APPLE__ )
	 operator bool() { return m_Pid != 0; }
 #endif

//////////////////////////////////////////////////////////////////////////

	 void         SetCommandLine( const std::wstring_view& rCommandLine ) { m_CommandLine = rCommandLine; }
	 void         Kill          ( void );
	 void         Start         ( FILE* pOutputStream );
	 int          Wait          ( void );
	 int          ResultOf      ( void );
	 std::wstring OutputOf      ( int& rResult );
	 std::wstring OutputOf      ( void );

private:

	std::wstring_view m_CommandLine;

	int m_ExitCode  = -2;

	ProcessID m_Pid = nullptr;

}; // Process
