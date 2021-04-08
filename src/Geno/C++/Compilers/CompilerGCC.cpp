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

#include "CompilerGCC.h"

#include "Misc/Settings.h"

//////////////////////////////////////////////////////////////////////////

static std::filesystem::path FindGCCLocation( void )
{

#if defined( _WIN32 )

	Settings& rSettings = Settings::Instance();

	if( GCL::Object& rMinGWPath = rSettings.m_Object[ "MinGW-Path" ]; rMinGWPath.IsString() )
		return rMinGWPath.String();

#endif // _WIN32

	return std::filesystem::path();

} // FindGCCLocation

//////////////////////////////////////////////////////////////////////////

std::wstring CompilerGCC::MakeCommandLineString( const CompileOptions& rOptions )
{
	std::wstring Command;
	Command.reserve( 1024 );

	// Start with GCC executable
	Command += ( FindGCCLocation() / L"bin/g++" ).lexically_normal();

	// Language
	switch( rOptions.Language )
	{
		case CompileOptions::Language::Unspecified: { Command += L" -x none";      } break;
		case CompileOptions::Language::C:           { Command += L" -x c";         } break;
		case CompileOptions::Language::CPlusPlus:   { Command += L" -x c++";       } break;
		case CompileOptions::Language::Assembler:   { Command += L" -x assembler"; } break;
	}

	// Verbosity
	if( rOptions.Verbose )
	{
		// Time the execution of each subprocess
		Command += L" -time";

		// Verbose logging
		Command += L" -v";
	}

	// Actions
	switch( rOptions.Action )
	{
		case CompileOptions::Action::OnlyPreprocess:     { Command += L" -E"; } break;
		case CompileOptions::Action::OnlyCompile:        { Command += L" -S"; } break;
		case CompileOptions::Action::CompileAndAssemble: { Command += L" -c"; } break;
		default:                                                                break;
	}

	// Assembler options
	if( rOptions.AssemblerFlags != 0 )
	{
		Command += L" -Wa";

		if( rOptions.AssemblerFlags & CompileOptions::AssemblerFlagReduceMemoryOverheads ) { Command += L",--reduce-memory-overheads"; }
	}

	// Preprocessor options
	if( rOptions.PreprocessorFlags != 0 )
	{
		Command += L" -Wp";

		if( rOptions.PreprocessorFlags & CompileOptions::PreprocessorFlagUndefineSystemMacros ) { Command += L",-undef"; }
	}

	// Set output file
	Command += L" -o " + rOptions.OutputFile.wstring();

	// Finally, the input source file
	Command += L" " + rOptions.InputFile.wstring();

	return Command;

} // MakeCommandLineString

//////////////////////////////////////////////////////////////////////////

std::wstring CompilerGCC::MakeCommandLineString( const LinkOptions& rOptions )
{
	std::wstring Command;
	Command.reserve( 256 );

	switch( rOptions.Kind )
	{
		case ProjectKind::Application:
		case ProjectKind::DynamicLibrary:
		{
			// Start with GCC executable
			Command += ( FindGCCLocation() / L"bin/g++" ).lexically_normal();

			// Create a shared library
			if( rOptions.Kind == ProjectKind::DynamicLibrary )
				Command += L" -shared";

			// Linker options
			if( rOptions.Flags != 0 )
			{
				Command += L" -Wl";

				if( rOptions.Flags & LinkOptions::LinkerFlagNoDefaultLibs ) { Command += L",-nodefaultlibs"; }
			}

			// Link libraries
			for( const std::filesystem::path& rLibrary : rOptions.LinkedLibraries )
			{
				Command += L" -L" + rLibrary.parent_path().wstring();
				Command += L" -l" + rLibrary.filename().replace_extension().wstring();
			}

			// Set output file
			{
				std::filesystem::path OutputFile = rOptions.OutputFile;

				// Add "lib" prefix for dynamic libraries
				if( rOptions.Kind == ProjectKind::DynamicLibrary )
					OutputFile.replace_filename( L"lib" + OutputFile.filename().wstring() );

				Command += L" -o " + OutputFile.wstring();
			}

			// Finally, set the input files
			for( const std::filesystem::path& rInputFile : rOptions.InputFiles )
				Command += L" " + rInputFile.lexically_normal().wstring();

		} break;

		case ProjectKind::StaticLibrary:
		{
			// Start with AR executable
			Command += ( FindGCCLocation() / "bin/ar" ).lexically_normal();

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

			// Do not build a symbol table
			if( rOptions.Flags & LinkOptions::LinkerFlagNoSymbolTable )
				Command += L'S';

			// Set output file with "lib" prefix
			std::filesystem::path OutputFile = rOptions.OutputFile;
			OutputFile.replace_filename( L"lib" + OutputFile.filename().wstring() );
			Command += L" " + OutputFile.wstring();

			// Set input files
			for( const std::filesystem::path& input_file : rOptions.InputFiles )
				Command += L" " + input_file.wstring();

		} break;
	}

	return Command;

} // MakeCommandLineString
