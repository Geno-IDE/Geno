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

std::wstring CompilerGCC::MakeCommandLineString( const std::filesystem::path& path, const Options& options )
{
	std::wstring cmd;
	cmd.reserve( 1024 );
	cmd += L"g++";

	// Language
	switch( options.language )
	{
		case Options::Language::Unspecified: { cmd += L" -x none";      } break;
		case Options::Language::C:           { cmd += L" -x c";         } break;
		case Options::Language::CPlusPlus:   { cmd += L" -x c++";       } break;
		case Options::Language::Assembler:   { cmd += L" -x assembler"; } break;
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
		case Options::Action::OnlyPreprocess:     { cmd += L" -E"; } break;
		case Options::Action::OnlyCompile:        { cmd += L" -S"; } break;
		case Options::Action::CompileAndAssemble: { cmd += L" -c"; } break;
		default:                                                     break;
	}

	// Assembler options
	if( options.assembler_flags != 0 )
	{
		cmd += L" -Wa";

		if( options.assembler_flags & Options::AssemblerFlagsReduceMemoryOverheads ) { cmd += L",--reduce-memory-overheads"; }
	}

	// Preprocessor options
	if( options.preprocessor_flags != 0 )
	{
		cmd += L" -Wp";

		if( options.preprocessor_flags & Options::PreprocessorFlagsUndefineSystemMacros ) { cmd += L",-undef"; }
	}

	// Linker options
	if( options.linker_flags != 0 )
	{
		cmd += L" -Wl";

		if( options.linker_flags & Options::LinkerFlagsNoDefaultLibs ) { cmd += L",-nodefaultlibs"; }
	}

	// Set output file
	cmd += L" -o " + options.output_file_path.lexically_normal().replace_extension( ".o" ).wstring();

	// Create a shared library
	cmd += L" -shared";

	// Finally, the input source file
	cmd += L" " + path.lexically_normal().wstring();

	return cmd;
}
