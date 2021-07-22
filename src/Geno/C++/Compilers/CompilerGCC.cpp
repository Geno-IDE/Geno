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

#include "Components/Project.h"

//////////////////////////////////////////////////////////////////////////

std::wstring CompilerGCC::MakeCommandLineString( const CompileOptions& rOptions )
{
	std::wstring Command;
	Command.reserve( 1024 );

	// Start with GCC executable
	Command += L"g++";

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

	const Project::Kind Kind = static_cast< Project::Kind >( rOptions.OutputType );

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

			// Linker options
			if( rOptions.Flags != 0 )
			{
				Command += L" -Wl";

				if( rOptions.Flags & LinkOptions::LinkerFlagNoDefaultLibs ) { Command += L",-nodefaultlibs"; }
			}

			// User-defined library directories
			for( const std::filesystem::path& rLibraryDirectory : rOptions.LibraryDirectories )
			{
				Command += L" -L" + rLibraryDirectory.wstring();
			}

			// Link libraries
			for( const std::string& rLibrary : rOptions.Libraries )
			{
				UTF8Converter Converter;

				Command += L" -l" + Converter.from_bytes( rLibrary );
			}

			// Set output file
			{
				std::filesystem::path OutputFile = rOptions.OutputFile;

				switch( Kind )
				{
					case Project::Kind::Application:
					{

					#if defined( _WIN32 )
						OutputFile.replace_extension( L".exe" );
					#endif // _WIN32

					} break;

					case Project::Kind::DynamicLibrary:
					{

					#if defined( _WIN32 )
						OutputFile.replace_extension( L".lib" );
					#else // _WIN32
						OutputFile.replace_filename(  L"lib" + OutputFile.filename().wstring() );
						OutputFile.replace_extension( L".so" );
					#endif // _WIN32

					} break;

					default:
					{
					} break;
				}

				// Set output file
				Command += L" -o " + OutputFile.wstring();
			}

			// Finally, set the object files
			for( const std::filesystem::path& rInputFile : rOptions.ObjectFiles )
				Command += L" " + rInputFile.lexically_normal().wstring();

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

			// Do not build a symbol table
			if( rOptions.Flags & LinkOptions::LinkerFlagNoSymbolTable )
				Command += L'S';

			// Set output file with "lib" prefix
			std::filesystem::path OutputFile = rOptions.OutputFile;
			OutputFile.replace_filename( L"lib" + OutputFile.filename().wstring() );
			Command += L" " + OutputFile.wstring();

			// Set input files
			for( const std::filesystem::path& rObjectFile : rOptions.ObjectFiles )
				Command += L" " + rObjectFile.wstring();

		} break;

		default:
		{
		} break;
	}

	return Command;

} // MakeCommandLineString
