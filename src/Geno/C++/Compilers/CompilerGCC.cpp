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

#include "CompilerGCC.h"

//////////////////////////////////////////////////////////////////////////

std::wstring CompilerGCC::MakeCompilerCommandLineString( const Configuration& rConfiguration, const std::filesystem::path& rFilePath )
{
	std::wstring Command;
	Command.reserve( 1024 );

	// Start with GCC executable
	Command += L"g++";

	// Make it so that we compile separately.
	Command += L" -c";

	// Language
	const auto FileExtension = rFilePath.extension();
	if     ( FileExtension == ".c"   ) Command += L" -x c";
	else if( FileExtension == ".cpp" ) Command += L" -x c++";
	else if( FileExtension == ".cxx" ) Command += L" -x c++";
	else if( FileExtension == ".cc"  ) Command += L" -x c++";
	else if( FileExtension == ".asm" ) Command += L" -x assembler";
	else                               Command += L" -x none";

	// TODO: Defines

	// Verbosity
	if( rConfiguration.m_Verbose )
	{
		// Time the execution of each subprocess
		Command += L" -time";

		// Verbose logging
		Command += L" -v";
	}

	// Set output file
	Command += L" -o " + GetCompilerOutputPath( rConfiguration, rFilePath ).wstring();

	// Finally, the input source file
	Command += L" " + rFilePath.wstring();

	return Command;

} // MakeCompilerCommandLineString

//////////////////////////////////////////////////////////////////////////

std::wstring CompilerGCC::MakeLinkerCommandLineString( const Configuration& rConfiguration, std::span< std::filesystem::path > InputFiles, const std::wstring& rOutputName, Project::Kind Kind )
{
	std::wstring Command;
	Command.reserve( 256 );

	UTF8Converter UTF8;

	switch( Kind )
	{
		case Project::Kind::Application:
		case Project::Kind::DynamicLibrary:
		{
			// Start with GCC executable
			Command += L"g++";

			// Create a shared library
			if( Kind == Project::Kind::DynamicLibrary )
				Command += L" -shared";

			// User-defined library directories
			for( const std::filesystem::path& rLibraryDirectory : rConfiguration.m_LibraryDirs )
			{
				Command += L" -L" + rLibraryDirectory.wstring();
			}

			// Link libraries
			for( const std::string& rLibrary : rConfiguration.m_Libraries )
			{
				Command += L" -l" + UTF8.from_bytes( rLibrary );
			}

			// Set output file
			Command += L" -o " + GetLinkerOutputPath( rConfiguration, rOutputName, Kind ).wstring();

			// Finally, set the object files
			for( const std::filesystem::path& rInputFile : InputFiles )
				Command += L" " + rInputFile.wstring();

		} break;

		case Project::Kind::StaticLibrary:
		{
			// Start with AR executable
			Command += L"bin/ar";

			// Command: Replace existing or insert new file(s) into the archive
			Command += L" r";

			// Use full path names when matching
			Command += L'P';

			// Only replace files that are newer than current archive contents
			Command += L'u';

			// Do not warn if the library had to be created
			Command += L'c';

			// Create an archive index (cf. ranlib)
			Command += L's';

			// Set output file
			Command += L" " + GetLinkerOutputPath( rConfiguration, rOutputName, Kind ).wstring();

			// Set input files
			for( const std::filesystem::path& rInputFile : InputFiles )
				Command += L" " + rInputFile.wstring();

		} break;

		default:
		{
		} break;
	}

	return Command;

} // MakeLinkerCommandLineString
