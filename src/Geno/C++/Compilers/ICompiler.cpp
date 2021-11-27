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

std::optional< std::filesystem::path > ICompiler::Compile( const Configuration& rConfiguration, const std::filesystem::path& rFilePath )
{
	const std::wstring CommandLine = MakeCompilerCommandLineString( rConfiguration, rFilePath );
	const int          ExitCode    = Process::ResultOf( CommandLine );

	if( ExitCode == 0 )
		return GetCompilerOutputPath( rConfiguration, rFilePath );

	return std::nullopt;

} // Compile

//////////////////////////////////////////////////////////////////////////

std::optional< std::filesystem::path > ICompiler::Link( const Configuration& rConfiguration, std::span< std::filesystem::path > InputFiles, const std::wstring& rOutputName, Project::Kind Kind )
{
	const std::wstring CommandLine = MakeLinkerCommandLineString( rConfiguration, InputFiles, rOutputName, Kind );
	const int          ExitCode    = Process::ResultOf( CommandLine );

	if( ExitCode == 0 )
		return GetLinkerOutputPath( rConfiguration, rOutputName, Kind );

	return std::nullopt;

} // Link

//////////////////////////////////////////////////////////////////////////

std::filesystem::path ICompiler::GetCompilerOutputPath( const Configuration& rConfiguration, const std::filesystem::path& rFilePath )
{
	return ( rConfiguration.m_OutputDir / rFilePath.stem() );

} // GetCompilerOutputPath

//////////////////////////////////////////////////////////////////////////

std::filesystem::path ICompiler::GetLinkerOutputPath( const Configuration& rConfiguration, const std::wstring& rOutputName, Project::Kind Kind )
{
	std::filesystem::path OutputFile = ( rConfiguration.m_OutputDir / rOutputName );

	switch( Kind )
	{
		case Project::Kind::Application:
		{

		#if defined( _WIN32 )
			OutputFile.replace_extension( L".exe" );
		#endif // _WIN32

		} break;

		case Project::Kind::StaticLibrary:
		{

		#if defined( _WIN32 )
			OutputFile.replace_extension( L".lib" );
		#else // _WIN32
			OutputFile.replace_filename(  L"lib" + OutputFile.filename().wstring() );
			OutputFile.replace_extension( L".a" );
		#endif // !_WIN32

		} break;

		case Project::Kind::DynamicLibrary:
		{

		#if defined( _WIN32 )
			OutputFile.replace_extension( L".lib" );
		#else // _WIN32
			OutputFile.replace_filename(  L"lib" + OutputFile.filename().wstring() );
			OutputFile.replace_extension( L".so" );
		#endif // !_WIN32

		} break;

		default:
		{
		} break;
	}

	return OutputFile;

} // GetLinkerOutputPath
