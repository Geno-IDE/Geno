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
#include "Components/Configuration.h"
#include "Components/Project.h"

#include <atomic>
#include <filesystem>
#include <future>
#include <span>
#include <string_view>
#include <string>

#include <Common/Aliases.h>
#include <Common/Event.h>
#include <Common/Macros.h>

class ICompiler
{
	GENO_DISABLE_COPY( ICompiler );

//////////////////////////////////////////////////////////////////////////

public:

	         ICompiler( void ) = default;
	virtual ~ICompiler( void ) = default;

//////////////////////////////////////////////////////////////////////////

	void Compile( const Configuration& rConfiguration, const std::filesystem::path& rFilePath );
	void Link   ( const Configuration& rConfiguration, std::span< std::filesystem::path > InputFiles, const std::wstring& rOutputName, Project::Kind Kind );

//////////////////////////////////////////////////////////////////////////

	virtual std::string_view GetName( void ) const = 0;

//////////////////////////////////////////////////////////////////////////

	struct
	{
		Event< ICompiler, void( int ExitCode ) > FinishedCompiling;
		Event< ICompiler, void( int ExitCode ) > FinishedLinking;

	} Events;

//////////////////////////////////////////////////////////////////////////

protected:

	virtual std::wstring MakeCompilerCommandLineString( const Configuration& rConfiguration, const std::filesystem::path& rFilePath ) = 0;
	virtual std::wstring MakeLinkerCommandLineString  ( const Configuration& rConfiguration, std::span< std::filesystem::path > InputFiles, const std::wstring& rOutputName, Project::Kind Kind ) = 0;

}; // ICompiler
