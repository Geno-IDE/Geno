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

static std::filesystem::path FindGCCLocation( void )
{
#if defined( _WIN32 )

	Settings& settings = Settings::Instance();

	if( GCL::Object& mingw_path = settings.object_[ "MinGW-Path" ]; mingw_path.IsString() )
		return mingw_path.String();

#endif

	return std::filesystem::path();
}

std::wstring CompilerGCC::MakeCommandLineString( const CompileOptions& options )
{
	std::wstring cmd;
	cmd.reserve( 1024 );

	// Start with GCC executable
	cmd += ( FindGCCLocation() / L"bin/g++" ).lexically_normal();

	// Language
	switch( options.language )
	{
		case CompileOptions::Language::Unspecified: { cmd += L" -x none";      } break;
		case CompileOptions::Language::C:           { cmd += L" -x c";         } break;
		case CompileOptions::Language::CPlusPlus:   { cmd += L" -x c++";       } break;
		case CompileOptions::Language::Assembler:   { cmd += L" -x assembler"; } break;
	}

	// Verbosity
	if( options.verbose )
	{
		// Time the execution of each subprocess
		cmd += L" -time";

		// Verbose logging
		cmd += L" -v";
	}

	// Actions
	switch( options.action )
	{
		case CompileOptions::Action::OnlyPreprocess:     { cmd += L" -E"; } break;
		case CompileOptions::Action::OnlyCompile:        { cmd += L" -S"; } break;
		case CompileOptions::Action::CompileAndAssemble: { cmd += L" -c"; } break;
		default:                                                     break;
	}

	// Assembler options
	if( options.assembler_flags != 0 )
	{
		cmd += L" -Wa";

		if( options.assembler_flags & CompileOptions::AssemblerFlagReduceMemoryOverheads ) { cmd += L",--reduce-memory-overheads"; }
	}

	// Preprocessor options
	if( options.preprocessor_flags != 0 )
	{
		cmd += L" -Wp";

		if( options.preprocessor_flags & CompileOptions::PreprocessorFlagUndefineSystemMacros ) { cmd += L",-undef"; }
	}

	// Set output file
	cmd += L" -o " + options.output_file.wstring();

	// Finally, the input source file
	cmd += L" " + options.input_file.wstring();

	return cmd;
}

std::wstring CompilerGCC::MakeCommandLineString( const LinkOptions& options )
{
	std::wstring cmd;
	cmd.reserve( 256 );

	switch( options.kind )
	{
		case ProjectKind::Application:
		case ProjectKind::DynamicLibrary:
		{
			// Start with GCC executable
			cmd += ( FindGCCLocation() / L"bin/g++" ).lexically_normal();

			// Create a shared library
			if( options.kind == ProjectKind::DynamicLibrary )
				cmd += L" -shared";

			// Linker options
			if( options.flags != 0 )
			{
				cmd += L" -Wl";

				if( options.flags & LinkOptions::LinkerFlagNoDefaultLibs ) { cmd += L",-nodefaultlibs"; }
			}

			// Link libraries
			for( const std::filesystem::path& library : options.linked_libraries )
			{
				cmd += L" -L" + library.parent_path().wstring();
				cmd += L" -l" + library.filename().replace_extension().wstring();
			}

			// Set output file
			{
				std::filesystem::path output_file = options.output_file;

				// Add "lib" prefix for dynamic libraries
				if( options.kind == ProjectKind::DynamicLibrary )
					output_file.replace_filename( L"lib" + output_file.filename().wstring() );

				cmd += L" -o " + output_file.wstring();
			}

			// Finally, set the input files
			for( const std::filesystem::path& input_file : options.input_files )
				cmd += L" " + input_file.lexically_normal().wstring();

		} break;

		case ProjectKind::StaticLibrary:
		{
			// Start with AR executable
			cmd += ( FindGCCLocation() / "bin/ar" ).lexically_normal();

			// Command: Replace existing or insert new file(s) into the archive
			cmd += L" r";

			// Use full path names when matching
			cmd += L'P';

			// Only replace files that are newer than current archive contents
			cmd += L'u';

			// Do not warn if the library had to be created
			cmd += L'c';

			// Create an archive index (cf. ranlib)
			cmd += L's';

			// Do not build a symbol table
			if( options.flags & LinkOptions::LinkerFlagNoSymbolTable )
				cmd += L'S';

			// Set output file with "lib" prefix
			std::filesystem::path output_file = options.output_file;
			output_file.replace_filename( L"lib" + output_file.filename().wstring() );
			cmd += L" " + output_file.wstring();

			// Set input files
			for( const std::filesystem::path& input_file : options.input_files )
				cmd += L" " + input_file.wstring();

		} break;
	}

	return cmd;
}
