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

#pragma once
#include "Compilers/CompileOptions.h"
#include "Compilers/LinkOptions.h"
#include "Components/Enums/ProjectKind.h"

#include <atomic>
#include <filesystem>
#include <future>
#include <string_view>
#include <string>

#include <Common/Aliases.h>
#include <Common/EventDispatcher.h>
#include <Common/Macros.h>

struct CompilationDone
{
	CompileOptions options;
	int            exit_code;
};

struct LinkingDone
{
	LinkOptions options;
	int         exit_code;
};

class ICompiler : public EventDispatcher< ICompiler, CompilationDone, LinkingDone >
{
public:

	GENO_DISABLE_COPY( ICompiler );

public:

	         ICompiler( void ) = default;
	virtual ~ICompiler( void ) = default;

public:

	void Compile ( const CompileOptions& options );
	void Link    ( const LinkOptions& options );

protected:

	virtual std::wstring MakeCommandLineString( const CompileOptions& options ) = 0;
	virtual std::wstring MakeCommandLineString( const LinkOptions& options )    = 0;

private:

	void CompileAsync ( CompileOptions options );
	void LinkAsync    ( LinkOptions options );

private:

	std::vector< std::future< void > > futures_;

};
