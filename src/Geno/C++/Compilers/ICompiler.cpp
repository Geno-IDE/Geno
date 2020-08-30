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

#include "ICompiler.h"

#include "Common/Platform/Windows/Win32Error.h"
#include "Common/Platform/Windows/Win32ProcessInfo.h"
#include "Common/LocalAppData.h"
#include "Common/Process.h"

#include <future>
#include <iostream>

void ICompiler::Compile( const CompileOptions& options )
{
	if( !std::filesystem::exists( options.input_file ) )
	{
		std::cerr << "Failed to compile " << options.input_file.string() << ". File does not exist.\n";
		return;
	}

//////////////////////////////////////////////////////////////////////////

	std::future future = std::async( &ICompiler::CompileAsync, this, options );

	futures_.emplace_back( std::move( future ) );
}

void ICompiler::Link( const LinkOptions& options )
{
	for( const std::filesystem::path& input_file : options.input_files )
	{
		if( !std::filesystem::exists( input_file ) )
		{
			std::cerr << "Failed to link " << input_file.string() << ". File does not exist.\n";
			return;
		}
	}

//////////////////////////////////////////////////////////////////////////

	std::future future = std::async( &ICompiler::LinkAsync, this, options );

	futures_.emplace_back( std::move( future ) );
}

void ICompiler::CompileAsync( CompileOptions options )
{
	std::wstring cmd_line  = MakeCommandLineString( options );
	Process      process( std::move( cmd_line ) );

	CompilationDone e;
	e.options   = options;
	e.exit_code = process.ExitCode();
	Publish( e );
}

void ICompiler::LinkAsync( LinkOptions options )
{
	std::wstring cmd_line  = MakeCommandLineString( options );
	Process      process( std::move( cmd_line ) );

	LinkingDone e;
	e.options   = options;
	e.exit_code = process.ExitCode();
	Publish( e );
}
