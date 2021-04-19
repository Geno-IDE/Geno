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

#include "ICompiler.h"

#include "Common/Platform/Win32/Win32Error.h"
#include "Common/Platform/Win32/Win32ProcessInfo.h"
#include "Common/LocalAppData.h"
#include "Common/Process.h"

#include <future>

//////////////////////////////////////////////////////////////////////////

void ICompiler::Compile( const CompileOptions& rOptions )
{
	std::future Future = std::async( &ICompiler::CompileAsync, this, rOptions );

	m_Futures.emplace_back( std::move( Future ) );

} // Compile

//////////////////////////////////////////////////////////////////////////

void ICompiler::Link( const LinkOptions& rOptions )
{
	std::future Future = std::async( &ICompiler::LinkAsync, this, rOptions );

	m_Futures.emplace_back( std::move( Future ) );

} // Link

//////////////////////////////////////////////////////////////////////////

void ICompiler::CompileAsync( CompileOptions Options )
{
	const std::wstring CommandLine = MakeCommandLineString( Options );
	const int          ExitCode    = Process::ResultOf( CommandLine );

	Events.FinishedCompiling( *this, Options, ExitCode );

} // CompileAsync

//////////////////////////////////////////////////////////////////////////

void ICompiler::LinkAsync( LinkOptions Options )
{
	const std::wstring CommandLine = MakeCommandLineString( Options );
	const int          ExitCode    = Process::ResultOf( CommandLine );

	Events.FinishedLinking( *this, Options, ExitCode );

} // LinkAsync
